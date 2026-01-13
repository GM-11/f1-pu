# %% F1 Power Unit Telemetry Analysis Dashboard
# Professional diagnostic plots used by F1 teams for engine development and testing

import warnings

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.gridspec import GridSpec

warnings.filterwarnings("ignore")

# Set professional styling
plt.style.use("seaborn-v0_8-whitegrid")
plt.rcParams["figure.facecolor"] = "white"
plt.rcParams["axes.facecolor"] = "white"
plt.rcParams["font.family"] = "sans-serif"
plt.rcParams["font.size"] = 10

# %%
# Load telemetry data
df = pd.read_csv("../data/engine_log.csv")
print(f"Loaded {len(df)} data points")
print(f"Columns: {list(df.columns)}")
df.head()

# %% ========================================================================
# PLOT 1: RPM vs Time - Basic engine speed trace
# ========================================================================
fig, ax = plt.subplots(figsize=(12, 4))
ax.plot(df["time"], df["rpm"], color="tab:red", linewidth=1.5)
ax.set_xlabel("Time (s)")
ax.set_ylabel("Engine Speed (RPM)")
ax.set_title("Engine RPM vs Time - Acceleration Run")
ax.axhline(y=15000, color="gray", linestyle="--", alpha=0.5, label="Rev Limit")
ax.legend()
ax.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("../data/01_rpm_vs_time.png", dpi=150, bbox_inches="tight")
print("Saved: 01_rpm_vs_time.png")

# %% ========================================================================
# PLOT 2: Torque and Power Curves vs RPM (Classic Dyno Plot)
# ========================================================================
# Bin data by RPM for cleaner curves
rpm_bins = np.arange(3000, 16000, 250)
df["rpm_bin"] = pd.cut(df["rpm"], bins=rpm_bins, labels=rpm_bins[:-1])
df_binned = df.groupby("rpm_bin", observed=True).median().reset_index()
df_binned["rpm_bin"] = df_binned["rpm_bin"].astype(float)

fig, ax1 = plt.subplots(figsize=(12, 6))

# Torque on left axis
color_torque = "tab:blue"
ax1.set_xlabel("Engine Speed (RPM)")
ax1.set_ylabel("Torque (Nm)", color=color_torque)
ax1.plot(
    df_binned["rpm_bin"],
    df_binned["torque_output"],
    color=color_torque,
    linewidth=2.5,
    label="Brake Torque",
)
ax1.tick_params(axis="y", labelcolor=color_torque)
ax1.set_ylim(bottom=0)

# Power on right axis
ax2 = ax1.twinx()
color_power = "tab:red"
ax2.set_ylabel("Power (kW)", color=color_power)
ax2.plot(
    df_binned["rpm_bin"],
    df_binned["total_power"] / 1000,
    color=color_power,
    linewidth=2.5,
    label="Total Power (ICE+MGU-K)",
)
ax2.plot(
    df_binned["rpm_bin"],
    df_binned["ice_power"] / 1000,
    color="tab:orange",
    linewidth=2,
    linestyle="--",
    label="ICE Power Only",
)
ax2.tick_params(axis="y", labelcolor=color_power)
ax2.set_ylim(bottom=0)

# Combined legend
lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax1.legend(lines1 + lines2, labels1 + labels2, loc="upper left")

ax1.set_title("Power Unit Dyno Curve - Torque and Power vs RPM")
ax1.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("../data/02_dyno_curve.png", dpi=150, bbox_inches="tight")
print("Saved: 02_dyno_curve.png")

# %% ========================================================================
# PLOT 3: Torque Breakdown - Indicated, Friction, Pumping Losses
# ========================================================================
fig, ax = plt.subplots(figsize=(12, 6))

ax.plot(
    df_binned["rpm_bin"],
    df_binned["indicated_torque"],
    label="Indicated Torque",
    linewidth=2,
    color="tab:green",
)
ax.plot(
    df_binned["rpm_bin"],
    df_binned["combustion_torque"],
    label="Combustion (Brake) Torque",
    linewidth=2,
    color="tab:blue",
)
ax.plot(
    df_binned["rpm_bin"],
    df_binned["friction_torque"],
    label="Friction Losses",
    linewidth=2,
    color="tab:red",
)
ax.plot(
    df_binned["rpm_bin"],
    df_binned["pumping_torque"],
    label="Pumping Losses",
    linewidth=2,
    color="tab:orange",
)
ax.plot(
    df_binned["rpm_bin"],
    df_binned["load_torque"],
    label="External Load",
    linewidth=2,
    color="tab:purple",
    linestyle="--",
)

