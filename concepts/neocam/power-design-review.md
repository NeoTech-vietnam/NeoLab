# NeoCam: hồ sơ thiết kế nguồn để đọc và duyệt

Ngày: **2026-11-06**. Phiên bản tham chiếu hiện hành **v0.6**: kế thừa TPS2121RUXR/bank tụ VIN của v0.5, phân tích thêm buck 3V3 và các nhánh nguồn tại ESP/microSD/bộ nhớ.

**Quyết định v0.6:** giữ **SGM6029C, L4 và mạng tụ stock**, không thêm tụ bulk 3V3; thể hiện buck và nhánh microSD ở trang KiCad riêng, thêm điểm truy cập 3V3 tại kit/ESP, SD sau ferrite và VDD_SPI. Đường chọn nguồn vẫn tháo Q1/D2 và bỏ QM1/R6 v0.4, dùng **TPS2121RUXR**, giữ R9/charger. Ba tụ **T520X337M010ATE025, mỗi tụ330µF/10V** nằm ở **VIN_SYS trước buck**. Đọc [thiết kế buck v0.6](buck-3v3-design.md) và [thiết kế chuyển nguồn kế thừa v0.5](usb-battery-switchover-design.md). [Bài tính Q1 v0.4](q1-design-decision.md) là lịch sử. Chưa rework hoặc đo kit thật; **chưa đủ bằng chứng stock buck giữ mọi rail không reset khi quay**.

**Kết quả của hồ sơ này:** sơ đồ nguyên lý phụ trợ hoàn chỉnh ở mức kết nối với XIAO có sẵn, bảng lựa chọn, công thức và dữ liệu tính lại được. **Chưa phải:** PCB sản xuất, pin được duyệt mua/lắp, hiệu năng quay hoặc thời lượng đã được chứng minh. Những mục cần đóng trước chế tạo/nghiệm thu được liệt kê ở phần 11.

## 1. Mục tiêu đang giải quyết

Thiết kế nguồn cho **một NeoCam hoạt động độc lập**: dùng pin riêng, nhấn nút để quay hình có tiếng, lưu trên microSD; nhấn dừng hoặc pin yếu thì hoàn tất tệp trước khi nghỉ. Mục tiêu quay thường là **1280 x 720, 30 FPS, tối thiểu 45 phút**, Wi-Fi/BLE tắt. Đây là yêu cầu sản phẩm, chưa là kết quả đạt.

Mục tiêu trước đó vừa ghi có tiếng vừa xem trước **720p/12 FPS không tiếng qua Wi-Fi Station** vẫn còn, được đánh giá riêng; không lấy thời lượng của chế độ tắt radio áp sang chế độ đó.

| Đầu vào | Quyết định / mức xác nhận |
|---|---|
| Board | XIAO ESP32-S3 của người dùng, FCC ID Z4T-XIAOESP32S3 |
| Camera | OV3660, người dùng xác nhận |
| Bo Sense thật | v1.1 in trên bo microSD/microphone |
| Mô hình bo mở rộng | **Exp. Board v1.2**, người dùng đồng ý dùng làm tham chiếu |
| Mô hình bo chính | V1.51 từ Seeed, **thay đường Q1/D2 bằng TPS2121**; chưa xác nhận pad/revision bo thật |
| Lưu trữ | SanDisk Ultra 32 GB; chưa có SKU đầy đủ |
| Firmware | ESP-IDF; chưa viết/nạp firmware trong công việc này |
| Cơ khí | Hướng tới phần camera GO 3; chưa có bản vẽ chứng minh lắp vừa |
| Điều kiện tính | Các ví dụ điện trở/nhiệt ghi rõ điều kiện; chưa công bố dải nhiệt sản phẩm |

Không thay camera bằng OV5640 vì tài liệu mới dùng sensor khác. Không mặc định có chống rung, H.264 phần cứng, chống nước hoặc mọi chức năng GO 3.

## 2. Đọc hồ sơ theo thứ tự nào?

1. Đọc tài liệu này: mục tiêu, lựa chọn, kết quả và điểm phải quyết định.
2. Đọc [thiết kế buck v0.6](buck-3v3-design.md), [thiết kế chuyển nguồn v0.5](usb-battery-switchover-design.md), rồi mở [sơ đồ KiCad](neocam-power-v0/neocam-power-v0.kicad_sch), [trang mux v0.6](neocam-power-v0/neocam-power-mux-v0.6.kicad_sch), [trang buck v0.6](neocam-power-v0/neocam-power-3v3-v0.6.kicad_sch) và [hướng dẫn sơ đồ](neocam-power-v0/README.md). Bài tính Q1 v0.4 là tài liệu lịch sử.
3. Đọc [ngân sách tải chi tiết](standalone-recording-power-budget.md), [audit buck/bo chính](references/buck-3v3-source-audit-2026-11-06.md), [audit tải động](references/buck-3v3-load-transient-audit-2026-11-06.md), [thẩm định pin/PCM](references/battery-pcm-selection-audit-2026-11-06.md).
4. Xem [phép tính buck JSON](design-output/buck-3v3-calculations.json)/[script buck](tools/calculate-buck-3v3.py), [phép tính chuyển nguồn JSON](design-output/source-switchover-calculations.json)/[script chuyển nguồn](tools/calculate-source-switchover.py), [kết quả số nền](design-output/calculations.json) và [script tính nền](tools/calculate-power-design.py).

