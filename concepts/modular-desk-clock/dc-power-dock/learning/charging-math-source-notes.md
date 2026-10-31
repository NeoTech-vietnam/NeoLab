# Charging mathematics — primary-source bridge to the dock

Date: 2026-10-30. Status: research and analytical learning notes. No charger, battery, component or numerical rating is selected. No circuit simulation, real-pack test or hardware acceptance is claimed.

These notes support the learning progression from the university bridge-rectifier exercise to the proposed DC-input Neo Modular Desk Clock dock. The existing exercise assumes ideal diodes and a fixed 120 V battery EMF; the dock concept receives external DC, has a changing battery state, and must also supply a system load.

## 1. Extend the model in controlled steps

Start with the original exercise to establish waveform, average current, RMS current, peak current and an explicitly defined power balance. Then change the input to a DC source, vary battery voltage, add controlled charging, and finally add system load and source limits. State each model's assumptions and compare its predictions with measurements before using it for a design choice.

The original resistor-limited charging law becomes, for a DC study model:

`I = max(0, (V_in - V_battery) / R)`

This equation assumes a one-way path, adequate source voltage/current and an otherwise ideal circuit. The diode/drop, wiring resistance, real cell behavior and reverse-current control require later additions. A low-voltage study model does not make the original mains-powered topology a dock implementation.

## 2. A resistor counterexample motivates a controlled charger

TI's power-path article explains that crude resistor limiting allows more current at lower battery voltage and lacks controlled conditioning. It also describes separate input and battery control paths, source current management and system-load prioritization. [TI — Designing a linear Li-Ion battery charger with power-path control](https://www.ti.com/lit/an/slyt333/slyt333.pdf)

Own calculation, using **illustrative assumptions only**: `V_in=5 V` and `R=(5-4.2)/0.5=1.6 Ω`.

| Assumed battery terminal voltage | Resistor-model current | Resistor heat, `I²R` |
|---|---:|---:|
| 3.0 V | 1.2500 A | 2.5000 W |
| 3.7 V | 0.8125 A | 1.05625 W |
| 4.2 V | 0.5000 A | 0.4000 W |

The engineering finding is that one resistor does not maintain the desired charge current as battery voltage changes. At 4.2 V this model still pushes 0.5 A; it does not regulate battery voltage or terminate charging. These voltages are study points, not an approved pack's permissible operating limits. The model is for calculation and simulation, not a Li-ion charging circuit to build.

## 3. Controlled charging introduces new requirements

TI's BQ2407x datasheet shows conditioning/precharge, constant-current charging, voltage regulation with tapering current, and termination behavior. Exact thresholds, allowed current and voltage depend on the chosen cell/pack and charger. The example family includes input-current management, battery sensing and power-path functions; it is a reference, not a selected BOM. [TI BQ24074 datasheet, §9.3.5 and charge-cycle figure](https://www.ti.com/lit/gpn/BQ24074)

For the dock, the charging model therefore needs a state machine as well as equations: absent/invalid pack, conditioning, CC, CV, terminated, inhibited/fault and possible recharge. The requested charge current is constrained by pack allowance, source power, thermal limits and the charging state. Reaching a voltage study point is not equivalent to having a full battery.

