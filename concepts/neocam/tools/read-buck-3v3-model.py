"""Read the project's imposed output-charge model, not a regulator simulation."""
from pathlib import Path
import csv
import json
import re
import struct

out = Path(__file__).resolve().parents[1] / "design-output"
data = (out / "buck-3v3-charge-envelope.qraw").read_bytes()
hraw, payload = data.split(b"Binary:\n", 1)
header = hraw.decode("utf-8")
nv = int(re.search(r"No\. Variables:\s+(\d+)", header)[1])
np = int(re.search(r"No\. Points:\s+(\d+)", header)[1])
assert "Flags: real" in header and len(payload) == np * nv * 8
names = [m[1].lower() for m in re.findall(r"^\s*(\d+)\s+(\S+)\s+\S+\s*$", header, re.M)]
assert len(names) == nv
rows = list(struct.iter_unpack("<" + "d" * nv, payload))
low = min(r[names.index("v(rail_lo)")] for r in rows)
high = max(r[names.index("v(rail_hi)")] for r in rows)
result = {"classification": "imposed_charge_RC_model_not_buck_line_transient_or_hardware_acceptance",
    "header": header, "rail_low_min_V_including_modeled_ESR": low,
    "rail_high_max_V_including_modeled_ESR": high,
    "load_low_min_V_after_branch30_ripple20_ring20mV_allowances": low - .030 - .020 - .020,
    "load_high_max_V_after_ripple20_ring20mV_allowances": high + .020 + .020,
    "not_modeled": ["actual VIN transition", "SGM6029 control loop or delays", "inductor and duty response",
        "actual ferrite/local decoupling", "VDD_SPI", "ESP32,SD,recording", "loaded DC voltage proof"]}
(out / "buck-3v3-model-results.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
with (out / "buck-3v3-model-waveforms.csv").open("w", newline="", encoding="utf-8-sig") as f:
    w = csv.writer(f)
    w.writerow(names)
    w.writerows(rows[::10] + [rows[-1]])
print(json.dumps(result, indent=2))
