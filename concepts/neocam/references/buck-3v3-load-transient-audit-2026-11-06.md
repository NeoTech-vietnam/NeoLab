# NeoCam: thẩm định buck 3,3 V và tải động

Ngày: **2026-11-06**. Phân tích từ tài liệu hãng và mô hình lý tưởng có điều kiện; chưa đo hoặc thay linh kiện trên XIAO. Mục tiêu là nối tiếp thiết kế mux đã có, không mặc định VIN được giữ thì mọi chân tải cũng được giữ đủ điện áp.

**Quyết định hiện hành v0.6:** xem [thiết kế buck 3V3](../buck-3v3-design.md), [script tính toán](../tools/calculate-buck-3v3.py) và [kết quả JSON](../design-output/buck-3v3-calculations.json). Giữ stock buck; tổng Ceff cần xác nhận là **8–10 µF**. Với baseline yêu cầu **3,234–3,366 V**, ngân sách sụt nhánh **30 mV**, gợn chuyển mạch **20 mV**, bước ESR **20 mV**, ringing **20 mV**, mục tiêu được chọn là **Q− ≤ 0,25 µC** và **Q+ ≤ 0,40 µC**. Chúng cho cửa sổ tính toán **3,11275–3,476 V**, khi mọi điều kiện cùng đúng. Hai mức **0,35/0,59 µC ở mục 6 bên dưới là khảo sát sát biên**, không phải mục tiêu hiện hành; giới hạn điện lượng tính từ cửa sổ 3,1–3,5 V là **0,352/0,592 µC** tại 8 µF.

Ngân sách nhánh 30 mV phải bao gồm **toàn bộ sai lệch truyền áp DC và transient tới tải**, kể cả FB1; không chỉ DCR. Điện dung mà vòng buck nhìn thấy và điện dung cục bộ sau ferrite không tự động bằng nhau: cần xét trở kháng theo tần số. Các con số trên là tiêu chí thiết kế cần xác nhận, **không phải thông số đáp ứng tối đa do SGM công bố**. Ví dụ có tổn hao ở cuối mục 7 cho thấy mục tiêu Q− 0,25 µC có thể quá chặt tại một góc tải; chưa thể kết luận mạch stock đạt mục tiêu đó.

## 1. Kết luận thiết kế đề xuất

**Giữ SGM6029C, L4 và C5/C6 stock cho bản tham chiếu; không thêm tụ hàng chục/hàng trăm µF trực tiếp vào 3V3.** Mạch bổ sung nên có điểm đo Kelvin 3V3 tại ESP, VDD_SPI và VCC_SD_3V3 sau FB1, mỗi điểm có GND gần. Khi đóng vòng linh kiện phải kiểm tra tổng Ceff của cả bo chính và Sense, không chỉ đọc hai tụ cạnh buck.

