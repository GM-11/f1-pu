class Turbocharger {
public:
  Turbocharger(double inertia, double turbine_efficiency,
               double compressor_efficiency, double bearing_loss_coeff);

  void update(double dt,
              double exhaust_mass_flow,     // kg/s
              double exhaust_pressure,      // Pa
              double exhaust_temperature,   // K
              double target_boost_pressure, // Pa (wastegate control)
              double mgu_torque);

  // Outputs to engine
  double getCompressorOutletPressure() const;
  double getCompressorOutletTemperature() const;
  double getAvailableAirMassFlow() const;

  double getShaftAngularSpeed() const;

private:
  double shaft_angular_speed; // rad/s

  double turbo_inertia;         // kg·m²
  double turbine_efficiency;    // 0–1
  double compressor_efficiency; // 0–1
  double bearing_loss_coeff;    // W per rad/s

  double compressor_outlet_pressure;    // Pa
  double compressor_outlet_temperature; // K
  double available_air_mass_flow;       // kg/s
};
