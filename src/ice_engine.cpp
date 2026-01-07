#include "../include/ice_engine.hpp"
#include "../include/constants.hpp"
#include <algorithm>
#include <cmath>

ICEEngine::ICEEngine()
    : angular_velocity(100.0), throttle(1.0), torque_output(0.0),
      intake_manifold_pressure(101325.0), intake_manifold_temperature(300.0),
      load_torque(5.0), exhaust_manifold_pressure(constants::ambient_pressure),
      exhaust_manifold_temperature(950),
      plenum_pressure(constants::ambient_pressure), spark_advance_deg(10.0),
      turbo(2e-5, 0.72, 0.74, 0.02),
      mguh(2e-6, 0.95, 120000.0) // inertia, eff, max power
{} // Higher load for dyno-like testing (adjust as needed)

void ICEEngine::setThrottle(double t) { throttle = std::clamp(t, 0.0, 1.0); }

double ICEEngine::getRPM() const {
  return angular_velocity * 60.0 / (2.0 * constants::PI);
}

double ICEEngine::getExhaustMassFlowRate() const {
  return exhaust_mass_flow_rate;
}

double ICEEngine::getAngularVelocity() const { return angular_velocity; }

double ICEEngine::getTorqueOutput() const { return torque_output; }

double ICEEngine::getNetPower() const {
  return torque_output * angular_velocity;
}

double ICEEngine::getThrottleAirMassFlow(double throttle, double P_down) const {
  double P_up = plenum_pressure;
  double T_up = constants::ambient_temperature;

  double effective_area = throttle * throttle * constants::throttle_area;

  if (effective_area <= 0.0)
    return 0.0;

  if (P_down >= P_up)
    return 0.0;

  double pressure_ratio = std::clamp(P_down / P_up, 0.0, 1.0);
  double critical_pressure_ratio = std::pow(
      2 / (constants::gamma + 1), constants::gamma / (constants::gamma - 1));

  double mass_flow_factor = constants::discharge_coefficient * effective_area *
                            P_up *
                            std::sqrt(constants::gamma / (constants::R * T_up));

  if (pressure_ratio <= critical_pressure_ratio) {
    double choked_flow_factor =
        std::pow(2 / (constants::gamma + 1),
                 (constants::gamma + 1) / (2 * (constants::gamma - 1)));
    return mass_flow_factor * choked_flow_factor;
  }

  const double nonchoked_sqrt_argument =
      (2.0 / (constants::gamma - 1.0)) *
      (std::pow(pressure_ratio, 2.0 / constants::gamma) -
       std::pow(pressure_ratio, (constants::gamma + 1.0) / constants::gamma));

  if (nonchoked_sqrt_argument <= 0.0) {
    return 0.0;
  }

  return mass_flow_factor * std::sqrt(nonchoked_sqrt_argument);
}

double exhaustMassFlowOut(double exhaust_pressure, double exhaust_temperature) {
  double pr = constants::ambient_pressure / exhaust_pressure;
  double pr_crit =
      std::pow(2.0 / (constants::gamma_exhaust + 1.0),
               constants::gamma_exhaust / (constants::gamma_exhaust - 1.0));

  double flow_coeff = constants::discharge_coefficient *
                      constants::exhaust_area * exhaust_pressure *
                      std::sqrt(constants::gamma_exhaust /
                                (constants::R * exhaust_temperature));

  if (pr <= pr_crit) {
    // choked
    double choked_factor =
        std::pow(2.0 / (constants::gamma_exhaust + 1.0),
                 (constants::gamma_exhaust + 1.0) /
                     (2.0 * (constants::gamma_exhaust - 1.0)));
    return flow_coeff * choked_factor;
  } else {
    // non-choked
    double term = (2.0 / (constants::gamma_exhaust - 1.0)) *
                  (std::pow(pr, 2.0 / constants::gamma_exhaust) -
                   std::pow(pr, (constants::gamma_exhaust + 1.0) /
                                    constants::gamma_exhaust));
    if (term <= 0.0)
      return 0.0;
    return flow_coeff * std::sqrt(term);
  }
}