ax.set_xlabel("Engine Speed (RPM)")
ax.set_ylabel("Torque (Nm)")
ax.set_title("Torque Breakdown vs RPM - Losses Analysis")
ax.legend(loc="upper left")
ax.grid(True, alpha=0.3)
ax.set_ylim(bottom=0)
plt.tight_layout()
plt.savefig("../data/03_torque_breakdown.png", dpi=150, bbox_inches="tight")
print("Saved: 03_torque_breakdown.png")

# %% ========================================================================
# PLOT 4: Power Breakdown - ICE, MGU-K, MGU-H
# ========================================================================
fig, ax = plt.subplots(figsize=(12, 6))

ax.plot(
    df_binned["rpm_bin"],
    df_binned["ice_power"] / 1000,
    label="ICE Power",
    linewidth=2.5,
    color="tab:blue",
)
ax.plot(
    df_binned["rpm_bin"],
    df_binned["mguk_power"] / 1000,
    label="MGU-K Power",
    linewidth=2,
    color="tab:green",
)
ax.plot(
    df_binned["rpm_bin"],
    df_binned["mguh_power"] / 1000,
    label="MGU-H Power",
    linewidth=2,
    color="tab:orange",
)
ax.plot(
    df_binned["rpm_bin"],
    df_binned["total_power"] / 1000,
    label="Total Power",
    linewidth=2.5,
    color="tab:red",
    linestyle="--",
)

ax.axhline(y=120, color="green", linestyle=":", alpha=0.7, label="MGU-K Limit (120 kW)")
ax.set_xlabel("Engine Speed (RPM)")
ax.set_ylabel("Power (kW)")
ax.set_title("Power Breakdown vs RPM - ICE and ERS Contributions")
ax.legend(loc="upper left")
ax.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("../data/04_power_breakdown.png", dpi=150, bbox_inches="tight")
print("Saved: 04_power_breakdown.png")

# %% ========================================================================
# PLOT 5: Mean Effective Pressure (MEP) Analysis
# ========================================================================
fig, ax = plt.subplots(figsize=(12, 6))

ax.plot(
    df_binned["rpm_bin"],
    df_binned["imep"],
    label="IMEP (Indicated)",
    linewidth=2.5,
    color="tab:green",
)
ax.plot(
    df_binned["rpm_bin"],
    df_binned["bmep"],
    label="BMEP (Brake)",
    linewidth=2.5,
    color="tab:blue",
)
ax.plot(
    df_binned["rpm_bin"],
    df_binned["fmep"],
    label="FMEP (Friction)",
    linewidth=2,
    color="tab:red",
)

ax.set_xlabel("Engine Speed (RPM)")
ax.set_ylabel("Mean Effective Pressure (kPa)")
ax.set_title("Mean Effective Pressure Analysis - Engine Load Characteristics")
ax.legend(loc="upper right")
ax.grid(True, alpha=0.3)
ax.set_ylim(bottom=0)
plt.tight_layout()
plt.savefig("../data/05_mep_analysis.png", dpi=150, bbox_inches="tight")
print("Saved: 05_mep_analysis.png")

# %% ========================================================================
# PLOT 6: Efficiency Metrics vs RPM
# ========================================================================
fig, ax1 = plt.subplots(figsize=(12, 6))

# Thermal and Mechanical Efficiency on left axis
ax1.plot(
    df_binned["rpm_bin"],
    df_binned["thermal_efficiency"] * 100,
    label="Thermal Efficiency",
    linewidth=2,
    color="tab:red",
)
ax1.plot(
    df_binned["rpm_bin"],
    df_binned["mechanical_efficiency"] * 100,
    label="Mechanical Efficiency",
    linewidth=2,
    color="tab:blue",
)
ax1.plot(
    df_binned["rpm_bin"],
    df_binned["volumetric_efficiency"] * 100,
    label="Volumetric Efficiency",
    linewidth=2,
    color="tab:green",
)
ax1.set_xlabel("Engine Speed (RPM)")
ax1.set_ylabel("Efficiency (%)")
ax1.set_ylim(0, 100)
ax1.legend(loc="upper left")
ax1.grid(True, alpha=0.3)

