"""Reproduce NeoCam reference-design calculations; no hardware or battery model.

Run: python concepts/neocam/tools/calculate-power-design.py
All numerical scenarios are identified as datasheet points or design allocations.
Outputs are written to concepts/neocam/design-output/.
"""
from pathlib import Path
from itertools import product
import csv
import json
import math

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "design-output"
OUT.mkdir(exist_ok=True)


def save_csv(name, rows):
    with (OUT / name).open("w", newline="", encoding="utf-8-sig") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


# Same OV3660 datasheet condition for all three rails, not a maximum.
camera_rails = [("AVDD", 2.8, .028), ("DVDD", 1.5, .064), ("DOVDD", 2.8, .006)]
sensor_w = sum(voltage * current for _, voltage, current in camera_rails)
camera_i_a = sum(current for _, _, current in camera_rails)
camera_rows = []
for vin in (3.3, 3.5, 3.7, 4.2, 5.0):
    pin = vin * camera_i_a
    camera_rows.append({"VCC_IN_V": vin, "input_reference_A_excluding_overhead": camera_i_a,
        "sensor_reference_W": sensor_w, "input_reference_W": pin,
        "LDO_loss_W_excluding_overhead": pin - sensor_w,
        "efficiency_excluding_overhead": sensor_w / pin,
        "classification": "same_sensor_typical_point_not_720p_measurement"})
save_csv("camera-ldo.csv", camera_rows)

# REJECTED ALTERNATIVE ONLY; final KiCad uses MAX17048, not this ADC branch.
# ADC: BAT -> RT -> DIV; RB DIV-to-GND; analog switch DIV-to-ADC;
# RP ADC-to-GND. C=100nF is on DIV, upstream of isolation switch.
rt = rb = rp = 100_000.0
cap_f = 100e-9
ron_ohm = 2.0  # TMUX typical illustration, not guaranteed across conditions.


def adc_values(vbat, rtop, rbottom, rpull, ron=0):
    req = 1 / (1 / rbottom + 1 / (ron + rpull))
    vdiv = vbat * req / (rtop + req)
    vadc = vdiv * rpull / (ron + rpull)
    return vadc, vdiv, (vbat - vdiv) / rtop


adc_corners = []
for at, ab, ap in product((.99, 1.01), repeat=3):
    vadc, vdiv, current = adc_values(4.234, rt * at, rb * ab, rp * ap)
    adc_corners.append({"RT_factor": at, "RB_factor": ab, "RP_factor": ap,
        "VBAT_example_V": 4.234, "ADC_V": vadc, "DIV_V": vdiv,
        "divider_A": current,
        "classification": "resistor_tolerance_only_not_ADC_total_accuracy"})
save_csv("adc-divider-corners.csv", adc_corners)
vadc, vdiv, divider_a = adc_values(4.234, rt, rb, rp, ron_ohm)
rth_on = 1 / (1 / rt + 1 / rb + 1 / (ron_ohm + rp))
rth_off = 1 / (1 / rt + 1 / rb)

# 50 mAh is a selectable project allocation, not measured finalization demand.
reserve_mah = 50.0
runtime_rows = []
for current_ma in (150, 250, 300, 350, 500, 750, 800, 1000):
    required = .75 * current_ma + reserve_mah
    runtime_rows.append({"recording_average_mA_scenario": current_ma,
        "recording_minutes_target": 45, "recording_mAh": .75 * current_ma,
        "boot_close_reserve_mAh_allocation": reserve_mah,
        "required_usable_mAh": required,
        "fraction_of_800mAh_label": required / 800,
        "fraction_of_1000mAh_label": required / 1000,
        "classification": "requirement_scenario_not_runtime_prediction"})
save_csv("runtime-envelope.csv", runtime_rows)

# v0.4 selects DMP2035U-7; 0.100 ohm is the HOT ACCEPTANCE BUDGET,
# not a manufacturer-guaranteed hot maximum. Stock reference remains compared.
# Vin_required and efficiency are chosen acceptance criteria, not guaranteed.
# Vpack_loaded is at protected PACK P+/P- terminals under load.
# Exclude cell/PCM impedance, already reflected in that measured voltage.
# Rexternal is connector/fuse/wiring after the protected pack, not inside it.
vin_req = 3.5
eta_req = .85
rq1_ref = .100
rexternal_budget = .100
icam_alloc = .150
iaux_alloc = .002
path_rows = []
for irail in (.100, .200, .300, .400, .600):
    ibat = 3.3 * irail / (eta_req * vin_req) + icam_alloc + iaux_alloc
    drop_external = ibat * rexternal_budget
    drop_q1 = ibat * rq1_ref
    path_rows.append({"I_3V3_A_allocation": irail, "VIN_required_V_choice": vin_req,
        "buck_efficiency_floor_choice": eta_req,
        "camera_input_A_allocation": icam_alloc, "aux_A_allocation": iaux_alloc,
        "calculated_battery_A": ibat, "external_path_drop_V": drop_external,
        "Q1_drop_V_reference": drop_q1,
        "loaded_pack_min_V_conditional": vin_req + drop_external + drop_q1,
        "Q1_loss_W_reference": ibat * ibat * rq1_ref,
        "classification": "v0.4_DMP2035U_hot_acceptance_budget_not_datasheet_hot_maximum"})
