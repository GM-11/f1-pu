#pragma once

class EnergyStore {
public:
  EnergyStore(double max_energy_J, double max_charge_power_W,
              double max_discharge_power_W);

  double getEnergy() const;
  double getSOC() const;

  double getAvailableChargePower() const;
  double getAvailableDischargePower() const;

  void charge(double energy_J);    // add energy
  void discharge(double energy_J); // remove energy

private:
  double energy_J;
  double max_energy_J;

  double max_charge_power_W;
  double max_discharge_power_W;
};