# BSFC on right axis
ax2 = ax1.twinx()
ax2.plot(
    df_binned["rpm_bin"],
    df_binned["bsfc"],
    label="BSFC",
    linewidth=2,
    color="tab:purple",
    linestyle="--",
)
ax2.set_ylabel("BSFC (g/kWh)", color="tab:purple")
ax2.tick_params(axis="y", labelcolor="tab:purple")

# Add BSFC to legend
lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax1.legend(lines1 + lines2, labels1 + labels2, loc="upper left")

ax1.set_title("Engine Efficiency Metrics vs RPM")
plt.tight_layout()
plt.savefig("../data/06_efficiency_metrics.png", dpi=150, bbox_inches="tight")
print("Saved: 06_efficiency_metrics.png")

# %% ========================================================================
# PLOT 7: BSFC Map (Fuel Consumption Map)
# ========================================================================
fig, ax = plt.subplots(figsize=(12, 6))

# Filter valid BSFC data
df_valid = df[(df["bsfc"] > 0) & (df["bsfc"] < 1000) & (df["bmep"] > 0)]

scatter = ax.scatter(
    df_valid["rpm"],
    df_valid["bmep"],
    c=df_valid["bsfc"],
    cmap="RdYlGn_r",
    s=2,
    alpha=0.7,
    vmin=200,
    vmax=400,
)
cbar = plt.colorbar(scatter, ax=ax)
cbar.set_label("BSFC (g/kWh)")

ax.set_xlabel("Engine Speed (RPM)")
ax.set_ylabel("BMEP (kPa)")
ax.set_title("Brake Specific Fuel Consumption Map - Operating Efficiency")
ax.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("../data/07_bsfc_map.png", dpi=150, bbox_inches="tight")
print("Saved: 07_bsfc_map.png")

# %% ========================================================================
# PLOT 8: Intake System - Boost and Manifold Pressures
# ========================================================================
fig, axes = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

# Time traces
ax1 = axes[0]
ax1.plot(
    df["time"],
    df["boost_pressure"] / 101325,
    label="Boost Pressure (Compressor Out)",
    linewidth=1.5,
    color="tab:red",
)
ax1.plot(
    df["time"],
    df["intake_manifold_pressure"] / 101325,
    label="Intake Manifold Pressure",
    linewidth=1.5,
    color="tab:blue",
)
ax1.plot(
    df["time"],
    df["plenum_pressure"] / 101325,
    label="Plenum Pressure",
    linewidth=1.5,
    color="tab:green",
    linestyle="--",
)
ax1.axhline(y=1.0, color="gray", linestyle=":", alpha=0.5, label="Atmospheric")
ax1.set_ylabel("Pressure (bar)")
ax1.set_title("Intake System Pressures vs Time")
ax1.legend(loc="upper left")
ax1.grid(True, alpha=0.3)

# vs RPM
ax2 = axes[1]
ax2.plot(
    df_binned["rpm_bin"],
    df_binned["boost_pressure"] / 101325,
    label="Boost Pressure",
    linewidth=2,
    color="tab:red",
)
ax2.plot(
    df_binned["rpm_bin"],
    df_binned["intake_manifold_pressure"] / 101325,
    label="Manifold Pressure",
    linewidth=2,
    color="tab:blue",
)
ax2.set_xlabel("Engine Speed (RPM)")
ax2.set_ylabel("Pressure (bar)")
ax2.set_title("Intake System Pressures vs RPM")
ax2.legend(loc="upper left")
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("../data/08_intake_pressures.png", dpi=150, bbox_inches="tight")
print("Saved: 08_intake_pressures.png")

# %% ========================================================================
# PLOT 9: Exhaust System Characteristics
# ========================================================================
fig, axes = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

