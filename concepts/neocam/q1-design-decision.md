# NeoCam v0.4: quyết định thay Q1, tính sụt áp và nhiệt

> **Đã được thay bởi thiết kế v0.5 (2026-11-06).** Tài liệu này giữ nguyên lịch sử bài tính stock/DMP2035U. Cấu hình hiện hành dùng **TPS2121RUXR**, tháo Q1/D2 stock và bỏ QM1/R6 v0.4; giữ R9, charger và buck. Không áp cửa sổ pin3,7V hoặc kết quả nhiệt Q1 ở đây sang v0.5. Đọc [thiết kế chuyển nguồn USB/pin](usb-battery-switchover-design.md), [hồ sơ hiện hành](power-design-review.md) và [BOM v0.5](neocam-power-v0/bom-reference-v0.5.csv). Chưa sửa kit thật.

Ngày: **2026-11-06**. Quyết định thiết kế: **thay chức năng Q1 LP0404N3T5G bằng DMP2035U-7**, thêm điện trở kéo gate **R6 = 10 kOhm** trên mạch phụ trợ. Không sửa hoặc hàn board thật trong công việc này.

Đây là biến thể **XIAO có sửa đường Q1** theo sơ đồ chính V1.51. DMP2035U là SOT23, Q1 cũ là SOT883: **không thay trực tiếp cùng footprint**. Sơ đồ v0.4 biểu diễn adapter và các net cần nối; revision/pad của board thật phải được xác nhận trước khi thực hiện rework.

## 1. Vì sao cần một tiêu chí trước khi chọn MOSFET?

Giữ bài toán khảo sát đã có: VIN đầu vào buck cần ít nhất **3,5 V**, pack tại P+/P- đang có tải **3,7 V**, đường connector/fuse/dây từ pack tới BAT XIAO phân bổ **0,100 Ohm**. Dòng bình thường của đường pin không vượt **1 A** theo lựa chọn pack tham chiếu; không coi 1 A là dòng quay trung bình.

Tại điểm phân bổ 1 A, tổng headroom là 3,7 - 3,5 = 0,2 V. Đường ngoài pack dùng 1 x 0,1 = 0,1 V, nên Q1 chỉ còn **0,1 V**:

\[
R_{Q1,allow}\le\frac{3,7-3,5-1(0,100)}{1}=0,100\ Ohm.
\]

Do đó tiêu chí lựa chọn/nghiệm thu là:

\[
\boxed{R_{Q1,hot}\le100\ mOhm;\quad \Delta V_{Q1}\le100\ mV;\quad P_{Q1}\le100\ mW\quad @1A.}
\]

Đây là **ngân sách thiết kế**, không phải thông số maximum mới của nhà sản xuất. VIN=3,5 V vẫn là tiêu chí khảo sát của bài toán Q1; khả năng buck giữ đủ rail/đáp ứng xung ở mức đó phải được đóng riêng. Không dùng 3,7 V danh định để thay cho điện áp pack có tải khi tính thời lượng.

## 2. Dữ liệu đúng linh kiện

| Đại lượng | Q1 cũ LP0404N3T5G | Q1 mới DMP2035U-7 |
|---|---|---|
| RDS(on) tối đa tại VGS=-2,5 V, 25 C | 0,670 Ohm, thử tại660 mA | **0,045 Ohm**, thử tại4 A |
| RDS(on) tối đa tại VGS=-4,5 V, 25 C | 0,480 Ohm | 0,035 Ohm |
| VDS / VGS absolute maximum | -20 V / +/-6 V | -20 V / +/-10 V |
| Package / chân | SOT883, D1/G2/S3 theo sơ đồ Seeed | **SOT23, G1/S2/D3** |
| Nhiệt trở công bố | 131 C/W trên fixture FR4/vùng đồng lớn của LRC | 153,5 C/W trên FR4/2oz/minimum pad;100 C/W khi đồng25x25 mm |

