"""NeoCam v0.6: stock buck dropout and output charge-budget analysis.

The SGM6029 internal controller is NOT modeled or assigned a timing maximum.
Run: python concepts/neocam/tools/calculate-buck-3v3.py
"""
from pathlib import Path
import csv
import json

BASE = Path(__file__).resolve().parents[1]
OUT = BASE / "design-output"
handoff = json.loads((OUT / "source-switchover-calculations.json").read_text())
vin_min = handoff["hold_up"]["SYS_min_V_bound"]
iout = .600  # Board total allocation, not a measured recording current.
vnom = 3.3
vbaseline_min = vnom * .98  # REQUIRED loaded baseline, not a new datasheet guarantee.
vbaseline_max = vnom * 1.02
pin_min, pin_max = 3.1, 3.5
trace_drop = .030
ripple_half = .020
esr_step = .020
ringing = .020
q_minus_target = .25e-6
q_plus_target = .40e-6
c_min_choice = 8e-6  # Conditional narrow design envelope within recommended 5..10uF.
r_hs_budget, r_l_budget, r_local_budget = .400, .080, .020
r_path_budget = r_hs_budget + r_l_budget + r_local_budget


def csv_out(name, rows):
    with (OUT / name).open("w", newline="", encoding="utf-8-sig") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)


dc_rows = []
for rv in (.310 + .054, .400, .500, .600, .750):
    for ia in (.100, .300, .600):
        avail = vin_min - ia * rv
        dc_rows.append({"VIN_V_from_conditional_mux_bound": vin_min,
            "I3V3_A_allocation": ia, "R_buck_path_ohm_scenario": rv,
            "V_100percent_duty_available_V": avail,
            "headroom_over_3V234_loaded_baseline_requirement_V": avail - vbaseline_min,
            "headroom_over_nominal3V3_V": avail - vnom,
            "classification": "DC_path_scenario_not_actual_hot_drop_or_transient"})
csv_out("buck-3v3-dropout-envelope.csv", dc_rows)

charge_rows = []
for cf in (5e-6, 8e-6, 10e-6):
    qn = cf * (vbaseline_min - pin_min - trace_drop - ripple_half - esr_step - ringing)
    qp = cf * (pin_max - vbaseline_max - ripple_half - esr_step - ringing)
    charge_rows.append({"total_Ceff_uF_scenario": cf * 1e6,
        "Qdeficit_allowed_uC": qn * 1e6, "Qsurplus_allowed_uC": qp * 1e6,
        "full_0V6A_deficit_time_allowed_us": qn / iout * 1e6,
        "Vpin_min_at_target_Qminus0V25uC_V": vbaseline_min - trace_drop - ripple_half - esr_step - ringing - q_minus_target / cf,
        "Vpin_max_at_target_Qplus0V4uC_V": vbaseline_max + ripple_half + esr_step + ringing + q_plus_target / cf,
        "classification": "allowed_charge_and_conditional_prediction_not_controller_spec"})
csv_out("buck-3v3-charge-envelope.csv", charge_rows)

ripple_rows = []
for vi in (vin_min, 3.7, 4.75, 5, 5.25):
    for lf in (.33e-6, .47e-6, 1e-6):
        fs = 4e6  # TYPICAL only; no minimum fSW published.
        di = vnom * (1 - vnom / vi) / (lf * fs)
        ripple_rows.append({"VIN_V": vi, "VOUT_V_ideal": vnom,
            "L_uH_scenario": lf * 1e6, "fSW_MHz_TYP_not_min": fs / 1e6,
            "dI_L_pp_A_ideal_CCM": di, "I_L_peak_A_at_0V6A": iout + di / 2,
            "I_L_valley_A_at_0V6A": iout - di / 2,
            "Vout_pp_V_with_Ceff8uF_ESR20mOhm": di / (8 * fs * c_min_choice) + di * .020,
            "classification": "ideal_CCM_typ_frequency_not_drop_out_or_transient_guarantee"})
csv_out("buck-3v3-ripple-examples.csv", ripple_rows)

delay_rows = []
for cf in (5e-6, 8e-6, 10e-6):
    for deficit in (.1, .3, .6):
        for us in (.1, .25, .5, 1, 2, 5, 50):
            dq = deficit * us * 1e-6
            floor = vbaseline_min - trace_drop - ripple_half - esr_step - ringing - dq / cf
            delay_rows.append({"Ceff_uF_scenario": cf * 1e6,
                "net_current_deficit_A": deficit, "duration_us_imposed": us,
                "Q_deficit_uC": dq * 1e6, "Vpin_floor_V_linear_charge_example": floor,
                "meets_3V1_in_charge_envelope": floor >= pin_min,
                "classification": "sensitivity_not_predicted_buck_latency; negative_voltage_means_model_failure"})
csv_out("buck-3v3-deficit-sensitivity.csv", delay_rows)