void ICEEngine::update(double dt) {

  double target_boost_pressure = 4 * constants::ambient_pressure;

  double rpm = std::max(getRPM(), 1.0);
  double cycles_per_second = std::max(rpm / 120.0, 1e-3);

  double boost = turbo.getCompressorOutletPressure();

  // temporary ECU
  if (throttle > 0.3 && boost < target_boost_pressure) {
    mguh.setMode(MGUHMode::MOTOR); // anti-lag
    mguh.setRequestedPower(80000.0);
  } else if (boost > target_boost_pressure * 1.05 &&
             exhaust_mass_flow_rate > 0.01) {
    mguh.setMode(MGUHMode::GENERATOR); // harvest
    mguh.setRequestedPower(60000.0);
  } else {
    mguh.setMode(MGUHMode::IDLE);
  }

  // --- MGU-H dynamics ---
  mguh.update(dt, turbo.getShaftAngularSpeed());

  // --- Turbo dynamics (MGU-H coupled here) ---
  turbo.update(dt, exhaust_mass_flow_rate, exhaust_manifold_pressure,
               exhaust_manifold_temperature, target_boost_pressure,
               mguh.getTorque());

  // VOLUMETRIC EFFICIENCY CALCULATION
  double volumetric_efficiency =
      constants::volumetric_efficiency_max *
      std::exp(-std::pow(
          (rpm - constants::volumetric_efficiency_peak_rpm) / 12500.0, 2));

  volumetric_efficiency = std::clamp(volumetric_efficiency, 0.0, 1.0);

  // volumetric_efficiency *=
  //     std::sqrt(intake_manifold_pressure / constants::ambient_pressure);

  volumetric_efficiency = std::clamp(volumetric_efficiency, 0.0, 1.2);

  // AIR FLOW CALCULATION
  intake_manifold_temperature = turbo.getCompressorOutletTemperature();

  double air_mass_per_cycle_per_cylinder =
      (intake_manifold_pressure * constants::Volume_displacement *
       volumetric_efficiency) /
      (constants::R * intake_manifold_temperature);

  plenum_pressure += (constants::ambient_pressure - plenum_pressure) * 0.1 * dt;

  double air_in = getThrottleAirMassFlow(throttle, intake_manifold_pressure);

  double air_out = air_mass_per_cycle_per_cylinder * cycles_per_second *
                   constants::NUM_CYLINDERS;

  double max_air_from_turbo = turbo.getAvailableAirMassFlow();

  double engine_air_demand = air_mass_per_cycle_per_cylinder *
                             cycles_per_second * constants::NUM_CYLINDERS;

  double actual_air_mass_flow = std::min(engine_air_demand, max_air_from_turbo);

  intake_manifold_pressure += (constants::R * intake_manifold_temperature /
                               constants::intake_manifold_volume) *
                              (air_in - actual_air_mass_flow) * dt;

  intake_manifold_pressure =
      std::clamp(intake_manifold_pressure, 0.3 * constants::ambient_pressure,
                 turbo.getCompressorOutletPressure());

  // FUEL MASS AND HEAT RELEASE CALCULATION
  double fuel_mass_per_cycle_per_cylinder =
      air_mass_per_cycle_per_cylinder /
      (constants::AFR_stoich * constants::lambda);

  double fuel_mass_flow = fuel_mass_per_cycle_per_cylinder * cycles_per_second *
                          constants::NUM_CYLINDERS;
  if (rpm >= 10500) {
    fuel_mass_flow = std::min(fuel_mass_flow, 100.0 / 3600.0);
  }

  fuel_mass_per_cycle_per_cylinder =
      fuel_mass_flow / (cycles_per_second * constants::NUM_CYLINDERS);

  // Chemical energy released by fuel
  double chemical_energy =
      fuel_mass_per_cycle_per_cylinder * constants::LHV_fuel;

  // Fraction converted to in-cylinder heat
  double thermal_energy = chemical_energy * constants::combustion_efficiency;

  double CA50 =
      360.0 - spark_advance_deg + 0.5 * constants::crank_angle_burn_duration;

  double combustion_phasing_efficiency = std::exp(
      -std::pow((CA50 - constants::CA50_opt) / constants::CA50_sigma, 2.0));

  double indicated_work = thermal_energy * constants::thermal_efficiency *
                          combustion_phasing_efficiency;

  // IMEP definition
  double indicated_mean_effective_pressure =
      indicated_work / constants::Volume_displacement;

  // Per-cylinder torque from IMEP (4-stroke)
  double indicated_torque = indicated_mean_effective_pressure *
                            constants::Volume_displacement /
                            (constants::PI * 4.0);

  // Total engine indicated torque
  indicated_torque *= constants::NUM_CYLINDERS;

  double rpm_krpm = rpm / 1000.0;

  double friction_pressure =
      constants::fmepA + constants::fmepB * rpm_krpm +
      constants::fmepC * rpm_krpm * rpm_krpm +
      constants::fmepD * indicated_mean_effective_pressure;
  // Per-cylinder friction torque
  double friction_torque = friction_pressure * constants::Volume_displacement /
                           (constants::PI * 4.0);

  // Total engine friction torque
  friction_torque *= constants::NUM_CYLINDERS;

  double exhaust_energy =
      thermal_energy * (1.0 - constants::thermal_efficiency);

  // Rough exhaust temperature rise model
  double exhaust_temperature =
      constants::exhaust_temp_base +
      exhaust_energy * cycles_per_second * constants::exhaust_temp_gain;

  exhaust_manifold_temperature = exhaust_temperature;

  exhaust_temperature = std::clamp(exhaust_temperature, 700.0, 1050.0);

  exhaust_mass_flow_rate =
      (air_mass_per_cycle_per_cylinder + fuel_mass_per_cycle_per_cylinder) *
      cycles_per_second * constants::NUM_CYLINDERS;

  double mass_out =
      exhaustMassFlowOut(exhaust_manifold_pressure, exhaust_temperature);

  double delta_p = (constants::R * exhaust_temperature /
                    constants::exhaust_manifold_volume) *
                   (exhaust_mass_flow_rate - mass_out) * dt;
  exhaust_manifold_pressure += delta_p;
  exhaust_manifold_pressure =
      std::clamp(exhaust_manifold_pressure, constants::ambient_pressure,
                 5.0 * constants::ambient_pressure);

  double pumping_pressure =
      exhaust_manifold_pressure - intake_manifold_pressure;
  pumping_pressure = std::max(pumping_pressure, 0.0);

  // Per-cylinder pumping torque
  double pumping_torque =
      pumping_pressure * constants::Volume_displacement / (constants::PI * 4.0);

  // Total engine pumping torque
  pumping_torque *= constants::NUM_CYLINDERS;

  double engine_torque = (indicated_torque - friction_torque - pumping_torque);

  double net_torque = engine_torque - load_torque;

  // DYNAMICS UPDATE

  double angular_acceleration = net_torque / constants::crank_inertia;

  angular_velocity += angular_acceleration * dt;

  if (angular_velocity < 0) {
    angular_velocity = 0;
  }
  torque_output = engine_torque;
}
