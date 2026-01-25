# F1 Power Unit (f1-pu)

A compact, single-process simulation of a modern F1-style power unit, focused on capturing the coupled dynamics between the Internal Combustion Engine (ICE), turbocharger, and ERS (MGU‑H + MGU‑K + battery). The program runs a time-stepped simulation, logs detailed “telemetry” to CSV, and includes a small Python client for turning logs into plots.

## What this project is

This repository contains:

- A C++ simulation executable (`f1-pu`) that:
  - steps the power unit forward in time at a fixed `dt`
  - applies a throttle input profile
  - computes engine/turbo/ERS states each step
  - writes a comprehensive CSV log to `data/engine_log.csv`
- A `data/` folder with example outputs (CSV plus PNG plots)
- A `python-client/` helper that reads the CSV and produces analysis plots (using Pandas + Matplotlib)

The model is intended to be understandable and hackable: each major subsystem is separated into its own module and then composed into the engine “plant”.

## Simulated subsystems

The simulation tracks the key power-unit components and their interactions:

- **ICE (Internal Combustion Engine)**
  - Converts fuel energy into crankshaft torque and hot exhaust flow.
  - Includes torque breakdown terms (combustion, friction, pumping, load) and derived metrics (BMEP/IMEP/FMEP, BSFC, efficiencies).
  - Produces exhaust conditions that drive the turbo.

- **Turbocharger (turbine + compressor + shaft)**
  - Uses exhaust mass flow, pressure, and temperature to compute turbine power/torque.
  - Computes compressor pressure ratio / outlet temperature and resulting available air mass flow.
  - Maintains shaft speed with inertia and a bearing loss term.
  - Accepts an external torque contribution from the MGU‑H (motor or generator).

- **MGU‑H**
  - Operates in motor, generator, or idle mode.
  - Couples electrically to the battery and mechanically to the turbo shaft.
  - Implements power limits and battery acceptance/supply constraints.

- **MGU‑K**
  - Operates in motor, generator, or idle mode.
  - Couples electrically to the battery and mechanically to the crankshaft.
  - Implements max power limits and battery charge/discharge constraints.

- **Energy Store (Battery)**
  - Tracks stored energy and state-of-charge (SOC).
  - Provides charge/discharge power availability bounds.
  - Starts at a mid SOC by default.

Most parameters (air properties, efficiencies, limits, nominal speeds, etc.) are centralized in `include/constants.hpp`.

## Build requirements

- A C++17-capable compiler
- CMake 3.10+

## Building

From the repository root:

- Configure and build with CMake (out-of-tree builds are recommended).
- The build generates a single executable named `f1-pu`.

## Running

When you run the executable:

- It performs a fixed number of iterations at a small timestep (high-fidelity stepping).
- It ramps throttle toward full over time (a simple acceleration-style input).
- It prints periodic console status (RPM, torque, power, boost, SOC, BSFC).
- It writes the main telemetry log to `data/engine_log.csv`.

Make sure you run the program from the repository root (or otherwise ensure the `data/` directory exists and is writable), since outputs are written using a relative path.

## Output artifacts

### CSV telemetry

The simulator writes a detailed log to:

- `data/engine_log.csv`

The CSV includes, among other fields:

- Time, RPM, angular velocity, throttle and effective throttle
- Torque breakdown terms and net torque/torque output
- Power breakdown: ICE power, MGU‑K power, MGU‑H power, total power
- Mean effective pressures and efficiency metrics (thermal/mechanical), BSFC, volumetric efficiency
- Intake pressures/temps, boost, compressor outlet temperature
- Exhaust pressures/temps, exhaust mass flow
- Turbo shaft speed (rad/s and RPM)
- Battery energy and SOC

This telemetry is meant to support post-processing and sanity-checking of model behavior.

### Pre-generated plots

The `data/` directory also contains a set of PNG plots (for example RPM vs time, torque/power breakdowns, turbo performance, ERS/battery traces). These are useful for quickly validating trends after changes.

## Python client (plotting / analysis)

The `python-client/` directory contains a small plotting utility intended to:

- read `data/engine_log.csv`
- generate visualizations of key metrics (engine speed, torque/power components, pressures, turbo speed, SOC, etc.)

It uses:

- Python 3.13+
- Pandas
- Matplotlib

Refer to `python-client/pyproject.toml` for declared dependencies and `python-client/main.py` for the entry point logic (run it from the repo root or ensure it can locate the CSV).

## Project structure

- `include/` — public headers for the simulation components and shared constants
- `src/` — C++ implementations and the main simulation entry point
- `data/` — generated logs and example plots
- `python-client/` — post-processing utility for analysis/plotting
- `CMakeLists.txt` — build configuration
- `working.md` — design notes and component-by-component modeling breakdown

## Notes and limitations

- This is a simplified physics model intended for experimentation and learning, not a regulation-accurate, track-validated F1 simulator.
- Control strategies (wastegate/boost control, ERS deployment logic, throttle shaping) are deliberately straightforward and are good candidates for extension.
- Many effects are not modeled (e.g., intercooler heat-rejection dynamics, detailed combustion phasing, surge/choke maps, gear/vehicle dynamics). The current structure is designed so you can add them as separate concerns.

## Common extension ideas

- Add closed-loop boost control and distinct wastegate behavior
- Implement an ERS strategy controller (SOC targets, deployment/harvesting schedules, traction-limited torque)
- Add a vehicle/drivetrain model to replace the implicit load approximation
- Replace simple compressor/turbine relationships with map-based models
- Add unit tests for component-level invariants (SOC bounds, power sign conventions, stability checks)