# The 14-ohm ESP internal SPI supply path is TYPICAL, not a maximum.
spi_rows = []
for vm in (3.1, 3.234, 3.3):
    for im in (.01, .02, .03, .04):
        spi_rows.append({"VDD3P3_RTC_V_example": vm, "SPI_memory_current_A_imposed": im,
            "R_SPI_ohm_TYP": 14, "VDD_SPI_V_example": vm - im * 14,
            "above_memory_2V7_in_this_example": vm - im * 14 >= 2.7,
            "classification": "typical_resistance_illustration_not_memory_power_guarantee"})
csv_out("buck-3v3-vdd-spi-examples.csv", spi_rows)

result = {
    "revision": "v0.6", "decision": "retain_stock_SGM6029C_L4_C5_C6; no_added_3V3_bulk",
    "classification": "design_envelopes_not_hardware_or_internal_controller_proof",
    "input": {"VIN_min_V_inherited_conditional": vin_min,
        "mux_bounds_still_required": True, "stock_board_3V3_total_A_ceiling": iout,
        "headroom_over_buck_UVLO_rising_max1V95_V": vin_min - 1.95,
        "input_bulk_Ceff_F_conditional": 750e-6,
        "VIN_coast_slew_V_per_s_at1A": 1 / 750e-6,
        "VIN_reverse_interval_slew_V_per_s_at3A": 3 / 750e-6,
        "VIN_ESR_plus_layout_step_V_budget": .045 + .020,
        "USB_reentry_slew_V_per_s_qualification": 200},
    "datasheet_facts_with_conditions": {
        "buck_part": "SGM6029CYG/TR", "stock_L_nom_H": .47e-6,
        "stock_C5_plus_C6_nom_F_not_total_effective": 9.4e-6,
        "manufacturer_recommended_effective_Cout_F": [5e-6, 10e-6],
        "fSW_Hz_TYP_only": 4e6, "R_HS_ohm_typ_max_at_table_conditions_100mA": [.185, .310],
        "R_HS_not_bound_at_actual_load_and_VIN": True,
        "output_accuracy_plus_minus_percent_PWM_IOUT0mA_table": 2,
        "RVSEL_ohm": 249000, "RVSEL_role": "startup_digital_voltage_code_not_feedback_divider",
        "source": "https://www.sg-micro.com/rect/assets/3323f5dc-718f-4b81-b820-398ddb702989/SGM6029.pdf"},
    "DC_qualification": {"R_HS_ohm_budget": r_hs_budget, "R_L_ohm_budget": r_l_budget,
        "R_local_ohm_budget": r_local_budget, "R_sum_ohm_budget": r_path_budget,
        "drop_V_at_600mA": iout * r_path_budget,
        "available_V_100percent_duty": vin_min - iout * r_path_budget,
        "Rsum_max_for_3V234_at_VINmin_600mA_ohm": (vin_min - vbaseline_min) / iout,
        "Rsum_max_for_nominal3V3_at_VINmin_600mA_ohm": (vin_min - vnom) / iout,
        "loaded_baseline_V_requirement_not_0mA_guarantee": [vbaseline_min, vbaseline_max]},
    "transient_qualification": {"total_Ceff_F_required_for_selected_envelope": [c_min_choice, 10e-6],
        "all_distributed_rail_capacitance_must_be_counted": True,
        "Ceff8uF_not_verified_stock": True,
        "load_pin_V_design_window": [pin_min, pin_max], "trace_drop_V_budget": trace_drop,
        "ripple_one_sided_V_budget": ripple_half, "ESR_step_V_budget": esr_step,
        "ringing_one_sided_V_budget": ringing, "Q_minus_C_target": q_minus_target,
        "Q_plus_C_target": q_plus_target,
        "Vpin_min_V_bound": vbaseline_min - trace_drop - ripple_half - esr_step - ringing - q_minus_target / c_min_choice,
        "Vpin_max_V_bound": vbaseline_max + ripple_half + esr_step + ringing + q_plus_target / c_min_choice,
        "Ceff_min_required_for_Qminus_target_F": q_minus_target / (vbaseline_min - pin_min - trace_drop - ripple_half - esr_step - ringing),
        "no_internal_controller_timing_max_known": True,
        "linear_drop_V_if_buck_off_600mA_50us_C8uF": iout * 50e-6 / c_min_choice,
        "C_required_full_off_600mA_50us_at_same_drop_budget_F": iout * 50e-6 / (vbaseline_min - pin_min - trace_drop - ripple_half - esr_step - ringing)},
    "memory_supply": {"VDD_SPI_V_separate_acceptance": [2.7, 3.6],
        "R_SPI_ohm_TYP_not_max": 14,
        "I_memory_A_at3V1_main_to2V7_SPI_using_typ14ohm": (3.1 - 2.7) / 14,
        "main3V3_window_does_not_guarantee_memory_rail": True},
    "unclosed": ["actual_mainboard_revision_and_L4_DCR_Isat", "total_Ceff_and_loop_stability_including_distributed_caps",
        "loaded_DC_regulation_and_actual_hot_R_HS", "actual_net_output_charge_deficit_and_surplus",
        "ESP_and_SD_local_rails_and_ferrite_DCR", "VDD_SPI_flash_and_PSRAM_supply", "600mA_workload_ceiling", "45minute_runtime"]}
(OUT / "buck-3v3-calculations.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
print(json.dumps(result, indent=2))
