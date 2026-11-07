"""NeoCam: averaged LC plant sensitivity to an IMPOSED mux-input profile.

This is NOT an SGM6029 controller model, macromodel, timing guarantee or test.
The duty law is an invented delayed feedforward policy, explicitly exposed below.
The input-voltage profile is imposed; input bank current and the buck are NOT
solved as a coupled system. Run from any cwd with standard-library Python.

The LC equations are integrated exactly for each zero-order-held average drive
u = D * VIN, sampled at 2 ns. A selected adverse case is repeated at 1 ns.
"""

from pathlib import Path
import csv
import json
import math
import time


BASE = Path(__file__).resolve().parents[1]
OUT = BASE / "design-output"
L = .47e-6
C = 8e-6
R = .500
LOAD = .600
DT = 2e-9
DELAY_US = (0., .25, 1., 2., 5.)
TARGETS = (3.234, 3.300, 3.366)
USB_V = 5.000  # A nominal profile, NOT an exhaustive 5.25 V corner analysis.
BAT_STEADY = 3.700
C_INPUT = 750e-6
COAST_I = 1.000
REVERSE_I = 2.000
REVERSE_TIME = 10e-6
HANDOFF_TIME = 50e-6
INPUT_JUMP = .065  # Allowed input-node ESR/ringing excursion; not output ESR.
RISE_RATE = 200.  # V/s qualification ASSUMPTION, not guaranteed in all mux modes.
SETTLE = 50e-6

COAST_RATE = COAST_I / C_INPUT
FALL_START = (USB_V - BAT_STEADY) / COAST_RATE
FALL_MID = FALL_START + REVERSE_TIME
FALL_END = FALL_START + HANDOFF_TIME
FLOOR = (BAT_STEADY - INPUT_JUMP
         - ((COAST_I * HANDOFF_TIME) + REVERSE_I * REVERSE_TIME) / C_INPUT)
RECOVERY_AFTER_JUMP = FLOOR + INPUT_JUMP
FALL_RECOVERY_END = FALL_END + (BAT_STEADY - RECOVERY_AFTER_JUMP) / RISE_RATE
RISE_END = (USB_V - BAT_STEADY - INPUT_JUMP) / RISE_RATE


def vin_fall(t):
    """Ordinary unplug envelope; reverse charge is exactly 20 uC."""
    if t < 0:
        return USB_V
    if t < FALL_START:
        return USB_V - COAST_RATE * t
    if t < FALL_MID:
        return (BAT_STEADY - INPUT_JUMP
                - (COAST_I + REVERSE_I) / C_INPUT * (t - FALL_START))
    if t < FALL_END:
        return (BAT_STEADY - INPUT_JUMP
                - (COAST_I + REVERSE_I) / C_INPUT * REVERSE_TIME
                - COAST_RATE * (t - FALL_MID))
    if t < FALL_RECOVERY_END:
        return RECOVERY_AFTER_JUMP + RISE_RATE * (t - FALL_END)
    return BAT_STEADY


def vin_rise(t):
    """Assumed plug-in: +65 mV upfront, then <=200 V/s to nominal 5 V."""
    if t < 0:
        return BAT_STEADY
    if t < RISE_END:
        return BAT_STEADY + INPUT_JUMP + RISE_RATE * t
    return USB_V


def transition(dt):
    # Exact homogeneous state transition for [i - LOAD, v - vsteady].
    alpha = R / (2 * L)
    discr = alpha * alpha - 1 / (L * C)
    if discr <= 0:
        raise ValueError("This chosen plant uses the overdamped expression only")
    beta = math.sqrt(discr)
    er = math.exp(-alpha * dt)
    co = math.cosh(beta * dt)
    si_over_beta = math.sinh(beta * dt) / beta
    return (er * (co - alpha * si_over_beta),
            -er * si_over_beta / L,
            er * si_over_beta / C,
            er * (co + alpha * si_over_beta))


