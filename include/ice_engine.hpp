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

private:
  Turbocharger turbo;

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
};
