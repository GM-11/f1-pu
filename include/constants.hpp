#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace constants {
double constexpr R = 287.05; // J/(kg·K) Specific gas constant for air
double constexpr AFR_stoich =
    14.7; // Stoichiometric air-fuel ratio for gasoli,,lm,kl, '[ ]
double constexpr LHV_fuel =
    43000 * 1000; // J/kg Lower heating value of gasoline (43 MJ/kg)
double constexpr Volume_displacement =
    0.0016 / 6; // m^3 Engine displacement (single cylinder, 400 cc)

double constexpr PI = 3.14159265358979323846;

double constexpr crank_inertia = 0.10; // kg·m² Crankshaft moment of inertia

double constexpr ambient_pressure = 101325.0;
double constexpr ambient_temperature = 300.0; // K
double constexpr gamma = 1.4;                 // Ratio of specific heats for air

double constexpr air_density = 1.18; // kg/m³ at sea level

double constexpr throttle_diameter = 0.040; // m
double constexpr radius = throttle_diameter * 0.5;
double constexpr throttle_area = PI * radius * radius;

double constexpr intake_manifold_volume = 0.002;

double constexpr k_pump = 8e-5;

double constexpr idle_rpm = 1800.0;

double constexpr thermal_efficiency = 0.38;

double constexpr volumetric_efficiency_peak_rpm = 12500.0;

double constexpr volumetric_efficiency_max = 0.95;

double constexpr NUM_CYLINDERS = 6;

double constexpr discharge_coefficient = 0.85;

double torqueCurve(double rpm);
} // namespace constants
#endif // CONSTANTS_HPP
