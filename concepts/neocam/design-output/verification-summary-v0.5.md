# NeoCam v0.5: source handoff design verification

Date: 2026-11-06. Scope: schematic, calculation, imposed-bound simulation and document checks. No kit rework, firmware flashing, PCB layout or hardware acceptance performed.

| Check | Observed result |
|---|---|
| Reproducible arithmetic | PASS: 3.541667 V conditional SYS bound; 41.667 mV above chosen 3.5 V criterion |
| Independent numeric review | PASS: hold-up, divider corners, inrush and selection audit; no TI timing MAX invented |
| Native KiCad ERC, two sheets | 0 errors, 0 warnings; no ignored checks; rerun 08:43:56 local |
| Native/review netlists | PASS: 12 named nets; all TPS2121 pins including both OUT; controls/cap polarities/removals/retained R9 |
| Review netlist transparency | Only retained R9 placement flag changed in temporary review copy; source-normalized hashes identical; child source unchanged |
| Schematic exports | Both A3 sheets rendered to 2520x1782 PNG and SVG; visually inspected |
| QSPICE run | Exit 0, 0.103213 s; 15119 points; ideal source/RC envelope, not vendor IC or full-system model |
| Model minima, excluding layout | USB-to-BAT 3.591667 V; BAT-to-USB 3.591649 V; imposed 50 us gap and 20 uC reverse stress |
| Current links | Local links in report/master/README/Q1/budget/KiCad README checked |

## Meaning of PASS

The selected components and native schematic agree with the saved calculations. Conditions include IN2 >=3.8 V under load, normal SYS <=0.9 A, 1 A hold load, Ceff >=750 uF, ESR <=15 mOhm, total gap <=50 us, reverse charge <=20 uC, capacitor peak <=3 A and layout allowance <=20 mV. These are qualification requirements, not all guaranteed manufacturer maxima.

Actual source transitions, buck/3V3 response, no-reset/file integrity, capacitor temperature/endurance, USB current advertisement, charger/cell/PCM fault coordination and 45-minute runtime remain open. The 5 us TI figure is typical only. The bank is not an energy reserve for full file finalization after loss of both sources.

Current BOM: [v0.5](../neocam-power-v0/bom-reference-v0.5.csv). Q1 v0.4 calculations and prior verification were archived, not relabeled as the current mux design.

## Reproduce

```powershell
python concepts/neocam/tools/calculate-source-switchover.py
Push-Location concepts/neocam/design-output
& 'C:\Program Files\QSPICE\QSPICE64.exe' source-switchover-envelope.cir
Pop-Location
python concepts/neocam/tools/read-source-switchover-model.py
python concepts/neocam/neocam-power-v0/verify_netlist.py
```

## SHA256 snapshot

- `usb-battery-switchover-design.md`: `87f8d20363fd8e5ab39f59c1933075b4abf24b176754645702bd031fb28e4e34`
- `tools/calculate-source-switchover.py`: `e5f6687ce4f59aea6fb8e2b65767f69c258cd9b60e30268375045634cadad00b`
- `tools/read-source-switchover-model.py`: `92f1ba64c5a377c81db1c477dd8a86de0cb6c39e588487c389258bda2ae7666d`
- `design-output/source-switchover-calculations.json`: `5f6498d85a2e76ecd5de3bc3182bb6ea395755a88699081f94afc1866883f0c8`
- `design-output/source-switchover-envelope.cir`: `5dc8fb2f87224bee523d38bbb158cfa53b00d893d150f01367237ee5c3da8a24`
- `design-output/source-switchover-model-results.json`: `b34bd004d03671292228c47900c094bdd25e41dc93095b403bba63b68b037255`
- `neocam-power-v0/neocam-power-v0.kicad_sch`: `586b3f4fddf5d34a50ae720e993b8e93100499f7d3e915584c2069ecfb63566d`
- `neocam-power-v0/neocam-power-mux-v0.5.kicad_sch`: `fe9d33bde652fa06a996315d65aebb656ea959e752789268599a828ea8d07111`
- `neocam-power-v0/exports/neocam-power-v0-erc.rpt`: `af0c2871669bf906112709afc128c73a19384e062a5b870b33fc5f3529f8654a`
- `neocam-power-v0/exports/connectivity-check.txt`: `d03e3e5583e203e6e92550a723a19399b29ff085eb7e561bd2c281f974bb1e01`
- `neocam-power-v0/exports/review-netlist-manifest.json`: `15c1e236de74726106c88f514862f9f0c512a24211032a1e1e3904466b671171`
- `neocam-power-v0/bom-reference-v0.5.csv`: `fdb2dae19fe7f55d6a9594af1db4127d166ddd118959f62cc2ef791e1357f4ec`
- `power-design-review.md`: `8f5481dd0303a2276a376d2c4536f421df91f4da05b75385c2f1e3cf03fd2594`
- `README.md`: `76e2c6cc55930f256b6ccd9073d5b60f649bd9762cfab37fcf912d0db97b921b`
- `q1-design-decision.md`: `f05cd829f0129e6c5a2476a83ebc9c0395c88b846a0261cbcec5bd4c827c7a6a`
- `standalone-recording-power-budget.md`: `78c2a3c9dd48741190af759a26c75914b35ff20838145b5733904e97e4b9d428`
- `neocam-power-v0/README.md`: `fcd034a046f22fe9ecaf6bc55f1ce66f2caadf620383025bdd56dda4d4137674`
- `references/usb-battery-mux-source-audit-2026-11-06.md`: `a47714f18243ca71969a60d600ee3d4766bc9253b977e3296c016c80935a6c9f`
- `references/usb-battery-holdup-audit-2026-11-06.md`: `1dcced3c500fd5ad645634a88f10ddf2bc0cf795544a03c47065096747a2f2a4`