The ideal relation `t_hours = ΔQ_Ah / I_A` applies to a constant **net battery charging current** over the stated charge interval. More generally, `ΔQ = ∫ I_battery(t) dt`. With `0 ≤ I_battery(t) ≤ I_max`, a fixed required charge implies `t ≥ ΔQ/I_max`. In a real charge cycle, precharge, CV taper, source limitations, load sharing and losses change the profile, so `capacity/commanded-current` is not a complete empty-to-full-time prediction. [TI — Battery Charging, Li-ion charging discussion](https://www.ti.com/lit/an/snva557/snva557.pdf)

## 4. Linear and switching conversion need different current reasoning

For an idealized **linear charging path** with negligible control current:

`I_input ≈ I_charge`

`P_loss ≈ (V_in - V_battery) × I_charge`

`η_charge_path ≈ V_battery / V_in`

For a **switching charging path**, use the power boundary:

`V_battery × I_charge ≈ η_charge_path × V_in × I_input_charge_branch`

Input current and battery current are therefore generally unequal. The Analog Devices article explains the voltage-ratio loss in linear charging and the power-limited behavior of switching PowerPath architectures. It also discusses prioritizing system load and reducing charging when input power is exhausted. Its legacy USB current examples must not be adopted as the current dock's source-support specification. [Analog Devices — Switching PowerPath and charger heat](https://www.analog.com/en/resources/technical-articles/speed-up-li-ion-battery-charging-and-reduce-heat.html)

For example, a controlled linear 0.5 A charge at 5 V input and 3.0 V battery loses about 1 W in its pass path. That differs from the uncontrolled 1.6 Ω study model's 2.5 W at the same battery voltage, because their currents differ. Avoid comparing losses while silently changing current.

## 5. Add the running clock using power conservation

The BQ24074 power-path description supplies the system while monitoring battery charging separately; its DPPM reduces charging when the input limit constrains the system. A separately powered system node also supports defined operation with a missing/depleted battery. [TI BQ24074 product information](https://www.ti.com/product/BQ24074)

For a general two-branch **analytical budget**, define both efficiencies from the same dock input boundary:

`P_input_required ≈ P_clock/η_system + (V_battery × I_charge)/η_charger + P_dock`

`I_charge_power_limit = max(0, (V_input_at_dock × I_input_allowed - P_clock/η_system - P_dock) × η_charger / V_battery)`

`I_charge_actual ≤ min(I_requested, I_pack_limit, I_charge_power_limit, I_thermal_limit)`

These are aggregate steady-state estimates. Actual control loop dynamics, shared converter paths, tolerances and low-power operating modes require the chosen topology and measurements. Never add output-rail amperes directly to cell amperes when the voltages differ. A shared loss must not be counted twice in both efficiency terms.

Own numerical check, **illustrative assumptions only**:

- Clock load 2 W; system efficiency 90%.
- Battery study point 4.2 V; requested charge current 0.5 A; charge-path efficiency 90%.
- Dock overhead 0.10 W; input at the dock 5.0 V.

Requested source power is `2/0.9 + (4.2×0.5)/0.9 + 0.10 = 4.65556 W`.

| Assumed permitted input current | Input power | Power-derived maximum battery charge current | With an illustrative 0.5 A charge cap |
|---|---:|---:|---:|
| 1.5 A | 7.50 W | 1.10952 A | 0.50000 A |
| 0.5 A | 2.50 W | 0.03810 A | 0.03810 A |

The 0.5 A input row represents a **qualified hypothetical source allowance**, not a claim that every default USB source permits that current. The tiny remaining charging margin can disappear with cable loss, lower efficiency, higher load or tolerances. If the system alone exceeds the source budget, this equation returns zero charging but does not guarantee continued system operation; battery supplement, reduced load or refusal/shutdown must be designed and tested.

## 6. Move from fixed EMF to a useful battery model

A fixed EMF is useful for the first circuit-law exercise. For SOC/runtime work, use charge integration and then characterize voltage and impedance versus SOC, temperature and age. A simple engineering extension is an SOC-dependent open-circuit source with series resistance; RC polarization branches are a later refinement. Equivalent-circuit models use these elements to reproduce observed behavior, not to assert that a whole battery is a single constant capacitor. [Analog Devices — SOC/SOH estimation and equivalent-circuit model](https://www.analog.com/en/resources/technical-articles/a-closer-look-at-state-of-charge-and-state-health-estimation-tech.html)

For energy sizing, distinguish `Q_Ah=∫I dt` from `E_Wh=∫V I dt`. Nominal-voltage times rated Ah is a first label-based energy estimate, not a measured usable energy curve. Runtime should use delivered energy after losses, cutoff/reserve and the actual clock profile.

## 7. Map the learning to feature decisions and evidence

| Feature | Mathematical contribution | Decision/evidence to seek |
|---|---|---|
| 01 — USB-C input | Source power, current allowance, cable `I²R` loss and voltage drop | Required input profile, enforced current limit and measured VBUS/current behavior |
| 02 — Charge while running | CC/CV state model and input/system/battery power balance | Charging allocation, termination and weak-source response measured separately |
| 03 — Replaceable battery | Charge/energy integration, low-cell current and usable-energy derating | Pack size/rating and measured runtime; safe service is a separate mechanical requirement |
| 04 — Temperature sensing | Dissipation estimate and thermal response | Sensor/protection policy from pack limits; verify real temperatures rather than equating watts directly to degrees |
| 05 — Protected output | Peak/RMS currents, transient energy, contact drop/loss | Current limits, fault coordination and component stress measurements |
| 07 — Clear status | State classification and sign of net battery current | Distinguish source present, charging inhibited, charging, supplement/discharging and full |

The exercise can become a practical first engineering analysis when its calculations answer measurable requirements and lead to experiments. It does not replace the earlier user-need definition or establish a product design from mathematics alone.

## 8. Verification status

Executed only a fresh arithmetic check of the resistor table and aggregate power-budget examples with Python on 2026-10-30. No electrochemical model calibration, QSPICE simulation, component evaluation or assembled-dock test was performed for these notes.
