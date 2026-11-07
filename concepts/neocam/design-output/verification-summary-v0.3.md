# NeoCam v0.3: kiểm tra bộ hồ sơ

Ngày: 2026-11-06. Không có đo phần cứng, nạp firmware hoặc duyệt chế tạo.

| Kiểm tra | Kết quả |
|---|---|
| Chạy calculate-power-design.py | PASS; ghi JSON và sáu bảng CSV, không dự báo thời lượng thực |
| KiCad 10 xuất schematic SVG/netlist | Thành công |
| ERC cuối | 0 errors, 0 warnings; Ignored checks: None |
| verify_netlist.py | PASS; 7 exact named-net checks, return PCM, fuse, LED, pogo isolation |
| Ảnh schematic cuối | Đã xem; 2520 x 1782, không chồng nhãn, chú thích trong khung |
| Link local và URL whitespace của tài liệu đầu vào | PASS |
| git diff --check trên tracked changes | PASS, chỉ có thông báo LF/CRLF của Git |
| QSPICE ADC alternative | Solver hoàn tất; read-qspice-reference.py đọc 12206 điểm, kết quả được lưu |

QSPICE chỉ kiểm tra phương án ADC lý tưởng KHÔNG CHỌN, không phải mô hình toàn NeoCam hoặc MAX17048. Các giới hạn nghiệm thu trong power-design-review.md còn mở.

## Tái lập

```powershell
python concepts/neocam/tools/calculate-power-design.py
python concepts/neocam/neocam-power-v0/verify_netlist.py
python concepts/neocam/tools/read-qspice-reference.py
```

## SHA256 của tài liệu thiết kế tại lần kiểm tra

- `power-design-review.md`: `42e1c9f728aeee08b04c1982673d6aa86b239902401a050ee033ffa6421e83d8`
- `tools/calculate-power-design.py`: `819b34d2b7dc3259a3c18b192634e15bf196035f8cddc7c149bd71807d4c8ed0`
- `neocam-power-v0/neocam-power-v0.kicad_sch`: `c14afed0c5eaa8ab9133001e8dd0f1ba841a66ebf8f3da5ccfbc28e122c5e701`
- `neocam-power-v0/exports/neocam-power-v0-erc.rpt`: `5a34f77cfdc66cda6893f331ffc887a7d5af504af3849ddc2e1b0d12616d889e`
- `neocam-power-v0/exports/neocam-power-v0.net`: `13155cace115539940f758a0b44912711afa2200ee25ae28973fef711d64598b`

Các hash ghi nhận snapshot, không thay nghiệm thu điện hoặc chức năng.
