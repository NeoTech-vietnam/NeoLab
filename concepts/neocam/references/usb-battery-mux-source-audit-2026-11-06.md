# NeoCam: chuyển USB/pin khi đang quay

Ngày: 2026-11-06. Nghiên cứu thiết kế bằng tài liệu hãng; không đo bo. Bo chính tham chiếu V1.51, revision thật chưa xác nhận. Mục tiêu là không reset hoặc gián đoạn lưu hình-tiếng khi cắm/rút USB trong một miền điện áp, tải và nhiệt độ xác định; không bao gồm pin bị rút, PCM cắt, ngắn mạch hoặc mọi lỗi nguồn có thể xảy ra.

**Quyết định cuối của v0.5: chọn TPS2121RUXR với SS và giới hạn dòng lập trình được; không dùng TPS2116 hoặc diode fallback trong BOM cuối.** Bộ tụ cuối gồm **3×330µF T520X337M010ATE025**, qualification Ceff>=750µF/ESR<=15mΩ, Qreverse<=20µC, Icap_peak<=3A và gap<=50µs. Đọc [hồ sơ chuyển nguồn hiện hành](../usb-battery-switchover-design.md) và [BOM v0.5](../neocam-power-v0/bom-reference-v0.5.csv). Các mục 1-7 lưu phương án ban đầu, không phải BOM hiện hành; các ngân sách2tụ/500µF/30mΩ/10µC/1,2A trước đây đã được thay. Mục0 dưới đây đã đối chiếu với cấu hình cuối.

## 0. Cấu hình hiện hành TPS2121 và các phép tính đã đối chiếu

