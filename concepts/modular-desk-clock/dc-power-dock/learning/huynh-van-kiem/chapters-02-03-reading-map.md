# Bản đồ đọc đầy đủ chương 2 và 3 - Huỳnh Văn Kiểm

Ngày kiểm kê: 2026-10-30. Trạng thái: hướng dẫn đọc và giao bài, **chưa thực hiện mô phỏng mạch hoặc đo nguyên mẫu**.

Nguồn đã đọc: [chương 2](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/chuong2_Linh_kien_CS.pdf), **17 trang PDF**; [chương 3](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/Chuong3_DK_ap_AC.pdf), **15 trang PDF**. Tổng phạm vi: 32 trang. Số trang trong bảng là vị trí PDF, tính từ 1; hai tập này cũng in số trang tương ứng. Đã đọc phần văn bản trên toàn bộ 32 trang và xem bản render để đối chiếu mục, sơ đồ, bảng hình và ký hiệu khi cần; font Việt cũ làm text extraction bị méo dấu.

Ô `[ ]` dành cho tiến độ học của người đọc, không phản ánh việc người lập bản đồ đã đọc tài liệu. Những dòng dùng chung trang có thể giao nhau vì một trang chứa cuối mục trước và đầu mục sau.

Mã tính năng: **01** đầu vào USB-C; **02** sạc khi hệ thống hoạt động/power path; **03** pin thay thế; **04** cảm biến và kiểm soát nhiệt; **05** đầu ra có bảo vệ; **06** cơ cấu kết nối; **07** chỉ báo trạng thái; **08** bảo trì bằng vít. `Trực tiếp` nghĩa kiến thức dùng để phân tích khối dock; `Nền` nghĩa vẫn đọc để hoàn thành môn và hiểu lựa chọn topology, không đưa nguyên mạch đó vào dock.

## 1. Chương 2 - Linh kiện ĐTCS, đọc toàn bộ trang 1-17