save_csv("path-sag-envelope.csv", path_rows)

# Coupled solution at a given PACK terminal voltage, larger-voltage branch.
coupled_rows = []
for vpack in (4.2, 4.0, 3.8, 3.6):
    irail = .600
    rpath = rq1_ref + rexternal_budget
    i0 = icam_alloc + iaux_alloc
    k = 3.3 * irail / eta_req
    discriminant = (vpack - rpath * i0)**2 - 4 * rpath * k
    if discriminant < 0:
        vin = current = qheat = None
    else:
        vin = ((vpack - rpath * i0) + math.sqrt(discriminant)) / 2
        current = i0 + k / vin
        qheat = current**2 * rq1_ref
    coupled_rows.append({"Vpack_loaded_V": vpack, "I_3V3_A_allocation": irail,
        "VIN_algebraic_V": vin, "Ipack_algebraic_A": current,
        "Q1_loss_W_reference": qheat,
        "meets_chosen_VIN_3V5_requirement": vin is not None and vin >= vin_req,
        "classification": "algebraic_model_only_invalid_regulation_points_not_operating_predictions"})
save_csv("coupled-path-examples.csv", coupled_rows)

# Q1 comparison: datasheet points vs explicit qualification requirement.
q1_cases = {
    "stock_LP0404_25C_reference": .670,
    "DMP2035U_25C_reference": .045,
    "DMP2035U_hot_acceptance_budget": .100,
}
fixed_rows, threshold_rows, coupled_comparison = [], [], []
for name, r_q in q1_cases.items():
    for current in (.250, .500, .800, 1.000):
        fixed_rows.append({"case": name, "I_Q1_A": current, "RQ1_ohm": r_q,
            "drop_V": current * r_q, "loss_W": current**2 * r_q,
            "meets_chosen_drop_100mV": current * r_q <= .100 + 1e-12,
            "meets_chosen_loss_100mW": current**2 * r_q <= .100 + 1e-12,
            "classification": "same_current_ohmic_comparison_not_all_temperature_bound"})
    for irail in (.100, .200, .300, .400, .600):
        current = 3.3 * irail / (eta_req * vin_req) + icam_alloc + iaux_alloc
        threshold_rows.append({"case": name, "I_3V3_A_allocation": irail,
            "VIN_required_V_choice": vin_req, "Ipack_A": current,
            "Vpack_required_V": vin_req + current * (rexternal_budget + r_q),
            "Q1_loss_W": current**2 * r_q,
            "classification": "conditional_required_pack_voltage_not_firmware_stop_threshold"})
        for vpack in (3.6, 3.7, 4.0, 4.2):
            rpath = rexternal_budget + r_q
            i0 = icam_alloc + iaux_alloc
            k = 3.3 * irail / eta_req
            disc = (vpack - rpath * i0)**2 - 4 * rpath * k
            vin = ((vpack - rpath * i0) + math.sqrt(disc)) / 2 if disc >= 0 else None
            current = i0 + k / vin if vin is not None else None
            coupled_comparison.append({"case": name, "Vpack_loaded_V": vpack,
                "I_3V3_A_allocation": irail, "VIN_algebraic_V": vin,
                "Ipack_algebraic_A": current,
                "Q1_drop_V": current * r_q if current is not None else None,
                "Q1_loss_W": current**2 * r_q if current is not None else None,
                "meets_chosen_VIN_3V5": vin is not None and vin >= vin_req,
                "classification": "invalid_regulation_points_not_operating_predictions"})
save_csv("q1-fixed-current-comparison.csv", fixed_rows)
save_csv("q1-required-pack-voltage.csv", threshold_rows)
save_csv("q1-coupled-comparison.csv", coupled_comparison)

thermal_rows = []
for ta in (25, 40, 60):
    for current in (.800, 1.000):
        for name, r_q in q1_cases.items():
            theta = 131 if name.startswith("stock") else 153.5
            loss = current**2 * r_q
            thermal_rows.append({"case": name, "ambient_local_C": ta,
                "I_Q1_A": current, "loss_W": loss, "theta_fixture_C_per_W": theta,
                "Tj_fixture_estimate_C": ta + loss * theta,
                "max_effective_theta_for_Tj85_C_per_W": (85 - ta) / loss,
                "classification": "fixture_calculation_not_actual_adapter_or_XIAO_temperature"})
