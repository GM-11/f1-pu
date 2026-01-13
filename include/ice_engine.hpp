#pragma once

#include "../include/mgu_h.hpp"
#include "../include/mgu_k.hpp"
#include "../include/turbocharger.hpp"
class ICEEngine {
public:
  ICEEngine();

  void setThrottle(double t); // 0..1
  void update(double dt);     // physics step

  double getRPM() const;
  double getTorqueOutput() const;
  double getAngularVelocity() const;
  double getNetPower() const;
  double getThrottleAirMassFlow(double throttle, double P_down) const;
  double getExhaustMassFlowRate() const;

  // ---------------- ENGINE / CRANK ----------------
  double getCombustionTorque() const;
  double getLoadTorque() const;

  // ---------------- TURBO / INTAKE ----------------
  double getTurboSpeed() const;
  double getBoostPressure() const;
  double getIntakeManifoldPressure() const;
  double getIntakeManifoldTemperature() const;
  double getAirMassFlow() const;

  // ---------------- EXHAUST ----------------
  double getExhaustManifoldPressure() const;
  double getExhaustTemperature() const;

  // ---------------- ERS ----------------
  double getMGUHTorque() const;
  double getMGUHPower() const;

  double getMGUKTorque() const;
  double getMGUKPower() const;

  double getBatteryEnergy() const;
  double getBatterySOC() const;

private:
  Turbocharger turbo;
  MGUH mguh;
  MGUK mguk;
  EnergyStore battery;

  // state
  double angular_velocity;             // rad/s
  double throttle;                     // 0..1
  double torque_output;                // Nm
  double intake_manifold_pressure;     // Pa
  double exhaust_manifold_pressure;    // Pa
  double intake_manifold_temperature;  // K
  double exhaust_manifold_temperature; // K
  double load_torque;                  // Nm
  double plenum_pressure;              // Pa
  double spark_advance_deg;            // BTDC
  double exhaust_mass_flow_rate;
  double mguk_torque;

  double combustion_torque;
};