# Exhaust pressure and temperature vs time
ax1 = axes[0]
ax1.plot(
    df["time"],
    df["exhaust_manifold_pressure"] / 101325,
    label="Exhaust Manifold Pressure",
    linewidth=1.5,
    color="tab:red",
)
ax1.set_ylabel("Pressure (bar)", color="tab:red")
ax1.tick_params(axis="y", labelcolor="tab:red")
ax1.grid(True, alpha=0.3)

ax1b = ax1.twinx()
ax1b.plot(
    df["time"],
    df["exhaust_temp"],
    label="Exhaust Temperature",
    linewidth=1.5,
    color="tab:orange",
)
ax1b.set_ylabel("Temperature (K)", color="tab:orange")
ax1b.tick_params(axis="y", labelcolor="tab:orange")

ax1.set_title("Exhaust System Characteristics vs Time")
lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax1b.get_legend_handles_labels()
ax1.legend(lines1 + lines2, labels1 + labels2, loc="upper left")

# Exhaust mass flow vs RPM
ax2 = axes[1]
ax2.plot(
    df_binned["rpm_bin"],
    df_binned["exhaust_mass_flow"] * 1000,
    label="Exhaust Mass Flow",
    linewidth=2,
    color="tab:purple",
)
ax2.set_xlabel("Engine Speed (RPM)")
ax2.set_ylabel("Mass Flow (g/s)")
ax2.set_title("Exhaust Mass Flow vs RPM")
ax2.legend(loc="upper left")
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("../data/09_exhaust_system.png", dpi=150, bbox_inches="tight")
print("Saved: 09_exhaust_system.png")

# %% ========================================================================
# PLOT 10: Turbocharger Performance
# ========================================================================
fig, axes = plt.subplots(2, 2, figsize=(14, 10))

# Turbo speed vs time
ax1 = axes[0, 0]
ax1.plot(df["time"], df["turbo_speed_rpm"] / 1000, linewidth=1.5, color="tab:blue")
ax1.set_xlabel("Time (s)")
ax1.set_ylabel("Turbo Speed (kRPM)")
ax1.set_title("Turbocharger Speed vs Time")
ax1.grid(True, alpha=0.3)

# Turbo speed vs engine RPM
ax2 = axes[0, 1]
ax2.plot(
    df_binned["rpm_bin"],
    df_binned["turbo_speed_rpm"] / 1000,
    linewidth=2,
    color="tab:blue",
)
ax2.set_xlabel("Engine Speed (RPM)")
ax2.set_ylabel("Turbo Speed (kRPM)")
ax2.set_title("Turbocharger Speed vs Engine RPM")
ax2.grid(True, alpha=0.3)

# Boost ratio (PR) vs turbo speed
ax3 = axes[1, 0]
df["boost_ratio"] = df["boost_pressure"] / 101325
ax3.scatter(
    df["turbo_speed_rpm"] / 1000,
    df["boost_ratio"],
    c=df["rpm"],
    cmap="viridis",
    s=2,
    alpha=0.5,
)
cbar = plt.colorbar(ax3.collections[0], ax=ax3)
cbar.set_label("Engine RPM")
ax3.set_xlabel("Turbo Speed (kRPM)")
ax3.set_ylabel("Pressure Ratio")
ax3.set_title("Compressor Operating Map (Colored by Engine RPM)")
ax3.grid(True, alpha=0.3)

# Compressor outlet temperature vs boost
ax4 = axes[1, 1]
ax4.plot(
    df_binned["rpm_bin"],
    df_binned["compressor_outlet_temp"] - 273.15,
    label="Compressor Outlet",
    linewidth=2,
    color="tab:red",
)
ax4.plot(
    df_binned["rpm_bin"],
    df_binned["intake_manifold_temp"] - 273.15,
    label="After Intercooler",
    linewidth=2,
    color="tab:blue",
)
ax4.set_xlabel("Engine Speed (RPM)")
ax4.set_ylabel("Temperature (°C)")
ax4.set_title("Intake Air Temperatures vs RPM")
ax4.legend(loc="upper left")
ax4.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("../data/10_turbo_performance.png", dpi=150, bbox_inches="tight")
print("Saved: 10_turbo_performance.png")

