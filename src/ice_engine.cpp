#include "../include/ice_engine.hpp"
#include "../include/constants.hpp"
#include <algorithm>
#include <cmath>

ICEEngine::ICEEngine()
    : angular_velocity(constants::engine_idle_rad_s), throttle(0.0),
      effective_throttle(0.0), torque_output(0.0),
      intake_manifold_pressure(constants::ambient_pressure),
      intake_manifold_temperature(constants::ambient_temperature),
      exhaust_manifold_pressure(constants::ambient_pressure),
      exhaust_manifold_temperature(900.0),
      plenum_pressure(constants::ambient_pressure), spark_advance_deg(10.0),
      turbo(2e-5, 0.72, 0.74, 0.02), mguh(2e-6, 0.95, 120000.0),
      battery(constants::battery_max_energy_J,
              constants::battery_max_charge_power,
              constants::battery_max_discharge_power),
      mguk(constants::mguk_efficiency, constants::mguk_max_power),
      mguk_torque(0.0), combustion_torque(0.0), friction_torque(0.0),
      pumping_torque(0.0), indicated_torque(0.0), net_torque(0.0),
      exhaust_mass_flow_rate(0.0), na_air_flow(0.0), actual_air_flow(0.0),
      fuel_mass_flow(0.0), volumetric_efficiency(0.0), imep(0.0), fmep(0.0),
      bmep(0.0) {}

// --------------------------------------------------
// BASIC SETTERS/GETTERS
// --------------------------------------------------

void ICEEngine::setThrottle(double t) { throttle = std::clamp(t, 0.0, 1.0); }

double ICEEngine::getRPM() const {
  return angular_velocity * 60.0 / (2.0 * constants::PI);
}

double ICEEngine::getAngularVelocity() const { return angular_velocity; }

double ICEEngine::getTorqueOutput() const {
  return combustion_torque + mguk_torque;
}

double ICEEngine::getNetPower() const {
  return (combustion_torque + mguk_torque) * angular_velocity;
}

double ICEEngine::getExhaustMassFlowRate() const {
  return exhaust_mass_flow_rate;
}

// --------------------------------------------------
// ENGINE / CRANK GETTERS
// --------------------------------------------------

double ICEEngine::getCombustionTorque() const { return combustion_torque; }

double ICEEngine::getLoadTorque() const {
  return constants::load_A + constants::load_B * angular_velocity +
         constants::load_C * angular_velocity * angular_velocity;
}

double ICEEngine::getFrictionTorque() const { return friction_torque; }

double ICEEngine::getPumpingTorque() const { return pumping_torque; }

double ICEEngine::getIndicatedTorque() const { return indicated_torque; }

double ICEEngine::getNetTorque() const { return net_torque; }

// --------------------------------------------------
// THROTTLE / AIRFLOW GETTERS
// --------------------------------------------------

double ICEEngine::getThrottle() const { return throttle; }

double ICEEngine::getEffectiveThrottle() const { return effective_throttle; }

double ICEEngine::getNAAirFlow() const { return na_air_flow; }

double ICEEngine::getActualAirFlow() const { return actual_air_flow; }

double ICEEngine::getFuelMassFlow() const { return fuel_mass_flow; }

double ICEEngine::getVolumetricEfficiency() const {
  return volumetric_efficiency;
}

// --------------------------------------------------
// TURBO / INTAKE GETTERS
// --------------------------------------------------

double ICEEngine::getTurboSpeed() const { return turbo.getShaftAngularSpeed(); }

double ICEEngine::getTurboSpeedRPM() const {
  return turbo.getShaftAngularSpeed() * 60.0 / (2.0 * constants::PI);
}

double ICEEngine::getBoostPressure() const {
  return turbo.getCompressorOutletPressure();
}

double ICEEngine::getPlenumPressure() const { return plenum_pressure; }

double ICEEngine::getIntakeManifoldPressure() const {
  return intake_manifold_pressure;
}

double ICEEngine::getIntakeManifoldTemperature() const {
  return intake_manifold_temperature;
}