save_csv("q1-thermal-envelope.csv", thermal_rows)

gate_r_stock = 100_000
gate_r_added = 10_000
gate_r_effective = 1 / (1 / gate_r_stock + 1 / gate_r_added)
gate_results = {"stock_R9_ohm": gate_r_stock, "added_R6_ohm": gate_r_added,
    "nominal_parallel_ohm": gate_r_effective,
    "max_parallel_ohm_using_only_added_Rplus1percent": gate_r_added * 1.01,
    "gate_offset_V_at_10uA_and_max_pull_down_R": 10e-6 * gate_r_added * 1.01,
    "additional_USB_current_A_at_5V": 5 / gate_r_added,
    "additional_resistor_loss_W_at_5V": 5**2 / gate_r_added,
    "additional_resistor_loss_W_at_5V25_Rminus1percent": 5.25**2 / (gate_r_added * .99),
    "VBUS_cap_F_nominal_stock": 2e-6,
    "unloaded_VBUS_RC_stock_s": gate_r_stock * 2e-6,
    "unloaded_VBUS_RC_new_s": gate_r_effective * 2e-6,
    "note": "unloaded_capacitor_model_not_USB_handoff_or_brownout_validation"}

load_rows = [
    {"block": "ESP32-S3", "voltage_V": 3.3, "current_mA": 81.3,
        "power_W": 3.3 * .0813, "kind": "typical_240MHz_dual_32bit_all_peripheral_clocks",
        "missing": "actual_AV_workload_R8_memory_and_maximum"},
    {"block": "OV3660_sensor", "voltage_V": "multiple", "current_mA": "not_add_at_load_rails",
        "power_W": sensor_w, "kind": "typical_external_DVDD_full_size_speed_compression",
        "missing": "720p30_active_mean_peak_and_LDO_overhead"},
    {"block": "microSD", "voltage_V": 3.3, "current_mA": .36 / 3.3 * 1000,
        "power_W": .36, "kind": "conditional_default_speed_budget_averaged_1s",
        "missing": "exact_Ultra_SKU_peak_inrush_write_busy"},
    {"block": "microphone", "voltage_V": 1.8, "current_mA": .670,
        "power_W": 1.8 * .000670, "kind": "typical_1V8_2point4MHz_not_board_3V3",
        "missing": "3V3_actual_clock_and_maximum"},
    {"block": "MAX17048", "voltage_V": "pack_2.5_to_4.5", "current_mA": .040,
        "power_W": 4.2 * .000040, "kind": "active_max_from_specified_electrical_table",
        "missing": "board_leakage_and_I2C_pullup_activity"},
]
save_csv("load-evidence.csv", load_rows)

