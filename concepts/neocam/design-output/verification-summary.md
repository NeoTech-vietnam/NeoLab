# NeoCam v0.6: kiểm tra thiết kế rail 3V3

Ngày 2026-11-06. Phạm vi: tính toán, schematic và mô hình có điều kiện; chưa sửa kit, đo phần cứng, nạp firmware hoặc làm PCB.

| Kiểm tra | Kết quả |
|---|---|
| Tính lại DC/Q/C | PASS: DC available 3,241667 V; cửa sổ charge có điều kiện 3,11275–3,476 V |
| Rà soát độc lập | PASS: điều kiện datasheet, sai số tại 0 mA, Ceff chưa biết, Qtarget chưa được chứng minh, nguồn bộ nhớ riêng |
| KiCad ERC toàn bộ 3 trang | 0 lỗi, 0 cảnh báo; không bỏ qua kiểm tra |
| Native/full-review netlist | PASS: 14 net, TPS2121, pin map SGM6029, EN=VIN, VOS=OUT, L4, các tụ stock, ferrite SD, ba tap nguồn riêng |
| Xuất phần stock để review | Chỉ 11 cờ placement thay trong bản tạm; hash chuẩn hóa từng trang trùng; schematic gốc giữ loại khỏi BOM/board |
| BOM v0.6 | 38 dòng; thêm TP6/7/8 so v0.5, không thêm tụ bulk 3V3 |
| Ảnh schematic | Ba trang A3 PNG/SVG; trang buck được kiểm tra trực quan |
| QSPICE charge envelope | Exit 0; mô hình dòng áp đặt/RC, không phải control model SGM6029 |
| Averaged LC sensitivity | 30 trường hợp; luật feedforward trễ giả định; kiểm tra độ phân giải số 2 ns/1 ns, không xác nhận controller thật |
| Tài liệu và mã tính | 10 tài liệu không thiếu liên kết local/whitespace; 3 script syntax hợp lệ |

## Giới hạn kết luận

Giữ buck/L4/C5/C6 stock và bank VIN của v0.5; không thêm bulk ở OUT. Điều kiện Rpath<=0,5 Ohm, Ceff8–10uF, baseline có tải3,234–3,366V, Qminus<=0,25uC/Qplus<=0,40uC là yêu cầu cần đóng, chưa là đặc tính stock đã xác nhận.

Chưa chứng minh không reset. MPN/DCR/Isat của L4, Ceff/ổn định, đáp ứng điều khiển thật, tải/đỉnh SD, các rail tại chân ESP/SD và VDD_SPI còn mở. Mô hình không biến thời gian mux50us thành thời gian buck tắt. Thời lượng45phút và ghi tệp vẫn chưa được xác nhận.

Đọc [bài tính buck](../buck-3v3-design.md), [BOM](../neocam-power-v0/bom-reference-v0.6.csv), [ERC](../neocam-power-v0/exports/neocam-power-v0-erc.rpt). Bằng chứng v0.5 được giữ trong verification-summary-v0.5.md, schematic trước đó trong previous-v0.5/.

## SHA256 hiện hành

- `buck-3v3-design.md`: `6220468af6ed9c2d28aa2c87acf337d19747a3595d86b98f3cf5bf1804c2d2a1`
- `power-design-review.md`: `d8f6a28d531ea8a504d7903731ef6ddf91c2db40e06d72a38b389d5b2462c1af`
- `README.md`: `ef6435c2823794c87c7d5c477bc0f37f062bf452ad51f24c5bad939b9d38bd1e`
- `standalone-recording-power-budget.md`: `787090baed457a3edb864049cb152d3a25875e26d3aa148cfb1277109441cafb`
- `tools/calculate-buck-3v3.py`: `67f63812586bf47be0d7ab941eb887a219285c6eab746543b8d2c5641838132d`
- `tools/calculate-buck-plant-sensitivity.py`: `79d05a49b77095511cd815cb8b8e77ec74136ae27445b7219dde09fa33312c4c`
- `tools/read-buck-3v3-model.py`: `2014ec35378cdb32c9355dc87569ac2b7d02d675cdfa850eb47743267c32b4d6`
- `design-output/buck-3v3-calculations.json`: `7625d707d69eefa4bcf11ac48ca048b30e53cf5bffb92e63d18f942168f0cfd5`
- `design-output/buck-plant-sensitivity.json`: `a2650ba97c7ac512c7aad2d4f44a692f995996dff4806f263f22dc6e2e68f2ed`
- `design-output/buck-plant-sensitivity.csv`: `a76241bd73abed06db5dda1b1dc4a2a09e205ccfd66a01c46444407d768463c2`
- `design-output/buck-3v3-charge-envelope.cir`: `6a3b2d71667f2b60b25913362aaebf613f22f96450e93f825c8b0e9d6510164e`
- `design-output/buck-3v3-model-results.json`: `f223d19c7233c942aeb6b498840b7c6cb06204f759a64b761062699ab90ef966`
- `neocam-power-v0/neocam-power-v0.kicad_sch`: `71af8c084b02d79b5d82300148adb13f45e6ca5f38b5a4e6606bae5fa9fb821a`
- `neocam-power-v0/neocam-power-mux-v0.6.kicad_sch`: `d4306ec41473c0f3ba9831f9001e58d0c0692e1891142d3469da7a7d2cd17d29`
- `neocam-power-v0/neocam-power-3v3-v0.6.kicad_sch`: `87547ee53f0e61786ec62856d690187c7b3a0d0402c4c62fb5b91438813e195e`
- `neocam-power-v0/bom-reference-v0.6.csv`: `9fb6078d794c19c947f7a1612fcd4e61b843732ef3fcb9fb812bf7ee09587fce`
- `neocam-power-v0/exports/neocam-power-v0-erc.rpt`: `381592b782e683d15e06325f3a6e9f359573bfdd71e839e002c7241b5313227a`
- `neocam-power-v0/exports/connectivity-check.txt`: `ce839890ca545abafac791171c4a0d06d34b2fba7b9c0ccf6a7d0488f38e2377`
- `neocam-power-v0/exports/review-netlist-manifest.json`: `2ea9bbe16ee423f4055fb1a69173741fdbbf0fe26aac23e1773b046f4e918c9d`
- `neocam-power-v0/exports/neocam-power-v0-3V3_Buck_and_SD.png`: `a166babd1cac40723acd5e465e483dcd93f9ae1ac4aed1fe960fc22033ee359c`
- `references/buck-3v3-source-audit-2026-11-06.md`: `c186148fa1fb7aef0c0b80a3b29825fb95aa76339a6e857735d1697b417edef6`
- `references/buck-3v3-load-transient-audit-2026-11-06.md`: `2dee9c291fae8e2c508c969583d7fe980627ea98ed207676f5d59890c897797b`