# %% ========================================================================
# PLOT 11: Air and Fuel Flow Analysis
# ========================================================================
fig, axes = plt.subplots(2, 1, figsize=(12, 8))

# Air mass flow comparison
ax1 = axes[0]
ax1.plot(
    df_binned["rpm_bin"],
    df_binned["actual_air_flow"] * 1000,
    label="Actual Air Flow (Engine)",
    linewidth=2,
    color="tab:blue",
)
ax1.plot(
    df_binned["rpm_bin"],
    df_binned["na_air_flow"] * 1000,
    label="Throttle Air Flow (NA)",
    linewidth=2,
    color="tab:green",
)
ax1.plot(
    df_binned["rpm_bin"],
    df_binned["turbo_air_flow"] * 1000,
    label="Turbo Air Flow",
    linewidth=2,
    color="tab:red",
    linestyle="--",
)
ax1.set_xlabel("Engine Speed (RPM)")
ax1.set_ylabel("Air Mass Flow (g/s)")
ax1.set_title("Air Mass Flow Comparison vs RPM")
ax1.legend(loc="upper left")
ax1.grid(True, alpha=0.3)

# Fuel flow and AFR
ax2 = axes[1]
ax2.plot(
    df_binned["rpm_bin"],
    df_binned["fuel_mass_flow"] * 1000 * 3600,
    label="Fuel Flow",
    linewidth=2,
    color="tab:orange",
)
ax2.set_xlabel("Engine Speed (RPM)")
ax2.set_ylabel("Fuel Flow (g/hr)")
ax2.set_title("Fuel Mass Flow vs RPM")
ax2.legend(loc="upper left")
ax2.grid(True, alpha=0.3)

# Add instantaneous AFR on twin axis
ax2b = ax2.twinx()
df_binned["afr"] = df_binned["actual_air_flow"] / df_binned["fuel_mass_flow"]
ax2b.plot(
    df_binned["rpm_bin"],
    df_binned["afr"],
    label="Air-Fuel Ratio",
    linewidth=2,
    color="tab:purple",
    linestyle="--",
)
ax2b.axhline(y=14.7, color="gray", linestyle=":", alpha=0.7, label="Stoich (14.7)")
ax2b.set_ylabel("Air-Fuel Ratio", color="tab:purple")
ax2b.tick_params(axis="y", labelcolor="tab:purple")
ax2b.set_ylim(10, 20)

plt.tight_layout()
plt.savefig("../data/11_airflow_analysis.png", dpi=150, bbox_inches="tight")
print("Saved: 11_airflow_analysis.png")

# %% ========================================================================
# PLOT 12: ERS (Energy Recovery System) Analysis
# ========================================================================
fig, axes = plt.subplots(2, 2, figsize=(14, 10))

# MGU-K torque and power vs time
ax1 = axes[0, 0]
ax1.plot(
    df["time"], df["mguk_torque"], label="MGU-K Torque", linewidth=1.5, color="tab:blue"
)
ax1.set_xlabel("Time (s)")
ax1.set_ylabel("Torque (Nm)")
ax1.set_title("MGU-K Torque vs Time")
ax1.grid(True, alpha=0.3)
ax1.legend()

ax2 = axes[0, 1]
ax2.plot(
    df["time"],
    df["mguk_power"] / 1000,
    label="MGU-K Power",
    linewidth=1.5,
    color="tab:green",
)
ax2.axhline(y=120, color="red", linestyle="--", alpha=0.7, label="120 kW Limit")
ax2.set_xlabel("Time (s)")
ax2.set_ylabel("Power (kW)")
ax2.set_title("MGU-K Power Deployment vs Time")
ax2.grid(True, alpha=0.3)
ax2.legend()

# MGU-H power vs time
ax3 = axes[1, 0]
ax3.plot(
    df["time"],
    df["mguh_power"] / 1000,
    label="MGU-H Power",
    linewidth=1.5,
    color="tab:orange",
)
ax3.set_xlabel("Time (s)")
ax3.set_ylabel("Power (kW)")
ax3.set_title("MGU-H Power vs Time (+ = generating, - = motoring)")
ax3.grid(True, alpha=0.3)
ax3.legend()

