# /// script
# requires-python = ">=3.10"
# dependencies = ["numpy", "matplotlib"]
# ///
"""Reproducible teaching calculations and plots; no SPICE or hardware evidence.

Run: uv run charging_math.py
All dock values are illustrative assumptions, not approved product specifications.
"""

import json
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


HERE = Path(__file__).resolve().parent
ASSETS = HERE / "assets"
ASSETS.mkdir(exist_ok=True)


def resistor_study(source_v, battery_v, resistance_ohm):
    current_a = max(0.0, (source_v - battery_v) / resistance_ohm)
    return {
        "battery_v": battery_v,
        "current_a": current_a,
        "resistor_loss_w": current_a**2 * resistance_ohm,
        "battery_terminal_power_w": battery_v * current_a,
    }


def charge_budget(source_v, allowed_a, clock_w, eta_system, eta_charge,
                  overhead_w, battery_v, configured_a):
    required_before_charging_w = clock_w / eta_system + overhead_w
    input_headroom_w = source_v * allowed_a - required_before_charging_w
    budget_a = max(0.0, eta_charge * input_headroom_w / battery_v)
    return {
        "source_v": source_v,
        "allowed_input_a": allowed_a,
        "available_input_w": source_v * allowed_a,
        "input_headroom_w": input_headroom_w,
        "charge_budget_before_pack_limits_a": budget_a,
        "charge_a_after_example_set_limit": min(configured_a, budget_a),
        "source_cannot_sustain_assumed_clock": input_headroom_w < 0,
    }


# Recompute the supplied university exercise from its ideal assumptions.
u_rms, frequency, emf, average_target = 220.0, 50.0, 120.0, 40.0
amplitude = math.sqrt(2) * u_rms
omega = 2 * math.pi * frequency
alpha = math.asin(emf / amplitude)
resistance = (2 * amplitude * math.cos(alpha)
              - emf * (math.pi - 2 * alpha)) / (math.pi * average_target)
times = (np.arange(200_000) + 0.5) / (200_000 * frequency)
rectified_v = np.abs(amplitude * np.sin(omega * times))
current = np.maximum(0.0, (rectified_v - emf) / resistance)
average_a = float(current.mean())
rms_a = float(np.sqrt(np.mean(current**2)))
peak_a = (amplitude - emf) / resistance
resistor_w = resistance * rms_a**2
battery_w = emf * average_a
original = {
    "resistance_ohm": resistance,
    "conduction_start_ms": alpha / omega * 1000,
    "average_a": average_a,
    "rms_a": rms_a,
    "peak_a": peak_a,
    "resistor_loss_w": resistor_w,
    "ideal_emf_power_w": battery_w,
    "useful_emf_efficiency": battery_w / (battery_w + resistor_w),
}
assert math.isclose(average_a, 40.0, abs_tol=1e-6)
assert math.isclose(rms_a, 51.031919342, abs_tol=1e-6)
assert math.isclose(resistance, 2.3248353048152, abs_tol=1e-10)

dc_rows = [resistor_study(5.0, v, 1.6) for v in (3.0, 3.7, 4.2)]
for row, expected_a in zip(dc_rows, (1.25, 0.8125, 0.5)):
    assert math.isclose(row["current_a"], expected_a, abs_tol=1e-12)

# Hypothetical 5 V source allowance; this does not prescribe USB default current.
budget_rows = [charge_budget(5.0, allowance, 2.0, 0.9, 0.9, 0.1, 4.2, 0.5)
               for allowance in (0.5, 1.5)]
assert math.isclose(budget_rows[0]["charge_budget_before_pack_limits_a"],
                    0.038095238095, abs_tol=1e-10)
assert math.isclose(budget_rows[1]["charge_budget_before_pack_limits_a"],
                    1.109523809524, abs_tol=1e-10)
required_input_w = 2.0 / 0.9 + 4.2 * 0.5 / 0.9 + 0.1
delivered_energy_wh = 3.7 * 5.0 * 0.8 * 0.85
results = {
    "evidence_type": "Analytical models and numerical integration only",
    "limitations": "No circuit simulation, SPICE run, pack selection or hardware test",
    "original_ideal_bridge": original,
    "dc_resistor_counterexample": dc_rows,
    "power_budget_assumptions": {
        "clock_w": 2.0, "system_efficiency": 0.9,
        "charge_efficiency": 0.9, "overhead_w": 0.1,
        "battery_terminal_v": 4.2, "example_charge_set_a": 0.5,
    },
    "power_budgets": budget_rows,
    "input_required_for_example_full_charge_w": required_input_w,
    "battery_energy_example": {
        "nominal_wh": 3.7 * 5.0,
        "delivered_wh_before_extra_overhead": delivered_energy_wh,
        "runtime_at_2w_h_before_extra_overhead": delivered_energy_wh / 2.0,
    },
    "contact_example": {"loop_ohm": 0.1, "current_a": 2.0,
                        "drop_v": 0.2, "loss_w": 0.4},
}
(HERE / "charging-math-results.json").write_text(
    json.dumps(results, indent=2) + "\n", encoding="utf-8")

plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 11,
                     "axes.spines.top": False, "axes.spines.right": False,
                     "axes.grid": True, "grid.alpha": 0.2})

fig, axes = plt.subplots(2, 1, figsize=(11, 7), sharex=True,
                         constrained_layout=True)
fig.suptitle("Starting model: the university battery-charger exercise", fontsize=16)
ms = times * 1000
axes[0].plot(ms, rectified_v, color="#1873ac", label="Ideal rectified source")
axes[0].axhline(emf, color="#aa4e00", linestyle="--", label="Fixed battery EMF: 120 V")
axes[0].fill_between(ms, emf, rectified_v, where=rectified_v > emf,
                     color="#1873ac", alpha=0.12, label="Conduction interval")
axes[0].set_ylabel("Voltage (V)")
axes[0].legend(loc="upper right", fontsize=10)
axes[1].plot(ms, current, color="#1873ac", label="Instantaneous charge current")
axes[1].axhline(average_a, color="#aa4e00", linestyle="--", label="Average: 40.00 A")
axes[1].axhline(rms_a, color="#6b4c9a", linestyle=":", label="RMS: 51.03 A")
axes[1].annotate(f"Peak: {peak_a:.2f} A", xy=(5, peak_a), xytext=(7.2, 78),
                 arrowprops={"arrowstyle": "->", "color": "#333333"})
axes[1].set(xlabel="Time (ms)", ylabel="Current (A)", xlim=(0, 20), ylim=(0, 93))
axes[1].legend(loc="lower right", fontsize=9)
fig.supxlabel("Ideal teaching model only: average transfers charge; RMS sets resistive heating.",
              fontsize=10)
fig.savefig(ASSETS / "01-university-waveforms.png", dpi=160)
plt.close(fig)

fig, axes = plt.subplots(1, 2, figsize=(13, 5.7), constrained_layout=True)
fig.suptitle("Extend the model toward the dock: current control and power allocation", fontsize=15)
battery_v = np.linspace(3.0, 4.2, 240)
axes[0].plot(battery_v, (5.0 - battery_v) / 1.6, color="#1873ac", linewidth=2,
             label="5 V source + 1.6 ohm resistor")
axes[0].axhline(0.5, color="#aa4e00", linestyle="--", label="Example CC target: 0.5 A")
axes[0].scatter([3.0, 3.7, 4.2], [1.25, 0.8125, 0.5], color="#1873ac", zorder=3)
axes[0].set(title="A resistor does not regulate charge current",
             xlabel="Assumed battery terminal voltage (V)", ylabel="Current (A)",
             xlim=(2.95, 4.25), ylim=(0, 1.4))
axes[0].text(0.04, 0.06, "At 4.2 V it still passes 0.5 A.\nNo voltage regulation or charge termination.",
             transform=axes[0].transAxes, fontsize=10)
axes[0].legend(loc="upper right", fontsize=9)

allowance_a = np.linspace(0, 1.5, 400)
minimum_source_a = (2.0 / 0.9 + 0.1) / 5.0
actual_a = [charge_budget(5.0, a, 2.0, 0.9, 0.9, 0.1, 4.2, 0.5)
            ["charge_a_after_example_set_limit"] for a in allowance_a]
axes[1].axvspan(0, minimum_source_a, color="#a33c32", alpha=0.1,
                label="Source cannot sustain the assumed clock")
axes[1].plot(allowance_a, actual_a, color="#1873ac", linewidth=2,
             label="Charge allocation, capped at 0.5 A")
restricted = budget_rows[0]["charge_a_after_example_set_limit"]
axes[1].scatter([0.5], [restricted], color="#aa4e00", zorder=3)
axes[1].annotate("0.5 A input allowance\nleaves about 38 mA charging",
                 xy=(0.5, restricted), xytext=(0.68, 0.15), fontsize=10,
                 arrowprops={"arrowstyle": "->", "color": "#333333"})
axes[1].set(title="The clock consumes part of the source budget",
             xlabel="Qualified input-current allowance at assumed 5 V (A)",
             ylabel="Battery charging current (A)", xlim=(0, 1.5), ylim=(0, 0.65))
axes[1].legend(loc="upper left", fontsize=9)
fig.supxlabel("Illustrative assumptions: clock 2 W; battery 4.2 V; both path efficiencies 90%; overhead 0.1 W.\n"
              "Models do not include a full CC/CV battery response, thermal limiting or hardware validation.",
              fontsize=10)
fig.savefig(ASSETS / "02-dock-current-and-budget.png", dpi=160)
plt.close(fig)
print(json.dumps({"checks": "passed", "original": original,
                  "dc_rows": dc_rows, "budgets": budget_rows,
                  "required_input_w": required_input_w,
                  "plots": ["01-university-waveforms.png", "02-dock-current-and-budget.png"]},
                 indent=2))