Các ghi chú cũ được giữ làm lịch sử. Khi khác nhau, **thiết kế buck/BOM/sơ đồ v0.6 cùng các điều kiện chuyển nguồn v0.5 được kế thừa** là cơ sở hiện tại. Các giá trị RQ1 0,670Ω/0,045Ω/0,100Ω và cửa sổ pin của v0.4 không được dùng như kết quả hiện hành; 1A dự trù chưa phải dòng quay đã đo.

## 3. Kiến trúc và ranh giới mạch

```text
Protected pack 1S -> JST PH -> fuse -> VBAT_XIAO -> TPS2121 IN2
                                        |              |
                                        +-> MAX17048   +-> SYS / VIN XIAO -> buck3V3 -> ESP/mic/SD
                                        |              |                 +-> hai LDO -> camera
                                        +-> BAT pads   +-> 3 x 330uF -> GND

XIAO USB-C / VBUS -> TPS2121 IN1
                 +-> charger stock -> BAT
Nút quay -> GPIO2; LED trạng thái <- GPIO4
```

**Giữ mạch sạc/buck và R9 trong XIAO; tháo Q1 và D2 chọn nguồn stock**, không để chúng bypass TPS2121. Không lắp QM1/R6 của v0.4 hoặc diode fallback. TPS2121 tự chọn nguồn bằng PR1/CP2, không chờ firmware; SS điều khiển khởi động, ILIM đặt giới hạn dòng. Tất cả GND là **P- phía ngoài PCM**, không nối B- của cell bên trong pack. Đầu nối có khóa không tự phát hiện dây crimp ngược cực.

USB-C trên XIAO là đường sạc hoạt động của bản tham chiếu. Pogo sau lưng được giữ thành **giao diện DNP, chưa nối nguồn**; không đưa 5 V vào BAT. Đưa pogo vào sử dụng cần thêm thiết kế chọn/cách ly nguồn 5 V và chống cắm sai. Đây là một đặc điểm sản phẩm còn tồn đọng, không bị coi là đã hoàn thành bởi ký hiệu DNP.

Không thêm công tắc cắt pin cứng làm thao tác dừng quay: nút quay là lệnh cho phần mềm đóng tệp. Khi tháo pack, nguồn vẫn mất đột ngột; chưa có UPS/supercapacitor bảo đảm cứu tệp.

## 4. Các lựa chọn linh kiện

| Chức năng | Lựa chọn v0.6 | Lý do / giới hạn |
|---|---|---|
| Bộ pin tham chiếu điện | **DNK102050, 1S 1000 mAh, có PCM** | Có datasheet đúng pack, 1 A liên tục; **chưa duyệt phối hợp sạc, mua hàng hoặc cơ khí** |
| Pin giảm chi phí/kích thước | Generic 102040 800 mAh | Giữ làm ứng viên; chưa có model/datasheet pack đủ để thay pin tham chiếu |
| Đầu nối | JST **B2B-PH-K-S / PHR-2 / SPH-002T-P0.5S**, AWG24 | Quy ước chân 1=P+, chân 2=P- của NeoCam; cần kiểm tra dây thật |
| Cầu chì thứ cấp | **Littelfuse 046701.5NR**, 1,5 A, 0603 | Trở thấp; không thay PCM, không giới hạn dòng tức thời ở 1,5 A |
| Đo điện áp pin | **MAX17048G+**, TDFN 2 x 2 mm | Đọc điện áp qua I2C; bỏ đường pin analog trực tiếp vào ADC |
| Tụ IC đo pin | 100 nF, X7R, >=10 V | Decoupling ngay sát VDD/GND |
| Pullup SDA/SCL | Hai điện trở **2,2 kOhm, 1%**, lên 3V3 XIAO | Bus chọn 100 kHz, ngân sách điện dung <=150 pF |
| Pullup ALRT/nút | 10 kOhm, 1%, lên 3V3 | Ngõ vào active-low |
| Nút quay | **Omron B3U-1000P**, thường hở, SMD; 100 nF lọc | RC 1 ms, phần mềm vẫn phải chống dội và giải mã nhấn giữ |
| LED | LED đỏ + **2,2 kOhm** | Dòng minh họa khoảng 0,59 mA tại Vf=2 V; chưa chốt độ sáng/mã LED |
| Nguồn USB tham chiếu | **5V/3A có xác nhận**, 4,75-5,25V tại MUX, cáp phù hợp | Không tự mặc định cổng PC hoặc đầu USB-C cấp được3A; không đưa9V vào XIAO |
| Linh kiện giữ trên XIAO | SGM40567-4.2, SGM6029C, L4/C3–C6/R13, R9 | Giữ charger/buck stock; Q1/D2 tháo; L/C chưa rõ MPN/Ceff/DCR |
| Chọn nguồn | **TPS2121RUXR**, RUX12 | IN1 USB, IN2 VBAT_XIAO; không lắp QM1/R6 v0.4 |
| PR1 và CP2 | Mỗi nhánh **20kΩ/10kΩ,1%** | XCOMP so sánh USB/pin, USB được ưu tiên trong miền đã tính |
| ILIM / SS | **80kΩ,1% / 1µF X7R10V** | Bảng ILIM tại80k:1/1,5/2A; slew200V/s là điều kiện thiết kế, không phải max hãng |
| Tụ SYS | **3×T520X337M010ATE025**, mỗi tụ330µF/10V | Qualification: Ceff>=750µF, ESR<=15mΩ; 990µF nominal không tự là Ceff |
| Nguồn camera trên Sense | SGM2036S 2,8 V và 1,5 V | Theo mô hình v1.2, không mua/lắp thêm bản sao |
| Nguồn 3V3 và nhánh SD | Giữ U3/L4/C3–C6/R13 và FB1/C8/C9 stock | Không lắp tụ bulk 3V3; ký hiệu stock không phải linh kiện mua thêm |
| Điểm truy cập nguồn | 3V3 tại kit/ESP, SD sau FB1, VDD_SPI | Kiểm tra từng nhánh; không cấp ngoài hoặc nối tắt VDD_SPI |

