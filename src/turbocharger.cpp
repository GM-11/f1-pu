#include "../include/turbocharger.hpp"
#include "../include/constants.hpp"
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

                          double target_boost_pressure) {

  double turbine_pressure_ratio =
      std::max(exhaust_pressure / constants::ambient_pressure, 1.0);

  double exhaust_gas_specific_heat = (constants::R * constants::gamma_exhaust) /
                                     (constants::gamma_exhaust - 1);
  double turbine_power =
      exhaust_mass_flow * exhaust_gas_specific_heat * exhaust_temperature *
      turbine_efficiency *
      (1.0 - std::pow(turbine_pressure_ratio, (1.0 - constants::gamma_exhaust) /
                                                  constants::gamma_exhaust));

  turbine_power = std::max(turbine_power, 0.0);

  double compressor_pressure_ratio =
      target_boost_pressure / constants::ambient_pressure;

  compressor_outlet_temperature =
      constants::ambient_temperature *
      (1 + (1 / constants::compressor_isentropic_efficiency) *
               (std::pow(constants::compressor_isentropic_efficiency,
                         (constants::gamma - 1) / constants::gamma) -
                1));

  double air_specific_heat =
      (constants::R * constants::gamma) / (constants::gamma - 1);

  double compressor_power =
      available_air_mass_flow * air_specific_heat *
      constants::ambient_temperature * (1.0 / compressor_efficiency) *
      (std::pow(compressor_pressure_ratio,
                (constants::gamma - 1.0) / constants::gamma) -
       1.0);

  compressor_power = std::max(compressor_power, 0.0);

  double power_loss = bearing_loss_coeff * shaft_angular_speed;

  double net_shaft_power = turbine_power - compressor_power - power_loss;

  double angular_acceleration = net_shaft_power / turbo_inertia;

  shaft_angular_speed += angular_acceleration * dt;
  shaft_angular_speed = std::max(shaft_angular_speed, 0.0);

  compressor_outlet_pressure =
      compressor_pressure_ratio * constants::ambient_pressure;

  available_air_mass_flow = (compressor_outlet_pressure /
                             (constants::R * compressor_outlet_temperature)) *
                            constants::intake_flow_capacity;
}