Lý do: [SGM6029 RevA.1, trang3](https://www.sg-micro.com/rect/assets/3323f5dc-718f-4b81-b820-398ddb702989/SGM6029.pdf) chỉ định **COUT hiệu dụng 5–10µF**, L hiệu dụng0,33–1µH cho bảnA/B/C. Trang18 khuyến nghị L0,47/1µH với COUT10µF. Controller bù bên trong; không có cơ sở gọi mọi giá trị tụ lớn hơn đều ổn định.

Bank990µF của mux nằm ở **VIN trước buck**, không phải COUT3V3. Không chuyển nó sang đầu ra3V3 để sửa sụt áp. Tụ VIN lớn vẫn cần kiểm tra inrush và tương tác nguồn, nhưng là bài toán khác với bù vòng đầu ra.

## 2. Các giới hạn điện áp cần giữ

| Tải / điểm đo | Dữ liệu hãng | Áp dụng |
|---|---|---|
| Các nguồn chính ESP32-S3 | Khuyến nghị3,0–3,6V | Đo tại chân nguồn, không tại pin hoặc OUTmux |
| PSRAM tích hợp ESP32-S3R8 | Bản3,3V; VCC PSRAM3,3V class2,7–3,6V | Không thay bằng dữ liệuR8V1,8V |
| Flash GD25Q64ENIGR trên mainboardV1.51 | DòngGD25Q64E2,7–3,6V | Cấp qua VDD_SPI, không nối trực tiếp mặc định3V3 |
| microSD chế độ3,3V | SDA công bố VDD2,7–3,6V | Chưa có SKU đầy đủ của SanDiskUltra32GB để gán ripple/write-current riêng |

Nguồn: [ESP32-S3 v2.2, bảng1-1,5-2,5-3,5-12](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf); [GigaDevice GD25Q64E chính hãng](https://www.gigadevice.com/product/flash/spi-nor-flash/gd25q64e), [datasheetRev1.6](https://download.gigadevice.com/Datasheet/DS-00484-GD25Q64E-Rev1.6.pdf); [SD Association overview](https://www.sdcard.org/developers/sd-standard-overview/); [mainboardSeeedV1.51trang4](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/new-res/202003753_XIAO%20ESP32S3%20Sense_V1.51_SCH_260611.pdf#page=4).

**3,0V ở nguồn chính ESP không tự chứng minh bộ nhớ hoạt động.** VDD_SPI3,3V được cấp từ VDD3P3_RTC qua trở nội RSPI14ohm typical. Espressif yêu cầu:

\[
V_{RTC}>V_{memory,min}+I_{memory,max}R_{SPI}.
\]

FlashGD25Q64E có readmax13mA tại80MHz/x4/Qopen,20mA tại133MHz, program/erase25mA theo điều kiện bảng-40..85C. Chỉ riêng25mA qua14ohmtyp đã gây350mV; thêmPSRAM và sai sốR cần xét. Không gọi14ohm là maximum hoặc giả định activePSRAM=0. Vì vậy tiêu chí VDD_SPI>=2,7V cần được giữ riêng; chọn nguồn chính3,1V không tự đóng điều kiện bộ nhớ. Không tự nối VDD_SPI thẳng3V3 hoặc đổi eFuse để giải vấn đề khi chưa xác nhận topology/cấu hình hãng.

## 3. Đọc stock buck và tổng điện dung

Theo sơ đồV1.51: U3SGM6029CYG/TR, L4ghi470nH, R13=249kilohm chọn3,3V; C5/C6mỗi4,7µF ngay đầu ra; rail được ghiImax600mA. Còn các tụ phân tán tại ESP, đầu nốiSense, microphone và SD. Giá trị danh định cộng lại không bằng Ceff tại3,3V và từng tần số.

Không có mã cuộn cảm/MLCC stock đầy đủ trong sơ đồ đọc được, nên chưa có DCRhot, Ltheodòngbias và CefftheoDCbias. Không kết luận boardstock vi phạm dải5–10µF chỉ bằng tổng Cnom: MLCC nhỏ có thể giảmC đáng kể theoDCbias và từng tụ xa được nối qua trace/ferrite. Ngược lại, cũng không coi board đã được xác nhận nằm trong dải đó.

Có thể chọn **Ceff tổng cần nghiệm thu8–10µF** để lập ngân sách độ sụt, nhưng việc này không phải một lệnh lắp thêm8–10µF. Trước khi đổi C5/C6 phải phân bổ cả C tải còn lại và xác nhận ổn định/đáp ứng. Mã outputMLCC cần có dữ liệuDCbias đúng part; không dùng47µF10V chung chung như một giá trị hiệu dụng.

## 4. Headroom khi gần100% duty

SGM6029 có100%duty mode; khiVIN thấp, high-side giữ dẫn liên tục. Bảng hãng cho Rhighside185milliohmtyp/310milliohmmax tạiIOUT100mA, điều kiệnVIN3,6V; không áp như giới hạn đúng cho mọi dòng/VIN/nhiệt.

Đặt điều kiện thiết kế cần nghiệm thu:

\[
R_{HS,hot}+DCR_{L,hot}+R_{trace}\le0,500\ ohm,
\quad I_{3V3}\le0,600A.
\]

Với VINmin3,541667V của mux, giới hạn áp theo đường dẫn trong100%duty là:

\[
V_{ceiling,saturated}\simeq3,541667-0,600(0,500)=3,241667V.
\]

Đây là khả năng DC theo mô hình trở, không phải bảo đảm regulator luôn cho đúng3,241667V. Nó cho thấy có headroom cho mục tiêu baseline3,234V, nhưng rất ít tại tải lớn. Nếu giả sử Rhighside310milliohm ở điều kiện bảng, còn190milliohm choDCR/trace; phải xác nhận đúng nhiệt độ/dòng, không coi310milliohm là Rhot đã đo.

Outputaccuracy±2% của hãng được ghi ởPWM/IOUT0mA trong bảng5; không dùng một mình để chứng minh dưới tải600mA và mọi chuyển tiếp. Có thể **yêu cầu** baseline tại đầu ra buck3,234–3,366V trong cấu hình thực, nhưng phải đóng riêng load/line regulation.

## 5. Ripple dòng cuộn cảm và dòng đỉnh

Trong CCM lý tưởng, lấyVout3,3V, L0,47µH, fSW4MHztyp:

\[
D\simeq V_o/V_i,\qquad
\Delta I_{L,pp}\simeq\frac{V_o(1-V_o/V_i)}{Lf_{SW}},\quad
I_{pk}\simeq I_o+\Delta I_L/2.
\]

| VIN | D lý tưởng | DeltaILpp | Ipk khiIo600mA |
|---:|---:|---:|---:|
| 3,541667V | 0,931765 | 119,775mA | 659,887mA |
| 3,9V | 0,846154 | 270,049mA | 735,025mA |
| 5,25V | 0,628571 | 651,976mA | 925,988mA |

Đây không phải worstcase: f4MHzchỉtyp; Lgiảm theo bias/nhiệt; ripple thay đổi ởPFM và khi duty gần1. ỞLmin0,33µH nhưng vẫngiữ4MHz, VIN5,25V cho ripplekhoảng928,6mA và Ipk1,0643A. Cuộn cảm cần thỏa dòng bão hòa và nhiệt theo Ipeak/RMS, không chỉIo600mA. High-sidecurrentlimitmin1,58A/max1,98A củaSGM tại25C không tự là bảo đảm ở mọi điều kiện hoặc rating dòng tải board.

Mã tham chiếu hãngSGM đề cập có **MurataDFE18SANR47ME0**. [Danh sách model chính hãngMurata](https://www.murata.com/-/media/webrenewal/tool/library/common-pdf/dynamic-model/component-list-powerinductor-d-2602.ashx?cvid=20260515010000000000&la=en) ghi0,47µH, DCRmax64milliohm, mức dòng dựa trên nhiệt2,4A/bias3,1A theo điều kiện Murata. Đây là đối chiếu kỹ thuật, không xác nhận L4stock chính là part đó; vòng đời/footprint/dữ liệuhot phải kiểm tra trước khi dùng cho thiết kế mới.

## 6. Chứng minh sụt áp bằng điện lượng thiếu, không bịa thời gian vòng điều khiển

Ở điểm outputtụ, định nghĩa:

\[
Q_- =\int\max(i_{load}-i_L,0)dt,\qquad
Q_+ =\int\max(i_L-i_{load},0)dt.
\]

Q− tạo droop, Q+ tạo overshoot. Khi dòng controller khác tải, phần chênh do COUT cấp/nhận. Với Ceffthấp nhất:

\[
\Delta V_C\le Q_-/C_{eff,min},\qquad
\Delta V_{C,up}\le Q_+/C_{eff,min}.
\]

Phải cộng ESRstep, ESL/ringing, ripple và điện trở trace tới tải. Không thêm nhiều lần cùng một phần gợn. Nếu mục tiêu bộ nhớ còn đòi cao hơn, phải siết ngân sách hoặc thay kiến trúc, không chỉ giữ nguồn chínhESP.

**Một bộ tiêu chí khảo sát có tính toán**, chưa phải maximum/controllerđãchứng minh:

- Baseline tại đầu ra buck3,234–3,366V dưới tải thực.
- Tổng Ceff8–10µF.
- Sụt trace đầu ra tới chânESP<=30mV.
- Biên gợn chuyển mạch20mV, biên ESRstep20mV, biên ESL/ringing20mV.
- Qdeficit<=0,35µC; Qsurplus<=0,59µC.

Khi đồng thời đáp ứng:

\[
V_{ESP,min}\ge3,234-0,030-0,020-0,020-0,020-0,35/8
=3,10025V.
\]

\[
V_{rail,max}\le3,366+0,020+0,020+0,020+0,59/8
=3,49975V.
\]

Trong hai biểu thức, µC/µF choV. Bộ tiêu chí giữ nguồn chính trên3,1V và dưới3,5V, nhưng còn phải giữVDD_SPI>=2,7V vàSDsupply>=2,7V tại chân riêng. Qcần nghiệmthu bằng dạng dòng/tích phân thích hợp hoặc modelđãxácnhận; datasheetSGM không cho maximum thời gian phục hồi toànđiều kiện để điền thay.

## 7. Những ví dụ cho thấy yêu cầu đáp ứng nhanh

TạiVIN3,541667V vàL0,47µH, ngay cả khi high-side luônON, slope lý tưởng lớn nhất để tăng dòng là:

\[
di_L/dt=(3,541667-3,3)/0,47\mu H=0,514184A/\mu s.
\]

Một bước tải tăng500mA cần ít nhất0,972414µs để tăng dòng, **chưa tính delay controller và tổn hao**. Diện tích thiếu tam giác tối thiểu ở mô hình này là0,243103µC. So với tiêu chí0,35µC, còn0,106897µC, tương đương khoảng0,214µs nếu thiếu đủ500mA. Đây là minh họa tiêu chí chặt, không phải chứng minh controller đáp ứng hoặc chắc chắn không đáp ứng.

Không dùng50µsgap của mux như50µsbuckngừngcấp. Nếu toàn bộ tải600mA phải doC8µF cấp trong50µs thì:

\[
\Delta V=0,6(50\mu s)/(8\mu F)=3,75V.
\]

Điều này không thể giữrail3,3V. Thiết kế inputhold-up phải giữbuck cònhoạtđộng; COUTnhỏ chỉ đỡ chênh dòng trong quá trình vòng điều khiển điều chỉnh.

Đảo chiềuUSB/pin: nếu giữ duty cũ củaVIN3,541667V khiVIN bất ngờ lên5,25V, averageSW theo duty cũ có thể tạo slope tăng dòngkhoảng3,387A/µs trong mô hình lý tưởng. Controller/SS phải giảm duty và giới hạnQsurplus. Nếu **dạng VINrise thực** được chặn200V/s, VINchỉ đổi50µV mỗi chu kỳ4MHz, khác hẳn một bước tức thời; không mặc định CSS của mux đảm bảo điều này trong mọi hotplugmode khi chưa kiểm tra.

Chỉ tính năng lượng cuộn cảm rồi kết luận overshoot nhỏ cũng không đủ: tạiL0,47µH, I0,926A giảm về0,1A, năng lượng chênh0,199µJ chỉ nângC8µF quanh3,366V khoảng7,39mV nếu không còn năng lượng đầu vào. Năng lượng buck tiếp tục cấp trong delay có thể lớn hơn; phải tínhQsurplus toàn khoảng.

**Đối chiếu thêm khi có tổn hao:** đặt VIN = 3,5416667 V, Vout ban đầu = 3,234 V, dòng cuộn cảm ban đầu = 0,1 A; tải bước tức thời lên 0,6 A; L = 0,47 µH, C = 8 µF, tổng trở đường dẫn = 0,5 Ω. Cho high-side bật 100% ngay, không có delay controller và chưa có ESR/ESL, rồi giải:

\[
L\frac{di_L}{dt}=V_{IN}-R i_L-V_o,\qquad
C\frac{dV_o}{dt}=i_L-0,6.
\]

RK4 với bước 1 ns cho Vout thấp nhất khoảng **3,192216 V tại 1,804 µs**. Điện lượng thiếu tới điểm thấp nhất là **0,334272 µC** theo CΔV. Nó đã lớn hơn mục tiêu hiện hành 0,25 µC, dù còn nhỏ hơn giới hạn 0,352 µC. Không lấy đây làm dự đoán controller stock: bước tải 0,5 A và R = 0,5 Ω đều là điều kiện khảo sát, chưa phải tải/tổn hao đã xác nhận của NeoCam. Tuy nhiên ví dụ chứng minh rằng phải giải bài toán đồng thời L, C, R và tải; không thể tự gán Q− thấp hơn nhu cầu vật lý của chính góc tính toán rồi gọi là đã đóng thiết kế.

## 8. Điều chỉnh mạch nào có căn cứ ở thời điểm này?

1. Giữ buck/L4/C5/C6stock và cấu hình3,3V; không chuyển bankmux sang3V3, không thay hàng loạt linh kiện khi chưa có dữ liệu hiệu dụng.
2. Bổ sung điểm truy cập3V3 tạiESP, VDD_SPI, SDVDD sauFB1 vàGND gần để review/kiểm chứng sau; không đặt shunt điện trở tùy ý vào mọi nhánh rồi làm thay đổi đường nguồn chưa tính.
3. Khóa tổng Ceff/ESR/trace và tải động thành điều kiện có con số như mục6. Nếu không đáp ứng, chọn một regulator mới có dảiCOUT/bandwidth phù hợp và thay **cả khối nguồn có bố trí đúng**, không chỉ thêm tụ lớn ngoài dải củaSGM.
4. Bản tham chiếu có thể hoàn chỉnh về kết nối và phép tính mà vẫn chưa được duyệt tiêu chí noreset. Tồn đọng cụ thể: revision/bomstock thật; L/DCR/Ceff; tảiSDthực vàbuffer; dòngPSRAM/VDD_SPI; độ sụt và overshoot ở các chân tải; ổn định vòng với tổng điện dung; năng lượng đóng tệp.

Không có kết quả thử phần cứng, mô phỏng macromodelSGM hoặc nạpESP-IDF trongaudit này. Không lấy đồ thịtypical củaSGM thành maximum được bảo đảm cho NeoCam.
