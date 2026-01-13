#include "../include/energy_store.hpp"

EnergyStore::EnergyStore(double maxE, double maxChargeP,
                         double maxDischargeP)
    : energy_J(0.5 * maxE), // start at 50% SOC
      max_energy_J(maxE), max_charge_power_W(maxChargeP),
      max_discharge_power_W(maxDischargeP) {}

// Queries

double EnergyStore::getEnergy() const { return energy_J; }

double EnergyStore::getSOC() const { return energy_J / max_energy_J; }

double EnergyStore::getAvailableChargePower() const {
  if (energy_J >= max_energy_J)
    return 0.0;
  return max_charge_power_W;
}

double EnergyStore::getAvailableDischargePower() const {
  if (energy_J <= 0.0)
    return 0.0;
  return max_discharge_power_W;
}

// Update States

void EnergyStore::charge(double energy_in_J) {
  if (energy_in_J <= 0.0)
    return;

  energy_J += energy_in_J;

  if (energy_J > max_energy_J) {
    energy_J = max_energy_J;
  }
}

void EnergyStore::discharge(double energy_out_J) {
  if (energy_out_J <= 0.0)
    return;

  energy_J -= energy_out_J;

  if (energy_J < 0.0) {
    energy_J = 0.0;
  }
}