def run(profile_name, vtarget, delay_us, dt=DT):
    if profile_name == "USB_to_battery":
        profile = vin_fall
        duration = FALL_RECOVERY_END + SETTLE
        handoff_start, handoff_end = FALL_START, FALL_END
    else:
        profile = vin_rise
        duration = RISE_END + SETTLE
        handoff_start, handoff_end = 0., RISE_END
    delay = delay_us * 1e-6
    p11, p12, p21, p22 = transition(dt)
    v = vtarget
    i = LOAD
    vmin = vmax = v
    tmin = tmax = 0.
    imin = imax = i
    hand_min = hand_max = v
    saturated_time = 0.
    max_duty = 0.
    count = math.ceil(duration / dt)
    for n in range(count):
        tm = (n + .5) * dt
        vin = profile(tm)
        vin_delayed = profile(tm - delay)
        # Exposed hypothetical policy. There is NO voltage feedback here.
        duty = min(1., max(0., (vtarget + R * LOAD) / vin_delayed))
        max_duty = max(max_duty, duty)
        if duty >= 1.:
            saturated_time += dt
        vsteady = duty * vin - R * LOAD
        wi, xv = i - LOAD, v - vsteady
        i = LOAD + p11 * wi + p12 * xv
        v = vsteady + p21 * wi + p22 * xv
        t = (n + 1) * dt
        if v < vmin:
            vmin, tmin = v, t
        if v > vmax:
            vmax, tmax = v, t
        imin, imax = min(imin, i), max(imax, i)
        if handoff_start <= t <= handoff_end:
            hand_min, hand_max = min(hand_min, v), max(hand_max, v)
    return {
        "profile": profile_name, "Vtarget_V": vtarget,
        "feedforward_delay_us_ASSUMED_not_controller_spec": delay_us,
        "integration_dt_ns": dt * 1e9, "steps": count,
        "Vout_min_V_averaged": vmin, "Vout_max_V_averaged": vmax,
        "time_min_relative_handoff_us": (tmin - handoff_start) * 1e6,
        "time_max_relative_handoff_us": (tmax - handoff_start) * 1e6,
        "Vout_min_during_handoff_window_V": hand_min,
        "Vout_max_during_handoff_window_V": hand_max,
        "I_L_min_A_averaged": imin, "I_L_max_A_averaged": imax,
        "max_duty": max_duty,
        "duration_duty_saturated_us": saturated_time * 1e6,
        "peak_net_Q_depletion_from_initial_uC": max(0., C * (vtarget - vmin) * 1e6),
        "peak_net_Q_addition_from_initial_uC": max(0., C * (vmax - vtarget) * 1e6),
        "Q_equivalent_below_required_loaded_baseline3V234_uC": max(0., C * (3.234 - vmin) * 1e6),
        "Q_equivalent_above_required_loaded_baseline3V366_uC": max(0., C * (vmax - 3.366) * 1e6),
        "Vpin_floor_after_other90mV_budget_V_conditional": vmin - .090,
        "Vpin_ceiling_after_other60mV_budget_V_conditional": vmax + .060,
        "Qminus_target_0V25uC_met_in_this_model": C * max(0., vtarget - vmin) <= .25e-6,
        "Qplus_target_0V40uC_met_in_this_model": C * max(0., vmax - vtarget) <= .40e-6,
        "classification": "hypothetical_delayed_feedforward_LC_plant_not_SGM6029_proof",
    }


