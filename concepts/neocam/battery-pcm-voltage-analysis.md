# NeoCam: phân tích pin 1S, PCM và điện áp tối thiểu

Ngày ghi nhận: **2026-11-04**. Đây là hồ sơ phân tích và bài tính từ tài liệu hãng; chưa phải thiết kế nguồn hoàn chỉnh, BOM đã chốt hoặc kết quả đo NeoCam.

## 1. Bài toán và trạng thái

NeoCam dự kiến dùng pin lithium một cell, **3,7 V danh định**, với XIAO ESP32-S3 Sense và firmware ESP-IDF. Mục tiêu người dùng đã chọn là quay và lưu **720p/30 FPS có âm thanh**, đồng thời xem trước hình **720p/12 FPS không tiếng** qua Wi-Fi trong **ít nhất 45 phút**. Đó là mục tiêu, chưa chứng minh board, firmware, pin hay dung lượng đã đáp ứng. Xem [bài toán quay độc lập](standalone-video-audio-problem.md).

Chưa chốt model cell, PCM, dung lượng pin, dòng pin trung bình/cực đại, dải nhiệt độ NeoCam hay ngưỡng dừng quay. Phân tích board dưới đây theo [sơ đồ XIAO Sense V1.51 của Seeed, trang 4](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/new-res/202003753_XIAO%20ESP32S3%20Sense_V1.51_SCH_260611.pdf#page=4); **revision board thực tế của người dùng chưa được xác nhận**.

### Đính chính các phép tính sơ bộ trước đó

- Ngân sách 0,825 A trên đường 3,3 V từng được dùng để minh họa, **không phải dòng NeoCam đã đo hoặc giới hạn trên đã chứng minh**. Mức này còn lớn hơn ghi chú 600 mA trên sơ đồ board, nên không thể dùng như bằng chứng rằng board đáp ứng tải.
- Với giả định thêm dòng camera 0,200 A, đầu vào 3,5 V và hiệu suất buck 90%, phép tính cũ cho 1,064 A. Đây chỉ là kết quả của bộ giả định, không phải dòng NeoCam thực tế.
- Từ phép tính đó từng suy ra mức tìm kiếm 1,5 A có dự phòng. **Không coi 1,5 A là yêu cầu đã chốt**, và không loại một bộ pin 1 A khỏi NeoCam chỉ vì phép tính này. Cần xác định dòng và điều kiện làm việc thực tế trước khi chốt.

## 2. PCM có vai trò gì?

PCM (Protection Circuit Module) thường gồm IC giám sát cell và các MOSFET điều khiển đường sạc/xả. Tùy thiết kế, nó có thể phát hiện quá sạc, quá xả, quá dòng và ngắn mạch. Khả năng theo dõi **nhiệt độ cell** phải xác nhận riêng; không mặc định PCM nào cũng có.

    Cell lithium 1S  ⇄  PCM và dây pin  ⇄  BAT+ / BAT− của XIAO
                                                │
                                      Q1 → VIN → U3 + L4 → VCC_3V3 → ESP
                                                └──────────→ nhánh board Sense

Sơ đồ trên là khối chức năng khi **dùng pin, không cắm USB**. PCM có thể đặt sẵn trong bộ pin và không nhất thiết là mạch do NeoCam tự thiết kế. Mạch sạc U4 trên XIAO là một khối khác: sạc bình thường theo thuật toán của nó; PCM xử lý các tình huống bảo vệ theo cấu hình của bộ pin.

Chọn phù hợp phải đối chiếu **cả cell và PCM**: điện áp sạc/xả, dòng liên tục, dòng xung theo thời gian, ngưỡng và độ trễ bảo vệ, sụt áp, nhiệt độ, kích thước và cách kết nối. Ngưỡng cắt quá dòng không đồng nhất với dòng xả liên tục cho phép của cell.

## 3. Dữ liệu dòng đã công bố và giới hạn sử dụng

[Seeed Studio: thông số XIAO ESP32-S3 Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) công bố các ví dụ dùng nguồn pin 3,8 V:

| Chế độ trong tài liệu Seeed | Dòng trung bình | Dòng đỉnh |
|---|---:|---:|
| Webcam Web application | khoảng 155 mA | khoảng 366 mA khi chụp hình |
| Thu âm microphone và ghi thẻ SD | khoảng 64,5 mA | khoảng 109,3 mA |

Đây là số liệu hãng cho **hai chế độ riêng**, không phải kết quả NeoCam. Không cộng trực tiếp hai dòng: các chế độ có thể dùng chung một phần tải và không cung cấp dạng dòng đồng thời. Chúng chưa cho biết dòng của cấu hình ghi video có tiếng **đồng thời** stream, cũng chưa xác nhận FPS mục tiêu.

Với đúng dòng trung bình tại pin của chế độ cần xét, quan hệ điện lượng là:

    Q_rút [mAh] = I_pin_trung_bình [mA] × 0,75 h

Điện lượng dùng được còn phụ thuộc dải điện áp vận hành, nhiệt độ, dòng và cell. Chưa đủ dữ liệu để tính dung lượng pin đạt 45 phút.

## 4. Bài tính mẫu: ngưỡng quá dòng của PCM

Xét **ví dụ phân tích linh kiện**, không khẳng định mạch này có trong pin của người dùng:

- [DW01A của H&M Semiconductor](https://hmsemi.com/downfile/DW01A.PDF): ngưỡng điện áp phát hiện quá dòng 120–180 mV, điển hình 150 mV, tại điều kiện bảng thông số 25 °C. Thời gian trễ quá dòng cấp 1 là 10 ms điển hình, 20 ms tối đa ở điều kiện nêu trong bảng.
- [FS8205A của Fortune](https://www.ic-fortune.com/upload/Download/FS8205A-DS-18_EN.pdf): MOSFET kép. Điện trở dẫn mỗi MOSFET là 30 mΩ điển hình, 37 mΩ tối đa tại VGS = 2,5 V, ID = 3 A, Tj = 25 °C.

Với mô hình hai MOSFET nối tiếp, lấy mỗi con 30 mΩ và bỏ qua điện trở đường mạch:

    R_đường dòng = 30 + 30 = 60 mΩ = 0,060 Ω
    V_cảm nhận ≈ I × R_đường dòng
    I_cắt_điển_hình ≈ 0,150 / 0,060 = 2,5 A

Giữ cố định 60 mΩ và chỉ thay ngưỡng IC:

    0,120 / 0,060 = 2,0 A
    0,180 / 0,060 = 3,0 A

**2–3 A chỉ là kết quả của mô hình với điện trở cố định**, không phải dải cắt được bảo đảm của một PCM hoàn chỉnh. RDS(on) còn phụ thuộc MOSFET thực tế, điện áp cổng, nhiệt độ và dòng; mạch in cũng có điện trở. Bảo vệ ngắn mạch là nhánh phát hiện riêng và cần xét thời gian tương ứng.

Điều kiện thiết kế gồm hai phía: ngưỡng và độ trễ không được gây cắt ở dòng làm việc/xung hợp lệ; khi có lỗi phải đối chiếu dòng theo thời gian với khả năng chịu lỗi của cell, MOSFET, dây và board. Một mức 3 A cắt sau vài mili giây không có cùng ý nghĩa với mức 3 A liên tục.

### Ví dụ sụt áp và nhiệt

Nếu giả sử R_đường dòng = 0,060 Ω và I = 1,064 A:

    V_sụt = I × R = 1,064 × 0,060 ≈ 0,064 V = 64 mV
    P_MOS = I² × R = 1,064² × 0,060 ≈ 0,068 W = 68 mW

Dòng 1,064 A ở đây **chỉ dùng lại như số minh họa**, không phải dòng được xác nhận. Điện trở MOSFET 30 mΩ cũng là giá trị điển hình tại điều kiện datasheet, không bảo đảm đúng trong toàn bộ điều kiện NeoCam. Điện trở dây và mạch in chưa tính.

## 5. So sánh sạc XIAO với PCM ví dụ

Sơ đồ V1.51 dùng SGM40567-4.2, chân F nối GND và R10 = 220 kΩ; công thức đặt dòng danh định cho khoảng **109 mA**, không phải dòng sạc đo liên tục. Xem [phân tích mạch nguồn XIAO](xiao-esp32s3-power-circuit.md).

[Datasheet SGM40567, trang 4](https://www.sg-micro.com/rect/assets/e95e555a-a8a2-4761-b86a-6554dff2d8ed/SGM40567.pdf#page=4) ghi điện áp sạc của biến thể 4.2 là **4,166–4,234 V** tại điều kiện bảng: VIN = 5 V, F = GND, IBAT = 6 mA, Tj = 25 °C và RIREF = 120 kΩ nếu không có ghi chú khác. Board được xem dùng 220 kΩ; không tự coi các số đo tại bảng là giới hạn của toàn bộ hệ trên mọi điều kiện.

DW01A của H&M ở ví dụ trên có ngưỡng quá sạc 4,25–4,35 V. So sánh sơ bộ:

    4,250 V − 4,234 V = 0,016 V = 16 mV

Hai ngưỡng được công bố tại các điều kiện và điểm đo khác nhau. Hiệu 16 mV không chứng minh tương thích trên toàn bộ dải nhiệt, dòng, sụt áp và thời gian trễ. Điện áp sạc thông thường và quá áp khi lỗi cần được đối chiếu với giới hạn của **cell cụ thể**.

## 6. Đối chiếu hai bộ pin thật

### 6.1. DNK102050: 1 A liên tục không phải ngưỡng cắt PCM

[Datasheet DNK102050](https://www.dnkpower.com/wp-content/uploads/2023/11/DNK102050-3.7V-1000mAh-Lipo-Battery-Specification.pdf) công bố cell/bộ pin 1000 mAh, dòng xả liên tục tối đa **1C = 1 A**, dòng xung **3C = 3 A trong 10 ms** và ngưỡng quá dòng của PCM **3–5 A**.

Không được hiểu ngưỡng bảo vệ 3–5 A là pin được phép xả liên tục ở mức đó. Đồng thời, các số này không tự chứng minh bộ pin mất an toàn: cần điều kiện dòng theo thời gian và thông tin bảo vệ đầy đủ. Với dòng NeoCam chưa xác định, **chưa kết luận DNK102050 phù hợp hoặc không phù hợp**.

### 6.2. LiPol LPHD7425048: ví dụ có PCM và NTC

[Datasheet LPHD7425048 của LiPol, bản lưu trên TI](https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/196/LPHD7425048-1000mAh.pdf) ghi:

| Thông số | Dữ liệu công bố |
|---|---|
| Cấu hình | 1S1P, 3,7 V danh định, 1000 mAh; có PCM |
| Dòng xả tiêu chuẩn / tối đa | 6 A / 10 A; tài liệu không nêu rõ ngay ở mục tối đa thời gian liên tục |
| Phát hiện quá dòng | 11–11,5 A, trễ 8–16 ms |
| Phát hiện quá sạc | 4,275 ± 0,050 V, trễ 0,7–1,3 s |
| Phát hiện quá xả | 2,75 ± 0,05 V, trễ 14–26 ms |
| NTC | 10 kΩ tại 25 °C |
| Dải nhiệt độ môi trường cho sạc | +10 đến +45 °C theo mục Ambient Conditions |
| Kích thước danh định / khối lượng | khoảng 49 × 25 × 7,4 mm / 25 g; xem dung sai trong bản vẽ |

Ngưỡng 11–11,5 A lớn hơn nhiều so với 366 mA đỉnh ở ví dụ webcam của Seeed, nhưng hai số không mô tả cùng điều kiện bảo vệ. Cần đối chiếu kịch bản lỗi, thời gian, dây và board trước khi đánh giá sự phù hợp. Datasheet bộ pin không xác nhận IC/MOSFET bên trong là DW01A/FS8205A.

Ngưỡng quá sạc thấp nhất của bộ pin này là 4,225 V. So với 4,234 V ở điều kiện bảng của SGM40567, các trị số có vùng chồng lấn 9 mV:

    4,225 − 4,234 = −0,009 V

Cần phân tích điện áp tại cell, sụt áp, điều kiện sạc và độ trễ; chưa kết luận sẽ ngắt nhầm. NTC có sẵn cũng không tự chứng minh XIAO đang đọc nó hoặc tự ngừng sạc theo nhiệt độ pin.

Đối chiếu cơ khí danh định với [Insta360 GO 3](https://onlinemanual.insta360.com/go3/en-us/specs/hardware), camera có chiều rộng ngoài 25,6 mm:

    25,6 − 25,0 = 0,6 mm tổng cộng
    25,6 − 25,5 = 0,1 mm nếu pin ở giới hạn rộng theo bản vẽ

Chưa trừ vỏ và khoảng lắp, nên không thể kết luận pin lắp vừa. GO 3 là kích thước tham khảo; NeoCam chưa có bản vẽ chế tạo đã duyệt.

## 7. Điện áp cell thấp nhất và giới hạn PCM

[Espressif: ESP32-S3 datasheet, bảng 5-2](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf#page=64) quy định các chân nguồn chính ở dải khuyến nghị **3,0–3,6 V**, điển hình 3,3 V. Giá trị này đo tại chân ESP, không phải tại cell.

SGM6029 là bộ **giảm áp**. Khi đầu vào tới gần đầu ra, duty cycle tăng và công tắc high-side có thể giữ dẫn liên tục; ở trạng thái xác lập, bộ giảm áp không thể duy trì 3,3 V khi đầu vào đã xuống thấp hơn mức cần thiết. [Datasheet SGM6029, 100% Duty Cycle Operation](https://www.sg-micro.com/rect/assets/3323f5dc-718f-4b81-b820-398ddb702989/SGM6029.pdf#page=16).

Hai mốc khác nhau cần xác định: mốc không còn duy trì **3,3 V danh định**, và mốc chân nguồn ESP xuống dưới **3,0 V**. Không giữ được đúng 3,3 V không tự động có nghĩa ESP reset ngay. Dưới 3,0 V là ngoài dải khuyến nghị, không có nghĩa chắc chắn hỏng hay dừng tức thì.

Với điện áp cell đang có tải và tổng sụt áp đến chân ESP:

    V_ESP = V_cell − ΔV_tổng
    V_ESP ≥ 3,0 V
    ⇒ ΔV_tổng ≤ V_cell − 3,0 V

| V_cell khi có tải | Tổng sụt áp tối đa theo điều kiện riêng của ESP |
|---:|---:|
| 3,6 V | 0,6 V |
| 3,3 V | 0,3 V |
| 3,1 V | 0,1 V |
| 3,0 V | 0 V |
| 2,8 V | Không đủ 3,0 V ngay cả nếu giả sử không có sụt áp |

Đây là **điều kiện cần**, chưa đủ cho toàn hệ thống. Camera OV3660, bộ nhớ và giao diện có các yêu cầu nguồn riêng; tải động và sụt áp tức thời cũng phải được xét.

Với PCM LPHD7425048, ngưỡng quá xả 2,70–2,80 V thấp hơn 3,0 V. Ở trạng thái xác lập khi chỉ dùng pin, không thể dựa vào ngưỡng này để giữ chân ESP trong dải khuyến nghị. NeoCam cần nhận biết pin yếu và kết thúc/lưu video **trước** khi đến giới hạn vận hành; chưa có đủ dữ liệu để chốt điện áp dừng.

## 8. Công việc tiếp theo

1. Đối chiếu revision board, sensor, model cell và PCM thực tế với các sơ đồ/datasheet đang dùng.
2. Xác định dải điện áp vận hành cần thiết cho ESP, flash, camera, microSD và nhánh nguồn của chúng; thu thập R_PCM, R_Q1 theo điều kiện làm việc, điện trở cuộn cảm/dây và dữ liệu dòng phù hợp. Không suy ra điện trở cuộn cảm chỉ từ 470 nH.
3. Thiết kế đường đọc pin cho ESP-IDF: chọn điểm đo, mạch chia áp phù hợp dải ADC và điều kiện cấp nguồn, sai số và ngưỡng dừng/lưu file. [Seeed ghi board không gán sẵn GPIO để đọc điện áp pin](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#battery-usage).
4. Đối chiếu điện áp sạc và bảo vệ cell với dung sai, độ trễ và nhiệt độ; xác định mạch giám sát nhiệt độ nếu cần dùng NTC.
5. Khi điều kiện thiết kế đã rõ, kiểm chứng dòng theo thời gian, sụt áp, nhiệt, ghi/lưu video và thời lượng trên đúng cấu hình. Các kiểm chứng này hiện **chưa thực hiện**, không phải kết quả đã đạt.