| Đọc | Mục/nhãn trong tài liệu | Trang PDF | Lý do và liên hệ dock | Đầu ra học tập nhỏ |
|---|---|---|---|---|
| [ ] | Mở đầu: trạng thái ON/OFF của linh kiện | 1 | Trực tiếp 02/05: chuyển từ công tắc lý tưởng sang linh kiện có sụt áp và dòng rò | Bảng so sánh công tắc lý tưởng với diode và transistor thật |
| [ ] | II.1 Diode - 1. Phân loại | 1 | Trực tiếp 02/05: sụt áp, tổn hao và lựa chọn diode theo chức năng | Tính tổn hao một diode với dòng đã cho; ghi rõ giả thiết Vf không đổi |
| [ ] | II.1 - 2. Đặc tính phục hồi của diode | 1-2 | Trực tiếp 02/05: hiểu xung dòng ngược, Qrr, di/dt và nguồn gây quá áp/nhiễu | Vẽ i(t) lúc diode chuyển từ dẫn sang khóa; phân biệt trr và Qrr |
| [ ] | II.2 Thyristor và SCR - 1. Cấu tạo, nguyên lý | 2-3 | Nền: hiểu phần tử tự giữ và vì sao việc tắt khác MOSFET trong đường DC | Vẽ trạng thái khóa/dẫn và điều kiện thay đổi trạng thái |
| [ ] | II.2 - 2. Đặc tính tĩnh | 3-4 | Nền: đọc đồ thị V-I, phân cực và dòng latching/holding | Gắn VRB, VFB, IL, IH lên hình tự vẽ; phân biệt rating và điểm làm việc |
| [ ] | II.2 - 2. Đặc tính động (nguồn lặp số 2) | 4-5 | Nền và phương pháp chuyển giao 05: xung, giới hạn di/dt/dv/dt, snubber | Đọc cả dạng sóng và mạch bảo vệ ở hình II.2.4/II.2.5 |
| [ ] | II.2 / động - a. Mở; b. Khóa | 4-5 | Nền: nhận biết ton/toff và phục hồi, không coi chuyển mạch là tức thời | Tô khoảng công suất tức thời có thể lớn khi chuyển trạng thái |
| [ ] | II.2 - 3. Đặc tính cổng | 5 | Nền cho kỹ năng đọc datasheet: tín hiệu kích cũng có giới hạn | Xác định vùng tín hiệu cổng cho phép và điều kiện đảm bảo kích |
| [ ] | II.2 - 4. Các linh kiện khác trong họ thyristor | 5-7 | Nền: phân loại công cụ thay vì mặc định dùng mọi linh kiện trong sản phẩm | Một bảng tên, cách kích, cách khóa, ứng dụng |
| [ ] | II.2.4 - a. Darlistor | 5-6 | Nền: khuếch đại cổng khi dòng công suất lớn | Giải thích mục đích cấu trúc nối tầng bằng một đoạn ngắn |
| [ ] | II.2.4 - b. TRIAC | 6 | Nền: phần tử AC hai chiều, phục vụ chương 3 | Vẽ hai SCR song song ngược làm mô hình lý tưởng |
| [ ] | II.2.4 - c. DIAC | 6 | Nền: phần tử ngưỡng cho kích/dao động | Phân biệt DIAC và TRIAC theo số cực và cách kích |
| [ ] | II.2.4 - d. LA SCR | 6-7 | Nền: ý tưởng cách ly điều khiển quang | Vẽ đường năng lượng và đường điều khiển quang riêng |
| [ ] | II.2.4 - e. GTO | 7 | Nền: phân biệt khả năng khóa bằng cổng với SCR thông thường | So sánh SCR/GTO/MOSFET theo quyền điều khiển ON/OFF |
| [ ] | II.3 Transistor công suất - mở đầu và 1. Transistor công suất | 7-8 | Trực tiếp 02/05 qua MOSFET; BJT/IGBT dùng để so sánh | Bảng đại lượng điều khiển: IB, VGS, VGE; không chọn gate drive từ Vth đơn thuần |
| [ ] | II.3.1 - a. Quá trình đóng ngắt BJT | 8-10 | Phương pháp trực tiếp 02/04/05: chồng lấn v-i, tổn hao chuyển mạch, tác dụng driver/snubber | Vẽ p(t)=v(t)i(t), tích phân một lần chuyển mạch và nhân tần số |
| [ ] | II.3.1 - b. Vùng hoạt động an toàn của BJT | 10 | Phương pháp trực tiếp 05: giới hạn đồng thời áp, dòng và thời gian | Đặt một quỹ đạo làm việc lên SOA; giải thích vì sao rating V và I riêng lẻ chưa đủ |
| [ ] | II.3.1 - b. Mạch lái MOSFET (nguồn lặp chữ b) | 10-11 | Trực tiếp 02/05: tốc độ đóng ngắt và dòng nạp/xả gate | Vẽ dòng gate lúc chuyển trạng thái; lập danh sách thông số gate cần tra datasheet |
| [ ] | II.4 Các linh kiện công suất mới | 11-13 | Nền: đọc phân loại và ký hiệu, nhận biết giới hạn công suất/tần số | Xem đủ hình II.4.1, II.4.2, II.4.3; đánh dấu nhóm có thể xét cho dock và nhóm chỉ là nền |
| [ ] | II.5 Đặc tính nhiệt (bên trong in thêm nhãn 4.) | 14-15 | Trực tiếp 04/05: công suất tổn hao đến nhiệt độ junction, case, môi trường | Tính một mạch điện trở nhiệt; chỉ rõ nút nào đang tính, không gán nhiệt junction thành nhiệt pin |
| [ ] | II.6 Bảo vệ bộ biến đổi và ngắt điện bán dẫn - 1. Bảo vệ dòng | 15 | Trực tiếp 01/02/05: ngắn mạch, quá tải có thời gian và phối hợp bảo vệ | Bảng lỗi: dòng đỉnh, thời gian, năng lượng, cách cắt và cách phục hồi |
| [ ] | II.6 - 2. Bảo vệ áp | 15-16 | Trực tiếp 01/05: quá áp xung, RC/snubber, varistor, lọc LC | Chỉ ra nguồn gây xung và đường năng lượng; không sao chép giá trị RC cũ vào PCB |
| [ ] | II.7 Tóm tắt các ý chính | 16 | Củng cố toàn chương | Tự giải thích nguyên lý, chọn rating, điều khiển và bảo vệ trước khi xem ghi chú |
| [ ] | Bài tập 1: quan hệ dòng RMS/trung bình của thyristor | 16 | Trực tiếp kỹ năng tính 02/05; hệ số phụ thuộc dạng sóng | Tự tích phân nửa sin để ra hệ số; ghi rõ đây không phải hệ số chung cho mọi dòng |
| [ ] | Bài tập 2: diode nạp ắc quy với E=12 V, R=1 ohm | 16 | Gần nhất 02: nối kiến thức với bài bridge-charger đã học | Tìm khoảng dẫn, vẽ i(t), tính Iavg/Irms/đỉnh; giữ mạch này là mô hình học |
| [ ] | Câu hỏi 3, các ý b/c/d/e/f in trong nguồn | 16-17 | Đọc đủ: bảo vệ quá tải, nguyên lý SCR/TRIAC, cực tính gate, V-I và bảo vệ | Một trả lời ngắn mỗi ý; nguồn không in ý a nên không tạo thêm ý a |
| [ ] | Phụ lục ghi “CHƯƠNG 1” - A. Hình dạng SCR | 17 | Nền, hỗ trợ kỹ năng 06/08: nhận biết vỏ, cách gá, đường tản nhiệt | Phân biệt symbol điện với package cơ khí; đây là SCR, chưa phải pinout linh kiện dock |
| [ ] | Phụ lục - B. Đặc tính kỹ thuật chi tiết của SCR | 17 | Kỹ năng đọc tài liệu linh kiện | Đọc đoạn giới thiệu. PDF kết thúc tại đây, không có các datasheet được đoạn này thông báo là “sau” |

