# NeoCam v0.4: kiểm tra quyết định Q1 và sơ đồ

Ngày: 2026-11-06. Phạm vi: sửa thiết kế/schematic; không sửa kit thật.

| Kiểm tra | Kết quả |
|---|---|
| Script tính lại | PASS; current Q1 hot qualification budget100mOhm, old/new25C comparison preserved |
| Rà soát số học độc lập | PASS; drop/loss/coupled model/thermal/gateRC conditions |
| KiCad native ERC | 0 errors,0 warnings; no ignored checks |
| Native netlist + full review | PASS;9 exact review nets,QM1 G1/S2/D3,D2 K1/A2,R6,testpoints |
| Stock D2/R9 export | Main retains on_board=no/in_bom=no; full review copy changes only2 placement flags; normalized hashes identical |
| SVG/PNG | Final2520x1782 visually inspected;version0.4 and Q1 rework visible |
| Local links / URL whitespace | PASS |
| git diff --check | PASS; only Git LF/CRLF notices |

Current BOM: ../neocam-power-v0/bom-reference-v0.4.csv. OldCSV was write-locked and preserved.

Hot resistance/actual layout theta/USB transitions remain unmeasured; ERC is not thermal or hardware acceptance. No new QSPICE run was used as proof of Q1 thermal performance; archived ADC simulation is unrelated.

## Reproduce

```powershell
python concepts/neocam/tools/calculate-power-design.py
python concepts/neocam/neocam-power-v0/verify_netlist.py
```

## SHA256 snapshot

- `q1-design-decision.md`: `4f3cbe54c6976cbea4a2b9b2441b79599c8c5e032ae1d3424882caf2d904d3d2`
- `power-design-review.md`: `98cc93c048d472b18a5e6cbf5dd3b883ff28f2ebbd6326b3d5774928acecf6ef`
- `tools/calculate-power-design.py`: `a80ee42e6e893caacf78d2183cad817b907fcceee6c663a57d80328fd745fad5`
- `neocam-power-v0/neocam-power-v0.kicad_sch`: `441c5f0340e9ace9384c6837a2a9ac24602aa08b16d87e9c86b2d07d3a6b2a15`
- `neocam-power-v0/exports/neocam-power-v0-erc.rpt`: `e3c828b93296fa2c276ba62a692bedd5a5e12a1f57fdcba68ddbeb5cd92ef1f5`
- `neocam-power-v0/bom-reference-v0.4.csv`: `7dc394a8217e605b08bc52c59b6ee5bb494006a84eab16666af6293974176dff`