# Battery SOC over time
ax4 = axes[1, 1]
ax4.plot(df["time"], df["battery_soc"] * 100, linewidth=1.5, color="tab:red")
ax4.axhline(y=100, color="green", linestyle="--", alpha=0.5, label="Full Charge")
ax4.axhline(y=0, color="red", linestyle="--", alpha=0.5, label="Empty")
ax4.set_xlabel("Time (s)")
ax4.set_ylabel("State of Charge (%)")
ax4.set_title("Battery State of Charge vs Time")
ax4.set_ylim(-5, 105)
ax4.grid(True, alpha=0.3)
ax4.legend()

plt.tight_layout()
plt.savefig("../data/12_ers_analysis.png", dpi=150, bbox_inches="tight")
print("Saved: 12_ers_analysis.png")

# %% ========================================================================
# PLOT 13: Battery Energy Flow
# ========================================================================
fig, ax = plt.subplots(figsize=(12, 6))

ax.plot(df["time"], df["battery_energy"] / 1e6, linewidth=1.5, color="tab:blue")
ax.axhline(y=4.0, color="red", linestyle="--", alpha=0.7, label="Max Energy (4 MJ)")
ax.fill_between(df["time"], 0, df["battery_energy"] / 1e6, alpha=0.3, color="tab:blue")
ax.set_xlabel("Time (s)")
ax.set_ylabel("Battery Energy (MJ)")
ax.set_title("Battery Energy vs Time")
ax.legend()
ax.grid(True, alpha=0.3)
ax.set_ylim(bottom=0)
plt.tight_layout()
plt.savefig("../data/13_battery_energy.png", dpi=150, bbox_inches="tight")
print("Saved: 13_battery_energy.png")

# %% ========================================================================
# PLOT 14: Throttle Response Analysis
# ========================================================================
fig, axes = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

# Throttle and effective throttle
ax1 = axes[0]
ax1.plot(
    df["time"],
    df["throttle"] * 100,
    label="Commanded Throttle",
    linewidth=1.5,
    color="tab:blue",
)
ax1.plot(
    df["time"],
    df["effective_throttle"] * 100,
    label="Effective Throttle (with idle control)",
    linewidth=1.5,
    color="tab:orange",
    linestyle="--",
)
ax1.set_ylabel("Throttle (%)")
ax1.set_title("Throttle Position vs Time")
ax1.legend(loc="upper left")
ax1.grid(True, alpha=0.3)

# RPM response
ax2 = axes[1]
ax2.plot(df["time"], df["rpm"], linewidth=1.5, color="tab:red")
ax2.set_xlabel("Time (s)")
ax2.set_ylabel("Engine RPM")
ax2.set_title("Engine RPM Response to Throttle Input")
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("../data/14_throttle_response.png", dpi=150, bbox_inches="tight")
print("Saved: 14_throttle_response.png")

# %% ========================================================================
# PLOT 15: Comprehensive Dashboard - Single Page Overview
# ========================================================================
fig = plt.figure(figsize=(20, 16))
gs = GridSpec(4, 4, figure=fig, hspace=0.3, wspace=0.3)

# Row 1: Time traces
ax1 = fig.add_subplot(gs[0, :2])
ax1.plot(df["time"], df["rpm"], color="tab:red", linewidth=1)
ax1.set_xlabel("Time (s)")
ax1.set_ylabel("RPM")
ax1.set_title("Engine Speed")
ax1.grid(True, alpha=0.3)

ax2 = fig.add_subplot(gs[0, 2:])
ax2.plot(df["time"], df["total_power"] / 1000, color="tab:blue", linewidth=1)
ax2.set_xlabel("Time (s)")
ax2.set_ylabel("Power (kW)")
ax2.set_title("Total Power Output")
ax2.grid(True, alpha=0.3)

# Row 2: Dyno curves
ax3 = fig.add_subplot(gs[1, :2])
ax3.plot(
    df_binned["rpm_bin"],
    df_binned["torque_output"],
    color="tab:blue",
    linewidth=2,
    label="Torque",
)
ax3.set_xlabel("RPM")
ax3.set_ylabel("Torque (Nm)")
ax3.set_title("Torque Curve")
ax3.grid(True, alpha=0.3)

