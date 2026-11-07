# NeoCam: ngân sách nguồn cho quay độc lập có tiếng

Ngày: **2026-11-05**. Cơ sở thiết kế **v0, chưa hoàn thiện**; chưa có kết quả đo hoặc chạy thử cho chế độ này.

Cập nhật: **2026-11-06**. Người dùng chấp nhận dùng **sơ đồ Exp. Board v1.2 làm mô hình tính toán**; bo mở rộng thực tế vẫn ghi là v1.1. Cấu hình hiện hành **v0.6 giữ SGM6029C/L4/mạng tụ stock và phân tích riêng3V3/SD/VDD_SPI, không thêm bulk3V3**. Đường mux kế thừa v0.5: TPS2121RUXR thay Q1/D2, bỏ QM1/R6 v0.4, giữ R9/charger. Xem [hồ sơ tổng hợp](power-design-review.md), [thiết kế buck](buck-3v3-design.md), [chuyển nguồn](usb-battery-switchover-design.md) và [BOM v0.6](neocam-power-v0/bom-reference-v0.6.csv). [Bài tính Q1 v0.4](q1-design-decision.md) là lịch sử; chưa rework hoặc đo bo thật.

## Phạm vi

Mục tiêu: tạo video có hình và tiếng trong **ít nhất 45 phút**, dùng pin riêng, **Wi-Fi/BLE tắt**, rồi hoàn tất tệp trước khi nguồn mất ổn định. Mục tiêu 720p/30 FPS chưa được xác nhận. Yêu cầu 45 phút vừa ghi vừa xem trước Wi-Fi được giữ riêng trong [bài toán trước](standalone-video-audio-problem.md).

Luồng sử dụng theo [yêu cầu hành vi](standalone-normal-recording-behavior.md). Dùng thẻ microSD người dùng đang có.

## Cấu hình phần cứng đang xét

| Hạng mục | Thông tin / mức xác nhận |
|---|---|
| Kit | **Seeed Studio XIAO ESP32-S3**, người dùng xác nhận |
| FCC ID | **Z4T-XIAOESP32S3**, người dùng cung cấp; khớp giấy cấp phép của Seeed |
| Camera | **OV3660**, người dùng xác nhận; không thay bằng OV5640 trong tính toán |
| Bo mở rộng Sense | **v1.1**; người dùng xác nhận chữ v1.1 in trên bo có khe microSD và microphone |
| Sơ đồ tham chiếu bo mở rộng | **Exp. Board v1.2**, người dùng chấp nhận ngày 2026-11-06; chưa xác nhận các linh kiện trên v1.1 giống hệt |
| Lưu trữ | **SanDisk Ultra microSD 32 GB**, người dùng xác nhận |
| Mã thẻ đầy đủ | Chưa có mã sản phẩm/SKU; không mặc định phiên bản Ultra cụ thể |
| Revision bo chính XIAO | Chưa xác nhận; mô hình V1.51 có mux TPS2121 và buck stock theo v0.6 |

