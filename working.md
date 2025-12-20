- Internal Combustion Engine
- Turbocharger
- MGU-H
- MGU-K
- Energy Store (Battery)

Good, now we’re doing this properly.

I’ll go **component by component**, and for each one I’ll give you:

* Inputs (with units + where they come from)
* Outputs (and where they go)
* Internal state (what it remembers between steps)
* Exact equations + concepts used

No handwaving.

We’re only talking about these 5:

1. Internal Combustion Engine (ICE)
2. Turbocharger (turbine + compressor + shaft)
3. MGU-H
4. MGU-K
5. Energy Store (Battery)

Assume:

* Working fluid: air/exhaust, ideal gas.
* Time-step simulation: at each `dt`, you update all of them.

---

## 1️⃣ INTERNAL COMBUSTION ENGINE (ICE)

### Role

Converts **fuel chemical energy** + **intake air** into:

* **Crankshaft torque**
* **Hot exhaust gas**

This drives the car and feeds the turbo.

---

### Inputs (at each time step)

1. **Engine speed**:

   * `rpm` [rev/min] or `ω` [rad/s]
   * Comes from drivetrain/vehicle speed & gear (or idle model if car is stationary).

2. **Manifold (intake) pressure**:

   * `p_man` [Pa]
   * Comes from compressor outlet (turbo).

3. **Manifold (intake) temperature**:

   * `T_man` [K]
   * From compressor + intercooler model.

4. **Throttle position**:

   * `throttle` [0–1]
   * Driver input (via ECU).

5. (Optional) **Lambda / AFR**:

   * If you model air–fuel ratio explicitly.

---

### Outputs

1. **Crankshaft torque**:

   * `T_ice` [Nm]
   * Goes to: **Drivetrain** and is later summed with MGU-K torque.

2. **Fuel mass flow**:

   * `m_dot_fuel` [kg/s]
   * Used for fuel consumption + exhaust calculation.

3. **Exhaust gas state**:

   * `p_exh_in`, `T_exh_in`, `m_dot_exh`
   * Goes straight into **turbine (turbocharger)**.

4. **Brake efficiency** (optional):

   * `η_b` [–]
   * Useful for logging / tuning.

---

### Internal state

* You can keep:

  * Previous `rpm`, `T_exh`, etc. for smoothing.
  * Transient stuff if you want (turbo lag, wall temps etc.) but you can ignore for v1.

---

### Core concepts and equations

**Concepts used:**

* Ideal gas law
* Brake Mean Effective Pressure (BMEP)
* Brake power vs torque
* Energy balance for fuel → mechanical + exhaust heat

---

#### Step 1: Air density in manifold

Use **ideal gas law**:

[
\rho_{\text{air}} = \frac{p_{\text{man}}}{R_{\text{air}} T_{\text{man}}}
]

* ( R_{\text{air}} \approx 287\ \text{J/(kg·K)} )

This tells you how “packed” the air is.

---

#### Step 2: Air mass flow into engine

For a 4-stroke engine:

Each cylinder fills every **2 revolutions**.

Approx air mass flow:

[
\dot{m}*{\text{air}} =
\rho*{\text{air}} \cdot V_d \cdot \eta_v \cdot \frac{N}{2}
]

Where:

* ( V_d ) = total displacement [m³]
* ( \eta_v ) = volumetric efficiency (0–1, often a map vs rpm & load)
* ( N ) = engine speed in rev/s = rpm / 60

---

#### Step 3: Fuel flow (stoichiometric or lambda-based)

For stoichiometric ratio:

[
\dot{m}*{\text{fuel,stoich}} = \frac{\dot{m}*{\text{air}}}{AFR_{\text{stoich}}}
]

* For gasoline, ( AFR_{\text{stoich}} \approx 14.7 )

You can multiply by a lambda factor if running richer/leaner.

---

#### Step 4: Brake power and torque

Fuel chemical power:

[
P_{\text{fuel}} = \dot{m}_{\text{fuel}} \cdot LHV
]

* ( LHV ) ≈ 42–44 MJ/kg for gasoline.

Brake power (use engine efficiency map):

[
P_{\text{brake}} = \eta_b \cdot P_{\text{fuel}}
]

Torque:

[
T_{\text{ICE}} = \frac{P_{\text{brake}}}{\omega}
]

* ( \omega = 2\pi \cdot \frac{\text{rpm}}{60} )