ax4 = fig.add_subplot(gs[1, 2:])
ax4.plot(
    df_binned["rpm_bin"],
    df_binned["total_power"] / 1000,
    color="tab:red",
    linewidth=2,
    label="Total",
)
ax4.plot(
    df_binned["rpm_bin"],
    df_binned["ice_power"] / 1000,
    color="tab:orange",
    linewidth=1.5,
    linestyle="--",
    label="ICE",
)
ax4.set_xlabel("RPM")
ax4.set_ylabel("Power (kW)")
ax4.set_title("Power Curves")
ax4.legend()
ax4.grid(True, alpha=0.3)

# Row 3: Efficiency and pressures
ax5 = fig.add_subplot(gs[2, 0])
ax5.plot(
    df_binned["rpm_bin"],
    df_binned["thermal_efficiency"] * 100,
    color="tab:green",
    linewidth=2,
)
ax5.set_xlabel("RPM")
ax5.set_ylabel("Efficiency (%)")
ax5.set_title("Thermal Efficiency")
ax5.grid(True, alpha=0.3)

ax6 = fig.add_subplot(gs[2, 1])
ax6.plot(df_binned["rpm_bin"], df_binned["bsfc"], color="tab:purple", linewidth=2)
ax6.set_xlabel("RPM")
ax6.set_ylabel("BSFC (g/kWh)")
ax6.set_title("Fuel Consumption")
ax6.grid(True, alpha=0.3)

ax7 = fig.add_subplot(gs[2, 2])
ax7.plot(
    df_binned["rpm_bin"],
    df_binned["boost_pressure"] / 101325,
    color="tab:red",
    linewidth=2,
)
ax7.set_xlabel("RPM")
ax7.set_ylabel("Boost (bar)")
ax7.set_title("Boost Pressure")
ax7.grid(True, alpha=0.3)

ax8 = fig.add_subplot(gs[2, 3])
ax8.plot(
    df_binned["rpm_bin"],
    df_binned["turbo_speed_rpm"] / 1000,
    color="tab:blue",
    linewidth=2,
)
ax8.set_xlabel("RPM")
ax8.set_ylabel("Turbo Speed (kRPM)")
ax8.set_title("Turbocharger Speed")
ax8.grid(True, alpha=0.3)

# Row 4: ERS
ax9 = fig.add_subplot(gs[3, 0])
ax9.plot(df["time"], df["mguk_power"] / 1000, color="tab:green", linewidth=1)
ax9.axhline(y=120, color="red", linestyle="--", alpha=0.5)
ax9.set_xlabel("Time (s)")
ax9.set_ylabel("MGU-K Power (kW)")
ax9.set_title("MGU-K Deployment")
ax9.grid(True, alpha=0.3)

ax10 = fig.add_subplot(gs[3, 1])
ax10.plot(df["time"], df["mguh_power"] / 1000, color="tab:orange", linewidth=1)
ax10.set_xlabel("Time (s)")
ax10.set_ylabel("MGU-H Power (kW)")
ax10.set_title("MGU-H Power")
ax10.grid(True, alpha=0.3)

ax11 = fig.add_subplot(gs[3, 2])
ax11.plot(df["time"], df["battery_soc"] * 100, color="tab:red", linewidth=1)
ax11.set_xlabel("Time (s)")
ax11.set_ylabel("SOC (%)")
ax11.set_title("Battery State of Charge")
ax11.grid(True, alpha=0.3)

ax12 = fig.add_subplot(gs[3, 3])
ax12.plot(
    df_binned["rpm_bin"], df_binned["bmep"], color="tab:blue", linewidth=2, label="BMEP"
)
ax12.plot(
    df_binned["rpm_bin"],
    df_binned["imep"],
    color="tab:green",
    linewidth=2,
    label="IMEP",
)
ax12.set_xlabel("RPM")
ax12.set_ylabel("MEP (kPa)")
ax12.set_title("Mean Effective Pressure")
ax12.legend(fontsize=8)
ax12.grid(True, alpha=0.3)

