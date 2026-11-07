"""Read this project's little-endian real/double QSPICE qraw, not other formats.

Post-process the REJECTED ADC alternative only; not final MAX17048 validation.
No proprietary chip macromodel or power-failure survival prediction is used.
"""
from pathlib import Path
import csv
import json
import re
import struct

root = Path(__file__).resolve().parents[1]
out = root / "design-output"
raw = (out / "adc-isolation-reference.qraw").read_bytes()
header_bytes, binary = raw.split(b"Binary:\n", 1)
header = header_bytes.decode("utf-8")
count = int(re.search(r"No\. Variables:\s+(\d+)", header)[1])
npoints = int(re.search(r"No\. Points:\s+(\d+)", header)[1])
assert count == 5 and "Flags: real" in header
assert len(binary) == npoints * count * 8, "Not the supported all-double raw layout"
rows = list(struct.iter_unpack("<" + "d" * count, binary))


def sample(t):
    before = max(row for row in rows if row[0] <= t)
    after = min(row for row in rows if row[0] >= t)
    if before[0] == after[0]:
        return before
    a = (t - before[0]) / (after[0] - before[0])
    return tuple(x + a * (y - x) for x, y in zip(before, after))


result = {"classification": "ideal_model_of_rejected_ADC_alternative_not_final_circuit",
    "QSPICE_header": header,
    "ADC_on_at_60ms_V": sample(.060)[4],
    "DIV_on_at_60ms_V": sample(.060)[3],
    "ADC_off_max_85to120ms_V": max(row[4] for row in rows if .085 <= row[0] <= .120),
    "DIV_off_at_120ms_V": sample(.120)[3],
    "limits": "ideal_switch_has_no_real_tOFF; imposed_2uA_not_empirical_leak; no_ESP_clamp_model"}
with (out / "adc-isolation-reference-results.json").open("w", encoding="utf-8") as f:
    json.dump(result, f, indent=2)
with (out / "adc-isolation-reference-waveforms.csv").open("w", newline="", encoding="utf-8-sig") as f:
    w = csv.writer(f)
    w.writerow(["time_s", "pack_V", "vdd_V", "div_V", "adc_V"])
    # Keep a bounded readable output, retaining every 20th sample plus final.
    w.writerows(rows[::20] + [rows[-1]])
print(json.dumps(result, indent=2))