BOM hiện hành là [bom-reference-v0.6.csv](neocam-power-v0/bom-reference-v0.6.csv); pin map nằm trong thư mục KiCad. BOM v0.4/v0.5 giữ làm lịch sử. Chưa có báo giá hiện hành; không lấy listing cũ làm tổng giá sản xuất. MAX17048 phục vụ đo pin; phương án chia áp rẻ hơn chỉ quay lại nếu giải được sai số và trình tự mất nguồn.

## 5. Ngân sách tải: biết gì, chưa biết gì?

| Khối | Dữ liệu sử dụng | Cách đưa vào bài toán |
|---|---|---|
| ESP32-S3 | 81,3 mA typical ở 3,3 V trong bài tải 240 MHz/hai lõi/32-bit/clocks ngoại vi bật | 268,29 mW tại điểm đó; chưa phải dòng tác vụ, maximum hoặc toàn bộ board/R8 |
| Camera | AVDD 28 mA @2,8 V; DVDD 64 mA @1,5 V; DOVDD 6 mA @2,8 V | Cùng điểm full-size/full-speed/compression, typical |
| SanDisk Ultra | SKU chưa đủ; không có Iwrite/peak của đúng thẻ | Dùng ngân sách chuẩn **0,36 W** có điều kiện Default Speed <=25 MHz, trung bình 1 s; không dùng làm giới hạn xung |
| Mic MSM261D3526H1CPM | 670 uA typical @1,8 V, clock2,4 MHz | Board cấp 3,3 V; giữ dòng đúng điều kiện thành biến chưa có |
| MAX17048 | Active tối đa 40 uA theo điều kiện bảng | Tải trực tiếp ở BAT; pullup I2C/ALRT tính ở nhánh 3V3 |
| Bộ nhớ, LED, GPIO và tổn hao | Chưa đủ dòng tác vụ/đường cong toàn điều kiện | Không tự gán bằng 0; nằm trong tổng nhánh 3V3 và tiêu chí cần xác nhận |

Nguồn và điều kiện đầy đủ trong [ngân sách tải](standalone-recording-power-budget.md). **Không cộng các điểm typical và phân bổ chuẩn khác điều kiện rồi đặt tên là maximum hay dòng trung bình NeoCam.** Có thể dùng chúng để lập mô hình và yêu cầu xác nhận.

### 5.1. Camera và tổn hao LDO

