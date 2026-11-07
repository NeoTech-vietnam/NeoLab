# NeoCam: quyết định thay Q1 để giảm sụt áp và nhiệt

Ngày 2026-11-06. Đây là thiết kế thay đổi có điều kiện theo **mainboard Seeed V1.51**, chưa xác nhận revision bo thật, chưa sửa phần cứng hoặc đo. Quyết định đề xuất: **thay chức năng Q1 bằng DMP2035U-7**, giữ topology chọn nguồn pin/USB; định nghĩa biến thể sửa bo riêng, không mô tả bo stock đã có thay đổi này.

## 1. Xác minh Q1 hiện tại

Đã đọc toàn trang 4 [sơ đồ Seeed V1.51](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/new-res/202003753_XIAO%20ESP32S3%20Sense_V1.51_SCH_260611.pdf#page=4): Q1 ghi **LP0404N3T5G**, chân 1 D nối VBAT, chân 3 S nối VIN, chân 2 G nối VBUS; R9 = 100 kΩ từ VBUS xuống GND. D2 Schottky nối VBUS sang VIN. PMOS được mắc để body diode dẫn **BAT -> VIN**, chặn **VIN -> BAT** khi channel OFF.

[LRC LP0404N3T5G Rev.G Aug2024](https://www.lrc.cn/Upload/PDF/Product/MOS/LP0404N3T5G.pdf), đã render trang1-2, xác nhận đơn vị **ohm**, không phải milliohm:

| Điều kiện25°C | RDS(on)typ/max |
|---|---:|
| VGS=-4,5V, ID=-780mA | 0,34/0,48Ω |
| VGS=-2,5V, ID=-660mA | 0,45/0,67Ω |
| VGS=-1,8V, ID=-100mA | 0,72/0,95Ω |

Đặc tính RDS dùng pulse <= 300 µs, duty <= 2% để tránh tự nóng. PD 950 mW và thetaJA 131°C/W thuộc fixture FR4 1,5 x 1,5 với pad 1 sq in/2 oz Cu; không phải giới hạn nhiệt bo XIAO. Mạch thực cần xét RDS tăng theo nhiệt.

## 2. Linh kiện thay thế được chọn

[Diodes DMP2035U, DS31830 Rev11-2 June2025](https://www.diodes.com/datasheet/download/DMP2035U.pdf), trang 1-2: PMOS -20 V, VGS absolute ±10 V, SOT23, RDS max **45 mΩ tại VGS = -2,5 V/ID = -4 A/25°C**, 35 mΩ tại -4,5 V và 62 mΩ tại -1,8 V. Mã mua **DMP2035U-7**. SOT23 chuẩn: **G1, S2, D3**. Không dùng VGS(th) để kết luận MOS đã dẫn tốt.

Fixture nhiệt nhỏ hơn có PD 0,81 W/thetaJA 153,5°C/W trên FR4/2 oz Cu/pad tối thiểu hãng khuyến nghị; PD 1,2 W/theta 100°C/W chỉ với đồng 25 x 25 mm. Chưa có RDS max bảo đảm toàn dải nhiệt ở VGS thực. Đồ thị nhiệt là typical.

**Ngân sách được chọn để thiết kế:** RQ1_hot<=**0,090Ω**, tức2x45mΩ. Đây là **điều kiện nghiệm thu/dự phòng thiết kế**, không phải full-temperature maximum của datasheet. Phạm vi vận hành phải xác nhận hoặc thay linh kiện nếu không đạt. Ví dụ ở I=0,8A:

\[
\Delta V_{Q1}=IR;\qquad P_{Q1}=I^2R.
\]

| Mô hình cùng dòng0,8A | R | Sụt áp | Công suất |
|---|---:|---:|---:|
| LP0404 số tham chiếu25°C | 0,670Ω | 536mV | 428,8mW |
| DMP2035U số tham chiếu25°C | 0,045Ω | 36mV | 28,8mW |
| DMP2035U ngân sách nóng chọn | 0,090Ω | 72mV | 57,6mW |

Cùng điều kiện dòng/R tham chiếu, tổn hao giảm khoảng93,3% ở dòng so sánh25°C; không coi0,8A là dòng quay trung bình. Tại0,8A, Rhot0,09Ω vàthetafixture153,5°C/W cho deltaT≈8,84°C; bo/vỏ/dây adapter có thermal khác nên đây chỉ là kiểm tra mô hình, không phải nhiệt độ đã xác nhận.

Phương án **AO3401A** từ [AOS Rev3.1 Dec2023](https://www.aosmd.com/sites/default/files/res/datasheets/AO3401A.pdf) có RDSmax85mΩ tại-2,5V/25°C, vẫn tốt hơn LP nhưng cao hơn45mΩ. DMP2035UQ là biến thể automotive có cùng định mức25°C theo [trang hãng](https://www.diodes.com/part/view/DMP2035UQ), chưa có lý do cần trả thêm cho chứng nhận automotive; không tự xem chữQ là full-temperature RDS guarantee. ChọnDMP2035U-7 cho phương án này, không liệt kê thay thế tự do trong BOM.

## 3. Kết nối thay thế và cơ khí

Định nghĩa bản sửa bo: **tháo Q1 cũ**, nối adapter 4 net tới VBAT/VIN/VBUS/GND. Không hàn SOT23 lên footprint SOT883 như linh kiện thay trực tiếp.

| DMP2035U mới | Net | Nút trên sơ đồQ1 cũ |
|---|---|---|
| D3 | VBAT | padchức năngD1 |
| S2 | VIN | padchức năngS3 |
| G1 | VBUS | padchức năngG2 |
| AdapterGND | GND | ĐiểmGND thật xác minh |

Đối chiếu footprint, cực tính và vị trí chân trên bo thực trước khi sửa. Dây nguồn ngắn, đủ tiết diện; giữ tổng điện trở đã phân bổ từ pack đến BAT và từ adapter MOS đến VIN. Dây gate đi xa node chuyển mạch buck. PCB cuối có thể tích hợp SOT23 đúng footprint thay cho adapter.

Giữ R9 stock 100 kΩ, thêm **Radd 11 kΩ 1% từ gate/VBUS tới GND** trên adapter, cho:

\[
R_{pull}=100k\parallel11k=9,9099k\Omega.
\]

Đây gần tương đương thay R9 bằng 10 kΩ mà không cần tháo điện trở nhỏ trên bo. Khi USB 5 V: I gần 0,5045 mA, P gần 2,5227 mW tổng hai điện trở. Khi chỉ dùng pin và VBUS gần 0, không có dòng tĩnh 0,5 mA từ pin qua Rpull. Hai điện trở này không nằm trên đường dòng nguồn chính.

Một phương án vật lý khác là adapter 3 net và thay R9 stock thành 10 kΩ. Không trộn hai phương án trong BOM. Hai PMOS mắc song song đúng D/S/G về nguyên lý vẫn có thể làm việc, nhưng bản sửa được định nghĩa loại bỏ Q1 cũ để mô hình/BOM rõ. Không tuyên bố mắc song song tự động gây mất cách ly. **Không thay Q1 bằng jumper D-S**: sẽ mất điều khiển ngăn VIN USB đi trực tiếp về BAT.

## 4. Các trạng thái điện và chuyển nguồn

| Trạng thái | Cơ chế và giới hạn |
|---|---|
| Pin, không USB | Gate gần 0; body diode BAT -> VIN khởi tạo VIN, sau đó VGS âm làm channel ON; dòng chạy D -> S qua channel. VGS khoảng -VIN, không tự gán -VBAT nếu có sụt áp. |
| USB 5 V ổn định | D2 cấp VIN gần VBUS - VF; gate = VBUS nên VGS gần +VF, channel OFF; body diode hướng BAT -> VIN chặn VIN -> BAT. Sạc vẫn qua U4 stock. |
| Cắm USB | Trong ramp đơn điệu chậm, gate lên và tắt PMOS trước khi D2 nâng VIN trên BAT. Chưa bảo đảm mọi cạnh nguồn, nhiễu hoặc cáp. |
| Rút USB | Gate không xuống tức thì vì chung VBUS với tụ stock. Body diode có thể cấp tải trước khi channel ON, tạo sụt áp lớn tạm thời. |
| Cáp USB có GND nhưng VBUS = 0 | Gate được kéo xuống; D2 chặn VIN -> VBUS. Không mặc định cáp đang cắm nghĩa là USB đang cấp nguồn. |

DMP body diode có VF max 1,0 V tại 1 A/25°C; không thể hứa duy trì VIN 3,5 V trong giai đoạn diode từ pin 3,7 V. Qgate 15,4 nC typ tại điều kiện bảng khác LP 1,14 nC typ; không lấy nanosecond trong bảng switching làm thời gian chuyển nguồn ứng dụng.

Trên mainboard, C10 = 1 µF và C25 = 1 µF cùng VBUS (chưa kể cáp). Mô hình tải đã ngắt có C gần 2 µF: stock R 100 kΩ có tau gần 200 ms, Rpull 9,91 kΩ có tau gần 19,82 ms. Charger và tải qua D2 làm VBUS xả nhanh hơn ở một phần giai đoạn; đây không phải dự đoán thời gian chuyển nguồn. Rpull mới giảm trễ nhưng **không giải quyết dứt điểm hot-unplug**. Channel ON dẫn hai chiều, nên khi hai nguồn chồng lấn cần xét dòng ngược trước khi gate tắt; body diode hướng đúng chỉ bảo vệ khi channel OFF.

Nếu yêu cầu không reset khi cắm/rút USB là bắt buộc, cần thiết kế power MUX/ideal diode có kiểm soát hoặc hold-up được tính riêng. Không tuyên bố Q1 thấp R tự động sửa được mọi chuyển nguồn. Với chế độ quay bằng pin, cải thiện DC vẫn hữu ích và có căn cứ.

## 5. Cập nhật ràng buộc điện áp

Nếu Rexternal 0,100 Ω là đường từ pack P+/P- đã có PCM đến BAT XIAO, bỏ PCM khỏi Rexternal để tránh tính lặp. Đặt điện áp đầu nguồn là **Vpack_loaded**, không phải điện áp OCV của cell:

\[
V_{pack,required}=V_{IN,required}+I_{bat}(R_{external}+R_{Q1}).
\]

Với VIN required 3,5 V, eta 0,85, rail 3,3 V phân bổ 600 mA, camera phân bổ 150 mA và aux 2 mA, Ibat gần 0,81755 A như bản tham chiếu. Khi đổi RQ1 từ 0,670 Ω sang 0,090 Ω, Vpack required giảm **4,12951 V -> 3,65533 V** trong chính mô hình đó; R 25°C 0,045 Ω cho 3,61854 V. Các số này không chứng minh pin chạy 45 phút, buck giữ 3,3 V ở mọi tải hoặc Q1 thật của bo giống V1.51.

## 6. Kết luận và việc còn lại

**Chọn thay Q1 bằng DMP2035U-7 trên biến thể sửa bo**, kèm Radd 11 kΩ và đường nguồn đúng D/S/G. Phương án xử lý sụt áp và tổn hao DC lớn của Q1 tham chiếu; không chọn nối tắt. Trước thi hành: xác minh revision, pad/net, khoảng gắn và đúng linh kiện. Sau lắp: đo Rpath, sag, nhiệt và chuyển nguồn. Ngân sách Rhot 0,09 Ω là tiêu chí kiểm chứng, không phải thông số hãng bảo đảm. Pin/PCM, nhiệt cell, 45 phút, lưu khi pin yếu và hot-unplug vẫn là các yêu cầu riêng.