Nguồn: [LRC LP0404N3T5G Rev.G](https://www.lrc.cn/Upload/PDF/Product/MOS/LP0404N3T5G.pdf), [Diodes DMP2035U DS31830 Rev.11-2, June2025](https://www.diodes.com/datasheet/download/DMP2035U.pdf), [Seeed V1.51 trang4](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/new-res/202003753_XIAO%20ESP32S3%20Sense_V1.51_SCH_260611.pdf#page=4).

Các trị số R trên được công bố tại điểm kiểm tra25 C, không là bảo đảm mọi dòng/nhiệt độ. Bản mới có đồ thị nhiệt typical nhưng không có maximum Rhot toàn lô. Vì vậy chọn **100 mOhm làm giới hạn cần nghiệm thu**, có tỷ số100/45=2,22 so với điểm maximum25 C; không viết rằng hệ số2,22 được hãng bảo đảm.

## 3. So sánh tại cùng dòng điện

Điện áp trên phần dẫn của MOSFET và nhiệt Joule:

\[
\Delta V=IR;\qquad P=I^2R.
\]

| Dòng qua Q1 | Cũ, R tham chiếu0,670 Ohm | Mới, R tham chiếu25 C0,045 Ohm | Mới, nếu đạt ngân sách nóng0,100 Ohm |
|---:|---:|---:|---:|
| 0,25 A | 167,5 mV /41,88 mW | 11,25 mV /2,81 mW | 25 mV /6,25 mW |
| 0,50 A | 335 mV /167,5 mW | 22,5 mV /11,25 mW | 50 mV /25 mW |
| 0,80 A | **536 mV /428,8 mW** | **36 mV /28,8 mW** | **80 mV /64 mW** |
| 1,00 A | 670 mV /670 mW | 45 mV /45 mW | 100 mV /100 mW |

Bảng dùng mô hình trở để so sánh, một số dòng khác điểm kiểm tra hãng; không gọi đó là bảng maximum được bảo đảm của board. Tại cùng dòng, tỷ số tổn hao mới/cũ là45/670≈6,72% tại điểm25 C; phần giảm khoảng**93,3% chỉ của Q1**, không phải giảm93,3% điện năng toàn NeoCam. Dùng ngân sách100 mOhm thì phần giảm cùng dòng là85,1%.

Nếu giữ Q1 cũ trong mô hình0,670 Ohm, điều kiện sụt<=100 mV yêu cầu:

\[
I\le0,100/0,670=0,1493\ A.
\]

Ngay trường hợp rail3V3 phân bổ100 mA đã cho tổng dòng pack khoảng263 mA ở VIN3,5 V khi camera/phụ cũng hoạt động. Vì vậy **giữ Q1 cũ không đáp ứng ngân sách sụt áp đã chọn** cho các cấu hình tải đang khảo sát. Điều này không nói kit Seeed bị lỗi hoặc không thể quay ở tải thấp hơn; quyết định dựa vào mục tiêu đường nguồn NeoCam.

## 4. Ghép với buck: dòng vào thay đổi theo sụt áp

Định nghĩa Vpack tại P+/P- sau PCM; Rext=0,100 Ohm không chứa nội trởcell/PCM đã nằm phía trước điểm đo. Phân bổ Icam=0,150 A, Iaux=0,002 A, eta>=0,85; I3V3 là tổng rail, không phải tải thêm ngoài MCU.

\[
I=0,152+\frac{3,3I_{3V3}}{0,85 V_{IN}},\qquad
V_{IN}=V_{pack}-I(R_{ext}+R_{Q1}).
\]

Đặt I0=0,152, k=3,3I3V3/0,85, R=Rext+RQ1:

\[
V_{IN}=\frac{V_{pack}-RI_0+\sqrt{(V_{pack}-RI_0)^2-4Rk}}{2}.
\]

Chỉ dùng nghiệm như trạng thái làm việc khi buck còn đáp ứng giả định ổn áp. Nghiệm VIN<3,3 V không mô tả một buck vẫn tạo được3,3 V; nó cho thấy giả định đã mất hiệu lực.

### 4.1. Điện áp pack cần để giữ VIN3,5 V

| Phân bổ rail3V3 | Dòng pack tạiVIN3,5 V | Giữ Q1 cũ0,670 Ohm | Mới25 C0,045 Ohm | Mới ngân sách nóng0,100 Ohm |
|---:|---:|---:|---:|---:|
| 100 mA | 262,9 mA | 3,7025 V | 3,5381 V | 3,5526 V |
| 200 mA | 373,8 mA | 3,7879 V | 3,5542 V | 3,5748 V |
| 300 mA | 484,8 mA | 3,8733 V | 3,5703 V | 3,5970 V |
| 600 mA | 817,5 mA | **4,1295 V** | **3,6185 V** | **3,6635 V** |

Tại600 mA rail, bản mới mở thêm khoảng4,1295-3,6635=**0,466 V headroom pack** trong mô hình ngân sách nóng. Không biến khoảng này thành số phút quay tăng thêm khi chưa có đường cong xả/tải.

### 4.2. Ví dụ ở pack3,7 V, rail600 mA

| Trường hợp | VIN tính được | Dòng pack tính được | Tổn hao Q1 | Ý nghĩa |
|---|---:|---:|---:|---|
| Cũ0,670 Ohm | 2,9813 V | 933,3 mA | 583,6 mW | Nghiệm không phù hợp giả định buck3,3 V; không dùng như dòng thật |
| Mới0,045 Ohm25 C | 3,5837 V | 802,0 mA | 28,94 mW | Đạt tiêu chí VIN3,5 V của mô hình |
| Mới ngân sách0,100 Ohm | **3,5379 V** | **810,4 mA** | **65,68 mW** | Đạt nếu các phân bổ/hiệu suất/Rhot thực đáp ứng |

Điểm pack3,6 V/rail600 mA vẫn không đạt tiêu chí VIN3,5 V ở trường hợp mới ngân sách0,100 Ohm. Không tuyên bố thay Q1 làm hệ chạy xuống mọi mức điện áp cell.

## 5. Tính nhiệt và yêu cầu PCB/adapter

Tại dòng thiết kế tối đa1 A và Rhot<=0,100 Ohm, P_Q1<=0,100 W. Quan hệ xác lập:

\[
T_J=T_{A,local}+P_{Q1}\theta_{JA,effective}.
\]

Chọn mục tiêu thiết kế cho riêng MOSFET **TJ<=85 C**; đây không phải giới hạn absolute150 C của chip hay nhiệt độ vỏ/cell được phép. Với Ta_local là nhiệt độ tại vùng mạch, không phải luôn bằng nhiệt độ phòng:

| Ta_local khảo sát | Theta hiệu dụng tối đa để TJ<=85 C @0,1 W | TJ nếu dùng fixture153,5 C/W của hãng |
|---:|---:|---:|
| 25 C | 600 C/W | 40,35 C |
| 40 C | 450 C/W | 55,35 C |
| 60 C | **250 C/W** | **75,35 C** |

Nhiệt trở fixture không tự là nhiệt trở adapter nhỏ trong vỏ. **Yêu cầu nghiệm thu adapter là theta hiệu dụng<=250 C/W tại điểm Ta_local60 C/P0,1 W**, hoặc chứng minh trực tiếp TJ đáp ứng ở cấu hình tương đương. Các nhiệt độ khảo sát không công bố dải nhiệt sản phẩm hay cho phép sạc cell ở60 C.

Ví dụ cùng0,8 A: cũ P0,4288 W nhân fixture131 C/W cho deltaT56,17 C; mới ngân sách P0,064 W nhân153,5 cho deltaT9,82 C. Hai fixture khác nhau; đây là phép tính độ nhạy, không phải đo XIAO/adapter. Tổn hao diode thân trong khởi động/chuyển nguồn chưa nằm trong I2R xác lập.

Yêu cầu bố trí: MOSFET trên PCB/adapter có land patternSOT23 đúng hãng, vùng đồng/đường dòng không bóp cổ, nối BAT/VIN ngắn; phần nối thêm vẫn phải nằm trong ngân sách Rext<=0,100 Ohm. Không để dây hoặc pad mới làm mất lợi ích RDS(on). Chưa route PCB trong quyết định này.

## 6. Mạch thay đổi cụ thể

```text
VBAT_XIAO ---- D(3) QM1 DMP2035U-7 S(2) ---- VIN_XIAO
                         G(1)
                           |
                       VBUS_XIAO ---- D2 stock ----> VIN_XIAO
                           |
                     R9 stock100k
                           |
                          GND

Thêm R6=10k từ VBUS_XIAO/G(1) xuống GND, song song R9.
Charger/buck stock được giữ; Q1 stock bị tháo ở biến thể này.
```

| DMP2035U mới | Nối tới net / pad Q1 cũ theo SeeedV1.51 |
|---|---|
| Gate1 | VBUS, pad Gate2 cũ |
| Source2 | VIN, pad Source3 cũ |
| Drain3 | VBAT, pad Drain1 cũ / BAT+ cùng net |

Diode thân hướng **VBAT -> VIN**. Không đảo Source/Drain và không dùng jumper nối tắt D-S. Hai PMOS song song đúng cảD/S/G về nguyên lý có thể làm việc; bản này chọn tháo Q1 cũ để mô hình và cấu hình một MOSFET rõ ràng, không vì mắc song song tự động làm mất bảo vệ. Đặt MOSFET mới **nối tiếp** mà vẫn giữ trở Q1 cũ sẽ không giải bài toán.

Giữ D2/R9 trên XIAO, chỉ biểu diễn chúng bên ngoài khối U1 để đọc đường nguồn; không mua/lắp thêm một D2/R9 trùng. Thêm test pointBAT/VIN/VBUS/GND để kiểm tra sau; chúng không mang ý nghĩa đã đo.

## 7. Vì sao thêm R6 kéo gate?

Giữ R9 stock100k và thêm R6=10k,1% tránh phải thay điện trở nhỏ trên XIAO:

\[
R_{pd}=100k\parallel10k=9,091\ kOhm.
\]

Gate leakage hãng cho±10 uA tại VGS±8 V/25 C. Trong điều kiện khảo sát giữ leakage<=10 uA, lấy riêng R6+1% đã chặn Rpd<=10,1k, nên gate lệch<=0,101 V. Nếu VIN>=3,5 V, độ lớn VGS còn ít nhất3,399 V, đủ lớn hơn điểm drive2,5 V dùng để chọn RDS(on). Điều kiện leakage/nhiệt thực vẫn phải xác nhận; không dùng VGS(th) như điện áp đã dẫn đủ tải.

Ở USB5 V, R6 tiêu thụ thêm **0,5 mA /2,5 mW**; tại5,25 V/R-1% điện trở tỏa2,784 mW, rất thấp so với điện trở0603>=0,1 W. Khi chỉ dùng pin và VBUS gần0, R6 không tự rút0,5 mA từ pack; còn leakage thực của đường gate phải tính riêng.

VBUS có khoảng2 uF tụ stock. Trong mô hình tụ xả chỉ qua pulldown: tau giảm từ100k x2uF=**200 ms** xuống9,091k x2uF=**18,18 ms**. Đây không phải thời gian chuyển nguồn thực, vì tải/charger/D2 cũng tham gia.

**Chuyển USB/pin không reset chưa được giải quyết trọn bởi phép tính DC.** DMP có gate charge và diode thân khác; khi rút USB, gate có thể còn cao và tải đi qua diode trước khi channel dẫn. Cần phân tích/kiểm chứng quá độ riêng nếu muốn vừa quay vừa cắm/rút. Không dùng switching-time nanosecond trong datasheet để khẳng định mạch VBUS100k/10k chuyển nhanh như vậy.

## 8. Trạng thái sau quyết định

**Đã chốt:** thay Q1 trong bản thiết kếv0.4, part/pinmap/adapter, R6, tiêu chí Rhot/drop/nhiệt và các phép tính DC. Sơ đồ/BOM/netlist được cập nhật; số liệu tái lập từ [script](tools/calculate-power-design.py).

[Mở schematic KiCad](neocam-power-v0/neocam-power-v0.kicad_sch) · [SVG hiện hành](neocam-power-v0/exports/neocam-power-v0.svg) · [BOM v0.4](neocam-power-v0/bom-reference-v0.4.csv) · [ERC](neocam-power-v0/exports/neocam-power-v0-erc.rpt).

**Chưa đóng:** revision/pad thật và thực hiệnrework; Rhot/theta/nhiệt adapter thực; chuyển USB/pin; khả năng buck tại VIN đã chọn; pin/PCM/sạc, dòng tác vụ và45 phút. Không sửa kit thật, không mua linh kiện và không tuyên bố đạt45 phút trong lượt này.

Các tệp dữ liệu:

- [So sánh cùng dòng](design-output/q1-fixed-current-comparison.csv).
- [Điện áp pack cần thiết](design-output/q1-required-pack-voltage.csv).
- [Quét nghiệm ghép](design-output/q1-coupled-comparison.csv).
- [Khảo sát nhiệt](design-output/q1-thermal-envelope.csv).
- [JSON toàn bộ phép tính](design-output/calculations.json).

Hai báo cáo audit có khảo sát67,5/90mOhm vàR11k là phương án đầu vào. **Quyết định hiện hành dùng Rhot nghiệm thu100mOhm và R6=10k** như tài liệu này; không trộn các con số của ba phương án.