### Cụm đọc chương 2 để học chậm

1. Trang 1-2: diode và recovery.
2. Trang 2-5: SCR, V-I, đặc tính động và cổng.
3. Trang 5-7: các họ thyristor.
4. Trang 7-11: BJT/MOSFET/IGBT, chuyển mạch, SOA và driver.
5. Trang 11-13: toàn bộ hình phân loại linh kiện.
6. Trang 14-16: nhiệt và bảo vệ.
7. Trang 16-17: tóm tắt, mọi bài/câu hỏi và phụ lục.

Kết quả cụ thể cho dock sau chương 2 là một **worksheet linh kiện - tổn hao - nhiệt - lỗi**. Tập trung tính toán trên nguồn DC thấp áp và nguồn giả lập tải/pin. Kiến thức SCR/TRIAC hoàn thành bằng tính toán và mô phỏng, không biến thành yêu cầu lắp SCR/TRIAC trong dock.

## 2. Chương 3 - Điều khiển công suất xoay chiều, đọc toàn bộ trang 1-15

Chương này chủ yếu là **nền**, vì dock hiện nhận DC từ adapter ngoài. Giá trị học nằm ở quan hệ tín hiệu điều khiển - khoảng dẫn - RMS/công suất, đáp ứng RL, đồng bộ và cách giải số. Không có lý do thêm điều áp AC vào dock chỉ để áp dụng hết chương.