plt.suptitle(
    "F1 Power Unit Telemetry Dashboard", fontsize=16, fontweight="bold", y=0.995
)
plt.savefig("../data/15_dashboard.png", dpi=150, bbox_inches="tight")
print("Saved: 15_dashboard.png")

# %% ========================================================================
# PLOT 16: Pressure-Volume Style Analysis (Engine Breathing)
# ========================================================================
fig, axes = plt.subplots(1, 2, figsize=(14, 6))

# Intake vs Exhaust pressure
ax1 = axes[0]
ax1.plot(
    df_binned["rpm_bin"],
    df_binned["intake_manifold_pressure"] / 1000,
    label="Intake Manifold",
    linewidth=2,
    color="tab:blue",
)
ax1.plot(
    df_binned["rpm_bin"],
    df_binned["exhaust_manifold_pressure"] / 1000,
    label="Exhaust Manifold",
    linewidth=2,
    color="tab:red",
)
ax1.axhline(y=101.325, color="gray", linestyle=":", alpha=0.7, label="Atmospheric")
ax1.set_xlabel("Engine Speed (RPM)")
ax1.set_ylabel("Pressure (kPa)")
ax1.set_title("Intake vs Exhaust Manifold Pressure")
ax1.legend()
ax1.grid(True, alpha=0.3)

# Pressure differential (pumping loss indicator)
ax2 = axes[1]
df_binned["pressure_diff"] = (
    df_binned["exhaust_manifold_pressure"] - df_binned["intake_manifold_pressure"]
) / 1000
ax2.plot(
    df_binned["rpm_bin"], df_binned["pressure_diff"], linewidth=2, color="tab:purple"
)
ax2.axhline(y=0, color="gray", linestyle=":", alpha=0.7)
ax2.set_xlabel("Engine Speed (RPM)")
ax2.set_ylabel("Pressure Differential (kPa)")
ax2.set_title("Exhaust-Intake Pressure Differential (Pumping Loss Indicator)")
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("../data/16_pressure_analysis.png", dpi=150, bbox_inches="tight")
print("Saved: 16_pressure_analysis.png")

# %% ========================================================================
# Summary Statistics
# ========================================================================
print("\n" + "=" * 60)
print("F1 POWER UNIT TELEMETRY SUMMARY")
print("=" * 60)
print(f"\nEngine Performance:")
print(f"  Peak RPM: {df['rpm'].max():.0f} rev/min")
print(
    f"  Peak Total Power: {df['total_power'].max() / 1000:.1f} kW ({df['total_power'].max() / 745.7:.0f} hp)"
)
print(
    f"  Peak ICE Power: {df['ice_power'].max() / 1000:.1f} kW ({df['ice_power'].max() / 745.7:.0f} hp)"
)
print(f"  Peak Brake Torque: {df['torque_output'].max():.1f} Nm")
print(f"  Peak BMEP: {df['bmep'].max():.1f} kPa ({df['bmep'].max() / 100:.1f} bar)")

print(f"\nEfficiency Metrics:")
print(f"  Peak Thermal Efficiency: {df['thermal_efficiency'].max() * 100:.1f}%")
print(f"  Peak Mechanical Efficiency: {df['mechanical_efficiency'].max() * 100:.1f}%")
print(f"  Min BSFC: {df[df['bsfc'] > 0]['bsfc'].min():.1f} g/kWh")

print(f"\nTurbocharger:")
print(f"  Peak Boost: {df['boost_pressure'].max() / 101325:.2f} bar")
print(f"  Peak Turbo Speed: {df['turbo_speed_rpm'].max():.0f} RPM")

print(f"\nERS:")
print(f"  Peak MGU-K Power: {df['mguk_power'].max() / 1000:.1f} kW")
print(f"  Final Battery SOC: {df['battery_soc'].iloc[-1] * 100:.1f}%")

print(f"\nAirflow:")
print(f"  Peak Air Mass Flow: {df['actual_air_flow'].max() * 1000:.1f} g/s")
print(f"  Peak Fuel Flow: {df['fuel_mass_flow'].max() * 1000 * 3600:.1f} g/hr")

print("\n" + "=" * 60)
print(f"All plots saved to ../data/")
print("=" * 60)
