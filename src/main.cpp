#include "../include/ice_engine.hpp"
#include <bits/stdc++.h>
#include <fstream>
#include <iomanip>
#include <iostream>

int main() {
  ICEEngine engine;

  double dt = 0.0001; // 0.1 ms timestep for high fidelity

  double throttle_init = 0.3;
  engine.setThrottle(throttle_init);

  std::ofstream log("data/engine_log.csv");

  // Comprehensive F1 telemetry header
  log << "time,"
      // Engine speed and throttle
      << "rpm,omega,throttle,effective_throttle,"
      // Torque breakdown
      << "indicated_torque,combustion_torque,friction_torque,pumping_torque,"
      << "load_torque,mguk_torque,mguh_torque,net_torque,torque_output,"
      // Power breakdown
      << "ice_power,mguk_power,mguh_power,total_power,"
      // Mean effective pressures (kPa for readability)
      << "imep,bmep,fmep,"
      // Efficiency metrics
      << "thermal_efficiency,mechanical_efficiency,bsfc,volumetric_efficiency,"
      // Intake system
      << "plenum_pressure,intake_manifold_pressure,intake_manifold_temp,"
      << "boost_pressure,compressor_outlet_temp,"
      // Exhaust system
      << "exhaust_manifold_pressure,exhaust_temp,exhaust_mass_flow,"
      // Airflow and fuel
      << "na_air_flow,actual_air_flow,turbo_air_flow,fuel_mass_flow,"
      // Turbo
      << "turbo_speed,turbo_speed_rpm,"
      // Battery / ERS
      << "battery_energy,battery_soc"
      << "\n";

  int iterations = 100000;
  int log_interval = 10; // Log every 10 iterations (1 ms)

  std::cout << std::fixed << std::setprecision(2);

  for (int i = 0; i < iterations; i++) {
    engine.update(dt);

    // Console output every 1000 iterations (100 ms)
    if (i % 1000 == 0) {
      std::cout << "t=" << std::setw(6) << i * dt << "s"
                << " | RPM=" << std::setw(7) << engine.getRPM()
                << " | Torque=" << std::setw(7) << engine.getTorqueOutput()
                << " Nm"
                << " | Power=" << std::setw(8) << engine.getTotalPower() / 1000
                << " kW"
                << " | Boost=" << std::setw(5)
                << engine.getBoostPressure() / 101325.0 << " bar"
                << " | SOC=" << std::setw(5) << engine.getBatterySOC() * 100
                << "%"
                << " | BSFC=" << std::setw(6) << engine.getBSFC() << " g/kWh"
                << "\n";
    }

    // Log to CSV at specified interval
    if (i % log_interval == 0) {
      log << std::fixed
          << std::setprecision(6)
          // Time
          << i * dt
          << ","
          // Engine speed and throttle
          << engine.getRPM() << "," << engine.getAngularVelocity() << ","
          << engine.getThrottle() << "," << engine.getEffectiveThrottle()
          << ","
          // Torque breakdown (Nm)
          << engine.getIndicatedTorque() << "," << engine.getCombustionTorque()
          << "," << engine.getFrictionTorque() << ","
          << engine.getPumpingTorque() << "," << engine.getLoadTorque() << ","
          << engine.getMGUKTorque() << "," << engine.getMGUHTorque() << ","
          << engine.getNetTorque() << "," << engine.getTorqueOutput()
          << ","
          // Power breakdown (W)
          << engine.getICEPower() << "," << engine.getMGUKPower() << ","
          << engine.getMGUHPower() << "," << engine.getTotalPower()
          << ","
          // Mean effective pressures (kPa)
          << engine.getIMEP() / 1000.0 << "," << engine.getBMEP() / 1000.0
          << "," << engine.getFMEP() / 1000.0
          << ","
          // Efficiency metrics
          << engine.getThermalEfficiency() << ","
          << engine.getMechanicalEfficiency() << "," << engine.getBSFC() << ","
          << engine.getVolumetricEfficiency()
          << ","
          // Intake system (Pa and K)
          << engine.getPlenumPressure() << ","
          << engine.getIntakeManifoldPressure() << ","
          << engine.getIntakeManifoldTemperature() << ","
          << engine.getBoostPressure() << ","
          << engine.getCompressorOutletTemperature()
          << ","
          // Exhaust system (Pa, K, kg/s)
          << engine.getExhaustManifoldPressure() << ","
          << engine.getExhaustTemperature() << ","
          << engine.getExhaustMassFlowRate()
          << ","
          // Airflow and fuel (kg/s)
          << engine.getNAAirFlow() << "," << engine.getActualAirFlow() << ","
          << engine.getAirMassFlow() << "," << engine.getFuelMassFlow()
          << ","
          // Turbo (rad/s and RPM)
          << engine.getTurboSpeed() << "," << engine.getTurboSpeedRPM()
          << ","
          // Battery / ERS (J and fraction)
          << engine.getBatteryEnergy() << "," << engine.getBatterySOC() << "\n";
    }

    // Throttle ramp-up profile (simulates acceleration run)
    if (throttle_init < 1.0) {
      throttle_init += 0.001; // Rate: 100% per second at 0.0001s timestep
      engine.setThrottle(throttle_init);
    }
  }

  log.close();

  std::cout << "\n=== Final Engine State ===\n";
  std::cout << "RPM: " << engine.getRPM() << " rev/min\n";
  std::cout << "Total Power: " << engine.getTotalPower() / 1000 << " kW\n";
  std::cout << "ICE Power: " << engine.getICEPower() / 1000 << " kW\n";
  std::cout << "MGU-K Power: " << engine.getMGUKPower() / 1000 << " kW\n";
  std::cout << "Brake Torque: " << engine.getTorqueOutput() << " Nm\n";
  std::cout << "BSFC: " << engine.getBSFC() << " g/kWh\n";
  std::cout << "Thermal Efficiency: " << engine.getThermalEfficiency() * 100
            << "%\n";
  std::cout << "Mechanical Efficiency: "
            << engine.getMechanicalEfficiency() * 100 << "%\n";
  std::cout << "Boost Pressure: " << engine.getBoostPressure() / 101325.0
            << " bar\n";
  std::cout << "Turbo Speed: " << engine.getTurboSpeedRPM() << " RPM\n";
  std::cout << "Battery SOC: " << engine.getBatterySOC() * 100 << "%\n";
  std::cout << "\nLog saved to data/engine_log.csv\n";

  return 0;
}