| Đọc | Mục/nhãn trong tài liệu | Trang PDF | Vì sao vẫn đọc hết | Đầu ra học tập nhỏ / liên hệ dock |
|---|---|---|---|---|
| [ ] | III.1 Thyristor là phần tử đóng ngắt mạch AC | 1-2 | Phân biệt line commutation và quyền tắt phần tử | Vẽ gate, dòng tải, áp tải; ghi rõ mất xung gate chưa làm SCR/TRIAC đang dẫn tắt ngay |
| [ ] | III.2 Các sơ đồ mạch động lực - a. Một pha | 2 | So sánh TRIAC, cặp SCR và sơ đồ SCR+diode | Đánh dấu đường dòng ở mỗi nửa chu kỳ; kiến thức chọn topology |
| [ ] | III.2 - b. Ba pha | 2 | Hoàn thành phạm vi; nhận biết quan hệ các nhánh nguồn | Vẽ sơ đồ năng lượng ba pha và giải thích dock không cần hệ nguồn này |
| [ ] | Mục in II.3: Điều khiển ON-OFF | 2-4 | Học điều khiển năng lượng theo tỷ lệ thời gian | Ghi nhãn tham chiếu II.3 như nguồn, hiểu vị trí logic là mục thứ ba của chương III |
| [ ] | ON-OFF - 1. Nguyên lý điều khiển công suất | 2 | Cơ sở duty ratio, nhưng thời gian đóng cắt là các chu kỳ lưới | Tự chứng minh Pavg=Pmax·ton/T dưới giả thiết; phân biệt với PWM DC tần số cao |
| [ ] | ON-OFF - 2. Đóng ngắt lúc áp qua không, a. Nguyên lý | 2-3 | Điều kiện switching làm thay đổi xung dòng/nhiễu | Vẽ switching tại zero và tại pha khác; liên hệ phương pháp giảm transient của 01/05, không áp dụng nguyên mạch |
| [ ] | ON-OFF - 2b. Lợi ích; 2c. Mạch điều khiển zero switching | 3 | Hiểu zero detector, AND và đường kích | Vẽ signal-flow, đầu vào/đầu ra/điều kiện enable; không coi DC có zero-cross lưới |
| [ ] | ON-OFF - 3. Ứng dụng | 3-4 | Đọc SSR/optron, ưu nhược điểm và giới hạn lỗi/nhiệt | Bảng điều khiển - cách ly - công suất - bảo vệ; liên hệ 05/07 ở mức tư duy hệ thống |
| [ ] | III.4 Điều khiển pha áp AC - mở đầu và định nghĩa alpha | 4-5 | Nắm cách xây mô hình theo từng khoảng dẫn | Tự vẽ điểm bắt đầu dẫn, điểm kết thúc; không gộp alpha và duty ratio thành một tham số |
| [ ] | III.4 - 2. Sơ đồ một pha, a. Tải điện trở | 5-6 | Nền tính tích phân RMS và P khi dạng sóng méo | Dẫn xuất Vrms(alpha), Irms và công suất; kiểm tra alpha=0, 90°, 180° |
| [ ] | III.4.2 - b. Tải RL | 6-8 | Học phương trình vi phân, điều kiện đầu và dòng không đổi tức thời | Giải Ri+Ldi/dt=v, tìm góc dẫn gamma bằng số; áp dụng tư duy này khi học cuộn cảm DC-DC ở chương 5 |
| [ ] | III.4.2b - nhận xét miền điều khiển và yêu cầu xung rộng | 7-8 | Hiểu điều kiện mô hình và quá độ, tránh suy từ tải R sang RL | So sánh alpha với phi; vẽ quá trình bắt đầu làm việc, ghi lại chỗ nguồn có lỗi dấu ở nhận xét |
| [ ] | III.4 - 3. Sơ đồ ba pha | 8-9 | Hoàn thành chương; học ảnh hưởng trạng thái dẫn của các pha và giới hạn giải tích | Đọc đủ trường hợp hai/ba nhánh dẫn; giải thích tại sao cần mô phỏng khi các trạng thái liên thuộc |
| [ ] | III.4 - 4. Ứng dụng, a. Đèn/ổn áp AC | 9 | Liên kết toán đến ứng dụng | Một dòng mô tả tải và biến điều khiển; không đưa dimmer vào dock DC |
| [ ] | III.4.4 - b. Đầu vào biến áp | 9 | Nhìn ranh giới adapter/nguồn ngoài | Vẽ adapter ngoài tách khỏi khối dock; bài AC phục vụ hiểu hệ nguồn phía trước |
| [ ] | III.4.4 - c. Động cơ không đồng bộ | 10 | Học inrush/soft-start và giới hạn tác dụng điều áp | Chuyển câu hỏi sang dock: “dòng khởi động tải gây sụt áp thế nào?”; không đồng nhất motor soft-start với IC soft-start |
| [ ] | III.5 Tóm tắt | 10 | Tổng kết khác nhau giữa ON-OFF và điều khiển pha | Tự trình bày lại ý chính bằng một hình và bảng điều kiện |
| [ ] | Bài tập/câu hỏi 1: duty của lò điện | 10 | Tính năng lượng trung bình theo thời gian | Chọn khoảng thời gian theo tải/cơ cấu, ghi rõ đó là bài AC |
| [ ] | Bài tập/câu hỏi 2: zero switching | 11 | Kiểm tra hiểu điều kiện đóng ngắt | Mô tả điều kiện áp và dòng riêng cho tải R và RL |
| [ ] | Bài tập/câu hỏi 3: dạng sóng và RMS | 11 | Ôn tích phân, phát hiện giả thiết còn thiếu | Nêu alpha khi kết luận Vrms=Vnguồn/sqrt(2); không kết luận đúng với mọi alpha |
| [ ] | Bài tập/câu hỏi 4: 220 Vrms xuống 110 Vrms | 11 | Bài toán ngược bằng đồ thị/giải số | Tìm alpha, kiểm tra lại bằng tích phân và đổi độ-radian đúng |
| [ ] | Phụ lục 1: mở đầu, mô hình SCR với tải RL | 11-12 | Không bỏ phụ lục: đây là cầu nối toán sang nghiệm số/mô phỏng | Ghi điều kiện đầu, Z, phi, tau, định nghĩa gamma và giả thiết dòng gián đoạn |
| [ ] | Phụ lục - 1. Tính góc dẫn gamma | 12 | Thực hành đọc đồ thị và giải phương trình phi tuyến | Làm cả ví dụ a (tìm gamma) và b (tìm XL); so sánh tra đồ thị với nghiệm số |
| [ ] | Phụ lục - 2. Tính trị hiệu dụng | 12-14 | Chuẩn hóa dòng, xử lý nhiều xung không chồng nhau | Dẫn xuất Irms,n=sqrt(n)·Irms,1; làm ví dụ thuận và ngược, đánh dấu mâu thuẫn 9 A/11 A trong nguồn |
| [ ] | Phụ lục - 3. Tính trị trung bình | 14-15 | Phân biệt avg và RMS với nhiều xung | Dẫn xuất Iavg,n=n·Iavg,1; đọc và làm ví dụ cầu một pha ở trang 15, kiểm tra điều kiện dòng gián đoạn trước khi tra |

