#include "../include/mgu_k.hpp"
#include <algorithm>

MGUK::MGUK(double eff, double maxP)
    : mode(MGUKMode::IDLE), efficiency(eff), max_power(maxP),
      requested_power(0.0), torque(0.0), electrical_power(0.0) {}

void MGUK::setMode(MGUKMode m) { mode = m; }

void MGUK::setRequestedPower(double p) { requested_power = p; }

void MGUK::update(double dt, double crank_omega, EnergyStore &battery) {
  torque = 0.0;
  electrical_power = 0.0;

  if (crank_omega < 1.0)
    return;

  switch (mode) {
  case MGUKMode::MOTOR: {
    double available_power =
        std::min(requested_power,
                 std::min(max_power, battery.getAvailableDischargePower()));

    electrical_power = -1 * available_power;

    torque = (available_power * efficiency) / (crank_omega);

    battery.discharge(available_power * dt);
    break;
  }

  case MGUKMode::GENERATOR: {
    double available_power =
        std::min(requested_power,
                 std::min(max_power, battery.getAvailableChargePower()));

    electrical_power = +available_power; // shaft → battery

    torque = -available_power / (efficiency * crank_omega);

    battery.charge(available_power * dt);
    break;
  }

  case MGUKMode::IDLE:

  default:
    break;
  }
}

double MGUK::getTorque() const { return torque; }

double MGUK::getElectricalPower() const { return electrical_power; }
