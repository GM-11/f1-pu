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
  double getFrictionTorque() const;
  double getPumpingTorque() const;
  double getIndicatedTorque() const;
  double getNetTorque() const;

  // ---------------- THROTTLE / AIRFLOW ----------------
  double getThrottle() const;
  double getEffectiveThrottle() const;
  double getNAAirFlow() const;
  double getActualAirFlow() const;
  double getFuelMassFlow() const;
  double getVolumetricEfficiency() const;

  // ---------------- TURBO / INTAKE ----------------
  double getTurboSpeed() const;
  double getTurboSpeedRPM() const;
  double getBoostPressure() const;
  double getPlenumPressure() const;
  double getIntakeManifoldPressure() const;
  double getIntakeManifoldTemperature() const;
  double getAirMassFlow() const;
  double getCompressorOutletTemperature() const;

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

  // ---------------- PERFORMANCE METRICS ----------------
  double getBMEP() const; // Brake Mean Effective Pressure
  double getIMEP() const; // Indicated Mean Effective Pressure
  double getFMEP() const; // Friction Mean Effective Pressure
  double getBSFC() const; // Brake Specific Fuel Consumption
  double getThermalEfficiency() const;
  double getMechanicalEfficiency() const;
  double getTotalPower() const; // ICE + MGU-K power
  double getICEPower() const;   // Pure ICE power (combustion - losses)

private:
  Turbocharger turbo;
  MGUH mguh;
  MGUK mguk;
  EnergyStore battery;

  // state
  double angular_velocity;             // rad/s
  double throttle;                     // 0..1
  double effective_throttle;           // 0..1 (after idle control)
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
  double friction_torque;
  double pumping_torque;
  double indicated_torque;
  double net_torque;

  // Airflow telemetry
  double na_air_flow;
  double actual_air_flow;
  double fuel_mass_flow;
  double volumetric_efficiency;

  // Performance metrics
  double imep;
  double fmep;
  double bmep;
};
