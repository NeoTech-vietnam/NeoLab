# NeoCam: đối chiếu nguồn bo chính và nhánh đo pin

Ngày: 2026-11-06. Nghiên cứu từ tài liệu hãng; không đo phần cứng. Phần cứng người dùng là XIAO ESP32-S3 + OV3660 + SanDisk Ultra 32 GB. Bo chính thực tế chưa rõ revision. Mô hình bên dưới dùng **mainboard V1.51**; quyền dùng Exp. Board v1.2 không tự xác nhận mainboard V1.51.

## 1. Sơ đồ và đường dòng

Nguồn gốc: [Seeed XIAO V1.51, trang 4](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/new-res/202003753_XIAO%20ESP32S3%20Sense_V1.51_SCH_260611.pdf#page=4), đã đọc ảnh toàn trang.

- Pin VBAT nối **drain Q1 LP0404N3T5G**; source Q1 nối VIN. Gate nối VBUS và R9 100 kΩ xuống GND. Khi chỉ có pin, gate thấp và MOSFET dẫn sau khi VIN được nạp qua diode thân.
- VBUS qua D2 LMBR4010BST5G sang VIN. Khi USB có điện, gate Q1 lên cao để cách ly pin khỏi đường VIN, trong khi USB nuôi VIN qua D2.
- VIN cấp U3 SGM6029CYG/TR + L4 470 nH. R13 249 kΩ chọn 3,3 V. Sơ đồ ghi **Imax = 600 mA** tại VCC_3V3.
- U4 SGM40567-4.2XG/TR có VIN nối VBUS, BAT nối VBAT, F nối GND, R10 220 kΩ đặt dòng sạc.
- D0 là GPIO1/ADC1_CH0, không nối sẵn pin. D1 là GPIO2 còn có thể chọn làm điều khiển nhánh đo, sau khi khóa pin map sản phẩm.

Đây là mạch chọn nguồn rời bằng MOSFET/diode cùng IC sạc, không phải bằng chứng về chuyển nguồn không gián đoạn. Không đặt thêm bộ sạc song song BAT khi vẫn dùng USB-C stock. Nếu sạc qua pogo ở bản này, nguồn ngoài phải đi vào **VBUS 5 V với cách ly/chọn nguồn phù hợp**, không đưa 5 V vào BAT; chưa thiết kế pogo ở đây.

## 2. Khả năng buck và sụt áp

[SGM6029, trang 3, 5 và 16](https://www.sg-micro.com/rect/assets/3323f5dc-718f-4b81-b820-398ddb702989/SGM6029.pdf): VIN 1,95-5,5 V; bản C cho 1 A tại VIN >= 2,3 V, 0,7 A dưới 2,3 V. Đây là giới hạn IC, không nâng ghi chú 600 mA của bo thành 1 A. Giữ **600 mA tổng rail 3,3 V** làm giới hạn tham chiếu bo. Không phân bổ thêm 600 mA ngoài tải ESP.

High-side RDS(on) 185 mΩ typical / 310 mΩ maximum tại IOUT 100 mA, với điều kiện bảng VIN 3,6 V. Không giả định đúng tại mọi dòng/VIN. 100% duty nối high-side dẫn liên tục khi VIN thấp; buck không nâng áp. Datasheet không cung cấp một ngưỡng pin duy nhất bảo đảm 3,3 V cho mọi tải. Đồ thị hiệu suất là typical, không có hiệu suất tối thiểu được bảo đảm cho toàn bo.

[Q1 LP0404N3T5G, LRC Rev.G](https://www.lrc.cn/Upload/PDF/Product/MOS/LP0404N3T5G.pdf) ghi **RDS(on) <= 0,67 Ω** tại VGS = -2,5 V, ID = -660 mA, 25°C; <= 0,48 Ω tại -4,5 V/-780 mA. Không thay Q1 bằng giả định 20 mΩ. Ví dụ riêng tại R = 0,67 Ω và I = 0,5 A:

\[
\Delta V_{Q1}=0,5(0,67)=0,335\ V;\quad P_{Q1}=0,5^2(0,67)=0,1675\ W.
\]

Đây không phải upper bound trên toàn dải nhiệt. Vì camera và buck cùng lấy VIN, Q1 mang tổng dòng hai nhánh. Gần 100% duty:

\[
V_{3V3}\simeq V_{cell}-I_{bat}(R_{PCM}+R_{wire}+R_{Q1})-I_{3V3}(R_{HS}+R_L).
\]

Muốn giữ rail tối thiểu \(V_{rail,min}\), phải có tổng sụt áp <= \(V_{cell}-V_{rail,min}\). Không chốt 3,5 V làm điện áp dừng chỉ bằng điện áp danh định ESP. Cần kiểm tra cả flash, PSRAM, SD và rail camera. ESP32-S3 khuyến nghị 3,0-3,6 V tại chân nguồn: [bảng 5-2](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf#page=64).

## 3. Sạc stock: tính được gì và thiếu gì

[SGM40567, trang 3-5, 11-12](https://www.sg-micro.com/rect/assets/e95e555a-a8a2-4761-b86a-6554dff2d8ed/SGM40567.pdf) cho:

\[
I_{CHG,nom}=24000/220=109,09\ mA.
\]

F = GND chọn 4,2 V danh định. Bảng 25°C tại IBAT = 6 mA ghi 4,166-4,234 V, nhưng **không coi 4,234 V là trần điện áp trong mọi tình huống**: thuật toán có bù sụt áp điện trở khoảng 2% (bảng 1,2-2,7%) khi phép kiểm tra đủ điều kiện, rồi duy trì fold-back khoảng 96% mức sạc. Phải đối chiếu điện áp thực tại cell và pack.

Không có chân TS/NTC. Giảm dòng khi die nóng khoảng 130°C không phải bảo vệ nhiệt độ cell. IREF có thể cấm sạc khi kéo > 1,6 V, nhưng stock không nối GPIO/NTC vào chân này. Ngưỡng kết thúc dòng theo bảng là 4,2-8,8% ICHG tại điều kiện công bố; tải trên BAT có thể ảnh hưởng phát hiện. Mạch USB chọn nguồn nhằm chuyển tải sang VBUS, nhưng cần xác nhận chuyển tiếp.

Với pin 800 mAh: \(800/109,09=7,33\ h\) chỉ là thời gian chia dung lượng cho dòng CC danh định; không phải thời gian đầy dự đoán, vì còn CV/điều kiện khởi đầu/nhiệt/dung sai. Chế độ sạc sản phẩm cần giới hạn nhiệt độ theo cell và cơ chế ngắt thực sự; firmware không đọc NTC thì không được mô tả đã có bảo vệ này.

## 4. Nhánh ADC: phương án đã phân tích, không chọn cho v0.3

Không đóng JP1 v0 để nối divider BAT/2 trực tiếp vào ESP nếu chưa xử lý nguồn chip tắt. Điện trở lớn chỉ giới hạn dòng, không tự chứng minh chân GPIO chịu được điện áp khi VDD = 0. [Espressif ADC checklist](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html#adc) khuyến nghị ADC1 và tụ lọc 0,1 µF; bảng DC pin gắn với điện áp miền nguồn, không đưa ra một dòng injection cho phép dùng để bỏ qua cấp ngược.

Lựa chọn tham chiếu có tài liệu: **TMUX1511PWR**, TSSOP-14. [TI datasheet Rev.B, trang 3-7, 22](https://www.ti.com/lit/ds/symlink/tmux1511.pdf): VDD 1,5-5,5 V; tín hiệu 0-3,6 V khi VDD < 1,5 V; powered-off I/O leakage tối đa ±2 µA trên dải -40..125°C tại VDD=0/VS<=3,6V/VD=0. Giá trị ±10 nA chỉ tại 25°C/VS<=3V. RON max4,5Ω, IDD max70µA, switching logic high >=1,2V, low<=0,45V.

Đề xuất cấu trúc:

```text
BAT -> Rt100k -> DIV -> switch CH1 -> Rs1k -> ADC(D0)
                  |                         |
                 Rb100k                    Rp100k
                  |                         |
                 GND                       GND
                  |
        C100nF mắc DIV-GND, trước switch
```

VDD switch lấy 3V3 XIAO, thêm 100 nF decoupling; SEL1 điều khiển bằng GPIO, mặc định kéo xuống 100 kΩ. Dùng phần mềm chỉ bật khi MCU/ADC sẵn sàng. Các SEL không dùng nối GND, S/D không dùng có thể nối GND cho sơ đồ rõ. Không dùng TMUX1101 chỉ vì có fail-safe logic: fail-safe logic và powered-off signal isolation là hai đặc tính khác.

TSSOP-14: SEL1=1, S1=2, D1=3, SEL2=4, S2=5, D2=6, GND=7, D3=8, S3=9, SEL3=10, D4=11, S4=12, SEL4=13, VDD=14.

Với Rt=Rb=Rp=R và r=Rs+RON, khi ON:

\[
\frac{V_{ADC}}{V_{BAT}}=\frac{R}{3R+2r}.
\]

Rs=1k/RON≈2Ω cho tỷ số ≈0,33112, VADC≈1,402 V tại VBAT=4,234 V; bỏ Rs thì tỷ số gần 1/3, không phải 1/2. Khi OFF, DIV gần VBAT/2; VADC có pulldown. Ước lượng DC từ leakage2µA vào Rp101k (dung sai+1%) là 0,202V; không coi đây là chứng minh transient hoặc bảo đảm GPIO-off chưa công bố.

**Đặt tụ lọc trước switch** để không cố ý giữ điện áp ~1,4 V bằng 100nF ở chính chân ADC khi MCU mất điện đột ngột. Dòng tích trữ, parasitic, tOFF và sequencing vẫn phải xác nhận. Bộ lọc trước switch có tau khoảng3,34ms khi ON/5ms khiOFF; chờ ít nhất5tau theo mô hình RC, không thay cho yêu cầu SAR settling/hiệu chuẩn. Một điện trở bottom bị hở có thể làm lỗi điện áp; đây chưa phải chứng nhận chịu mọi single fault.

## 5. Cách dùng trong ngân sách và quyết định

- Phân biệt số liệu tải datasheet, phân bổ thiết kế và điều kiện nghiệm thu. Có thể chọn dung lượng dòng rail3,3V<=600mA và nhánh camera theo hai LDO, nhưng tổng các giới hạn linh kiện không tự là maximum của tác vụ quay.
- Tính dòng qua đường pin bằng \(I_{bat}=I_{cam,VIN}+P_{3V3}/(\eta V_{IN})+I_{aux}\) khi đường camera lấy VIN từ pin và buck đang ổn áp; VIN là điện áp **sau** sụt áp PCM/dây/Q1, không tự thay bằng Vbat. Giải cùng quan hệ \(V_{IN}=V_{cell,loaded}-I_{bat}R_{path}\). \(\eta\) là biến hoặc mục tiêu nghiệm thu có ghi nguồn, không gọi 80/90% là min datasheet.
- Tính năng lượng 45 phút từ dòng trung bình; tính cell/PCM/dây từ dòng liên tục và xung riêng. Không dùng pin800mAh/1A allocation để bảo đảm thời lượng.
- Giữ USB-C stock là đường sạc duy nhất của bản nguyên mẫu; phần phát triển NTC, pogo/chọn nguồn, thermal, brownout/save và revision thực là tồn đọng cụ thể.

## 6. Tài liệu và mức bằng chứng

Tất cả URL bên trên là nhà sản xuất hoặc tài liệu hãng lưu bởi Seeed. Không đo NeoCam; không mô phỏng switch/charger/buck bằng macromodel chính hãng; không chốt cell/PCM; không xác nhận cùng quay720p30/thu tiếng/ghiSD. Sơ đồ có thể hoàn chỉnh ở mức kết nối tham chiếu mà các điều kiện nghiệm thu này vẫn còn mở.

## 7. Cập nhật quyết định cuối lượt: dùng MAX17048 thay nhánh ADC

Sau khi xét tắt nguồn nhanh, nhánh TMUX ở mục 4 được giữ làm **phân tích phương án**, không phải lựa chọn cuối. TMUX1511 vẫn có tOFF(VDD) hữu hạn (max4µs tại điều kiện bảng), nên tụ DIV có thể phóng vào ADC trong khoảng switch chưa mở khi VDD sụp nhanh. Đặt tụ trước switch cải thiện cách ly sau khi mở nhưng không tự chứng minh toàn bộ quá trình tắt nguồn. Hướng cuối đề xuất là **MAX17048G+**, đọc điện áp qua I2C; chưa dùng số SOC làm bằng chứng thời lượng.

[Analog Devices MAX17048/MAX17049 Rev7](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX17048-MAX17049.pdf), bảng điện trang2 và pin trang6: MAX17048 dành cho1S, VDD2,5-4,5V; SCL/SDA/ALRT từ-0,3đến5,5V tham chiếuGND. **VDD là cả nguồn và đầu đo điện áp**. Đấu TDFN8: CTG1/GND4/EP xuốngGND; CELL2 vàVDD3 lênVBAT_XIAO sau fuse (CELL2 khôngnối trong MAX17048 nhưng theo hướng dẫn nốiBAT+); ALRT5 nốiD0/GPIO1 với10k pullup3V3; QSTRT6 xuốngGND; SCL7/SDA8 sanghostD5/GPIO6 vàD4/GPIO5. C100nF VDD-GND. PullupSCL/SDA**2,2k** lên**3V3 củaESP**, tuyệtđối không kéo lênBAT.

SCL là input; SDA/ALRT là open-drain, không có ngõ ra push-pull kéo lên nguồn pin. Do đó tránh đường analogBAT vào GPIO khiESP tắt. Hành vi mất pin/USB còn phải đưa vào kế hoạch xác nhận. Chọn100kHz với pullup2,2k và ngân sáchCb150pF: tr=0,8473*2200*150pF≈280ns, dưới300ns của bảng timing. Pullup+1% cho khoảng282ns. Tại3,6V/VOL0,4V/R-1%, dòng sink≈1,47mA, dưới điểm thử4mA. Đây là tiêu chí layout, chưa phải điện dung bus đã đo.

Đọc thanh ghi VCELL0x02 thành **word16bit nhân78,125µV**, theo table2trang10; không nhầm độ phân giảiADC1,25mV với LSBword16bit. Active cập nhật250ms; hibernate45s, nên đặtHIBRT=0 khi ghi hình để giữactive. Iactive23µAtyp/40µAmax ở-20..70°C. Sai số điện áp±7,5mV tại3,6V/25°C và±20mV trên điều kiện bảng. Đây không thay thế bảo vệ brownout/xung nhanh.

## 8. Bổ sung nhiệt Q1 và quy ước ngân sách sụt áp

Ảnh trang1 datasheet LRC Rev.G xác nhận PD950mW, ID1,4A tạiTa25°C, thetaJA131°C/W với fixture **FR4 1,5x1,5; padđồng1sqin,2ozCu**. Không áp nguyên thermalfixture này cho XIAO nhỏ. Ví dụP0,431W ->deltaT56,5°C chỉ trên fixture đó. MứcR0,67Ω tại660mA/25°C không phải điện trở được bảo đảm tại800mA/mọi nhiệt độ. Phải ghi rõ extrapolation và thẩm định nhiệt.

Nếu Rexternal là tổng đường từ **cọcP+/P- bộ pin đã cóPCM tớiBAT+/BAT- củaXIAO**, đặt tên nguồn tính toán là **Vpack_loaded**. Sụt ápPCM vàđiện trở trongcell đã nằm phía trước điểm này; không tínhlặp vàoRexternal. Nếu lấy điện áp tại cell thô thì cần thêmPCM riêng. Q1 nối từBAT tớiVIN vẫn tách riêng. Các tiêu chíVINrequired3,5V, hiệu suất0,85 vàRexternal0,100Ω là **lựa chọn thiết kế/điều kiện nghiệm thu**, không phải giới hạn hãng đã chứng minh.

Script tính banđầu vớiIcamallocation0,150A/Iaux0,002A cho I3V3allocation0,600A: Ibat≈0,81755A, Vpackrequired≈4,12951V (Rexternal+RQ1=0,770Ω). Với0,300A: Ibat≈0,48477A, Vpackrequired≈3,87327V. Các kịch bản phơi bày giới hạn đường nguồn stock; không dự đoán dòng quay trungbình hoặc thời lượng.