double ICEEngine::getAirMassFlow() const {
  return turbo.getAvailableAirMassFlow();
}

double ICEEngine::getCompressorOutletTemperature() const {
  return turbo.getCompressorOutletTemperature();
}

// --------------------------------------------------
// EXHAUST GETTERS
// --------------------------------------------------

double ICEEngine::getExhaustManifoldPressure() const {
  return exhaust_manifold_pressure;
}

double ICEEngine::getExhaustTemperature() const {
  return exhaust_manifold_temperature;
}

// --------------------------------------------------
// ERS GETTERS
// --------------------------------------------------

double ICEEngine::getMGUHTorque() const { return mguh.getTorque(); }

double ICEEngine::getMGUHPower() const { return mguh.getElectricalPower(); }

double ICEEngine::getMGUKTorque() const { return mguk.getTorque(); }

double ICEEngine::getMGUKPower() const { return mguk.getElectricalPower(); }

double ICEEngine::getBatteryEnergy() const { return battery.getEnergy(); }

double ICEEngine::getBatterySOC() const { return battery.getSOC(); }

// --------------------------------------------------
// PERFORMANCE METRICS GETTERS
// --------------------------------------------------

double ICEEngine::getBMEP() const { return bmep; }

double ICEEngine::getIMEP() const { return imep; }

double ICEEngine::getFMEP() const { return fmep; }

double ICEEngine::getBSFC() const {
  // Brake Specific Fuel Consumption (g/kWh)
  double brake_power = combustion_torque * angular_velocity;
  if (brake_power <= 0.0)
    return 0.0;
  // fuel_mass_flow is in kg/s, convert to g/h and divide by kW
  return (fuel_mass_flow * 1000.0 * 3600.0) / (brake_power / 1000.0);
}

double ICEEngine::getThermalEfficiency() const {
  // Thermal efficiency = Brake power / Fuel energy input
  double fuel_power = fuel_mass_flow * constants::LHV_fuel;
  if (fuel_power <= 0.0)
    return 0.0;
  double brake_power = combustion_torque * angular_velocity;
  return brake_power / fuel_power;
}

double ICEEngine::getMechanicalEfficiency() const {
  // Mechanical efficiency = Brake power / Indicated power
  double indicated_power = indicated_torque * angular_velocity;
  if (indicated_power <= 0.0)
    return 0.0;
  double brake_power = combustion_torque * angular_velocity;
  return brake_power / indicated_power;
}

double ICEEngine::getTotalPower() const {
  // ICE + MGU-K combined power
  return (combustion_torque + mguk_torque) * angular_velocity;
}

double ICEEngine::getICEPower() const {
  // Pure ICE brake power
  return combustion_torque * angular_velocity;
}

// --------------------------------------------------
// THROTTLE MASS FLOW
// --------------------------------------------------

double ICEEngine::getThrottleAirMassFlow(double throttle_cmd,
                                         double P_down) const {
  double P_up = plenum_pressure;
  double T_up = turbo.getCompressorOutletTemperature();

  double area = throttle_cmd * throttle_cmd * constants::throttle_area;
  if (area <= 0.0 || P_down >= P_up)
    return 0.0;

  double pr = std::clamp(P_down / P_up, 0.0, 1.0);

  double crit_pr = std::pow(2.0 / (constants::gamma + 1.0),
                            constants::gamma / (constants::gamma - 1.0));

  double base = constants::discharge_coefficient * area * P_up *
                std::sqrt(constants::gamma / (constants::R * T_up));

  if (pr <= crit_pr) {
    double choked =
        std::pow(2.0 / (constants::gamma + 1.0),
                 (constants::gamma + 1.0) / (2.0 * (constants::gamma - 1.0)));
    return base * choked;
  }

  double term = (2.0 / (constants::gamma - 1.0)) *
                (std::pow(pr, 2.0 / constants::gamma) -
                 std::pow(pr, (constants::gamma + 1.0) / constants::gamma));

  return term > 0.0 ? base * std::sqrt(term) : 0.0;
}

