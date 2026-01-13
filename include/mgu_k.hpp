#pragma once

#include "energy_store.hpp"

enum class MGUKMode {
  MOTOR,     // deploy
  GENERATOR, // regen
  IDLE
};

class MGUK {
public:
  MGUK(double efficiency, double max_power);

  void setMode(MGUKMode m);
  void setRequestedPower(double p);

  void update(double dt, double crank_omega, EnergyStore &battery);

  double getTorque() const;
  double getElectricalPower() const;

private:
  MGUKMode mode;
  double efficiency;
  double max_power;

  double requested_power;
  double torque;
  double electrical_power;
};