Theo [OV3660 bảng 8-3](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/res/OV3660_datasheet.pdf#page=154) và cây nguồn v1.2:

\[
P_{sensor}=2,8(0,028+0,006)+1,5(0,064)=0,1912\ W
\]

Hai LDO cùng lấy VCC_IN; khi ổn áp, dòng đầu vào xấp xỉ dòng đầu ra cộng dòng nuôi IC:

\[
I_{cam,in}\simeq34+64=98\ mA\quad\text{(chưa gồm overhead)}.
\]

Tại VCC_IN=3,7 V, ví dụ tham chiếu cho Pin=0,3626 W và nhiệt LDO=0,1714 W. Tại VCC_IN=5 V, cùng điểm tải cho Pin=0,4900 W và nhiệt=0,2988 W. Nguồn USB có thể làm LDO nóng hơn dù camera không đổi tải. Không lấy P_sensor/Vbat thay cho dòng nhánh LDO này. Các giá trị chưa đại diện 720p/30 FPS.

Sơ đồ v1.2 còn tên net 1V3 nhưng mã U2 và ghi chú sửa đổi là 1,5 V; mô hình dùng 1,5 V có ghi rõ. Camera/mic/SD không có công tắc nguồn riêng trên sơ đồ này, nên ESP sleep không đồng nghĩa cả NeoCam đã tắt.

### 5.2. Lịch sử tính sụt áp Q1 trước v0.5

**Mục này giữ nguyên phép tính đối chiếu stock/v0.4, không phải đường nguồn hiện hành.** Trong v0.5, thay RQ1 bằng đường MUX và xét riêng khoảng chuyển nguồn theo mục5.3; không áp cửa sổ pin3,7V của v0.4 sang thiết kế hiện tại.

Định nghĩa **Vpack** là điện áp có tải tại P+/P- của bộ pin đã có PCM. Điện trở ngoài pack **Rext** gồm connector, fuse, dây và PCB tới BAT XIAO; không cộng lại nội trở cell/PCM vào Vpack đã có tải.

\[
I_{pack}=I_{cam,in}+\frac{3,3 I_{3V3}}{\eta V_{IN}}+I_{aux},\qquad
V_{IN}=V_{pack}-I_{pack}(R_{ext}+R_{Q1}).
\]

Không tự thay VIN bằng 3,7 V danh định hoặc coi hiệu suất typical là giới hạn bảo đảm. Với I0=Icam+Iaux, k=3,3 I3V3/eta, R=Rext+RQ1:

\[
V_{IN}^2-(V_{pack}-RI_0)V_{IN}+Rk=0.
\]

Nghiệm điện áp cao là nghiệm làm việc của mô hình này nếu buck còn ổn áp; không có nghiệm phù hợp là dấu hiệu mô hình/tải không giữ được trạng thái yêu cầu, không phải dự báo chính xác brownout.

**Kịch bản phân bổ để đánh giá đường nguồn**, không phải số đo/max tải:

- Tổng rail 3V3 khảo sát 100-600 mA; 600 mA là ghi chú rail của sơ đồ board, không phải tải cộng thêm ngoài MCU.
- Camera phân bổ 150 mA so với điểm typical 98 mA; khoản này cần xác nhận, không phải maximum hãng.
- Phụ trực tiếp phân bổ 2 mA; hiệu suất yêu cầu >=85%; VIN cần >=3,5 V là tiêu chí thiết kế khảo sát, chưa là ngưỡng dừng sản phẩm.
- Rext <=0,100 Ohm là ngân sách đường dẫn. RQ1=0,67 Ohm là điểm datasheet tại VGS=-2,5 V, 660 mA,25 C, **không là mô hình bảo đảm toàn dải**. [LRC Q1](https://www.lrc.cn/Upload/PDF/Product/MOS/LP0404N3T5G.pdf).

**Bảng đối chiếu cấu hình stock v0.3:**

| Phân bổ I3V3 | Ipack tại VIN=3,5 V | Vpack cần theo mô hình | Nhiệt Q1 stock tham chiếu |
|---:|---:|---:|---:|
| 100 mA | 263 mA | 3,702 V | 46 mW |
| 200 mA | 374 mA | 3,788 V | 94 mW |
| 300 mA | 485 mA | 3,873 V | 157 mW |
| 600 mA | 818 mA | 4,130 V | 448 mW |

Tại Vpack=4,2 V và phân bổ 600 mA, giải đồng thời cho Ipack≈0,802 A, VIN≈3,582 V, nhiệtQ1≈0,431 W. Áp dụng RQ1 tới dòng/điện áp khác điểm kiểm tra là ngoại suy tham chiếu. Rating nhiệt Q1 trên fixture đồng lớn không chứng minh board XIAO nhỏ tản được nhiệt đó.

**Kết quả cập nhật v0.4:** Q1 stock được thay bằng DMP2035U-7. Tại cùng điểm VIN 3,5 V / rail 600 mA, pack cần 4,1295 V với điểm trở cũ, 3,6185 V với điểm Rmax 25 C / 45 mOhm mới, và **3,6635 V với ngân sách nóng 100 mOhm**. Đây là yêu cầu của mô hình, chưa là điện áp dừng sản phẩm.

Tại pack 3,7 V / rail 600 mA với ngân sách mới 100 mOhm, nghiệm ghép là **Ipack 810,4 mA, VIN 3,5379 V, nhiệt QM1 65,68 mW**. Giới hạn thiết kế 1 A / Rhot 100 mOhm cho sụt/nhiệt <=100 mV /100 mW; mục tiêu TJ<=85 C tại Ta_local 60 C đòi theta_effective<=250 C/W. Rhot 100 mOhm là điều kiện cần nghiệm thu, không phải maximum nóng hãng bảo đảm. Toàn bộ phép tính/giới hạn ở [hồ sơ Q1](q1-design-decision.md).

Các kết quả Q1 trên là lịch sử thiết kế. Pack đủ1A chưa tự duyệt hệ nguồn; v0.5 thay toàn bộ đường chọn nguồn này bằng TPS2121.

### 5.3. Chuyển USB/pin v0.5

Đọc [thiết kế và nguồn dẫn chứng đầy đủ](usb-battery-switchover-design.md), [JSON tính lại được](design-output/source-switchover-calculations.json). Miền thiết kế hiện hành:

- **IN2/VBAT_XIAO >=3,8V khi có tải** tại nút gauge đo; packP+/P->=3,9V cùng Rext<=0,100Ω và dòng<=1A là điều kiện đủ từ phía pack.
- Tải SYS bình thường <=0,9A; góc tính hold-up dùng1A. Không đồng nhất với tải rail3V3 hoặc dòng quay trung bình.
- RMUX<=0,100Ω tại điều kiện vận hành; Ceff>=750µF; ESR<=15mΩ; Icap_peak<=3A; Qreverse<=20µC; gap_total<=50µs; parasitic/ESL<=20mV. Các giá trị này là **điều kiện thiết kế/nghiệm thu**, không tự là MAX datasheet.

\[
V_{SYS,steady}\ge3,8-1(0,100)=3,7V.
\]

\[
V_{SYS,min}\ge3,7-3(0,015)-\frac{1(50\mu s)+20\mu C}{750\mu F}-0,020
=\boxed{3,541667V}.
\]

Margin so với VIN yêu cầu3,5V là41,667mV trong mô hình; khoảng gap tối đa tương ứng81,25µs. **Đây không phải minimum được bảo đảm toàn hệ**: tFSW5µs và response RCB của TPS2121 là typical; phải thỏa các giới hạn đã chọn, bao gồm dòng ngược và điện dung hiệu dụng. Nguồn pin dự phòng phải valid/settled trước khi cho AV hoạt động.

Cold start chọn tải chuẩn bị<=0,5A và SR<=200V/s. Ba tụ330µF ở dung sai+20% cho Cmax1188µF:

\[
I_{startup,design}\le0,5+(1188\mu F)(200V/s)=0,7376A.
\]

CSS1µF cho khoảng88V/s trong bảng estimated ở5V; SR200V/s không phải max được hãng công bố. ILIM80k cho1,5A nominal và bảng1..2A tại đúng80k; dung sai R và IC còn phải xét. Nhánh diode fallback không được chọn vì bypass SS/ILIM và tạo dòng rò về pin. V0.5 đã có kiến trúc và bài tính chuyển nguồn, nhưng chưa chứng minh chạy AV không reset trên kit thật hoặc đạt45phút trong cửa sổ pin mới.

### 5.4. Buck 3V3 và nguồn tại tải v0.6

Đọc [thiết kế đầy đủ](buck-3v3-design.md), [JSON](design-output/buck-3v3-calculations.json) và [audit nguồn](references/buck-3v3-source-audit-2026-11-06.md). Giữ SGM6029C/L4/mạng tụ stock; EN nối VIN, R13=249kΩ chọn mã3,3V qua R2D rồi cho phép tự chuyển PSM/PWM. IC có duty100%, nên bound VIN3,541667V không đòi buck phải tắt/khởi động lại. Tuy vậy VIN lớn hơn UVLO không chứng minh VOUT giữ đúng3,3V.

Với tổng rail3V3≤600mA và **ngân sách nghiệm thu** R_HS+R_L+R_local≤0,500Ω:

\[
V_{OUT,available}\simeq3,541667-0,600(0,500)=3,241667V.
\]

Đây là khả năng DC có điều kiện, không phải bảo đảm giữ3,300V. Muốn còn giữ3,300V cần Rsum≤**0,402778Ω**; muốn giữ mép baseline3,234V cần≤**0,512778Ω**. Baseline **3,234–3,366V là yêu cầu tại tải**, không phải kéo dòng accuracy±2% ở0mA thành bảo đảm tại600mA.

Mạng COUT của hãng khuyến nghị **5–10µF hiệu dụng**, nên không thêm tụ bulk3V3. Miền tính chọn Ceff**8–10µF** chưa được xác nhận trên stock. Tại8µF, nếu baseline đạt và mỗi nhánh có drop≤30mV, ripple/ESR/ringing mỗi khoản≤20mV, thiếu/dư điện lượng Q−≤0,25µC/Q+≤0,40µC, phép tính cho cửa sổ chân tải **3,11275–3,476V**. Đây là **ngân sách cần đạt**, không phải đáp ứng SGM6029 đã được chứng minh. Ví dụ tải nhảy đồng thời có thể vượt Qtarget ngay trong mô hình plant lý tưởng; không chọn một Qtarget thuận lợi rồi tuyên bố toàn miền đạt.

Nguồn SD phải xét **sau FB1**, còn VDD_SPI đi qua đường nội ESP với **14Ω typical**, nên điện áp tại đầu buck không bảo đảm mọi tải đủ nguồn. L4/C chưa rõ MPN và đặc tính nóng/bias; datasheet thiếu giới hạn độ trễ/vòng điều khiển tại điểm này. Kết luận hiện tại: **phần DC có thể đạt theo điều kiện trở dẫn, nhưng chưa chứng minh ESP/microSD không reset**. Phép tính/mô hình giả thiết không đóng các dữ liệu còn thiếu hoặc yêu cầu45phút.

## 6. Vì sao chọn MAX17048 cho nhánh đo pin?

Chia áp trực tiếp có vấn đề khi ESP mất nguồn; thêm analog switch vẫn phải xét trễ ngắt và điện tích tụ lọc. Bản v0.3 chọn giao tiếp số, không nối điện áp pin analog vào chân ADC.

Theo [MAX17048/MAX17049 Rev7](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX17048-MAX17049.pdf): VDD hoạt động 2,5-4,5 V, nối BAT; SCL là input, SDA/ALRT open-drain. CTG/QSTRT/GND/EP xuống GND, CELL nối BAT theo bảng chân. Pullup chỉ lên **3V3 XIAO**, không lên pin. VDD decouple 100 nF.

Đọc word VCELL 0x02: **V = raw16 x 78,125 uV**. Đây là hệ số quy đổi word, không phải độ chính xác phép đo; bảng ghi độ phân giải đo 1,25 mV. Active cập nhật 250 ms; chọn tắt hibernate trong lúc quay (`HIBRT=0`) để tránh chu kỳ 45 s. Sai số bảng ±7,5 mV tại 3,6 V/25 C và ±20 mV trong dải chỉ định; chưa cộng layout/ripple. Ban đầu dùng điện áp, **chưa dùng SOC làm bằng chứng thời lượng**. IC không đo dòng, không thay PCM hoặc cảm biến nhiệt cell.

MAX17048 đọc **VBAT_XIAO sau fuse/dây**, khác Vpack tại cọc P+/P- ở mô hình phần 5.2. Ngưỡng phần mềm phải dùng đúng điểm đo. ALRT chỉ là đường báo ngắt; cần cấu hình VALRT/STATUS và xử lý cờ, không dùng mặc định SOC 4% làm ngưỡng dừng NeoCam.

Lựa chọn pullup được tính:

\[
t_r\simeq0,8473R_PC_{bus}=0,8473(2200)(150 pF)=279,6\ ns.
\]

Với R+1%, khoảng282,4 ns, dưới tiêu chí300 ns. Tại rail3,6 V/VOL0,4 V/R-1%, dòng sink≈1,47 mA, dưới điểm thử4 mA của IC. Cbus<=150 pF là yêu cầu layout cần xác nhận, không phải điện dung đã đo. Thông tin số250 ms không bắt được mọi xung sụt áp microsecond; vẫn cần dự phòng nguồn/brownout.

## 7. Pin, PCM, fuse và thời lượng

[DNK102050](https://www.dnkpower.com/wp-content/uploads/2023/11/DNK102050-3.7V-1000mAh-Lipo-Battery-Specification.pdf) có 1 A liên tục,3 A/10 ms, sạc tối đa 500 mA; PCM quá dòng 3-5 A không cho phép xả 3-5 A liên tục. Kích thước pack 52 x 20 x 10 mm chưa chứng minh vừa vỏ GO 3. DNK là **mẫu tham chiếu có điều kiện**, không phải phê duyệt mua/lắp. Tiêu chí tìm mua 1,5 A trước đây chưa phải nhu cầu dòng đã được chứng minh; nếu giữ thành yêu cầu bắt buộc, DNK không đạt.

Cầu chì1,5 A với derating 25% cho 1,125 A tại điều kiện cơ sở; phải giảm thêm theo nhiệt độ. Rnguội danh định 38,5 mOhm cho sụt 30,8 mV/nhiệt 24,64 mW tại 0,8 A. Không bảo đảm trở nóng bằng trở danh định hoặc cầu chì ngắt ngay tại 1,5 A. [Littelfuse467](https://www.littelfuse.com/assetdocs/fuse-467-datasheet?assetguid=4a59f034-1cca-460e-a5ba-e1e66247c76d).

Ngân sách Rext 100 mOhm trừ fuse danh định còn 61,5 mOhm cho connector/dây/PCB; nghiệm thu phải dùng giá trị nóng và dung sai thực. Bố trí fuse gần đầu nối pack. Không bypass PCM qua dây GND phụ.

Khả năng chịu xung của cell không tự là khả năng chịu xung của cả đường nguồn. Khi có dạng dòng, tính \(\int i^2dt\) và đối chiếu đường cong xung/derating fuse: ví dụ3 A trong 10 ms cho0,09 A²s, trong khi I2t nóng chảy danh định fuse là0,0766 A²s. So sánh danh định này chưa kết luận chắc chắn fuse đứt, nhưng cho thấy **không được hứa đường mạch chịu xung3 A/10 ms chỉ vì pack cho phép**.

Với điện lượng mAh/dòng mA/thời gian h:

\[
Q_{usable}\ge Q_{boot}+0,75 I_{rec,avg}+Q_{close,worst}+Q_{reserve}.
\]

Nếu **chọn phân bổ** tổng chuẩn bị/lưu/dự phòng 50 mAh, các điều kiện là:

| Irec trung bình giả định | Qusable cần cho 45 phút +50 mAh |
|---:|---:|
| 150 mA | 162,5 mAh |
| 300 mA | 275 mAh |
| 500 mA | 425 mAh |
| 800 mA | 650 mAh |
| 1000 mA | 800 mAh |

50 mAh chưa phải điện lượng lưu tệp đã đo. Qusable là điện lượng trước ngưỡng hệ thống phải dừng, **khác dung lượng nhãn đo tới ngưỡng xả cell**. V0.5 yêu cầu IN2 có tải>=3,8V trong miền chuyển nguồn đã tính; phải tính lại Qusable trong cửa sổ này, không giữ kết luận từ mô hình pin3,7V trước. Chưa xác nhận pack800/1000mAh chạy45phút; không tự nhân75/80% rồi gọi là dữ liệu pin.

## 8. Sạc và pin yếu

[SGM40567](https://www.sg-micro.com/rect/assets/e95e555a-a8a2-4761-b86a-6554dff2d8ed/SGM40567.pdf) với RIREF 220 kOhm cho Icharge danh định 24000/220=109,09 mA. Thời gian 1000/109,09=9,17 h chỉ là phép chia Q/I, chưa phải thời gian sạc đầy. Tải BAT, thuật toán CV/foldback và nhiệt còn ảnh hưởng.

V0.5 chọn nguồn **USB-C5V/3A có xác nhận**, điện áp4,75-5,25V tại IN1 MUX. Không dùng USB2 của PC như nguồn3A mặc định. Để thấy vì sao cần xét cả sạc và SYS: tải SYS bình thường0,9A cộng sạc danh định109,09mA và phụ2mA cho ví dụ phân bổ:

\[
I_{USB,alloc}\simeq0,900+0,10909+0,002=1,01109\ A.
\]

Đây không phải dòng quay đo được. MUX có bảng giới hạn dòng tới2A tại RILIM80k; startup và chuyển nguồn còn có xung, nên không lấy1,01109A làm maximum của nguồn. Nguồn3A là lựa chọn năng lực cấp; đầu USB-C không tự cho phép rút3A nếu chưa xác nhận khả năng nguồn/cáp. Phối hợp sạc/nhiệt vẫn là yêu cầu riêng.

Bảng 25 C ghi tối đa 4,234 V tại điều kiện của bảng; IC còn bù sụt áp khoảng 2%, nên **4,234 V không là trần toàn thuật toán**. PCM DNK có ngưỡng quá sạc thấp nhất 4,225 V. Hiệu -9 mV đã cho thấy chưa đóng phối hợp; cần cả dung sai sạc, điện áp thực tại cell, bù áp và thời gian PCM. Chưa duyệt sạc pack này trên mọi điều kiện chỉ từ dòng 109 mA<500 mA.

Stock không có TS/NTC; bảo vệ nhiệt die IC không phải bảo vệ nhiệt cell. Sạc tự động theo nhiệt độ cell và vừa sạc vừa quay là các yêu cầu phải xử lý riêng trước mức sản phẩm. Không vẽ NTC hai dây giả hoặc mô tả đã có chức năng này.

Ba hành vi pin yếu: **từ chối bắt đầu / cảnh báo / buộc dừng và lưu**. Chưa gán một điện áp hoặc phần trăm tùy ý. Điều kiện:

\[
E_{remaining,before\ instability}\ge E_{finalize,worst}+E_{reserve}.
\]

Phần mềm phải tính thời gian/điện lượng đóng tệp, điện áp có tải, sai số đo, hysteresis và độ trễ; không chờ PCM cắt. MAX17048 không tự đóng tệp hoặc duy trì nguồn sau khi pack bị tháo.

Tại nút mà MAX17048 đo, dạng điều kiện chọn ngưỡng buộc dừng là:

\[
V_{stop,VBAT\_XIAO}\ge\max\left(3,8V,\ V_{IN,required}+I_{SYS,finalize}R_{MUX}+\Delta V_{dynamic}\right)
+\delta V_{measurement}+\Delta V_{during\ finalize}.
\]

Đây là dạng điều kiện v0.5, không phải ngưỡng firmware đã chốt. Mốc3,8V giữ miền đầu vào MUX của bài toán chuyển nguồn; các số hạng khác dự phòng sụt áp động, sai số đo và giảm áp khi lưu. Không cộng lại fuse/dây phía trước nút đo. Năng lượng dự phòng vẫn là điều kiện riêng, không thay bằng mỗi so sánh điện áp.

Khi USB đang cấp nguồn nhưng pack bị tháo, IC đo có thể đọc điện áp ở nút BAT do bộ sạc tạo ra. Một giá trị điện áp hợp lệ không tự chứng minh pack đang gắn; trạng thái này phải được xử lý trong logic nguồn và kế hoạch kiểm chứng.

## 9. Kiểm tra thiết kế và mô phỏng đã làm

- Script tái lập toàn bộ số học, quét dung sai ba điện trở của **phương án ADC cũ**, ngân sách45 phút và sụt áp theo tải. Dữ liệu JSON/CSV ghi nhãn rõ typical, phân bổ và điều kiện.
- Sơ đồ KiCad được xuất netlist/ERC và SVG; kết quả cuối cùng ghi tại thư mục exports. ERC kiểm tra kết nối/loại chân, không chứng minh hiệu năng/độ an toàn/nhiệt hoặc khả năng lắp PCB.
- QSPICE đã chạy **mô hình lý tưởng của phương án ADC không chọn**: VADC khi ON≈1,411315 V; OFF ≈0,200002 V với nguồn rò áp đặt 2 uA. Kết quả khớp RC/DC của mô hình; model không có trễ switch thật/ESPinjection và **không là mô phỏng MAX17048, charger hoặc toàn nguồn NeoCam**. Giữ lại để đọc phép tính và lý do loại phương án.
- V0.5 bổ sung [script số học chuyển nguồn](tools/calculate-source-switchover.py), [JSON kết quả](design-output/source-switchover-calculations.json), [mô hình chuyển nguồn](design-output/source-switchover-envelope.cir) và [script đọc kết quả](tools/read-source-switchover-model.py). Đây là mô hình miền điều kiện, không thay tSW maximum còn thiếu hoặc chứng minh toàn bộ AV/buck/charger. Kết quả chạy/kiểm tra hiện hành nằm trong [hồ sơ chuyển nguồn](usb-battery-switchover-design.md).
- V0.6 bổ sung [script số học buck](tools/calculate-buck-3v3.py), [JSON](design-output/buck-3v3-calculations.json), các quét dropout/charge và [mô hình charge-envelope](design-output/buck-3v3-charge-envelope.cir). Mô hình áp đặt thiếu/dư dòng và phục hồi lý tưởng, không có vòng điều khiển silicon SGM6029; xem [giới hạn và kết quả](buck-3v3-design.md#7-dữ-liệu-tính-và-mô-hình-đã-lưu).
- Chưa nạp ESP-IDF, chưa đo dòng/thời lượng/nhiệt, chưa mô phỏng buck/charger bằng model hãng. Không có Gerber sản xuất hoặc PCB đã route trong bộ này.

Chạy lại từ root NeoLab:

```powershell
python concepts/neocam/tools/calculate-power-design.py
python concepts/neocam/tools/calculate-source-switchover.py
python concepts/neocam/tools/calculate-buck-3v3.py
Push-Location concepts/neocam/design-output
& 'C:\Program Files\QSPICE\QSPICE64.exe' 'source-switchover-envelope.cir'
Pop-Location
python concepts/neocam/tools/read-source-switchover-model.py
```

## 10. Các lựa chọn được giữ và loại

| Phương án | Quyết định |
|---|---|
| Pin protected + charger/buck stock | Giữ v0.6; kế thừa mux v0.5, giữ SGM/L/C có điều kiện, không thêm tụ bulk3V3 |
| Giữ Q1 LP0404N3T5G stock | Không chọn; bài tính stock được giữ làm đối chiếu |
| DMP2035U-7 + R6=10k qua adapter | Đã chọn ởv0.4 để giảm trở; **không lắp trong v0.5**, chưa giải quyết đầy đủ chuyển nguồn |
| TPS2121RUXR +3×330µF | **Chọn v0.5**, XCOMP, ILIM80k, SS1µF và miền hold-up có điều kiện |
| Diode fallback BAT->SYS | Không chọn trong BOM cuối vì bypass SS/ILIM và có dòng rò ngược |
| Vẽ lại toàn bộ MCU/camera/charger ngay | Không chọn cho bước này; dùng board đã có để giải bài toán nguồn phụ trợ |
| Divider100k/100k nối thẳng D0 | Không chọn; nguồn chip tắt chưa được giải quyết |
| Divider + TMUX1511 | Đã tính/vẽ/mô phỏng lý tưởng; không chọn v0.3 do trình tự mất nguồn còn phải chứng minh |
| MAX17048 đọc qua I2C | Chọn cho sơ đồ hiện tại; phần trăm SOC chưa được duyệt với cell |
| Tụ lớn không có bài tính | Không chọn. V0.5 dùng3×330µF sau khi tính Ceff/ESR, gap, dòng ngược, ESL và startup; không chữa mọi lỗi nguồn |
| Tụ bulk mới trên OUT3V3 | Không chọn v0.6; giữ miền COUT hiệu dụng5–10µF của hãng, cần xác nhận stock và đáp ứng thực |
| Đổi pin lớn hơn là đủ45 phút | Không kết luận; còn Qusable trong cửa sổ IN2>=3,8V và dòng/năng lượng thực |
| USB-C + pogo cấp chung không ORing | Không chọn; pogo DNP chưa hoạt động |

## 11. Mục tiêu đã đạt và tồn đọng

| Hạng mục | Đã làm | Điều kiện đóng tiếp theo |
|---|---|---|
| Sơ đồ phụ trợ | Đã định nghĩa toàn bộ kết nối pin/đo pin/nút/LED với board | Đối chiếu pad và revision thật trước lắp |
| Tính toán/lựa chọn | Đã lưu công thức, điều kiện, script/CSV/JSON và nguồn | Chọn cell/pack mua được, xác nhận datasheet đúng biến thể |
| Chọn nguồn USB/pin | V0.5 dùng TPS2121,3×330µF, tính được floor3,541667V theo bộ điều kiện | Đóng bound gap/Qreverse/Icap_peak/Ceff/ESR/RMUX, nguồn5V3A, thao tác rework và rail3V3 thực |
| Buck3V3/ESP/SD/bộ nhớ | V0.6 có trang nguồn riêng, ngân sách DC và Q/C; không thêm bulkOUT | Xác nhận L4/Ceff, baseline có tải, Rhot và đáp ứng thực; từng rail ESP/SD/VDD_SPI đủ áp; chưa chứng minh no-reset |
| Sạc | Đã xác định IC, dòng đặt, thuật toán và khoảng chồng PCM | Đóng phối hợp điện áp/nhiệt/PCM; bổ sung giám sát nhiệt cell cho mức sản phẩm |
| Ghi720p/30 có tiếng | Đã có mục tiêu và hành vi | Định dạng/nén, sample rate/kênh âm thanh, buffer, ghi SD và đồng bộ A/V; hiệu năng ESP32 chưa chứng minh |
| Quay45 phút | Có tiêu chí Qusable và ngân sách dự phòng; chưa đạt | Irec theo thời gian, dung lượng dùng được trong cửa sổ v0.5 IN2>=3,8V, năng lượng lưu tệp |
| Pin yếu/lưu tệp | Có MAX17048/ALRT và yêu cầu trạng thái | Xác định ngưỡng từ tải/sai số và năng lượng thực; firmware chưa triển khai |
| StreamWi-Fi | Đã giữ riêng mục tiêu 720p/12 không tiếng | Hiệu năng, mạng/chất lượng ảnh, tải và thời lượng riêng |
| Pogo/nhiệt/cơ khí/giá | Đã chỉ rõ giao diện và phạm vi | Chọn nguồn 5V, chống sai/chập, NTC, CAD vỏ, kích thước pack và BOM báo giá |
| PCB | Chỉ có schematic và thư viện tham chiếu | Footprint module thật, bố trí/route/DRC/cơ khí, review trước Gerber |

**Điểm chốt để cùng phân tích:** v0.6 giữ stock buck/L/C, bổ sung phép tính3V3 và điểm truy cập từng rail; kế thừa mux và floorVIN3,541667V có điều kiện từ v0.5. DC có thể đáp ứng theo ngân sách trở, nhưng Q/C và đáp ứng vòng điều khiển chưa được bảo đảm bằng dữ liệu hãng/bo; chưa kết luận ESP/SD không reset. Yêu cầu45phút, phối hợp pack/charger và khả năng sản xuất vẫn chưa đạt. Các đầu vào còn thiếu được chỉ rõ trong [hồ sơ buck](buck-3v3-design.md), không thay bằng kết quả ERC hoặc mô hình lý tưởng.