Nguồn: [TI TPS2121, Rev.F](https://www.ti.com/lit/ds/symlink/tps2121.pdf), pin trang4-5, điện trang7-8, soft start trang12-13, XCOMP trang25-26; [đúng mã TPS2121RUXR](https://www.ti.com/product/TPS2121/part-details/TPS2121RUXR). Chọn TPS2121 vì kiểm soát được khởi động, không phải vì 5µs là maximum: số đó vẫn là TYP.

| Chân RUX12 | Kết nối cuối |
|---|---|
| 1 và 8 OUT | Cùng nối SYS/VIN XIAO, tụ bulk SYS-GND |
| 2 IN2 | Pin 1S được bảo vệ, sau cầu chì |
| 3 CP2 | Divider pin 20k/10k xuống GND |
| 4 OV2, 5 OV1 | GND, không dùng OVP; chỉ nhận nguồn USB 5V đúng hợp đồng |
| 6 PR1 | Divider USB 20k/10k xuống GND |
| 7 IN1 | USB VBUS |
| 9 ST | Theo hãng nối GND nếu không đọc; nếu đọc phải theo mạch status đúng datasheet |
| 10 ILIM | 80kΩ,1%, xuống GND |
| 11 SS | 1µF X7R10V xuống GND; tính dung lượng hiệu dụng |
| 12 GND | GND chung; dùng đúng footprint RUX12, không dùng RNW11 do tên cũ ở bảng nhiệt |

Tháo cả Q1/DMP adapter và D2 chọn nguồn stock; giữ mạch sạc VBUS -> U4 -> BAT và buck stock. Không có diode bypass MUX trong lựa chọn cuối, vì nó sẽ bỏ qua SS/ILIM và thêm nhánh dòng rò về pin.

### 0.1. XCOMP và USB priority có xét góc dung sai

Hai divider cùng Rt=20kΩ/Rb=10kΩ, từng điện trở1%. Gọi alpha=Rb/(Rt+Rb):

\[
\alpha_{min}=0,3289036545;\qquad\alpha_{max}=0,3377926421.
\]

Rth lớn nhất dùng để chặn sai số leakage là6,733kΩ; leakage controlpin ±0,1µA tạo sai số tối đa0,673mV mỗi nhánh. Với USB>=4,75V và pin<=4,35V:

\[
V_{PR1,min}=4,75\alpha_{min}-0,000673=1,561619V;
\]
\[
V_{CP2,max}=4,35\alpha_{max}+0,000673=1,470071V.
\]

Hiệu91,548mV lớn hơn offset comparator max40mV, còn **51,548mV margin**. Tại pin3,8V, CP2min=1,24916V>VREF rising max1,1V; tại3,9V, CP2min=1,28205V. Do đó input pin trong miền vận hành đã chọn đủ điều kiện bật fast mode sau khi valid và settled. Đây là kiểm tra DC và logic, không biến5µsTYP thành MAX.

### 0.2. Giới hạn dòng và khởi động

Công thức hãng, R tính kΩ:

\[
I_{LIM,typ}=65,2/R_{ILIM}^{0,861}.
\]

R80k cho1,498605A; dung sai điện trở±1% cho1,485821..1,511629A theo **công thức nominal**. Bảng hãng tại đúng80k ghi1..2A trên dải nhiệt. Không gọi khoảng nominal có dung sai R là khoảng thực của IC. Nếu tạm scale bound1A theo Rmax80,8k, được khoảng0,99147A; đây là ngoại suy điều kiện, chưa là giới hạn hãng riêng. Vì vậy tải SYS1A đụng sát giới hạn thấp; nên dùng **tải vận hành<=0,9A**, vẫn tính hold-up với góc1A, hoặc xác nhận lại setting khác trước khi tuyên bố không current-limit.

CSS1µF tại5V cho slew khoảng88V/s trong bảng **estimated**; chọn yêu cầu thiết kế SR<=200V/s. Ba tụ330µF, dung sai+20%, có tổngCmax1188µF:

\[
I_{cap,max,design}=C_{max}SR_{max}=1188\mu F(200V/s)=237,6mA.
\]

Nếu tải chuẩn bị cold start<=0,5A, tổng<=0,7376A, dưới ngân sách pack1A. SR200V/s là tiêu chí thiết kế/điều kiện cần xác nhận, không phải MAX datasheet. Tính cả startup delay, input settling và lần soft start khi nguồn thứ hai xuất hiện. Chỉ cho quay khi nguồn pin dự phòng đã valid và settled; không bật AV nặng trong lúc cold start rồi dùng phép tính0,5A.

### 0.3. Hold-up trong miền hoạt động cuối

Chọn: Vpack_loaded>=3,9V, I_SYS<=1A cho góc tính toán (AV bình thường<=0,9A), Rext<=0,100Ω, RMUX<=0,100Ω, **Ceff>=750µF, ESR<=15mΩ, Qreverse<=20µC, Icap_peak<=3A**, parasitic/ESL budget20mV, tổng gap<=50µs. RMUX0,1Ω tại pin thấp là ngân sách chấp nhận thiết kế: bảng max100mΩ của hãng nêu VIN>=5V/200mA, không tự coi bảo đảm tại pin3,9V/1A.

\[
V_{SYS,steady}\ge3,9-1(0,1+0,1)=3,7V;
\]
\[
V_{SYS,min}\ge3,7-3(0,015)-0,020-\frac{1(50\mu s)+20\mu C}{750\mu F}
=\boxed{3,541667V}.
\]

Theo mô hình, cao hơn VIN required3,5V khoảng41,667mV. Gap lớn nhất ngân sách tụ hỗ trợ sau khi dành điện lượng cho dòng ngược:

\[
t_{gap,allow}=\frac{(3,7-3,5-0,045-0,020)(750\mu F)-20\mu C}{1A}=81,25\mu s.
\]

Đây là chứng minh **bất đẳng thức của mô hình và miền thiết kế**. Gap50µs là giới hạn toàn hệ được yêu cầu, không gắn nhãn MAX từ datasheet; gồm detection/commutation/đường nguồn và hành vi reverse blocking. Với các điều kiện còn lại giữ nguyên, gap lớn hơn81,25µs sẽ không đạt. Bộ tụ990µF nominal ở dung sai-20% còn792µF trước các yếu tố khác; qualification cuối yêu cầu Ceff>=750µF và ESR<=15mΩ. Không dùng990µF trên nhãn để mặc định Ceff990µF.

### 0.4. Các điều kiện giữ nguyên trạng thái chưa đóng

Điện áp chuẩn bị3,9V là tại cọc pack có tải; không phải OCV và không tính lặp R_PCM vào Rext. Định mức pack1A không có nghĩa mọi giới hạn quá dòng MUX2A đều phù hợp cell; PCM/fuse phối hợp riêng. USB phải đúng5V, vì OV1/OV2 nốiGND không bảo vệ buck/camera khỏi9V; TPS2121 chịu22V không làm cả NeoCam chịu22V. Không claim pin đạt45phút hoặc hoạt động không reset trên mọi nhiệt độ chỉ từ những kiểm tra này.

### 0.5. Điện áp tại IN2 và giới hạn của các bound chuyển nguồn

Điều kiện nguồn pin được đặt trực tiếp tại **IN2/VBAT_XIAO >=3,8V khi có tải**, nút MAX17048 đo. Vpack_loaded >=3,9V cùng Rexternal <=0,1Ω và dòng<=1A là điều kiện đủ từ phía pack, không phải điều kiện cần duy nhất. Không trừ Rexternal thêm lần nữa khi đã lấy điện áp tại IN2. Tải AV bình thường<=0,9A; góc hold-up dùng1A để có dự phòng.

Nguồn USB được yêu cầu **5V/3A có xác nhận**, điện áp4,75-5,25V tại MUX. Khả năng cấp dòng của nguồn/cáp phải được xác định; đầu USB-C không tự là nguồn3A. OV1/OV2 nốiGND không bảo vệ XIAO khỏi nguồn9V.

Các giới hạn gap50µs, Qreverse20µC, Icap_peak3A, RMUX0,1Ω tại pin3,8V và SR200V/s là **điều kiện thiết kế/nghiệm thu**, chưa là maximum toàn hệ do hãng bảo đảm. Datasheet RCB có ngưỡng dòng ngược tới2A và response10µs TYP; dòng tụ có thể gồm tải và dòng ngược, nên chọn góc3A thay cho1,2A trước đó. Ngưỡng2A không tự giới hạn overshoot hoặc chứng minh Qreverse<=20µC. Số3,541667V đúng số học của mô hình nhưng chưa là minimum guaranteed của sản phẩm.

Các ngân sách hai tụ330µF/Ceff500µF/ESR30mΩ/Qreverse10µC/Icap_peak1,2A đã bỏ; không dùng lại cho BOM v0.5. Bộ ba tụ cuối và các bound mới thay thế chúng, giữ nguyên sự phân biệt giữa thiết kế có điều kiện, mô phỏng và kết quả phần cứng.

## 1. Phương án ban đầu: TPS2116DRLR, đã thay bằng TPS2121

[TI TPS2116 Rev.A](https://www.ti.com/lit/ds/symlink/tps2116.pdf), đã xem trực tiếp bảng trang 7: tSW **8 µs tại 5 V; 6,2 µs tại 3,3 V; 17,7 µs tại 1,8 V đều ở cột TYP, 25°C**. Không có maximum. Điện áp vào 1,6-5,5 V, dòng liên tục 2,5 A. MODE nối VIN1, PR1 dùng divider tạo ưu tiên USB bằng phần cứng. RON max 59 mΩ tại VIN 3,3 V/IOUT 200 mA/-40..105°C; không tự chuyển thành giới hạn ở mọi dòng.

[TPS2121 Rev.F](https://www.ti.com/lit/ds/symlink/tps2121.pdf), cũng đã xem bảng trang 7: fast tFSW **5 µs chỉ TYP**, yêu cầu CP2 >= VREF; chế độ thường 100 µs cũng TYP. Có soft start và giới hạn dòng điều chỉnh được, nhưng không giải quyết thiếu tSW maximum; RON maximum trong bảng chỉ tại VIN >= 5 V. Nó lớn và tiêu thụ tĩnh hơn, không có lợi thế rõ cho pin 1S của NeoCam.

[LM66200](https://www.ti.com/lit/ds/symlink/lm66200.pdf) tự chọn nguồn điện áp cao nhất, không đặt ngưỡng ưu tiên USB riêng; thời gian chuyển cũng TYP. [TPS2117](https://www.ti.com/lit/ds/symlink/tps2117.pdf) giảm RON và tăng dòng, nhưng không tạo bằng chứng mới về tSW maximum. Chọn TPS2116 cho nguyên lý tham chiếu này; không chọn theo duy nhất con số microsecond ở trang đầu.

## 2. Sửa đường nguồn thật, không mắc thêm ngoài bo rồi bỏ qua đường stock

Theo [Seeed V1.51, trang 4](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/new-res/202003753_XIAO%20ESP32S3%20Sense_V1.51_SCH_260611.pdf#page=4), phải loại đường chọn nguồn cũ khỏi VIN:

- Tháo Q1 stock và không lắp DMP2035U của biến thể v0.4.
- Tháo D2 stock USB -> VIN. Không để nó bypass MUX.
- Giữ USB-C/VBUS -> U4 SGM40567 -> BAT để sạc; giữ U3 SGM6029 và nhánh VIN cấp expansion board.
- TPS2116 VIN1 = VBUS; VIN2 = pack đã bảo vệ sau cầu chì; VOUT = VIN thật của XIAO.
- MAX17048 tiếp tục đo điện áp pack/BAT, không nối nguồn nó vào SYS rồi gọi là điện áp pin.

Pin TPS2116: GND1; VOUT2,7 cùng nối SYS; VIN1 pin3; PR1 pin4; MODE5 nối VIN1; VIN2 pin6; ST8 open-drain để NC hoặc kéo lên 3V3 host nếu đọc trạng thái. Hai chân VOUT đều phải nối và đi đường đủ rộng. Thêm tụ ít nhất 1 µF ở mỗi input, tụ bulk SYS đặt sát IC/đường về GND.

## 3. Ngưỡng USB có xét dung sai

TPS2116 VREF = 0,92..1,08 V. Chọn Rtop **32,4 kΩ 1%** từ VBUS đến PR1, Rbottom **10 kΩ 1%** từ PR1 xuống GND; MODE nối VBUS. Khi USB vắng, MODE/PR1 thấp và nguồn pin vẫn được chọn theo bảng trạng thái.

\[
V_{USB,trip}=V_{REF}(1+R_t/R_b)+I_{PR1}R_t.
\]

Nominal 4,24 V. Xét tỷ số điện trở và leakage PR1 tối đa độ lớn 0,1 µA cho khoảng **3,84-4,65 V**. Vì vậy USB hợp lệ cho tính năng ưu tiên được định nghĩa **4,75-5,25 V tại đầu vào MUX**, không phải tại adapter xa đầu dây. Nếu dùng nominal threshold 4,5 V mà bỏ ±8% VREF thì có thể không chọn USB ở 4,75 V.

Không thêm tụ lọc lớn tại PR1 rồi bỏ qua trễ phát hiện. Có thể tính thêm tụ nhỏ sau khi mô hình hóa cạnh; ngưỡng không có hysteresis được đảm bảo riêng trong bảng này, nên nguồn rung gần threshold vẫn là tình huống cần xử lý.

## 4. Hold-up bằng tụ: công thức đúng và giới hạn bằng chứng

\[
V_{SYS,min}\ge V_{pack,loaded}-I_{SYS}(R_{ext}+R_{MUX})
-I_{SYS}ESR-\frac{I_{SYS}t_{gap}}{C_{eff}}-V_{parasitic}.
\]

Ví dụ lựa chọn thiết kế: I_SYS <= 0,8 A; Vpack_loaded >= 3,8 V; Rext <= 0,100 Ω; RMUX <= 0,080 Ω; Ceff >= 220 µF; ESR <= 50 mΩ; tgap <= 20 µs. Bỏ parasitic riêng để thấy số học:

\[
V_{SYS,min}\ge3,8-0,8(0,100+0,080)-0,8(0,050)
-0,8(20\mu s)/(220\mu F)=3,54327\ V.
\]

So với VIN required 3,5 V của mô hình buck, còn khoảng 43 mV trước parasitic. **20 µs ở đây là tiêu chí dự phòng thiết kế, không phải tSW max của TI**, nên phép tính này chưa phải chứng minh vô điều kiện không reset. Tại pin 3,7 V, cùng ngân sách chỉ còn 3,443 V và không đạt. Ceff là điện dung tối thiểu thực ở điện áp DC/nhiệt độ/dung sai, không lấy 220 µF in trên tụ làm 220 µF dùng được.

Khi chuyển USB cao xuống pin thấp, MUX chờ SYS giảm gần pin để tránh đẩy điện vào pin; thời gian coast 5 V -> gần pin không đồng nhất với khoảng thiếu nguồn dưới mức pin. Khi chuyển pin thấp lên USB cao, dòng nạp lại tụ cũng phải tính.

## 5. Nhánh diode dự phòng: giảm phụ thuộc vào tSW, thêm điều kiện khác

Nhánh thụ động **anode BAT sau cầu chì -> cathode SYS** có thể cấp nguồn trong khoảng MUX break-before-make. Nó không đợi GPIO hoặc timer. Tuy nhiên diode không có sụt áp bằng 0 và không chặn dòng rò tuyệt đối.

[Nexperia PMEG2010ER, July2023](https://assets.nexperia.com/documents/data-sheet/PMEG2010ER.pdf): 1 A, SOD123W 2,6 x 1,7 x 1 mm; pin1 K/cathode nối SYS, pin2 A/anode nối BAT. VF max **340 mV tại 1 A/25°C**; IR typ60 µA tại VR5 V/25°C, max1 mA tại VR20 V/25°C. Không công bố VF max toàn dải nhiệt ở dòng của NeoCam.

[PMEG3050EP](https://assets.nexperia.com/documents/data-sheet/PMEG3050EP.pdf) giảm VF max xuống275 mV tại1 A/25°C và cho nhiều dự phòng dòng hơn, nhưng kích thước3,8x2,6mm và IR typ330µA tạiVR5V cao hơn. Không nên bỏ qua dòng rò chỉ vì tên gọi Schottky.

Đề xuất ưu tiên **PMEG2010ER** cho nhánh fallback khi I_SYS <= 0,8 A và có tính dòng xung; dùng ngân sách VF **0,40 V là điều kiện thiết kế cần được xác nhận** cho dải nhiệt sản phẩm, không viết thành max do hãng bảo đảm. Với Vpack >= 4,0 V, Rext <= 0,1 Ω và parasitic/ringing <= 20 mV:

\[
V_{SYS,floor}\ge4,0-0,8(0,1)-0,40-0,02=3,50\ V.
\]

Tại đúng 25°C, thay ngân sách0,40V bằng bảng0,34V cho3,56V; vẫn cần kiểm tra điều kiện dòng không vượt1 A. Nhánh này tạo sàn DC độc lập với **thời gian BBM MUX**, nhưng không bao gồm MUX hỏng/đang dẫn dòng ngược vào một đầu USB bị cưỡng bức ngắn mạch. Cắm/rút USB bình thường khác với đầu vào hard-short.

**Các đánh đổi phải ghi trong bản mạch:**

- Nhánh diode bypass soft start/current limit/hard-off của MUX ở phía pin. PCM và fuse vẫn hoạt động nhưng không được coi như mạch hạn dòng nạp tụ chủ động.
- Khi cắm pin ban đầu, tụ SYS có thể nạp nhanh qua diode; dòng/điện lượng/inrush, giới hạn xung cell/PCM/fuse cần tính riêng. Nếu bắt buộc kiểm soát, đặt thêm load switch có slew rate ở trước cả VIN2 và diode, ví dụ [TPS22918](https://www.ti.com/lit/ds/symlink/tps22918.pdf); nó thêm sụt áp và cần tính lại toàn bộ floor. Không tự coi ON/CT là giới hạn dòng được bảo đảm.
- Dòng rò diode từ SYS cao về BAT khi USB cắm bỏ qua đường sạc chuẩn. Phải đánh giá overcharge/trickle theo cell và PCM, kể cả nhiệt độ; không mô tả diode là bảo đảm zero reverse charge.
- Nếu dùng tụ bulk nhỏ hơn nhờ fallback, vẫn cần tụ xử lý inductance, reverse-current transient và cạnh diode; layout không được bỏ qua.

## 6. Khởi động, dòng USB và RCB

TPS2116 dùng soft start khi SYS <1 V; khi đổi nguồn và SYS >1 V, không áp soft start. tSS5V=1,7ms chỉ TYP. Công thức Iinrush = COUT*VOUT/tSS cho330µF/5V khoảng0,97A TYP, chưa cộng tải và109mA sạc stock; không coi nó là max. Cần nguồn USB/cáp đáp ứng và cơ chế phát hiện khả năng cấp dòng nếu dùng USB-C chuẩn; có đầu USB-C không tự cho phép rút3A.

RCB có threshold điện áp và dòng, response2µs là TYP theo bảng, không phải zero reverse current tức thì. MUX không thay charger hoặc PCM. Datasheet còn yêu cầu nguồn dự phòng >=2,5V khi nguồn đang cấp sụp nhanh hơn1V/10µs để tránh reset chính IC; pack >=3,8V đáp ứng điều kiện này, nhưng không suy ra mọi rail tải chắc chắn ổn định.

## 7. Kết luận phương án ban đầu, được thay bởi mục 0

Phân tích ban đầu đề xuất TPS2116 thay Q1/DMP/D2 và xét PMEG2010ER fallback. **Không dùng đề xuất này làm BOM cuối**; đã chuyển sang TPS2121 ở mục0 để có SS/ILIM lập trình được và bỏ diode bypass. Các công thức ở trên vẫn giải thích đánh đổi và giới hạn nguồn dữ liệu.

Tài liệu hiện đủ để vẽ nguyên lý, đặt thông số và mô phỏng các góc thiết kế. Chưa đủ dữ liệu hãng để tuyên bố không reset trên toàn nhiệt độ chỉ từ một con số5/8µs. Cần giữ riêng các mục chưa đóng: bound timing MUX hoặc bound fallbackVF, leakage sạc qua diode, startup inrush, dynamic buck/3V3 và thực thi giới hạn pin yếu. Mô phỏng ideal hoặc model hãng chỉ chứng minh mô hình đã dùng, không bổ sung một max datasheet còn thiếu.