// --------------------------------------------------
// MAIN UPDATE
// --------------------------------------------------

void ICEEngine::update(double dt) {

  /* ============================================================
     IDLE THROTTLE CONTROL (physical: airflow, not torque)
     ============================================================ */
  double idle_error = constants::engine_idle_rad_s - angular_velocity;
  // Only add throttle if we are below idle speed
  double idle_contribution =
      std::max(0.0, constants::idle_throttle_gain * idle_error);
  effective_throttle = std::clamp(throttle + idle_contribution, 0.0, 1.0);

  /* ============================================================
     BASIC SPEED / CYCLE INFO
     ============================================================ */
  double rpm = std::max(getRPM(), 1.0);
  double cycles_per_sec = rpm / 120.0; // 4-stroke

  /* ============================================================
       EXHAUST DYNAMICS
       ============================================================ */
  // Update exhaust temperature based on engine power
  double engine_power_output =
      std::max(0.0, combustion_torque * angular_velocity);
  exhaust_manifold_temperature =
      constants::exhaust_temp_base +
      constants::exhaust_temp_gain * engine_power_output;
  exhaust_manifold_temperature =
      std::clamp(exhaust_manifold_temperature, 400.0, 1273.0);

  // Update exhaust pressure (backpressure increases with mass flow)
  // Higher backpressure drives the turbine but increases pumping losses
  double k_turbine = 1.6e6;
  double target_exh_press =
      constants::ambient_pressure + (exhaust_mass_flow_rate * k_turbine);
  exhaust_manifold_pressure +=
      (target_exh_press - exhaust_manifold_pressure) * 10.0 * dt;

  /* ============================================================
     TURBO + MGU-H CONTROL (PID-like behavior)
     ============================================================ */
  double target_boost = 4.0 * constants::ambient_pressure;
  double boost_error = target_boost - turbo.getCompressorOutletPressure();

  if (effective_throttle > 0.5) {
    if (boost_error > 0) {
      mguh.setMode(MGUHMode::MOTOR);
      // Use more aggressive power to overcome compressor drag at high RPM
      mguh.setRequestedPower(constants::mguk_max_power * throttle);
    } else {
      mguh.setMode(MGUHMode::GENERATOR);
      mguh.setRequestedPower(std::min(120000.0, -boost_error * 0.2));
    }
  }

  mguh.update(dt, turbo.getShaftAngularSpeed());

  // Exhaust pressure increases with mass flow (turbine restriction)
  double turbine_restriction = 1.5e6;
  exhaust_manifold_pressure = constants::ambient_pressure +
                              (exhaust_mass_flow_rate * turbine_restriction);

  // Update plenum pressure from turbo compressor
  plenum_pressure = turbo.getCompressorOutletPressure();

  turbo.update(dt, exhaust_mass_flow_rate, exhaust_manifold_pressure,
               exhaust_manifold_temperature, target_boost, mguh.getTorque());

  /* ============================================================
     INTAKE AIRFLOW (WITH INTERCOOLER)
     ============================================================ */
  double intercooler_eff = 0.85;
  double t_comp = turbo.getCompressorOutletTemperature();
  intake_manifold_temperature =
      t_comp - intercooler_eff * (t_comp - constants::ambient_temperature);

  // Naturally aspirated airflow through throttle
  // Airflow through throttle from plenum to manifold
  na_air_flow =
      getThrottleAirMassFlow(effective_throttle, intake_manifold_pressure);

  // Calculate Volumetric Efficiency based on current RPM
  volumetric_efficiency =
      constants::volumetric_efficiency_max *
      std::exp(-std::pow(
          (rpm - constants::volumetric_efficiency_peak_rpm) / 12500.0, 2));

  // The engine "swallows" air based on displacement and manifold state
  actual_air_flow = (constants::NUM_CYLINDERS * constants::Volume_displacement *
                     cycles_per_sec) *
                    (intake_manifold_pressure /
                     (constants::R * intake_manifold_temperature)) *
                    volumetric_efficiency;

  // Manifold pressure changes based on (Throttle Flow In - Engine Consumption
  // Out)
  intake_manifold_pressure += (constants::R * intake_manifold_temperature /
                               constants::intake_manifold_volume) *
                              (na_air_flow - actual_air_flow) * dt;

  intake_manifold_pressure =
      std::clamp(intake_manifold_pressure, 0.3 * constants::ambient_pressure,
                 turbo.getCompressorOutletPressure());

  /* ============================================================
     FUEL FLOW (DIRECTLY COUPLED TO AIRFLOW)
     ============================================================ */
  fuel_mass_flow =
      actual_air_flow / (constants::AFR_stoich * constants::lambda);

  double fuel_mass_per_cycle =
      fuel_mass_flow / (cycles_per_sec * constants::NUM_CYLINDERS);

  /* ============================================================
     COMBUSTION & INDICATED TORQUE
     ============================================================ */
  double chemical_energy = fuel_mass_per_cycle * constants::LHV_fuel;

  double thermal_energy = chemical_energy * constants::combustion_efficiency;

  double CA50 =
      360.0 - spark_advance_deg + 0.5 * constants::crank_angle_burn_duration;

  double phasing_eff = std::exp(
      -std::pow((CA50 - constants::CA50_opt) / constants::CA50_sigma, 2.0));

  double indicated_work =
      thermal_energy * constants::thermal_efficiency * phasing_eff;

  imep = indicated_work / constants::Volume_displacement;

  indicated_torque = imep * constants::Volume_displacement /
                     (constants::PI * 4.0) * constants::NUM_CYLINDERS;

  /* ============================================================
     LOSSES (FRICTION + PUMPING)
     ============================================================ */
  double rpm_krpm = rpm / 1000.0;

  fmep = constants::fmepA + constants::fmepB * rpm_krpm +
         constants::fmepC * rpm_krpm * rpm_krpm + constants::fmepD * imep;

  friction_torque = fmep * constants::Volume_displacement /
                    (constants::PI * 4.0) * constants::NUM_CYLINDERS;

  double pumping_pressure =
      std::max(exhaust_manifold_pressure - intake_manifold_pressure, 0.0);

  pumping_pressure =
      std::min(pumping_pressure, 0.15 * constants::ambient_pressure);

  pumping_torque = pumping_pressure * constants::Volume_displacement /
                   (constants::PI * 4.0) * constants::NUM_CYLINDERS;

  /* ============================================================
     NET COMBUSTION TORQUE & BMEP
     ============================================================ */
  combustion_torque = indicated_torque - friction_torque - pumping_torque;

  // Calculate BMEP from combustion torque
  bmep = combustion_torque * (constants::PI * 4.0) /
         (constants::Volume_displacement * constants::NUM_CYLINDERS);

  /* ============================================================
     EXHAUST FLOW (CONSISTENT WITH AIR + FUEL)
     ============================================================ */
  exhaust_mass_flow_rate = actual_air_flow + fuel_mass_flow;

  /* ============================================================
     MGU-K
     ============================================================ */
  if (effective_throttle > 0.1) {
    mguk.setMode(MGUKMode::MOTOR);
    mguk.setRequestedPower(constants::mguk_max_power * effective_throttle);
  } else {
    mguk.setMode(MGUKMode::IDLE);
  }
  mguk.update(dt, angular_velocity, battery);
  mguk_torque = mguk.getTorque();

  /* ============================================================
     CRANKSHAFT DYNAMICS
     ============================================================ */
  double load_torque = constants::load_A +
                       constants::load_B * angular_velocity +
                       constants::load_C * angular_velocity * angular_velocity;

  net_torque = combustion_torque - load_torque + mguk_torque;

  angular_velocity += (net_torque / constants::crank_inertia) * dt;
  if (angular_velocity < 10.0)
    angular_velocity = 10.0; // safety lower bound only

  angular_velocity = std::max(angular_velocity, constants::engine_idle_rad_s);

  torque_output = combustion_torque;
}
