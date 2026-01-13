#include "../include/mgu_h.hpp"
#include <algorithm>

static inline double clamp(double v, double lo, double hi) {
  return std::max(lo, std::min(v, hi));
}

MGUH::MGUH(double inertia, double efficiency, double max_power)
    : inertia(inertia), efficiency(efficiency), maxPower(max_power), omega(0.0),
      torque(0.0), electricalPower(0.0), requestedPower(0.0),
      mode(MGUHMode::IDLE) {}

void MGUH::setMode(MGUHMode m) { mode = m; }

void MGUH::setRequestedPower(double p) {
  requestedPower = clamp(p, 0.0, maxPower);
}

void MGUH::update(double /*dt*/, double turbo_omega) {

  // avoid division blow-up at low speed
  if (turbo_omega < 1.0) {
    torque = 0.0;
    electricalPower = 0.0;
    return;
  }

  switch (mode) {
  case MGUHMode::GENERATOR:
    electricalPower = clamp(requestedPower, 0.0, maxPower);
    torque = -electricalPower / (efficiency * turbo_omega);
    break;

  case MGUHMode::MOTOR:
    electricalPower = -clamp(requestedPower, 0.0, maxPower);
    torque = +(efficiency * (-electricalPower)) / turbo_omega;
    break;

  case MGUHMode::IDLE:
  default:
    torque = 0.0;
    electricalPower = 0.0;
    break;
  }
}

double MGUH::getTorque() const { return torque; }

double MGUH::getElectricalPower() const { return electricalPower; }