[Giấy FCC do Seeed lưu](https://files.seeedstudio.com/Seeed_Certificate/documents_certificate/113991114-FCC.pdf) ghi chung FCC ID này cho **XIAO ESP32S3 và XIAO ESP32S3 Sense**. FCC ID nhận diện dòng sản phẩm vô tuyến, không đủ xác nhận bo chính V1.51, bo mở rộng v1.2 hoặc mã IC nguồn của bo thực tế. Các phép tính dựa trên sơ đồ đó tiếp tục được ghi có điều kiện.

Tiếp tục thiết kế với v1.2 theo quyết định trên, không chờ tìm sơ đồ v1.1. Mọi kết luận về mã linh kiện, đường nguồn và khả năng ngắt nguồn dưới đây thuộc **mô hình v1.2**. Việc đối chiếu bo v1.1 thực tế được giữ thành một hạng mục xác nhận trước khi áp dụng thiết kế lên phần cứng; revision này không xác nhận revision bo chính hoặc cụm sensor.

### Cây nguồn theo Exp. Board v1.2

Đã đọc trực tiếp [sơ đồ chính thức Seeed](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/res/XIAO_ESP32S3_Exp_Board_v1.2_SCH_PDF_260810.pdf), một trang. Bản nguồn được lưu nguyên trạng tại [references/seeed-xiao-esp32s3-exp-board-v1.2.pdf](references/seeed-xiao-esp32s3-exp-board-v1.2.pdf).

| Nhánh | Đường nguồn / linh kiện trên sơ đồ |
|---|---|
| Đầu vào camera | `VIN -> R14 = 0 ohm -> VCC_IN`; nhánh dự phòng `VCC_3V3 -> D7 LMBR4010BST5G -> VCC_IN` |
| Camera AVDD và DOVDD | U1 **SGM2036S-2.8XXDH4G/TR**, đầu ra 2,8 V; AVDD qua FB2, DOVDD nối rail trực tiếp |
| Camera DVDD | U2 **SGM2036S-1.5XXDH4G/TR**, đầu ra theo mã linh kiện **1,5 V** |
| Microphone | MIC1 **MSM261D3526H1CPM**, nguồn trực tiếp `VCC_3V3` |
| microSD | `VCC_3V3 -> FB1 -> VCC_SD_3V3 -> JA2`, chân nguồn thẻ 3,3 V |

**Sơ đồ có điểm không nhất quán:** net đầu ra U2 còn tên `VCC_1V3`, block diagram còn ghi 1,3 V; mã U2 và ghi chú sửa đổi ngày 2026/08/10 chỉ ra **1,5 V**. Phép tính dùng 1,5 V theo mã U2 và ghi chú sửa đổi, không coi tên net là số đo điện áp.

EN của cả hai LDO nối IN; không có GPIO tắt riêng hai rail. Mic và microSD cũng không có công tắc nguồn riêng trên sơ đồ. PWDN camera kéo xuống GND, không nối GPIO. Vì vậy dừng tác vụ hoặc cho ESP32 ngủ chưa chứng minh camera đã mất nguồn; ngay cả ngắt VIN cũng cần xét nhánh cấp dự phòng D7. FB1/FB2 ghi `120R-1.3A` là ferrite bead, không coi là điện trở DC 120 ohm.

Tài liệu chính hãng có nhiều thế hệ SanDisk Ultra 32 GB. Ví dụ [datasheet dòng Ultra 120 MB/s](https://documents.sandisk.com/content/dam/asset-library/en_us/assets/public/sandisk/product/memory-cards/ultra-uhs-i-microsd/data-sheet-ultra-uhs-i-microsd-120mb.pdf) công bố C10/U1/A1 cho 32 GB, nhưng chưa cung cấp dòng ghi trung bình/đỉnh, dòng nghỉ hoặc thời gian busy ghi bất lợi. Không gán tốc độ hay các cấp hiệu năng của ví dụ này cho chính thẻ người dùng khi chưa có SKU. Không suy ra dòng từ dung lượng/cấp tốc độ, và không lấy số liệu thẻ Industrial hoặc Extreme thay cho Ultra.

### Ngân sách nhánh microSD có điều kiện

Đề xuất dùng **SDSPI Default Speed, tối đa 25 MHz, không chuyển High Speed** làm cấu hình tham chiếu khi tính nguồn. Chưa cấu hình firmware hoặc xác nhận băng thông đáp ứng video. Theo [SD Association Physical Layer Simplified v7.10, bảng 3-6/3-9 và mục 4.3.10.4](https://www.sdcard.org/cms/wp-content/themes/sdcard-org/dl.php?f=Part1_Physical_Layer_Simplified_Specification_Ver7.10.pdf), có thể cấp phát **0,36 W** cho nhánh thẻ ở chế độ này. Giới hạn chuẩn dựa trên giá trị trung bình một giây; không dùng làm giới hạn dòng xung/khởi tạo, hoặc dòng ghi thực tế của SanDisk Ultra.

Tại rail 3,3 V, mức phân bổ tương đương \(0,36/3,3\approx109\,mA\). Trong 45 phút quay, năng lượng phân bổ tại thẻ là \(0,36\times0,75=0,27\,Wh\), chưa gồm tổn hao đường nguồn. Nếu chọn chế độ bus khác phải xét lại ngân sách. Trần dữ liệu thô tại 25 MHz SPI là \(25/8=3,125\,MB/s\) trước overhead; cần đối chiếu đường dữ liệu hình-tiếng trước khi chốt tốc độ.

## Bốn trạng thái cần tính

| Trạng thái | Các khối liên quan | Đại lượng còn thiếu tại đường pin |
|---|---|---|
| **Chuẩn bị** | MCU, bộ nhớ, khởi tạo camera/microphone và microSD | Dòng trung bình, dòng đỉnh, thời gian khởi động |
| **Quay** | MCU, PSRAM/flash, camera, microphone, ghi microSD, LED và nguồn | Dòng trung bình/đỉnh khi ghi hình-tiếng đúng cấu hình |
| **Hoàn tất tệp** | MCU, bộ đệm, microSD; xử lý dữ liệu chờ và đóng tệp | Điện lượng/thời gian kết thúc trong trường hợp bất lợi |
| **Nghỉ** | Những phần còn cần cho đánh thức/quản lý nguồn | Dòng toàn board và thời gian chờ |

Ngừng lấy hình/tiếng không đồng nghĩa đã cắt điện camera/microphone. Khả năng standby hoặc ngắt rail phải đối chiếu sơ đồ và điều khiển thực tế.

## Các điểm dữ liệu hãng có thể dùng

**ESP32-S3:** datasheet bảng 5-9 công bố typical **81,3 mA** ở 240 MHz, hai lõi chạy truy cập dữ liệu 32-bit, mọi clock ngoại vi bật, trong Modem-sleep. Quy đổi tại 3,3 V:

\[
P_{\rm ESP,tham\ chiếu}=3,3\times0,0813=0,26829\,W
\]

Đây là bài tải tham chiếu, chưa phải dòng quay và không phải max. Hãng lưu ý biến thể có PSRAM, gồm **S3R8**, có thể tiêu thụ cao hơn. [ESP32-S3 datasheet, bảng 5-9](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf#page=67).

**OV3660:** với cây nguồn v1.2, dùng trường hợp **DVDD ngoài 1,5 V, DOVDD 2,8 V** của [bảng 8-3](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/res/OV3660_datasheet.pdf#page=154):

| Rail cảm biến | Điện áp | Dòng typical | Công suất tại tải |
|---|---:|---:|---:|
| AVDD | 2,8 V | 28 mA | 78,4 mW |
| DVDD | 1,5 V | 64 mA | 96 mW |
| DOVDD | 2,8 V | 6 mA | 16,8 mW |

U1 cấp chung AVDD và DOVDD nên tải U1 là **34 mA**; tải U2 là **64 mA**. Công suất tại cảm biến:

\[
P_{\rm sensor,ref}=2,8(0,028+0,006)+1,5(0,064)=0,1912\,W
\]

Điều kiện hãng là full-size/full-speed/compression, **typical, không phải maximum hoặc dòng đã xác nhận khi NeoCam ghi 720p/30**. Chú thích preview 720p trở xuống khoảng nửa dòng không cung cấp một giới hạn được bảo đảm cho tác vụ ghi của NeoCam. Ví dụ cũ 0,2044 W ở cấu hình DVDD nội/I/O 1,8 V được thay bằng cấu hình nguồn này.

### Quy dòng camera qua hai LDO

Với LDO đang ổn áp, dòng đầu vào bằng dòng đầu ra cộng phần dòng nuôi IC/điều khiển. Cả hai đầu vào cùng ở `VCC_IN`, do đó:

\[
I_{\rm cam,in}=I_{\rm U1,out}+I_{\rm U2,out}+I_{\rm overhead}
\approx34+64+I_{\rm overhead}=98\,mA+I_{\rm overhead}
\]

Đây là phép cộng các dòng **đã quy về cùng nút đầu vào LDO**, không cộng tùy ý dòng ở các điện áp khác nhau. Khi bỏ qua overhead để minh họa, tại **VCC_IN = 3,7 V**:

\[
P_{\rm cam,in}\approx3,7\times0,098=0,3626\,W
\]

\[
P_{\rm LDO,loss}\approx(3,7-2,8)\times0,034+(3,7-1,5)\times0,064
=0,1714\,W
\]

Kiểm tra cân bằng: **362,6 mW = 191,2 mW tại cảm biến + 171,4 mW tổn hao LDO**. Hiệu suất nhánh camera ở điểm minh họa này khoảng 52,7%, chưa tính overhead, đường dẫn và tải phụ. `VCC_IN` không luôn bằng điện áp pin: còn đường nguồn bo chính và nhánh diode dự phòng. Không lấy riêng 0,1912 W chia Vbat rồi coi đó là dòng pin của nhánh LDO.

[SGM2036S, trang 8 và 12](https://www.sg-micro.com/rect/assets/17ec502a-e4c4-4cc4-8df9-77986a7c65af/SGM2036S.pdf#page=8) công bố dòng nuôi **30 microampere typical / 45 microampere maximum tại không tải, 25 C**. Dòng chân GND thay đổi theo tải; không dùng 2 x 30 microampere như overhead active chính xác. Phải dùng đúng **SGM2036S**, không thay bằng datasheet SGM2036.

### Microphone và tải 3,3 V

Theo [datasheet MSM261D3526H1CPM, trang 3-5](https://files.seeedstudio.com/wiki/XIAO-BLE/mic-MSM261D3526H1CPM-ENG.pdf#page=3):

| Chế độ microphone PDM | Điều kiện bảng | Dòng typical |
|---|---|---:|
| Standard | VDD 1,8 V; clock 2,4 MHz | 670 microampere |
| Low power | VDD 1,8 V; clock 768 kHz | 290 microampere |
| Sleep | Clock <= 50 kHz; điều kiện chung VDD 1,8 V | 1 microampere |

Trên sơ đồ v1.2 microphone dùng **3,3 V**, nằm trong dải nguồn cho phép 1,6-3,6 V; các dòng trên chưa phải dòng được bảo đảm tại 3,3 V, và bảng không cho maximum. Giữ `I_mic(3,3 V, f_PDM)` thành biến còn thiếu; không tự đổi dòng theo tỷ lệ điện áp. Phần xử lý/ghi âm trong ESP32 và thẻ nhớ được tính ở các nhánh tương ứng, không gộp vào dòng riêng microphone.

Điểm tải ESP32, cảm biến và ngân sách SD chưa cùng mô tả một tác vụ NeoCam; **không cộng để gọi đó là dòng quay trung bình hay maximum toàn hệ thống**. Seeed cũng công bố khoảng 155 mA cho webcam và 64,5 mA cho thu microphone/ghi SD từ pin 3,8 V. Hai ứng dụng có tải chung nên không cộng nguyên các số đó. [Thông số Seeed](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#specification).

## Công thức thiết kế

Tại cùng thời điểm, quy các tải và tổn hao về pin:

\[
P_{\rm bat}(t)=\sum_j V_j(t)I_j(t)+P_{\rm loss}(t)+\frac{dE_{\rm stored}}{dt},\qquad
I_{\rm bat}(t)=P_{\rm bat}(t)/V_{\rm bat}(t)
\]

\(E_{\rm stored}\) là năng lượng tích trữ trong tụ và cuộn cảm của đường nguồn; ở trạng thái xác lập tuần hoàn, trung bình số hạng đạo hàm bằng 0. Không cộng trực tiếp dòng của những rail điện áp khác nhau. Khi đã tính ở đường pin, không nhân thêm hiệu suất một lần nữa.

Với điện lượng tính bằng mAh, dòng mA và thời gian giờ:

\[
Q_{\rm usable}\ge Q_{\rm boot}+0,75I_{\rm rec,avg}+Q_{\rm close,worst}+Q_{\rm reserve}
\]

Pin ghi nhãn **800 mAh** chưa xác định được \(Q_{\rm usable}\) trước ngưỡng hệ thống dừng. Điều kiện cho dòng quay là:

\[
I_{\rm rec,avg}\le\frac{Q_{\rm usable}-Q_{\rm boot}-Q_{\rm close,worst}-Q_{\rm reserve}}{0,75}
\]

Chưa thay dòng dự trù 1 A hoặc tỷ lệ dung lượng giả định vào như kết quả thực tế.

## Dữ liệu cần bổ sung

SKU và đặc tính điện/ghi của SanDisk Ultra 32 GB; dòng tác vụ ESP32/PSRAM/flash, microphone tại 3,3 V, LED và tải phụ; định dạng/nén hình-tiếng, bộ đệm và thao tác ghi; hiệu suất và điện trở đường nguồn; dung lượng dùng được, thời gian/điện lượng kết thúc tệp. Revision bo chính còn cần xác nhận để gán chính xác IC nguồn và đường pin-VIN-3,3 V. Không chờ sơ đồ v1.1 để tiếp tục mô hình tham chiếu v1.2 đã được người dùng chấp nhận; việc xác nhận tương đương với v1.1 giữ riêng trước khi áp dụng lên phần cứng.

**Đã đạt:** có cây nguồn bo mở rộng, phép quy dòng qua LDO, lịch sử Q1, mux TPS2121/MAX17048 và sơ đồ v0.6 thể hiện riêng buck/nhánh SD. Phép tính giữ SYS trên3,5V và ngân sách DC/Q/C3V3 đều đã lưu có điều kiện. **Chưa đạt:** đáp ứng stock buck/điện áp từng nhánh được bảo đảm, dòng quay trung bình/đỉnh, thời lượng45phút trong cửa sổ pin mới, duyệt cell/PCM/sạc hoặc ngưỡng pin yếu. Chưa kết luận ESP/SD không reset; chưa lập trình hoặc thử phần cứng.

## Ngân sách tổng hợp: mux v0.5 được kế thừa trong v0.6

Phân tích bo chính và phép quy về pack nằm trong [hồ sơ tổng hợp](power-design-review.md). Định nghĩa Vpack tại P+/P- sau PCM; IN2/VBAT_XIAO sau connector/fuse/dây; SYS/VIN sau TPS2121. Trong chế độ pin xác lập, mô hình quy dòng dùng Ipack=Icam+P3V3/(eta VIN)+Iaux và VIN=Vpack-Ipack(Rext+RMUX). Không dùng RQ1 hoặc cửa sổ pin3,7V của v0.4 cho cấu hình hiện hành; không tính lặp nội trở cell/PCM đã nằm trước điểm Vpack có tải.

V0.5 yêu cầu **IN2>=3,8V khi có tải**, tải SYS bình thường<=0,9A. Nguồn USB yêu cầu5V/3A có xác nhận và4,75-5,25V tại MUX. Chọn ILIM80k (bảng đúng80k:1/1,5/2A), SS1µF và3×330µF **T520X337M010ATE025**. Qualification: Ceff>=750µF, ESR<=15mΩ, RMUX<=0,100Ω, hold load1A, gap_total<=50µs, Qreverse<=20µC, Icap_peak<=3A, ESL/parasitic<=20mV:

\[
V_{SYS,min}\ge(3,8-1\times0,1)-3\times0,015-
\frac{1\times50\mu s+20\mu C}{750\mu F}-0,020=3,541667V.
\]

Các bound trên là **điều kiện thiết kế/nghiệm thu**, không phải số đo hoặc MAX datasheet; fast switchover5µs của TPS2121 là TYP. Startup với Cmax1188µF, SR<=200V/s và tải chuẩn bị<=0,5A cho Istartup<=0,7376A. Pin dự phòng phải valid/settled trước khi quay. Xem [bài tính chuyển nguồn](usb-battery-switchover-design.md), [JSON](design-output/source-switchover-calculations.json), [script tính](tools/calculate-source-switchover.py), [script đọc mô hình](tools/read-source-switchover-model.py).

**Yêu cầu45phút vẫn chưa đạt:** Qusable phải tính lại trước khi IN2 chạm3,8V và còn dự phòng lưu tệp; không lấy dung lượng nhãn đo tới ngưỡng cell hoặc kết quả v0.4 làm bằng chứng thời lượng v0.6.

## Ngân sách buck3V3 và các nhánh tải v0.6

Giữ SGM6029C, L4=470nH và mạng tụ stock; R13=249kΩ chọn mã3,3V bằng R2D lúc startup, sau đó cho phép tự chuyển PSM/PWM. Stock EN nối VIN. Duty100% cho phép buck tiếp tục hoạt động khi VIN giảm, nhưng không boost hoặc bảo đảm rail luôn bằng3,300V.

Với VINmin kế thừa**3,541667V**, tổng rail**600mA** và ngân sách trở nóng **RHS+RL+Rlocal≤0,500Ω**, khả năng DC:

\[
V_{OUT,available}\simeq3,541667-0,600(0,500)=3,241667V.
\]

Muốn còn giữ3,300V cần Rsum≤**0,402778Ω**, hoặc≤**0,512778Ω** để giữ mép3,234V. Đây là điều kiện thiết kế; L4 chưa có MPN/DCR và RHSmax trong datasheet được cho tại100mA/điều kiện bảng. Baseline **3,234–3,366V phải đạt khi có tải**, không được coi dòng accuracy±2% ở0mA là bằng chứng tại600mA.

SGM khuyến nghị COUT**5–10µF hiệu dụng**. Không lắp thêm bulk3V3; miền tính chọn **8–10µF** chưa xác nhận trên stock, phải tính mạng tụ phân tán/bias và ổn định. Nếu Ceff≥8µF, baseline đạt, branchdrop≤30mV, ripple/ESR/ring mỗi khoản≤20mV, Q−≤0,25µC và Q+≤0,40µC thì bound tại tải là **3,11275–3,476V**. Q là tích phân thiếu/dư dòng cuộn cảm so với tải; nó là **mục tiêu đáp ứng cần đạt**, không phải silicon đã được chứng minh đạt. Bước tải cùng lúc chuyển nguồn có thể vượt bộ mục tiêu này ngay trong kịch bản LC lý tưởng.

microSD cần xét tại VDD sau **FB1**, còn flash/PSRAM qua đường **VDD_SPI với14Ω typical** bên trong ESP. Vì vậy đầu buck đủ áp không bảo đảm từng nhánh. Sơ đồ bổ sung điểm truy cập kit/ESP, SD sau FB1 và VDD_SPI; các phần stock chỉ được biểu diễn, không mua/lắp bản sao hoặc cấp ngoài VDD_SPI.

Xem [thiết kế buck](buck-3v3-design.md), [JSON số học](design-output/buck-3v3-calculations.json), [script](tools/calculate-buck-3v3.py), [audit nguồn](references/buck-3v3-source-audit-2026-11-06.md) và [trang KiCad](neocam-power-v0/neocam-power-3v3-v0.6.kicad_sch). **DC có thể đạt theo miền trở đã đặt; chưa đủ dữ liệu vòng điều khiển/Ceff/L/tải để chứng minh ESP/microSD ổn định khi quay.** Đây là tồn đọng thực của thiết kế v0.6, bên cạnh mục tiêu45phút và phối hợp sạc/PCM còn mở.

Nhánh đo pin hiện tại dùng **MAX17048**, tối đa40uA active theo điều kiện datasheet; IC lấy nguồn BAT, pullup SDA/SCL/ALRT lên3V3. Điện trở I2C2,2k và ALRT10k, nút10k và LED2,2k nằm trong ngân sách3V3, không cộng chúng hai lần ở pin. Dòng từng pullup phụ thuộc thời gian logic-low; cần giữ hệ số hoạt động khi tính trung bình. Điểm mic670uA ở1,8V không thay bằng một maximum tại3,3V.

Kết quả tải và lịch sử tính nền: [JSON](design-output/calculations.json), [load-evidence.csv](design-output/load-evidence.csv), [path-sag-envelope.csv](design-output/path-sag-envelope.csv), [runtime-envelope.csv](design-output/runtime-envelope.csv). Các bảng RQ1 cũ là lịch sử, không thay JSON chuyển nguồn v0.5. Các số khảo sát là phân bổ thiết kế, không phải đo NeoCam.
