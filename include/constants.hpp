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
double constexpr gamma_exhaust = 1.33;
double constexpr lambda = 1.1;

double constexpr exhaust_area = 3e-4;

double constexpr air_density = 1.18; // kg/m³ at sea level

double constexpr throttle_diameter = 0.040; // m
double constexpr radius = throttle_diameter * 0.5;
double constexpr throttle_area = PI * radius * radius;

double constexpr exhaust_manifold_volume = 1e-3;
double constexpr intake_manifold_volume = 0.002;

double constexpr k_pump = 8e-5;

double constexpr idle_rpm = 1800.0;

double constexpr thermal_efficiency = 0.38;
double constexpr combustion_efficiency = 0.98;

double constexpr volumetric_efficiency_peak_rpm = 12500.0;

double constexpr volumetric_efficiency_max = 0.95;

double constexpr NUM_CYLINDERS = 6;

double constexpr discharge_coefficient = 0.85;

double constexpr CA50_opt = 368.0;  // deg
double constexpr CA50_sigma = 15.0; // deg

double constexpr crank_anlge_at_combustion_start_deg = 335;
double constexpr crank_angle_burn_duration = 35;

constexpr double exhaust_temp_base = 720.0;  // K
constexpr double exhaust_temp_gain = 2.0e-5; // K / W

constexpr double fmepA = 0.4e5;   // Pa
constexpr double fmepB = 0.02e5;  // Pa / (krpm)
constexpr double fmepC = 0.002e5; // Pa / (krpm^2)
constexpr double fmepD = 0.08;    // load sensitivity

constexpr double compressor_isentropic_efficiency = 0.7;

constexpr double intake_flow_capacity = 0.27;

double torqueCurve(double rpm);
} // namespace constants
#endif // CONSTANTS_HPP
