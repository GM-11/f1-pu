#include "../include/ice_engine.hpp"
#include "../include/constants.hpp"
#include <algorithm>
#include <cmath>
ICEEngine::ICEEngine()
    : angular_velocity(100.0), throttle(1.0), torque_output(0.0),
      frictionA(30.0),  // Increased for 6-cylinder drag
      frictionB(0.025), // Slightly higher viscous losses
      frictionC(6e-6),  // Higher squared term for F1 redline (~15,000 RPM)
      intake_manifold_pressure(101325.0), intake_manifold_temperature(300.0),
      load_torque(30.0) {
} // Higher load for dyno-like testing (adjust as needed)

void ICEEngine::setThrottle(double t) { throttle = std::clamp(t, 0.0, 1.0); }

double ICEEngine::getRPM() const {
  return angular_velocity * 60.0 / (2.0 * constants::PI);
}

double ICEEngine::getAngularVelocity() const { return angular_velocity; }

double ICEEngine::getTorqueOutput() const { return torque_output; }

double ICEEngine::getNetPower() const {
  return torque_output * angular_velocity;
}

double ICEEngine::getThrottleAirMassFlow(double throttle, double P_down) const {
  double P_up = constants::ambient_pressure;
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

void ICEEngine::update(double dt) {

  double rpm = getRPM();

  // VOLUMETRIC EFFICIENCY CALCULATION
  double volumetric_efficiency =
      constants::volumetric_efficiency_max *
      std::exp(-std::pow(
          (rpm - constants::volumetric_efficiency_peak_rpm) / 12500.0, 2));

  volumetric_efficiency = std::clamp(volumetric_efficiency, 0.0, 1.0);

  // double deltaP =
  //     std::max(0.0, constants::ambient_pressure - intake_manifold_pressure);

  // AIR FLOW CALCULATION
  double mass_per_cycle_single_cylinder =
      (intake_manifold_pressure * constants::Volume_displacement *
       volumetric_efficiency) /
      (constants::R * intake_manifold_temperature);

  double cycles_per_second = rpm / 120.0;

  double air_in = getThrottleAirMassFlow(throttle, intake_manifold_pressure);
  // double air_in = throttle * constants::discharge_coefficient *
  //                   constants::throttle_area *
  //                   std::sqrt(2.0 * constants::air_density * deltaP);

  double air_out = mass_per_cycle_single_cylinder * cycles_per_second *
                   constants::NUM_CYLINDERS;

  intake_manifold_pressure += (constants::R * intake_manifold_temperature /
                               constants::intake_manifold_volume) *
                              (air_in - air_out) * dt;

  intake_manifold_pressure = std::clamp(intake_manifold_pressure, 10000.0,
                                        constants::ambient_pressure);

  // FUEL MASS AND HEAT RELEASE CALCULATION
  double fuel_mass_per_cycle =
      mass_per_cycle_single_cylinder / constants::AFR_stoich;

  double heat_release = fuel_mass_per_cycle * constants::LHV_fuel;

  // TORQUE CALCULATION
  double indicated_torque = (heat_release * constants::thermal_efficiency *
                             constants::NUM_CYLINDERS) /
                            (constants::PI * 4);

  double friction_torque = frictionA + frictionB * angular_velocity +
                           frictionC * angular_velocity * angular_velocity;

  double pumping_torque =
      constants::k_pump *
      (constants::ambient_pressure - intake_manifold_pressure) *
      constants::NUM_CYLINDERS;

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