def main():
    started = time.monotonic()
    rows = []
    for profile in ("USB_to_battery", "battery_to_USB"):
        for target in TARGETS:
            for delay in DELAY_US:
                row = run(profile, target, delay)
                rows.append(row)
                print(json.dumps(row), flush=True)
    # Repeat selected adverse falling profile with half the time step.
    coarse = next(r for r in rows if r["profile"] == "USB_to_battery"
                  and r["Vtarget_V"] == 3.3
                  and r["feedforward_delay_us_ASSUMED_not_controller_spec"] == 5.)
    fine = run("USB_to_battery", 3.3, 5., dt=1e-9)
    convergence = {
        "selected_case": "USB_to_battery, Vtarget3.3, hypothetical_delay5us",
        "coarse_dt_ns": 2., "fine_dt_ns": 1.,
        "Vout_min_change_V": fine["Vout_min_V_averaged"] - coarse["Vout_min_V_averaged"],
        "Vout_max_change_V": fine["Vout_max_V_averaged"] - coarse["Vout_max_V_averaged"],
        "fine_result": fine,
        "meaning": "numerical_resolution_check_not_physical_model_validation",
    }
    result = {
        "revision": "v0.6 averaged plant sensitivity",
        "classification": "engineering_sensitivity_not_controller_simulation_or_hardware_evidence",
        "plant": {"L_H": L, "Cout_effective_F_ASSUMED": C,
                  "R_total_ohm_ASSUMED": R, "Iload_A_constant": LOAD,
                  "equations": ["L di/dt = D*VIN - R*i - VOUT", "C dV/dt = i - Iload"],
                  "duty_policy": "clip((Vtarget+R*Iload)/VIN(t-delay),0,1)",
                  "initial_condition": "I_L=Iload, VOUT=Vtarget, corresponding steady duty"},
        "imposed_input": {"C_input_effective_F": C_INPUT,
            "USB_nominal_V_not_5V25_corner": USB_V,
            "battery_steady_input_V": BAT_STEADY,
            "coast_load_current_A_reserve": COAST_I,
            "reverse_current_A_assumed_rectangle": REVERSE_I,
            "reverse_duration_s": REVERSE_TIME,
            "reverse_charge_C": REVERSE_I * REVERSE_TIME,
            "input_ESR_and_ringing_jump_V": INPUT_JUMP,
            "first10us_falling_slope_V_per_us": -(COAST_I + REVERSE_I) / C_INPUT * 1e-6,
            "next40us_falling_slope_V_per_us": -COAST_I / C_INPUT * 1e-6,
            "handoff_input_charge_loss_C": COAST_I * HANDOFF_TIME + REVERSE_I * REVERSE_TIME,
            "VIN_floor_V": FLOOR,
            "fall_recovery": "+65mV jump then imposed +200V/s to3.7V; not mux-guaranteed waveform",
            "rise": "+65mV jump then imposed +200V/s to5V; not a guarantee for every SS/hotplug mode",
            "rise_rate_V_per_s_ASSUMED": RISE_RATE},
        "results": rows, "convergence": convergence,
        "limitations": [
            "No SGM6029 control law, feedback, current limit, switching, dropout transition, PFM, or recovery maximum is modeled.",
            "Duty delay values are imposed sensitivity parameters, not measured or manufacturer controller timing.",
            "Input waveform is imposed; capacitor input-current feedback and mux dynamics are not solved.",
            "R,L,C are assumed fixed; actual hot resistance, bias, total distributed Ceff and stability remain open.",
            "Peak net charge C*deltaV is not the cumulative integral of every negative/positive oscillatory current interval.",
            "Q target flags use each case's INITIAL Vtarget. A3.3V case can exceed this relative-depletion target while remaining above the separate3.234V baseline floor; inspect actual voltages and baseline-relative columns.",
            "Output ESR, ESL, switching ripple and branch/ferrite pin transfer are excluded from the LC waveform; their budgets remain separate.",
            "Conditional pin bounds merely subtract90mV/add60mV output budgets; they are not a solution for actual local rail or memory voltage.",
            "Constant load0.6A is not a proof against simultaneous additional load steps.",
            "Nominal5V profile does not exhaust 5.25V USB corner, fault, cold-start, missing-cell or cable-hard-short behavior.",
            "VDD_SPI and SD local voltages are not modeled; passing main-rail numbers does not establish memory functionality.",
        ],
        "elapsed_s": time.monotonic() - started,
    }
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "buck-plant-sensitivity.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
    with (OUT / "buck-plant-sensitivity.csv").open("w", newline="", encoding="utf-8-sig") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(json.dumps({"saved_cases": len(rows), "convergence": convergence,
                      "elapsed_s": result["elapsed_s"]}), flush=True)


if __name__ == "__main__":
    main()