### Cụm đọc chương 3 để học chậm

1. Trang 1-4: III.1, III.2 và toàn bộ ON-OFF/zero switching/SSR.
2. Trang 4-6: định nghĩa điều khiển pha và tải R.
3. Trang 6-8: tải RL, phương trình vi phân, góc dẫn và kích xung rộng.
4. Trang 8-10: ba pha, toàn bộ ứng dụng và tóm tắt.
5. Trang 10-11: đủ bốn bài/câu hỏi.
6. Trang 11-15: phụ lục đầy đủ, gồm ba phần gamma/RMS/trung bình và các ví dụ.

Kết quả cụ thể sau chương 3: một báo cáo nhỏ **điều khiển - dạng sóng - công suất - giới hạn mô hình**. Có thể chạy mạch AC bằng mô phỏng và kiểm tra tích phân số; việc đọc xong chương không yêu cầu chế tạo mạch điện lưới. Dùng kiến thức quá độ và xung làm nền khi sang chương 5.

## 3. Cách đọc có phê bình và giới hạn của nguồn

Tài liệu ghi học kỳ 2 năm 2004-2005. Các nguyên lý vẫn hữu ích, nhưng thông số ví dụ, phạm vi công nghệ và quy tắc chọn linh kiện cần đối chiếu datasheet hiện hành/pack thực khi thiết kế dock. Không biến các hệ số an toàn, nhiệt độ vỏ hay sơ đồ mẫu trong giáo trình thành tiêu chí nghiệm thu mặc định của sản phẩm.

