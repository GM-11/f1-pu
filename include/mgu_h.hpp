enum class MGUHMode { MOTOR, GENERATOR, IDLE };

class MGUH {
public:
  MGUH(double inertia, double efficiency, double max_power);

  void setMode(MGUHMode m);
  void setRequestedPower(double p); // W

  void update(double dt, double turbo_omega);

  double getTorque() const;          // Nm (applied on turbo shaft)
  double getElectricalPower() const; // W (+gen, -motor)

private:
  // parameters
  double inertia;    // kg·m² (mostly for completeness)
  double efficiency; // 0–1
  double maxPower;   // W

  // state
  double omega;           // rad/s
  double torque;          // Nm
  double electricalPower; // W
  double requestedPower;  // W

  MGUHMode mode;
};