Clamp fuel flow to 100 kg/h:

[
\dot{m}_{\text{fuel}} \le 100/3600 \approx 0.02778\ \text{kg/s}
]

If you hit this limit, reduce torque accordingly.

---

#### Step 5: Exhaust mass flow and temperature

Mass:

[
\dot{m}*{\text{exh}} \approx \dot{m}*{\text{air}} + \dot{m}_{\text{fuel}}
]

Temperature: pick a simple map:

* `T_exh_in = f(load, rpm)`

  * low load ~ 700–800 K
  * high load ~ 900–1050 K

This is what you feed into the turbine.

---

## 2️⃣ TURBOCHARGER (Turbine + Compressor + Shaft)

Turbo has:

* **Turbine** (uses exhaust → makes shaft power)
* **Compressor** (uses shaft power → compresses intake air)
* **Shaft** (with inertia, shared by both + MGU-H)

---

### Inputs

1. **Exhaust gas entering turbine**:

   * `T3`, `p3`, `m_dot_exh`
   * From ICE.

2. **Turbine outlet pressure**:

   * `p4` (near atmospheric + backpressure).

3. **Compressor inlet state**:

   * `T1`, `p1`, `m_dot_air`
   * p1 ~ ambient, T1 ~ ambient or post-filter.

4. **MGU-H torque on shaft**:

   * `T_mguh` [Nm] (+assisting, -harvesting).

5. **Previous shaft speed**:

   * `ω_turbo` [rad/s] from last timestep.

---

### Outputs

1. **Compressor outlet state (to manifold)**:

   * `p2` [Pa] (boost pressure)
   * `T2` [K] (charge temperature)
   * `m_dot_air` [kg/s]

2. **Turbine outlet state**:

   * `T4`, `p4` (for completeness).

3. **Updated shaft state**:

   * `ω_turbo` (new turbo speed)
   * Net shaft torque (for logging).

These go to:

* **ICE** (boost pressure & temp).
* **MGU-H** (needs shaft speed).

---

### Internal state

* Shaft speed `ω_turbo`
* Optional: filter states for pressure, etc.

---

### Core concepts and equations

**Concepts:**

* Ideal gas
* Isentropic turbine & compressor
* Turbomachinery efficiencies
* Rotational dynamics: ( P = T\omega, J d\omega/dt = T )

---

#### Turbine side (exhaust → shaft power)

Given `T3, p3, p4, m_dot_exh, η_turb`:

Isentropic outlet temperature:

[
T_{4s} = T_3 \cdot \left(\frac{p_4}{p_3}\right)^{(\gamma - 1)/\gamma}
]

Actual outlet temp:

[
T_4 = T_3 - \eta_t \cdot (T_3 - T_{4s})
]

Turbine power:

[
P_{\text{turb}} = \dot{m}_{\text{exh}} \cdot c_p \cdot (T_3 - T_4)
]

---

#### Compressor side (shaft power → higher pressure)

Desired pressure ratio:

[
\pi_c = \frac{p_2}{p_1}
]

Isentropic outlet temp:

[
T_{2s} = T_1 \cdot \pi_c^{(\gamma - 1)/\gamma}
]

Actual:

[
T_2 = T_1 + \frac{T_{2s} - T_1}{\eta_c}
]

Compressor power:

[
P_{\text{comp}} = \dot{m}_{\text{air}} \cdot c_p \cdot (T_2 - T_1)
]

Where:

* ( \gamma \approx 1.4 ), ( c_p \approx 1005\ \text{J/kg·K} )

In v1, you might invert this: choose `P_turb` and get possible `π_c`. For now, simplest: treat `π_c` as commanded or map-based.

---

#### Shaft dynamics (turbine – compressor – MGU-H)

Total shaft power:

[
P_{\text{shaft,net}} = P_{\text{turb}} - P_{\text{comp}} - P_{\text{MGUH}}
]

Where:

* ( P_{\text{MGUH}} > 0 ) when **harvesting** (MGU-H as generator)
* ( P_{\text{MGUH}} < 0 ) when **assisting** (MGU-H as motor)

Relate power and angular acceleration:

[
P_{\text{shaft,net}} = T_{\text{net}} \cdot \omega_{\text{turbo}}
]
[
T_{\text{net}} = J \frac{d\omega_{\text{turbo}}}{dt}
]

So:

