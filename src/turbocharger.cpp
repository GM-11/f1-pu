#include "../include/turbocharger.hpp"
#include "../include/constants.hpp"
#include <algorithm>
#include <bits/stdc++.h>
#include <cmath>

Turbocharger::Turbocharger(double inertia, double turbine_eff,
                           double compressor_eff, double bearing_loss)
    : turbo_inertia(inertia), turbine_efficiency(turbine_eff),
      compressor_efficiency(compressor_eff), bearing_loss_coeff(bearing_loss),
      shaft_angular_speed(0.0), // Start at rest
      compressor_outlet_pressure(constants::ambient_pressure),
      compressor_outlet_temperature(constants::ambient_temperature),
      available_air_mass_flow(0.0) {}
double Turbocharger::getCompressorOutletPressure() const {
  return compressor_outlet_pressure;
}

double Turbocharger::getCompressorOutletTemperature() const {
  return compressor_outlet_temperature;
}

double Turbocharger::getShaftAngularSpeed() const {
  return shaft_angular_speed;
}

double Turbocharger::getAvailableAirMassFlow() const {
  return available_air_mass_flow;
}

void Turbocharger::update(double dt, double exhaust_mass_flow,
                          double exhaust_pressure, double exhaust_temperature,
                          double target_boost_pressure, double mguh_torque) {

  exhaust_pressure =
      std::max(exhaust_pressure, 1.1 * constants::ambient_pressure);

  double turbine_pr =
      std::clamp(exhaust_pressure / constants::ambient_pressure, 1.01, 5.0);

  double cp_exhaust = (constants::R * constants::gamma_exhaust) /
                      (constants::gamma_exhaust - 1.0);

  double turbine_power =
      exhaust_mass_flow * cp_exhaust * exhaust_temperature *
      turbine_efficiency *
      (1.0 - std::pow(turbine_pr, (1.0 - constants::gamma_exhaust) /
                                      constants::gamma_exhaust));

  turbine_power = std::max(turbine_power, 0.0);

  double requested_pr = target_boost_pressure / constants::ambient_pressure;

  double achievable_pr =
      constants::turbo_pr_idle +
      (constants::turbo_max_pr - constants::turbo_pr_idle) *
          std::clamp(shaft_angular_speed / constants::turbo_nominal_speed, 0.0,
                     1.0);

  double compressor_pr = std::min(requested_pr, achievable_pr);

  compressor_outlet_temperature =
      constants::ambient_temperature *
      (1.0 + (1.0 / compressor_efficiency) *
                 (std::pow(compressor_pr,
                           (constants::gamma - 1.0) / constants::gamma) -
                  1.0));

  double cp_air = (constants::R * constants::gamma) / (constants::gamma - 1.0);

  double compressor_power = 0.0;
  if (available_air_mass_flow > 1e-6) {
    compressor_power =
        available_air_mass_flow * cp_air *
        (compressor_outlet_temperature - constants::ambient_temperature);
  }

  double turbine_torque = (shaft_angular_speed > 10.0)
                              ? turbine_power / shaft_angular_speed
                              : turbine_power / 10.0; // prevents stall

  double compressor_torque = (shaft_angular_speed > 10.0)
                                 ? compressor_power / shaft_angular_speed
                                 : 0.0;

  double bearing_torque = bearing_loss_coeff * shaft_angular_speed;

  double net_torque =
      turbine_torque - compressor_torque - bearing_torque + mguh_torque;

  double angular_accel = net_torque / turbo_inertia;

  shaft_angular_speed += angular_accel * dt;
  shaft_angular_speed =
      std::max(shaft_angular_speed, 0.05 * constants::turbo_nominal_speed);

  compressor_outlet_pressure = compressor_pr * constants::ambient_pressure;

  double speed_ratio = std::clamp(
      shaft_angular_speed / constants::turbo_nominal_speed, 0.0, 1.5);

  available_air_mass_flow =
      std::min(speed_ratio * constants::turbo_max_air_flow, exhaust_mass_flow);
}