Những điểm xác nhận trực tiếp trên trang render:

- Chương 2 lặp số `2` cho đặc tính tĩnh và động của SCR (trang 3/4), lặp chữ `b` cho SOA và driver MOSFET (trang 10). Bản đồ giữ nguyên nhãn kèm tên để tránh trỏ nhầm.
- Chương 2 trang 14 gọi công suất trung bình nhưng biểu thức in `ΔP=∫_T v(t)i(t)dt` thiếu phép chia chu kỳ. Khi tính phải dùng `Pavg=(1/T)∫_0^T vi dt`; tích phân không chia T là năng lượng trong chu kỳ. Đây là kiểm tra đơn vị vật lý, không phải sửa PDF.
- Chương 2 trang 10 viết MOSFET điều khiển bằng áp “không cần công suất”, nhưng ngay sau đó bàn dòng nạp/xả điện dung. Khi thiết kế, cần tra gate charge/driver và tổn hao chuyển mạch, không hiểu câu đầu là driver không tiêu thụ năng lượng.
- Chương 2 trang 15 có các mốc nhiệt độ vỏ và hệ số dự phòng chung. Tiêu chí cho dock phải phân biệt junction, case, bề mặt chạm tay và pin; dùng giới hạn của phần tử/pack đã chọn cùng phép đo.
- Chương 2 trang 17 ghi phụ lục chương 1, có mục B hứa các datasheet SCR “sau”, nhưng chính PDF 17 trang này kết thúc ở đoạn giới thiệu. Không có cơ sở đánh dấu các datasheet đó là đã có trong tập.
- Chương 3 trang 2 in `II.3` cho ON-OFF; bản đồ không tạo một mục III.3 giả là nhãn in sẵn.
- Chương 3 trang 7 có câu kết dùng `alpha>phi` trong đoạn đang mô tả `alpha<phi` với xung rộng. Học bằng điều kiện và dạng sóng trước/sau, ghi câu này là điểm cần tự kiểm tra thay vì học thuộc dấu.
- Chương 3 trang 11 ghi `114°=1.92 rad`; đổi đúng là khoảng `1.9897 rad`. Phải tự đổi đơn vị trước khi thay công thức.
- Chương 3 trang 13 đặt ví dụ ngược với 9 A, nhưng trang 14 thay 11 A. Chọn rõ dữ kiện của lần giải, không trộn hai bài.
- Bài tập 3 chương 3 trang 11 thiếu góc kích rõ ràng cho phần điều khiển pha. Giá trị Vrms=Vnguồn/sqrt(2) chỉ áp dụng cho trường hợp thích hợp (với điều khiển pha tải R là alpha=90°), không phải mọi góc.

Đã hoàn thành việc kiểm kê và lập bản đồ đọc toàn bộ 32 trang, gồm bảng/hình ít text ở chương 2 trang 11-13 và phụ lục chương 3 trang 11-15. Chưa giải mới toàn bộ bài tập, chưa chạy QSPICE mới, chưa chọn linh kiện, chưa đo hay nghiệm thu dock. Hoàn tất đọc, hoàn tất bài giải và hoàn tất kiểm chứng là ba trạng thái riêng.