[
\frac{d\omega}{dt} = \frac{P_{\text{shaft,net}}}{J \cdot \omega + \epsilon}
]

Integrate:

[
\omega_{\text{new}} = \omega_{\text{old}} + \frac{P_{\text{shaft,net}}}{J \cdot \omega_{\text{old}} + \epsilon} \cdot dt
]

---

## 3️⃣ MGU-H (On Turbo Shaft)

### Role

* **Motor mode:** uses electrical power to spin turbo (anti-lag, boost control).
* **Generator mode:** harvests shaft power from turbo → converts to electricity.

---

### Inputs

1. **Turbo shaft speed**:

   * `ω_turbo` [rad/s].

2. **Mode**:

   * Motor / Generator / Idle (from ECU).

3. **Power command**:

   * `P_cmd` [W] (desired electrical power).

4. **Efficiency params**:

   * `η_motor`, `η_gen`.

---

### Outputs

1. **Mechanical torque on turbo shaft**:

   * `T_mguh` [Nm]
   * Goes into **turbo shaft balance**.

2. **Electrical power**:

   * `P_elec` [W]
   * Goes to **Battery** or sometimes directly to MGU-K through power electronics.

---

### Core equations & concepts

Concept: **rotating electric machine** with:

[
P = T\omega
]

#### Motor mode (electrical → mechanical)

You want to **add power** to the shaft.

Clamp desired mechanical power:

[
P_{\text{mech}} = \min(|P_{\text{cmd}}|, P_{H,\max})
]

Mechanical torque:

[
T_{\text{H}} = \frac{P_{\text{mech}} \cdot \eta_{\text{motor}}}{\omega_{\text{turbo}}}
]

Electrical power drawn from battery:

[
P_{\text{elec}} = -\frac{P_{\text{mech}}}{\eta_{\text{motor}}}
]

Sign:

* `T_H` > 0 (assist turbine/shaft).
* `P_elec` < 0 (battery → MGU-H).

---

#### Generator mode (mechanical → electrical)

You want to **absorb shaft power**.

Clamp electrical power:

[
P_{\text{elec}} = \min(P_{\text{cmd}}, P_{H,\max})
]

Mechanical power absorbed:

[
P_{\text{mech}} = \frac{P_{\text{elec}}}{\eta_{\text{gen}}}
]

Torque:

[
T_{\text{H}} = -\frac{P_{\text{mech}}}{\omega_{\text{turbo}}}
]

Sign:

* `T_H` < 0 (brakes turbo shaft).
* `P_elec` > 0 (to battery).

---

## 4️⃣ MGU-K (On Crankshaft)

### Role

* **Motor mode:** adds torque to crankshaft (boost).
* **Generator mode:** absorbs torque from crankshaft (regen braking).

---

### Inputs

1. **Crankshaft speed**:

   * `ω_crank` [rad/s].

2. **Mode**:

   * Motor / Generator / Idle (from ECU).

3. **Torque or power command**:

   * Typically torque command: `T_cmd` [Nm].

4. **Gear ratio between MGU-K and crank**:

   * `r_g` so that `ω_K = r_g * ω_crank`.

5. **Efficiencies**:

   * `η_motor`, `η_gen`.

---

### Outputs

1. **Torque at MGU-K shaft**:

   * `T_K` [Nm].

2. **Torque contribution at crank**:

   * `T_K_crank = T_K / r_g` [Nm]
   * This is added to ICE torque.

3. **Electrical power**:

   * `P_elec` [W] to/from battery.

---

### Core equations & concepts

Again: **P = T * ω**. Motor/generator with power limits.

Let:

[
\omega_K = r_g \cdot \omega_{\text{crank}}
]

#### Motor mode (boost)

You want MGU-K to **add torque**.

Desired electrical power:

[
P_{\text{elec,req}} = \frac{T_{\text{cmd}} \cdot \omega_K}{\eta_{\text{motor}}}
]

Clamp by max deploy power:

[
P_{\text{elec}} = \max(-P_{K,\max}, \min(0, P_{\text{elec,req}}))
]

Actual shaft torque:

[
T_K = \frac{P_{\text{elec}} \cdot \eta_{\text{motor}}}{\omega_K}
]

Crank torque contribution:

[
T_{K,\text{crank}} = \frac{T_K}{r_g}
]

Sign:

* `P_elec` < 0 (battery → MGU-K).
* `T_K_crank` > 0 (helps crank).

