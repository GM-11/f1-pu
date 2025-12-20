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

private:
  // state
  double angular_velocity;            // rad/s
  double throttle;                    // 0..1
  double torque_output;               // Nm
  double intake_manifold_pressure;    // Pa
  double intake_manifold_temperature; // K
  double load_torque;                 // Nm

  // friction model parameters
  double frictionA;
  double frictionB;
  double frictionC;
};