summary = {
    "classification": "reference_design_calculations_not_measurements",
    "design_revision": "v0.4",
    "selected_Q1": {"part": "DMP2035U-7", "pin_map": {"1": "G_VBUS", "2": "S_VIN", "3": "D_VBAT"},
        "RDS25C_reference_ohm_at_VGS_minus2V5": .045,
        "R_hot_qualification_budget_ohm_not_datasheet_guarantee": rq1_ref,
        "drop_budget_V_at_1A": .1, "loss_budget_W_at_1A": .1,
        "stock_reference_ohm": .67,
        "maximum_current_for_stock_to_meet_drop_budget_A_reference": .1 / .67,
        "fixed_current_drop_loss_reduction_at_25C_reference": 1 - .045 / .67,
        "fixed_current_drop_loss_reduction_using_new_qualification_budget": 1 - .100 / .67,
        "maximum_effective_theta_C_per_W_for_85C_at_ambient60_loss100mW": (85 - 60) / .1,
        "integration": "remove_old_Q1; new_SOT23_adapter_not_drop_in; stock_D2_R9_remain"},
    "selected_Q1_gate": gate_results,
    "coupled_example_at_pack4V2": coupled_rows[0],
    "camera": {"sensor_W": sensor_w, "input_A_excluding_overhead": camera_i_a,
        "input_W_at_3V7": 3.7 * camera_i_a,
        "LDO_loss_W_at_3V7": 3.7 * camera_i_a - sensor_w},
    "rejected_adc_alternative": {"selected_in_final_KiCad": False,
        "reason": "finite_poweroff_switch_delay_and_ADC_injection_limits_not_closed",
        "resistors_ohm": [rt, rb, rp], "nominal_scale_ideal_Ron0": 3,
        "Vadc_at_4V234_with_typical_Ron": vadc, "divider_A": divider_a,
        "min_ADC_V_1percent_resistors": min(r["ADC_V"] for r in adc_corners),
        "max_ADC_V_1percent_resistors": max(r["ADC_V"] for r in adc_corners),
        "tau_on_s": rth_on * cap_f, "tau_off_DIV_s": rth_off * cap_f,
        "settle_on_0point1percent_s": -math.log(.001) * rth_on * cap_f,
        "off_ADC_V_DC_at_2uA_and_RPplus1percent": 2e-6 * rp * 1.01,
        "caveat": "Ipoff_bound_test_VD0_not_exact_loaded_leak_model; ADC_leak_and_transient_not_included"},
    "selected_MAX17048": {"VDD_operating_V": [2.5, 4.5],
        "active_current_max_A_specified_conditions": 40e-6,
        "active_charge_45minutes_mAh_max_specified_conditions": .040 * .75,
        "pullups_ohm_to_host_3V3": 2200,
        "bus_capacitance_F_design_budget": 150e-12,
        "rise_time_s_design_budget": .8473 * 2200 * 150e-12,
        "max_C_for_300ns_at_Rplus1percent_F": 300e-9 / (.8473 * 2200 * 1.01),
        "low_current_A_at_rail3V6_and_VOL0V4_Rminus1percent": (3.6 - .4) / (2200 * .99),
        "VCELL_raw16_scale_V": 78.125e-6,
        "voltage_measurement_resolution_V_datasheet": 1.25e-3,
        "raw16_example_at_3V7": round(3.7 / 78.125e-6),
        "I2C_bus_frequency_Hz_choice": 100000,
        "hibernate_while_recording_choice": "disabled_HIBRT_0x0000",
        "note": "read_voltage_only_initially; no_accurate_SOC_or_current_measurement_claim"},
    "charge": {"nominal_mA": 24000 / 220,
        "nominal_Q_over_I_1000mAh_hours_not_charge_prediction": 1000 / (24000 / 220),
        "PCM_min_minus_table_charge_max_V_not_global_bound": 4.225 - 4.234},
    "USB_reference_allocation": {"source_voltage_V": 5,
        "source_advertised_current_A_choice": 1.5,
        "VIN_USB_V_example_after_drop": 4.5,
        "I_3V3_A_allocation": .600, "camera_A_allocation": .150,
        "charge_nominal_A": (24000 / 220) / 1000,
        "efficiency_choice": .85,
        "calculated_USB_A_example": 3.3 * .600 / (.85 * 4.5) + .150 + (24000 / 220) / 1000 + .002,
        "note": "source_capability_choice_not_measured_USB_current; no_PD_voltage_request"},
    "fuse": {"part": "046701.5NR", "nominal_cold_R_ohm": .0385,
        "continuous_25C_after_25percent_derating_A": 1.5 * .75,
        "drop_at_0point8A_V": .8 * .0385,
        "loss_at_0point8A_W": .8**2 * .0385},
    "button": {"pullup_ohm": 10000, "capacitor_F": 100e-9,
        "nominal_RC_s": 10000 * 100e-9, "pressed_DC_A_at_3V3": 3.3 / 10000},
    "LED": {"R_ohm": 2200, "typical_Vf_example_V": 2,
        "current_A_example": (3.3 - 2) / 2200,
        "current_upper_if_LED_short_and_rail_3V6_A_at_nominal_R": 3.6 / 2200,
        "current_upper_if_LED_short_rail3V6_Rminus1percent_A": 3.6 / (2200 * .99),
        "R_power_upper_if_LED_short_rail3V6_Rminus1percent_W": 3.6**2 / (2200 * .99)},
    "sources": {
        "sensor": "https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/res/OV3660_datasheet.pdf",
        "switch": "https://www.ti.com/lit/ds/symlink/tmux1511.pdf",
        "fuel_gauge": "https://www.analog.com/media/en/technical-documentation/data-sheets/MAX17048-MAX17049.pdf",
        "Q1": "https://www.lrc.cn/Upload/PDF/Product/MOS/LP0404N3T5G.pdf",
        "Q1_new": "https://www.diodes.com/datasheet/download/DMP2035U.pdf",
        "charger": "https://www.sg-micro.com/rect/assets/e95e555a-a8a2-4761-b86a-6554dff2d8ed/SGM40567.pdf",
    },
}
with (OUT / "calculations.json").open("w", encoding="utf-8") as handle:
    json.dump(summary, handle, indent=2, ensure_ascii=False)
print(json.dumps(summary, indent=2, ensure_ascii=False))
