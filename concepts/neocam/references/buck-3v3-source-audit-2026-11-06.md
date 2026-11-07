# Kiểm tra nguồn sơ cấp: buck 3V3 của XIAO ESP32-S3 Sense

Ngày đọc nguồn: **2026-11-06**. Phạm vi: **U3 SGM6029CYG/TR của sơ đồ mainboard V1.51**, không khẳng định bo đang cầm có đúng revision này. ExpBoard v1.2 được dùng làm tham chiếu riêng; số revision đó không xác định revision mainboard.

**Kết luận:** SGM6029C có chế độ duty 100%, nên không tự tắt chỉ vì đầu vào giảm từ USB về pin. Tuy nhiên, điều kiện `VIN_SYS ≥ 3.541667 V` của ngân sách chuyển nguồn v0.5 **chưa đủ chứng minh rail 3V3 không sụt dưới giới hạn của ESP/flash/microSD**. Ở dòng 600 mA, biên áp của buck gần dropout; datasheet không cung cấp giới hạn lớn nhất cho thời gian phản ứng điều khiển hoặc line transient tại cấu hình 3.3 V này. Cần giữ rõ kết quả tính theo mô hình và bảo đảm của nhà sản xuất.

## 1. Nguồn đã đọc

| Mã | Nguồn sơ cấp | Phần dùng |
|---|---|---|
| S1 | [Seeed, XIAO ESP32S3 Sense V1.51, SCH 2026-06-11](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/new-res/202003753_XIAO%20ESP32S3%20Sense_V1.51_SCH_260611.pdf) | Trang 4/4: U3, L4, C3–C6, R13, tải 3V3, ghi chú 600 mA |
| S2 | [SGMICRO, SGM6029 datasheet, November 2022 Rev. A.1](https://www.sg-micro.com/rect/assets/3323f5dc-718f-4b81-b820-398ddb702989/SGM6029.pdf) | Trang 2–6, 14–19: điều kiện làm việc, đặc tính điện, điều khiển, lựa chọn L/C |
| S3 | [SGMICRO, EVKIT-SGM6029](https://www.sg-micro.com/evm-detail/EVKIT-SGM6029) | Tải và đọc **SGM6029 Demo Board Test Report_V3.pdf**, 28 trang; BOM trang 5, thử quá độ trang 22/25 |
| S4 | [Murata, specification DFE18SAN series](https://pim.murata.com/asset/pim4/inductor/JETE243A-0036_PDF_INDUCTOR?lastModifiedDatetime=20250708072123) | Phân biệt dòng bão hòa và dòng giới hạn nhiệt của DFE18SANR47MG0L |
| S5 | [SGMICRO, trang sản phẩm SGM6029](https://www.sg-micro.com/product/SGM6029), [danh mục simulation models](https://www.sg-micro.com/simulation-models) | Tìm tài nguyên mô phỏng công khai; chưa tìm được control macromodel cho SGM6029 |

Các thông số dưới đây là diễn giải số liệu kỹ thuật, không phải kết quả đo trên NeoCam. Trang EVM cung cấp tên bản PDF và liên kết tải bằng cơ chế của website; bản tải đọc được có dung lượng 1,658,037 byte. Không đưa access token của website vào tài liệu dự án.

## 2. Mạch stock chính xác trong S1

```text
VIN_SYS → U3.B1 VIN
VIN_SYS → U3.C2 EN
VIN_SYS → C3 2.2µF || C4 4.7µF → GND

U3.B2 SW → L4 470nH → VCC_3V3
VCC_3V3 → U3.A2 VOS
VCC_3V3 → C5 4.7µF || C6 4.7µF → GND

U3.C1 VSEL/MODE → R13 249kΩ 1% → GND
U3.A1 → GND
```

S1 ghi **Imax = 600 mA** tại đầu ra VCC_3V3. Giá trị 1 A trong datasheet IC không tự nâng giới hạn thiết kế của cả mainboard lên 1 A.

Các tụ C5/C6 có tổng **danh định 9.4 µF**, chưa phải tổng điện dung hiệu dụng của rail. Trên mainboard còn các tụ phân tán cấp ESP, ví dụ C9/C13/C16 = 2.2 µF, nhiều tụ 100 nF và các nhánh lọc; ExpBoard/microSD cũng làm thay đổi tải và điện dung nhìn từ rail. Không được giả định chỉ có C5+C6, hoặc cộng mọi giá trị danh định rồi coi đó là điện dung thực ở mọi tần số.

**S1 không cho MPN, DCR, dòng bão hòa hoặc đường cong L(I,T) của L4; cũng không cho MPN/đặc tính DC-bias của C3–C6.** Vì vậy không thể rút từ sơ đồ một giới hạn dropout/ripple/ổn định có bảo đảm cho chính bo thực.

## 3. Đặc tính có căn cứ và điều kiện đi kèm

| Đại lượng | Số liệu S2 | Điều kiện / giới hạn cách dùng |
|---|---:|---|
| VIN làm việc | 1.95–5.5 V | Đây là khả năng IC hoạt động, **không** có nghĩa VIN=1.95 V vẫn tạo được 3.3 V |
| Dòng đầu ra khuyến nghị SGM6029C | tối đa 1 A khi VIN≥2.3 V; 0.7 A khi VIN<2.3 V | Phải thỏa điều kiện nhiệt, L/C và headroom; S1 vẫn ghi 600 mA |
| L hiệu dụng | 0.33–1 µH, điển hình 0.47 µH | Bao gồm dung sai và suy giảm do dòng/nhiệt |
| COUT hiệu dụng | 5–10 µF | Miền khuyến nghị cho mạch bù nội và tụ gốm ESR thấp |
| CIN hiệu dụng | 2.2–4.7 µF điển hình | Mục chọn tụ vào cho phép lớn hơn, không đặt giới hạn trên; nguồn trở kháng cao nên dùng ít nhất 10 µF |
| Tần số SGM6029C | 4 MHz **typ** | Không có fmin/fmax được công bố trong bảng điện |
| RDS(on) high-side | 185 mΩ typ, 310 mΩ max | Bảng dùng VIN=3.6 V; dòng điều kiện **IOUT=100 mA** |
| RDS(on) low-side | 125 mΩ typ, 230 mΩ max | Cùng điều kiện trên |
| Giới hạn dòng high-side | 1.58 / 1.78 / 1.98 A min/typ/max | **TJ=25°C**, không có giới hạn toàn nhiệt độ cho dòng này |
| Giới hạn dòng low-side | 0.90 / 1.20 / 1.50 A | **TJ=25°C**; không đồng nhất với dòng tải DC |
| Sai số điện áp PWM | −2% / 0 / +2% | Dòng điều kiện in trong bảng: **IOUT=0 mA**; không phải giới hạn gộp của line/load transient |
| Sai số PSM | 0.5% **typ** | Không có min/max; không dùng 0.5% làm biên bảo đảm |
| UVLO khi VIN tăng | 1.76 V typ, 1.95 V max | Không có trị số falling/hysteresis đủ để dựng một ngưỡng cắt pin có bảo đảm |
| EN | VIH≥0.85 V; VIL≤0.40 V | Stock EN nối VIN, không phải chân shutdown do MCU điều khiển |
| VSEL/MODE | VIH≥0.89 V; VIL≤0.40 V | Chức năng chọn áp chỉ trong startup, sau đó là chọn mode |
| Điện dung ký sinh VSEL/MODE | tối đa 30 pF | Không thêm RC lớn/probe tải lớn tùy tiện vào R13 |
| Khởi động sau EN lên | 700 µs typ / 1170 µs max | VSEL=16; đây là startup, không phải thời gian phản ứng handoff khi buck vẫn đang chạy |
| Soft-start SGM6029C | 410 µs typ / 785 µs max | Bảng in điều kiện bất thường, xem ghi chú dưới; không dùng cho mô hình handoff |
| θJA | 214°C/W | Phụ thuộc fixture/PCB, không tự áp cho mọi enclosure NeoCam |
| Tắt nhiệt PWM | 160°C typ, hồi sai 20°C typ | Không phải nhiệt độ hoạt động mục tiêu; phần mô tả nói không có bảo vệ nhiệt ở PSM |

**Hai điểm bất nhất cần lưu nguyên trạng:** trang 3 ghi khoảng TJ làm việc −40…125°C và trang đặt hàng ghi −40…85°C; nhưng header bảng điện trang 5/6 được nhìn trực tiếp lại in **“TJ = +40°C to +125°C”**. Bảng soft-start in **“0.95% × VOUT nominal”**, trái cách định nghĩa soft-start thông thường. Không âm thầm sửa các ký tự đó rồi coi thành giới hạn nhà sản xuất ở điều kiện mới; cần nhà sản xuất xác nhận nếu dùng để chứng minh toàn dải nhiệt/khởi động. Các dòng có điều kiện nhiệt độ riêng vẫn được ghi riêng như bảng trên.

## 4. R13 không phải cầu chia áp hồi tiếp

U3 bơm dòng vào R13 lúc khởi động và bộ chuyển đổi **điện trở → mã số** chọn một trong các mức điện áp nội. Với SGM6029C, bảng 1 cho **mã 16, R=249 kΩ hoặc lớn hơn → 3.3 V**. Sau đó mạch đo điện trở ngừng hoạt động; VSEL/MODE trở thành đầu vào logic.

Do R13 kéo xuống GND, cấu hình stock cho phép **PSM ở tải nhẹ và PWM tự động ở tải cao**, không phải ép FPWM liên tục. Sai số 1% của R13 bảo đảm chọn đúng mã theo quy định E96/TC; không cộng thẳng thành “3.3 V ±1%”. S2 nêu phương pháp R2D giúp sai số điện trở không trực tiếp đặt độ chính xác điện áp.

## 5. Duty 100% giải quyết được gì?

Khi VIN tiến gần VOUT, duty tăng đến 100% và high-side bật liên tục. Bộ buck lúc đó tương đương đường dẫn có điện trở nối tiếp; nó **không boost**. Mô hình DC bậc một:

\[
V_{3V3,100\%}\approx V_{IN}-I_{3V3}(R_{HS}+R_L+R_{trace})
\]

Muốn còn điều chỉnh quanh giá trị mục tiêu `Vset`, điều kiện cần là:

\[
V_{IN}\ge V_{set}+I_{3V3}R_{path}
\]

Lấy **VIN=3.541667 V, I3V3=0.600 A** từ kịch bản tính handoff hiện tại, thử một **giả thiết cần kiểm chứng** `Rpath≤0.400 Ω`:

\[
V_{100\%}\ge3.541667-0.6\times0.400=3.301667\;V.
\]

Điều này chỉ cho thấy mạch có thể còn khoảng **1.667 mV** trên giá trị danh định 3.3 V trong mô hình DC; nó chưa chứng minh sai số điều chỉnh hay quá độ. Nếu cần giữ mức đặt cao nhất `3.3×1.02=3.366 V`, điều kiện tương tự là **VIN≥3.606 V**. Nếu chỉ yêu cầu tải không xuống dưới 3.0 V thì ngân sách khác, nhưng phải dùng giới hạn của **mọi** tải liên quan và tính cả sai số/quá độ/đường cấp flash.

Một ví dụ khác dùng `RHS=0.310 Ω` của S2 và `RL=0.054 Ω` của cuộn cảm EVM ở nhiệt độ tham chiếu cho ra `V100%=3.323267 V`; **không phải kết quả worst-case của XIAO** vì điều kiện RHS khác điểm làm việc và L4 chưa xác định. Không được đổi ví dụ đó thành “buck chắc chắn giữ 3V3”.

## 6. Phương trình cho mô hình và khoảng còn thiếu

### 6.1 Ripple và dòng đỉnh khi đang PWM

Theo S2:

\[
\Delta I_L\simeq\frac{V_O(1-V_O/V_I)}{Lf_{SW}},\qquad
I_{L,pk}=I_O+\frac{\Delta I_L}{2}.
\]

Với VOUT=3.3 V, IOUT=0.6 A, L=0.47 µH, f=4 MHz **danh định/typ**:

| VIN | ΔIL p-p tính được | IL,pk |
|---:|---:|---:|
| 5.25 V | 0.651976 A | 0.925988 A |
| 3.541667 V | 0.119775 A | 0.659887 A |

Các số này chỉ là ước lượng PWM lý tưởng. Chưa có fmin, L(I,T) stock hoặc profile chuyển mode để gọi chúng là dòng đỉnh lớn nhất. Dòng giới hạn switch 1.58–1.98 A ở 25°C không thay thế việc kiểm tra Isat của L4.

Ripple điện áp PWM bậc một:

\[
\Delta V_{pp}\simeq\frac{\Delta I_L}{8fC_{eff}}+ESR\,\Delta I_L.
\]

Công thức này không phải giới hạn ripple PSM hoặc sụt áp khi VIN/tải đổi nhanh.

### 6.2 Sụt áp do thiếu dòng tức thời

Phương trình vật lý cần dùng thay cho “tụ đủ lớn là ổn”:

\[
C\frac{dV_O}{dt}=I_L-I_{load},\qquad
L\frac{dI_L}{dt}=V_{SW}-V_O-I_LR_L.
\]

Nếu bộ điều khiển chưa tăng dòng trong thời gian `td`, xấp xỉ thiếu dòng `ΔI`:

\[
\Delta V\approx\frac{\Delta I\,t_d}{C_{eff}}+ESR\,\Delta I.
\]

Ví dụ minh họa độ nhạy, **không phải thời gian trễ do SGMICRO bảo đảm**: ΔI=0.5 A, td=1 µs, Ceff=5 µF → riêng phần nạp/xả tụ đã là **100 mV**. Vì vậy bảo toàn VIN bằng bank tụ đầu vào chưa tự chứng minh VOUT không sụt.

Trong duty 100%, với VIN tạm coi cố định:

\[
L\frac{dI_L}{dt}=V_{IN}-V_O-I_L(R_{HS}+R_L),
\]

cho thấy headroom càng nhỏ thì tốc độ bù dòng càng hạn chế. Mô hình cho handoff phải xét đồng thời **VIN(t), tải(t), dòng L trước sự kiện, Rpath(T), Ceff(V,T), ESR, độ trễ và chế độ PSM/PWM**. S2 không cung cấp đủ giới hạn của tất cả biến này.

## 7. L/C và kết quả EVM không được gán nhầm cho stock

S2 dùng linh kiện tham chiếu ở mục ứng dụng: L=**DFE18SANR47MG0L**, COUT=**GRM155R61A106ME44D** 10 µF/X5R/10 V/0402. S3 dùng cùng MPN L/COUT và CIN **LMK105BBJ475MVLF** 4.7 µF/X5R/10 V/0402. Đây là BOM demo; S1 không xác nhận chúng là BOM XIAO.

Theo Murata S4, cuộn DFE18SANR47MG0L có **DCR max 54 mΩ**, dòng bão hòa **3.3 A tại giảm L 30%**, và dòng giới hạn nhiệt **2.6 A tại tăng 40°C**. BOM S3 ghi “2.6 A ISAT”; khi chọn linh kiện cần ưu tiên đúng định nghĩa trong datasheet của chính nhà sản xuất cuộn cảm. Giá trị DCR này cũng không mặc nhiên còn bằng 54 mΩ khi cuộn nóng.

S3 đo SGM6029C với **VOUT=1.8 V**. Các load-transient hình 8-9…8-12 dùng VIN=3.3 V, PSM, tải chuyển 0/1 mA ↔ 50/100 mA, slew 200 mA/µs. VIN-on/off dùng slew 1 V/ms. Đây là bằng chứng hãng có kiểm tra mạch demo trong các điều kiện đó; không phải kiểm tra 3.3 V/600 mA khi VIN từ USB giảm sát dropout.

**Không tự thêm tụ lớn ở 3V3:** S2 khuyến nghị COUT hiệu dụng 5–10 µF và mạch bù nội. Thêm 100–1000 µF vào đầu ra để “chống reset” sẽ vượt cấu hình công bố, thay đổi điều khiển/startup; cần một thiết kế ổn định riêng. Bank **3×330 µF của v0.5 nằm ở VIN_SYS** nên không phải COUT của buck. Tuy vậy vẫn phải giữ tụ gốm sát U3 cho vòng dòng đóng cắt.

## 8. Có model hãng để chứng minh trên QSPICE không?

Tại thời điểm đọc, trang sản phẩm công khai liên kết datasheet và EVKIT; chưa tìm được **SPICE/control macromodel SGM6029** trên đó hoặc khi tìm trong tài nguyên simulation của SGMICRO. Danh mục simulation có tồn tại nhưng việc mở đầy đủ bằng công cụ bị timeout; kết luận đúng là **“chưa tìm được model hãng công khai”**, không phải “hãng không có model”. File PCB/CAD/footprint không mô tả vòng điều khiển.

Một mô hình tự dựng bằng công tắc lý tưởng hoặc điều khiển trễ có thể tính và kiểm tra độ nhạy với tham số. Nó cần ghi rõ là **mô hình thiết kế theo giả thiết**, không được gắn nhãn đã xác nhận hành vi silicon. Các thông số còn thiếu để đưa ra giới hạn toàn mạch bằng tài liệu gồm:

- MPN + DCR/L/Isat theo nhiệt độ và dòng của **L4 stock**; Ceff/ESR theo bias, nhiệt độ và dung sai của cả mạng tụ đầu ra.
- fmin/fmax, minimum on/off time, giới hạn phản ứng line/load/control-loop của SGM6029C ở VOUT3.3 V và VIN sát dropout.
- Giới hạn RHS đúng VIN, dòng và nhiệt độ của kịch bản; làm rõ header nhiệt độ/soft-start bất nhất trong S2.
- Tải dòng đồng thời của ESP/flash/SD và giới hạn điện áp cục bộ sau đường cấp/đường mạch; model source mux không chứa các đặc tính đó.

Vì vậy, báo cáo hiện tại hỗ trợ **tính miền thiết kế và xác định dữ kiện phải có**, không cấp chứng nhận “không reset” hoặc thời lượng 45 phút. Phần chọn kiến trúc, tính quét tham số và cập nhật sơ đồ thuộc tài liệu thiết kế chính của NeoCam.
