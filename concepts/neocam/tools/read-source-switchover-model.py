"""Post-process this project's real all-double QSPICE handoff envelope only."""
from pathlib import Path
import csv
import json
import re
import struct

base = Path(__file__).resolve().parents[1]
out = base / "design-output"
data = (out / "source-switchover-envelope.qraw").read_bytes()
hraw, payload = data.split(b"Binary:\n", 1)
header = hraw.decode("utf-8")
nvars = int(re.search(r"No\. Variables:\s+(\d+)", header)[1])
npoints = int(re.search(r"No\. Points:\s+(\d+)", header)[1])
assert "Flags: real" in header and len(payload) == npoints * nvars * 8
names = [m[1].lower() for m in re.findall(r"^\s*(\d+)\s+(\S+)\s+\S+\s*$", header, re.M)]
assert len(names) == nvars
rows = list(struct.iter_unpack("<" + "d" * nvars, payload))
sys_index = names.index("v(sys)")
events = []
for name, start, end in (("USB_to_BAT_post_coast_envelope", .0002, .0003),
                         ("BAT_to_USB_envelope", .001, .0011)):
    measured = min(row[sys_index] for row in rows if start <= row[0] <= end)
    events.append({"event": name, "model_SYS_min_V_excluding_ESL": measured,
        "model_min_minus_20mV_layout_allowance_V": measured - .020,
        "meets_SYS3V5_in_this_imposed_model": measured - .020 >= 3.5})
result = {"classification": "imposed_bound_RC_model_not_actual_TPS2121_or_ESP32_validation",
    "header": header, "events": events,
    "limits": ["50us blackout imposed", "20uC reverse current imposed", "ideal unidirectional sources",
        "no comparator threshold,SS,RCB delay,full buck,MCU,SD,charger or USB cable model",
        "20mV ESL allowance subtracted after simulation,not a parasitic model"]}
(out / "source-switchover-model-results.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
with (out / "source-switchover-model-waveforms.csv").open("w", newline="", encoding="utf-8-sig") as f:
    w = csv.writer(f)
    w.writerow(names)
    w.writerows(rows[::5] + [rows[-1]])
print(json.dumps(result, indent=2))
