"""NeoCam v0.5: reproduce USB/battery handoff design requirements.

No measured performance or undocumented TI timing maximum is asserted.
Run from NeoLab: python concepts/neocam/tools/calculate-source-switchover.py
"""
from itertools import product
from pathlib import Path
import csv
import json
import math

BASE = Path(__file__).resolve().parents[1]
OUT = BASE / "design-output"
OUT.mkdir(exist_ok=True)


def save_csv(name, rows):
    with (OUT / name).open("w", encoding="utf-8-sig", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


vin_floor = 3.5                 # Design criterion, not a measured buck dropout.
bat_input_min = 3.8             # TRUE voltage at MUX IN2 / MAX17048 VDD.
normal_sys_a = .9               # Normal load contract; not a workload measurement.
hold_load_a = 1.0               # Conservative forward load for handoff calculation.
mux_r_budget = .100             # Qualification at actual voltage/current/temperature.
external_r_budget = .100
gap_s = 50e-6                  # Qualification bound, NOT TI tFSW maximum.
reverse_q_c = 20e-6             # Qualification bound, NOT TI reverse-charge maximum.
cap_peak_a = 3.0                # Total capacitor current incl. reverse-current pulse.
cap_count = 3
cap_each_f = 330e-6
cap_nom_f = cap_count * cap_each_f
cap_min_initial_f = cap_nom_f * .8
cap_max_initial_f = cap_nom_f * 1.2
cap_eff_min_f = 750e-6          # Additional temperature/age/layout condition required.
esr_budget = .015               # Effective bank/connection ESR qualification.
esl_drop_v = .020               # Layout/ringing allowance; not a measured value.
steady_floor_v = bat_input_min - hold_load_a * mux_r_budget
available_v = steady_floor_v - vin_floor
q_forward = hold_load_a * gap_s
q_total = q_forward + reverse_q_c
drop_c_v = q_total / cap_eff_min_f
drop_esr_v = cap_peak_a * esr_budget
drop_total_v = drop_c_v + drop_esr_v + esl_drop_v
sys_min_v = steady_floor_v - drop_total_v
cap_required_f = q_total / (available_v - drop_esr_v - esl_drop_v)
gap_supported_s = ((available_v - drop_esr_v - esl_drop_v) * cap_eff_min_f - reverse_q_c) / hold_load_a

gap_rows = []
for us in (5, 10, 20, 50, 75, 81.25, 100, 200, 1000):
    gap = us * 1e-6
    dq = hold_load_a * gap + reverse_q_c
    dv = dq / cap_eff_min_f + drop_esr_v + esl_drop_v
    gap_rows.append({"gap_us_scenario": us, "Ceff_uF_qualification": cap_eff_min_f * 1e6,
        "forward_load_A": hold_load_a, "reverse_charge_uC_budget": reverse_q_c * 1e6,
        "cap_peak_A_budget": cap_peak_a, "ESR_ohm_budget": esr_budget,
        "total_drop_V_bound": dv, "SYS_floor_V_bound": steady_floor_v - dv,
        "meets_design_SYS_3V5": steady_floor_v - dv >= vin_floor - 1e-12,
        "classification": "conditional_requirement_not_TPS2121_timing_prediction"})
save_csv("source-switchover-gap-envelope.csv", gap_rows)

cap_rows = []
for count in (1, 2, 3):
    cf = count * cap_each_f * .8
    # This row uses only initial +/-20% capacitance and 25C ESR rating.
    er = .025 / count
    dv = q_total / cf + cap_peak_a * er + esl_drop_v
    cap_rows.append({"count_330uF": count, "nominal_uF": count * 330,
        "initial_minus20percent_uF": cf * 1e6, "ESR_parallel_25C_ohm": er,
        "drop_V_with_same_50us_20uC_3A_stress": dv,
        "SYS_floor_V_example": steady_floor_v - dv,
        "classification": "initial_capacitance_and_ESR_only_not_full_temperature_bound"})
save_csv("source-switchover-capacitor-choice.csv", cap_rows)

# Comparator inputs from same 20k/10k divider, independent +/-1% resistors.
divider_rows = []
for rt_usb, rb_usb, rt_bat, rb_bat, leak_usb, leak_bat in product(
        (19800, 20200), (9900, 10100), (19800, 20200), (9900, 10100),
        (-.1e-6, .1e-6), (-.1e-6, .1e-6)):
    au = rb_usb / (rt_usb + rb_usb)
    ab = rb_bat / (rt_bat + rb_bat)
    ru = rt_usb * rb_usb / (rt_usb + rb_usb)
    rb = rt_bat * rb_bat / (rt_bat + rb_bat)
    pr_min_usb = 4.75 * au - leak_usb * ru
    cp_max_bat = 4.35 * ab - leak_bat * rb
    cp_min_bat = bat_input_min * ab - leak_bat * rb
    divider_rows.append({"R_USB_top_ohm": rt_usb, "R_USB_bottom_ohm": rb_usb,
        "R_BAT_top_ohm": rt_bat, "R_BAT_bottom_ohm": rb_bat,
        "USB_leak_A": leak_usb, "BAT_leak_A": leak_bat,
        "PR1_at_USB4V75_V": pr_min_usb, "CP2_at_BAT4V35_V": cp_max_bat,
        "USB_priority_margin_after_40mV_offset_V": pr_min_usb - cp_max_bat - .040,
        "CP2_at_BAT3V8_V": cp_min_bat,
        "fast_mode_margin_above_VREF_max1V1_V": cp_min_bat - 1.1})
save_csv("source-switchover-divider-corners.csv", divider_rows)

region_rows = []
for bat in (3.6, 3.7, 3.8, 3.9, 4.2):
    for load in (.3, .6, .9, 1.0):
        base_v = bat - load * mux_r_budget
        dq = load * gap_s + reverse_q_c
        floor = base_v - dq / cap_eff_min_f - drop_esr_v - esl_drop_v
        region_rows.append({"true_MUX_IN2_V": bat, "forward_load_A_scenario": load,
            "SYS_steady_floor_V": base_v, "SYS_handoff_floor_V": floor,
            "meets_SYS_3V5": floor >= vin_floor,
            "classification": "operating_region_if_all_other_qualification_bounds_hold"})
save_csv("source-switchover-operating-region.csv", region_rows)

# Reuse recording allocation; check consistency with <=0.9A normal SYS load.
i0 = .150 + .002
k = 3.3 * .600 / .85
disc = (bat_input_min - mux_r_budget * i0)**2 - 4 * mux_r_budget * k
coupled_v = ((bat_input_min - mux_r_budget * i0) + math.sqrt(disc)) / 2
coupled_i = i0 + k / coupled_v

summary = {
    "design_revision": "v0.5",
    "classification": "source_selection_and_hold_up_requirements_not_hardware_acceptance",
    "selected_mux": {"part": "TPS2121RUXR", "mode": "XCOMP_USB_PR1_vs_BAT_CP2",
        "datasheet_fast_switchover_typ_s_not_MAX": 5e-6,
        "datasheet_RON_max_25C_ohm_at_input5V_200mA": .070,
        "qualification_RON_at_battery_voltage_ohm": mux_r_budget,
        "normal_SYS_load_limit_A": normal_sys_a,
        "IN2_true_min_V": bat_input_min,
        "USB_at_IN1_V_contract": [4.75, 5.25], "USB_source_A_contract": 3,
        "R_ILIM_ohm": 80000, "R_ILIM_tolerance": .01,
        "ILIM_nominal_formula_A": 65.2 / 80**.861,
        "ILIM_table_A_at_exact80k": [1, 1.5, 2],
        "ILIM_qualification_A": [.95, 2.2],
        "ILIM_note": "not_1A_cell_fault_protection; resistor_tolerance_not_covered_by_exact80k_test_point"},
    "hold_up": {"gap_total_s_qualification": gap_s, "load_A_bound": hold_load_a,
        "reverse_charge_C_bound": reverse_q_c, "cap_peak_A_bound": cap_peak_a,
        "Ceff_min_F_qualification": cap_eff_min_f, "ESR_max_ohm_qualification": esr_budget,
        "ESL_layout_drop_V_budget": esl_drop_v, "steady_floor_V": steady_floor_v,
        "forward_charge_C": q_forward, "total_charge_C": q_total,
        "drop_capacitance_V": drop_c_v, "drop_ESR_V": drop_esr_v,
        "total_drop_V": drop_total_v, "SYS_min_V_bound": sys_min_v,
        "margin_over_SYS3V5_V": sys_min_v - vin_floor,
        "minimum_Ceff_F_for_chosen_bounds": cap_required_f,
        "maximum_gap_s_for_chosen_other_bounds": gap_supported_s,
        "capacitor_energy_3V7_to_3V5_J": .5 * cap_eff_min_f * (3.7**2 - 3.5**2)},
    "selected_capacitors": {"part_each": "T520X337M010ATE025", "quantity": cap_count,
        "Cnom_total_F": cap_nom_f, "Cmin_initial_tolerance_F": cap_min_initial_f,
        "Cmax_initial_tolerance_F": cap_max_initial_f,
        "ESR_parallel_25C_ohm": .025 / cap_count,
        "endurance_corner_example_not_full_life_guarantee": {
            "C_F_initial_minus20_then_endurance_minus20": cap_min_initial_f * .8,
            "ESR_ohm_initial_limit_times2_parallel": .025 * 2 / cap_count,
            "SYS_floor_V_same_other_bounds_no_added_temperature_or_layout":
                steady_floor_v - q_total / (cap_min_initial_f * .8)
                - cap_peak_a * (.025 * 2 / cap_count) - esl_drop_v,
            "note": "does_not_meet_selected_750uF_15mOhm_contract; separate_age_temperature_qualification_required"},
        "dimensions_each_mm": [7.3, 4.3, 4.3],
        "note": "verify_correct_part_lifecycle_and_Ceff_ESR_over_temperature_age; no_MLCC_counted_in_hold_bound"},
    "priority_dividers": {"top_ohm": 20000, "bottom_ohm": 10000, "tolerance": .01,
        "worst_USB_priority_margin_V": min(x["USB_priority_margin_after_40mV_offset_V"] for x in divider_rows),
        "worst_CP2_fast_mode_margin_V": min(x["fast_mode_margin_above_VREF_max1V1_V"] for x in divider_rows)},
    "startup": {"C_SS_F": 1e-6, "slew_5V_estimated_typ_V_per_s": 88,
        "slew_V_per_s_qualification": 200,
        "cap_inrush_A_at_Cmax_and_qualified_slew": cap_max_initial_f * 200,
        "PREP_load_A_contract": .5,
        "startup_SYS_A_bound": .5 + cap_max_initial_f * 200,
        "startup_battery_A_bound_including_aux2mA": .5 + cap_max_initial_f * 200 + .002,
        "USB_current_A_bound_with_qualified_ILIMmax_and_charge_alloc": 2.2 + .150 + .002,
        "note": "slew_and_ilimit_bounds_require_validation; initial_and_resettling_states_are_not_5us"},
    "recording_allocation_check": {"I_3V3_A": .600, "camera_A": .150,
        "aux_A_allocation": .002, "buck_efficiency_floor_choice": .85,
        "mux_input_V": bat_input_min, "SYS_coupled_V": coupled_v,
        "SYS_coupled_I_A": coupled_i, "normal_0V9A_contract_met_in_model": coupled_i <= normal_sys_a},
    "sources": {"mux": "https://www.ti.com/lit/ds/symlink/tps2121.pdf",
        "cap": "https://yageogroup.com/content/datasheet/asset/file/KEM_T2076_T52X-530"},
    "unclosed": ["actual_gap_reverse_charge_and_cap_peak", "Ceff_ESR_layout_temperature",
        "buck_3V3_line_step_response", "actual_revision_and_rework", "pack_charger_fault_coordination",
        "runtime_45min_with_new_voltage_eligibility", "USB_standard_and_overvoltage_transient_compliance"],
}
(OUT / "source-switchover-calculations.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")
print(json.dumps(summary, indent=2))
