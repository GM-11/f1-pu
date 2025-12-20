# %%
import matplotlib.pyplot as plt
import pandas as pd

# %%

df = pd.read_csv("../data/engine_log.csv")
df.head()
# %%

plt.figure()
plt.plot(df["time"], df["rpm"])
plt.xlabel("Time (s)")
plt.ylabel("RPM")
plt.title("Engine RPM vs Time")
# plt.show()
# save this plot as png
plt.savefig("../data/rpm_vs_time.png")
# %%
df_sorted = df.sort_values("rpm")

fig, ax1 = plt.subplots()

# Plot Torque on Left Axis
ax1.set_xlabel("RPM")
ax1.set_ylabel("Torque (Nm)", color="tab:blue")
ax1.plot(df_sorted["rpm"], df_sorted["torque"], color="tab:blue", label="Torque")
ax1.tick_params(axis="y", labelcolor="tab:blue")
ax1.grid(True, alpha=0.3)

# Plot Power on Right Axis
ax2 = ax1.twinx()
ax2.set_ylabel("Power (W)", color="tab:orange")
ax2.plot(df_sorted["rpm"], df_sorted["power"], color="tab:orange", label="Power")
ax2.tick_params(axis="y", labelcolor="tab:orange")

plt.title("Torque and Power Curve")
fig.tight_layout()
plt.savefig("../data/torque_curve.png")