---

#### Generator mode (regen)

You want MGU-K to **absorb** torque.

Electrical power:

[
P_{\text{elec,req}} = T_{\text{cmd}} \cdot \omega_K \cdot \eta_{\text{gen}}
]

Clamp:

[
P_{\text{elec}} = \min(P_{\text{elec,req}}, P_{K,\max})
]

Torque:

[
T_K = \frac{P_{\text{elec}}}{\omega_K \cdot \eta_{\text{gen}}}
]

Crank torque contribution (now negative):

[
T_{K,\text{crank}} = -\frac{T_K}{r_g}
]

Sign:

* `P_elec` > 0 (to battery).
* `T_K_crank` < 0 (brakes crank).

---

## 5️⃣ ENERGY STORE (Battery)

### Role

Stores electrical energy and enforces:

* SOC limits
* Charge/discharge power limits

---

### Inputs

1. **Electrical power from MGU-K**:

   * `P_K` [W] (+ to battery, - from battery).

2. **Electrical power from MGU-H**:

   * `P_H` [W] (+ to battery, - from battery).

3. (Optionally) other consumers (pump, control, etc., but ignore in v1).

---

### Outputs

1. **State of Charge**:

   * `SOC` [0–1].

2. **Total stored energy**:

   * `E` [J].

3. **Charge/discharge limits**:

   * `P_max_charge`, `P_max_discharge`.

These are sent to **ECU** so it can decide how much ERS to deploy or harvest.

---

### Internal state

* `E` (energy stored)
* `SOC`

---

### Core equations & concepts

**Concept:** energy integration over time:

[
E_{k+1} = E_k + (P_{\text{in,total}} - P_{\text{out,total}}) \cdot dt
]

Where:

* ( P_{\text{in,total}} ) = power **to** ES (from MGU-K, MGU-H).
* ( P_{\text{out,total}} ) = power **from** ES (to MGU-K, MGU-H).

You can separate charge/discharge efficiencies:

[
E_{k+1} =
E_k +
(P_{\text{from K,H if >0}} \cdot \eta_{\text{charge}} -
\frac{|P_{\text{to K,H if <0}}|}{\eta_{\text{discharge}}}) \cdot dt
]

Clamp:

[
E_{k+1} = \text{clamp}(E_{k+1}, E_{\min}, E_{\max})
]

SOC:

[
SOC = \frac{E_{k+1}}{E_{\max}}
]

Where ( E_{\max} = \text{capacity in J} ).

---

## 6️⃣ ORDER OF DATA FLOW (PER TIME STEP)

Here’s the clear sequence – **what feeds what**:

1. **Known at start of step:**

   * `vehicleSpeed`, `gear`, `SOC`, `ω_turbo_prev`, `boost_prev`

2. **Drivetrain → rpm**

   * Compute engine `rpm` from `vehicleSpeed` & `gear` (or idle model).

3. **ECU**

   * Inputs: throttle, brake, rpm, SOC, lap distance
   * Outputs: MGU-K mode + torque cmd, MGU-H mode + power cmd

4. **ICE**

   * Inputs: rpm, throttle, `p_man` & `T_man` (from previous turbo step)
   * Outputs: `T_ice`, `m_dot_exh`, `T_exh`, etc.

5. **MGU-H**

   * Inputs: `ω_turbo_prev`, mode, power cmd
   * Outputs: `T_mguh`, `P_H`

6. **Turbocharger**

   * Inputs: exhaust from ICE + `T_mguh`
   * Outputs: new `ω_turbo`, new `p_man`, `T_man`

7. **MGU-K**

   * Inputs: rpm, mode, torque cmd
   * Outputs: `T_K_crank`, `P_K`

8. **Energy Store**

   * Inputs: `P_K`, `P_H`
   * Outputs: new `SOC` (for next step’s ECU)

9. **Crankshaft total torque**

   * `T_crank = T_ice + T_K_crank`

10. **Drivetrain + Vehicle**

    * Convert `T_crank` to wheel torque → acceleration → new `vehicleSpeed`

11. **Repeat** with updated states.

---

If you want next, I can:

* Turn this into a **printable one-page flow diagram**
* Or give you a **step-by-step checklist** to implement and test each component in order

You now have the full, deep, **non-vague** description of what each unit does, what goes in, what comes out, and what equations sit inside.
