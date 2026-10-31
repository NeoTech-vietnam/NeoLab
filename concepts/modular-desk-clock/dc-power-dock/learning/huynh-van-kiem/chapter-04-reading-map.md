# Bản đồ đọc Chương 4 - Chỉnh lưu điều khiển pha

Ngày rà soát: 2026-10-30. Trạng thái: lộ trình đọc và đề xuất bài học, **chưa chạy mô phỏng mạch hoặc kiểm thử dock**.

Nguồn đã kiểm tra: [PDF Chương 4](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/Chuong4_Chinh_luu_DK_pha.pdf), **37 trang**. Tất cả số trang dưới đây là trang PDF, bắt đầu từ 1; trong bản này số in trên trang cũng chạy từ 1 đến 37. SHA-256: `04F49FDF0A760AB621CF268739B85B19FF693BC242F42AFE3A481DC423BBD609`.

Đã rà văn bản/nội dung trên toàn bộ 37 trang và xem ảnh render các trang 12, 14, 16, 27, 29, 30, 35-37 để xác nhận tiêu đề, hình và các chỗ đánh số bất nhất. Font tiếng Việt cũ làm văn bản trích xuất có lỗi; không dùng chuỗi trích xuất để kết luận công thức chính xác khi chưa nhìn bản gốc.

## 1. Lý do đọc hết chương này

Chương 4 là cầu nối trực tiếp với bài `01_solution_note.md`: tải RE, khoảng có dòng nạp, dòng trung bình/RMS/đỉnh và công suất. Nó cũng tập cho người thiết kế cách xác định đường dòng ở từng trạng thái, điều kiện áp dụng công thức, ứng suất linh kiện, ripple, đặc tính ra và đường truyền năng lượng.

Dock hiện dùng đầu vào DC ngoài qua USB-C. Các mạch SCR, nguồn ba pha, đổi chiều và nghịch lưu trong chương là kiến thức nền hoặc giúp hiểu phần AC/DC nằm ở phía adapter. Không đưa SCR chỉnh lưu lưới vào dock chỉ vì đã học chương này. Mô hình nạp RE cũng chưa cung cấp điều khiển sạc Li-ion CC/CV, kết thúc sạc, giám sát nhiệt hay power-path cấp đồng hồ đồng thời.

Quy ước liên hệ: **01** đầu vào USB-C; **02** sạc khi hệ thống chạy; **03** pin; **04** nhiệt/NTC; **05** rail và bảo vệ đầu ra; **06** kết nối/cơ khí; **07** trạng thái; **08** bảo trì. “Nền” nghĩa là chuyển giao cách phân tích, không chứng minh topology trong chương thích hợp cho dock.

## 2. Bảng bao phủ toàn bộ mục chính và mục con

Đọc cả phần giải thích, hình, ví dụ và điều kiện trong khoảng trang; các dòng dưới đây không thay thế việc đọc PDF. Bài nhỏ là đầu ra đề xuất để kiểm tra hiểu, chưa phải bài đã giải hay test đã chạy.

| Mục trong bản PDF | Trang | Vì sao cần đọc / kiến thức đem sang dock | Bài nhỏ sau khi đọc |
|---|---:|---|---|
| **IV.1 Chỉnh lưu diod (không điều khiển)** | 1-6 | Nền dạng sóng, tích phân, đường dẫn và chọn ứng suất; liên hệ 02/05 | Lập bảng so sánh 2/3/6 xung: đường dòng, ripple, áp ngược |
| IV.1.1 Chỉnh lưu hai xung | 1-3 | Bắt đầu bằng mạch quen thuộc, phân biệt cầu và điểm giữa | Vẽ đường dòng hai bán kỳ cho cả hai sơ đồ |
| IV.1.1.a Hoạt động ở tải R | 1 | Tách average/RMS, công suất tải và định nghĩa công suất DC | Tích phân để có average áp; giải thích vì sao hiệu suất theo thành phần DC không phải tổn hao cầu lý tưởng |
| IV.1.1.b Hoạt động ở tải RL | 1-2 | Học `v=Ri+L di/dt`, điều kiện đầu và tính chu kỳ; nền 02/05 | Viết ODE mỗi trạng thái và mô tả L làm dòng phẳng hơn |
| IV.1.1.c Hoạt động ở tải RE | 2-3 | **Liên hệ sát nhất với bài nạp ắc quy hiện có**: ngưỡng áp và góc dẫn; 02/03/05 | Dùng bài 220 Vrms, E=120 V để tìm khoảng dẫn, R, average/RMS/peak; chỉ ra giả thiết E cố định |
| IV.1.2 Chỉnh lưu ba xung, hình tia ba pha | 3-4 | Hiểu chọn pha có điện áp cao nhất, trung tính và áp ngược; kiến thức nền | Vẽ một chu kỳ, đánh dấu diode dẫn mỗi khoảng và tần số ripple |
| IV.1.3 Chỉnh lưu sáu xung | 4-6 | Đọc cách cấu trúc nhiều nhánh thay đổi đường dòng, ripple và rating; nền | So sánh thời gian dẫn từng linh kiện với thời gian có dòng tải |
| IV.1.3.a Cầu ba pha | 4-5 | Phân biệt áp pha/áp dây và hai nhóm diode; nền adapter | Chọn một cặp dẫn, viết `v_o=v_+−v_-` và xác định áp dây tương ứng |
| IV.1.3.b Tia sáu pha | 5 | Topology khác có thể cho cùng số xung nhưng khác rating/biến áp | So sánh áp ngược, thời gian dẫn và nhu cầu biến áp với cầu ba pha |
| IV.1.3.c Sáu pha có kháng cân bằng | 5-6 | Trung bình áp bằng nhau không có nghĩa áp tức thời bằng nhau; nền chia dòng | Giải thích vì sao cần xử lý áp chênh khi hai nhánh cùng cấp tải; không suy ra được phép nối song song hai dock |
| **IV.2 Chỉnh lưu điều khiển pha (SCR)** | 6-11 | Học trạng thái đóng/ngắt và quan hệ điều khiển-đầu ra; nền cho 02/05 | Vẽ đường đặc tính đầu ra theo góc kích, ghi rõ loại tải |
| IV.2.1 SCR hai xung | 6-9 | So sánh tải R, RL gián đoạn và dòng liên tục; không áp dụng một công thức cho mọi tải | Với cùng α, vẽ ba dạng áp/dòng; giải thích khi nào `V_o=V_do cosα` dùng được |
| Các trường hợp tải R / RL / dòng liên tục và điều kiện liên tục trong IV.2.1 (không đánh mục chữ riêng) | 7-9 | Điều kiện đầu, góc dẫn γ và giới hạn L; nền cho mô hình chuyển mạch | Thay đổi L hoặc α, dự đoán lúc dòng về 0 trước khi xem mô phỏng |
| IV.2.2 SCR ba xung | 9-10 | Mốc α=0 theo chuyển mạch tự nhiên, không tùy tiện theo zero của một pha | Đánh dấu điểm tự nhiên và xung kích; phân biệt phạm vi kích khi tải R với dòng liên tục |
| IV.2.3 SCR sáu xung | 10-11 | Thứ tự kích, cặp dẫn và xung phụ; nền phối hợp công tắc | Lập bảng sáu khoảng dẫn và giải thích yêu cầu xung kép |
| **IV.3 Các bài toán của chỉnh lưu điều khiển pha** | 11-20 | Phần gần tư duy thiết kế nhất: chế độ, rating, không lý tưởng, ripple và đặc tính ra | Tạo checklist giả thiết trước mọi phép tính nguồn |
| IV.3.1 Biện luận chế độ dòng khi tải RL | 11-12 | Xác định miền gián đoạn/liên tục trước khi dùng công thức | Chọn một RL và α, kiểm tra góc dẫn so với khoảng giữa hai lần chuyển mạch |
| Các tiểu ý trong IV.3.1: average khi gián đoạn/liên tục; biểu thức tổng quát; so sánh; giả thiết dòng phẳng | 12 | Tập phân biệt giả thiết, kết quả và trường hợp biên | Viết hai cột “đúng khi nào / sai khi nào” cho công thức average |
| IV.3.2 Tính trung bình dòng tải | 12 | Thành phần DC qua R/RLE; 02/03 | Giải thích `I_o=(V_o−E)/R` bằng average KVL trong trạng thái chu kỳ thích hợp |
| IV.3.3 Dòng qua SCR và biến áp đầu vào | 12-14 | Rating linh kiện dựa trên dòng thật của từng nhánh, không chỉ dòng tải; nền 05/04 | Từ dạng dòng chọn một SCR, tính average/RMS, rồi tính RMS một cuộn biến áp |
| Bài Fourier/hệ số công suất nằm trong IV.3.3 | 13-14 | Dòng không sin làm hệ số công suất khác cosφ; nền chất lượng nguồn | Tách dòng cơ bản khỏi dòng tổng và giải thích khác nhau giữa lệch pha với méo |
| IV.3.4 Biến áp của bộ chỉnh lưu | 14 | Đổi mức áp, cách ly, VA và từ hóa; nền hiểu adapter | Nêu hai nhiệm vụ biến áp và vì sao dòng một chiều làm khó mạch từ |
| IV.3.5 “Sự chuyển mạch của SCR…” | 14-16 | L ký sinh ngăn dòng đổi tức thời, tạo trùng dẫn và sụt áp; nền 05 | Vẽ dòng nhánh cũ giảm và nhánh mới tăng trong μ; không đồng nhất với handover pin của dock |
| IV.3.5 “Chỉnh lưu tải RLE và chế độ nghịch lưu” (sách **lặp số 5**) | 16-17 | Chiều công suất và nguồn tích cực; 02/03/05 ở mức nguyên lý | Đặt quy ước dấu V/I/P và phân biệt sạc, cấp tải, trả năng lượng; không suy ra dock hỗ trợ trả điện lưới |
| IV.3.6 Sóng hài áp và dòng ngõ ra | 17-19 | Ripple RMS, lọc và kiểm tra giả thiết; hữu ích trực tiếp về phương pháp cho 05 | Đọc ví dụ LC, tính lại với giả thiết, rồi kiểm tra hài tiếp theo và dòng liên tục |
| Các cách tăng số xung trong IV.3.6: tăng pha; nối tiếp/song song lệch pha | 18 | Học đánh đổi giữa ripple, kích thước và rating; nền | So sánh giảm ripple bằng topology với tăng L/C; không đưa nhiều pha lưới vào dock |
| IV.3.7 Đặc tính ngoài | 19-20 | `V_o=f(I_o)`, sụt áp và miền tải; liên hệ 01/05 | Vẽ đường áp theo tải và lập budget sụt áp gồm dây/tiếp điểm/công tắc cho dock |
| **IV.4 Nối song song và nối tiếp chỉnh lưu** | 20-21 | Tập xét chia dòng, dòng vòng và ranh giới nguồn; nền cho modular power | Viết các điều kiện để hai nguồn có thể ghép an toàn thay vì chỉ so sánh điện áp nhãn |
| IV.4.1 Nối song song | 20-21 | Chia dòng, kháng cân bằng, ripple; kiến thức nền | Giải thích vì sao áp trung bình bằng nhau chưa đủ loại dòng tuần hoàn |
| IV.4.2 Nối nối tiếp | 21 | Cộng áp, khả năng dẫn dòng và phối hợp điều khiển; kiến thức nền | Phân biệt rating điện áp và dòng khi nối tiếp với song song |
| **IV.5 Chỉnh lưu hỗn hợp SCR và diod** | 21-24 | Đường dòng hồi tiếp/freewheel và tác động lên rating; nền 02/05 | So sánh full-control với mixed-control ở cùng tải |
| IV.5.1 Các sơ đồ | 21 | Topology quyết định đường dòng và điểm chung điều khiển | Ghi vai trò của mỗi SCR, diode và diode phóng điện |
| IV.5.2 Sơ đồ một pha tải RL | 21-23 | Năng lượng trong L cần đường đi khi nguồn ngắt; nguyên lý hữu ích cho converter DC | Vẽ dòng khi nguồn dẫn và khi freewheel, tính average/RMS theo khoảng dẫn |
| IV.5.3 Cầu ba pha hỗn hợp tải RL | 23-24 | Tính bất đối xứng và tác động lên nguồn; nền | So sánh dạng dòng nguồn ở hai α, ghi hạn chế áp dụng |
| **IV.6 Bộ chỉnh lưu đảo chiều** | 24-25 | Tách đổi dấu áp, đổi chiều dòng và đổi chiều công suất; nền 02/05 | Vẽ quy ước các góc phần tư, rồi đánh dấu những chiều dock v0 cho phép |
| IV.6.1 Các sơ đồ | 24-25 | Đảo tiếp điểm và hai bộ biến đổi có ràng buộc khác nhau | Đánh dấu nguy cơ nếu hai nguồn ngược chiều cùng dẫn |
| IV.6.2 Nguyên lý điều khiển | 25 | Điều khiển riêng/chung, khoảng ngắt và dòng cân bằng; nền interlock | So sánh hai cách điều khiển và vai trò thời gian không chồng dẫn |
| **IV.7 Ngắt điện chuyển mạch cưỡng bức cho chỉnh lưu** | 25-27 | Phân biệt chuyển mạch lưới với điều khiển chủ động, quan hệ đến PWM; nền | Lập bảng biến điều khiển: α, góc tắt, độ rộng xung; không coi chúng là cùng một luật |
| IV.7.1 Điều khiển góc tắt | 26 | Điều khiển thời điểm tắt thay đổi dòng và phản kháng; nền | Vẽ một dạng dòng và xác định chiều lệch pha |
| IV.7.2 Góc đối xứng / điều rộng xung | 26-27 | Phương pháp định hình sóng và giới hạn HSCS khi dòng không sin | Giải thích vì sao dòng cùng pha vẫn chưa bảo đảm HSCS=1 |
| IV.7.3 Làm việc hai phần tư I và II | 27 | Hai chiều dòng trong cấu trúc IGBT; kiến thức nền | Đánh dấu diode/công tắc ở hai chiều công suất |
| **IV.8 Mạch kích SCR điều khiển pha** | 27-32 | Khối điều khiển, gain/delay, driver và chống nhiễu; nền cho 02/05/07 | Vẽ khối cảm nhận/so sánh/logic/driver/power stage và nêu giao diện |
| IV.8.1 Nguyên lý điều khiển pha | 27-28 | Đồng bộ, so sánh và định thời | Vẽ Uđb/Uđk, đánh dấu giao điểm và thời điểm kích |
| IV.8.2 Hàm truyền bộ biến đổi | 28-29 | Tuyến tính hóa, gain và trễ; nền điều khiển | Với ví dụ của sách, giải thích ý nghĩa K và τ, ghi giới hạn gần đúng |
| IV.8.3 “Mạch kích… theo nguyên lý so sánh” | 29-30 | Kiến trúc tín hiệu và sinh xung; kiến thức nền | Theo dõi một tín hiệu từ đồng bộ đến xung cổng |
| IV.8.3.a Sơ đồ khối | 29 | Chức năng từng khối, logic nhiều pha và cách ly | Chú thích đầu vào/đầu ra của từng khối trên Hình 4.8.3 |
| IV.8.3.c Mạch phát xung dùng KĐTT (bản PDF **không có mục b riêng**) | 29-30 | Comparator, integrator, one-shot và bảo vệ đầu vào | Vẽ lại năm dạng sóng Hình 4.8.4 và giải thích mỗi khối tạo cạnh nào |
| IV.8.3 “Áp đồng bộ dạng cosin và khối lệch pha” (sách **lặp số 3**) | 30-31 | Tập thiết kế quan hệ điều khiển tuyến tính và giới hạn chống nhiễu | So sánh quan hệ Uđk→α→V_o với ramp và cosin, giữ giả thiết dòng liên tục |
| IV.8.4 Khuếch đại xung và ghép khối động lực | 31-32 | Driver cung cấp năng lượng; cách ly, biến áp xung, reset từ thông; nền 05 | Tìm đường dòng driver khi ON/OFF và giải thích nguy cơ bão hòa biến áp xung |
| **IV.9 Ứng dụng chỉnh lưu điều khiển pha** | 32-35 | Hiểu chọn kiến trúc theo tải và phân biệt nguồn công nghiệp với nguồn điện tử | Nêu lý do nguồn của dock đi theo external DC + converter phù hợp |
| IV.9.1 Nguồn một chiều điều khiển pha | 32-34 | Nhìn toàn hệ thống: biến áp, chỉnh lưu, lọc, điều chỉnh | So sánh yêu cầu tải điện hóa với yêu cầu rail điện tử; chỉ ra phần ở adapter và phần ở dock |
| IV.9.2 Điều khiển động cơ một chiều | 34-35 | Tải RLE, dòng-moment, vòng điều khiển; nền NeoVerse sau này | So sánh pin với động cơ: đều có E nhưng quan hệ E và mục tiêu điều khiển khác nhau |
| IV.9.2.a Sơ đồ khối và đặc tính cơ | 34-35 | Mô hình tải tích cực và trạng thái xác lập | Giải thích dấu của E và quan hệ tốc độ-moment, không dùng làm mô hình SOC pin |
| IV.9.2.b Các bài toán điều khiển động cơ | 35 | Phân biệt vòng dòng, tốc độ, vị trí và giới hạn bảo vệ | Viết mục tiêu tương ứng của dock: giới hạn dòng, điện áp, nhiệt, không có vòng tốc độ |
| **IV.10 Tóm tắt chương** | 35 | Tự kiểm lại giả thiết lý tưởng và giới hạn phạm vi | Tạo một trang “đã hiểu / cần đo / chưa áp dụng cho dock” |

## 3. Đọc và làm toàn bộ phần cuối chương

Không bỏ phần cuối vì phần này kiểm tra khả năng áp dụng và nêu giả thiết. Các bài mạch lưới chỉ giải tích/mô phỏng trong lộ trình này; không yêu cầu dựng mạch 220 V.

| Mục cuối chương | Trang | Đầu ra học tập bắt buộc để tự đánh dấu hoàn thành |
|---|---:|---|
| Bài tập 1 | 35 | Tính average áp và áp ngược tại ba góc; kiểm tra ảnh hưởng sụt áp SCR |
| Bài tập 2 | 35 | Lặp với cầu hỗn hợp có diode phóng điện; giải thích kết quả khác Bài 1 |
| Bài tập 3 | 35-36 | Tìm worst case average/RMS của SCR, diode cầu và diode phóng điện khi α thay đổi |
| Bài tập 4, các ý a/b/c | 36 | Vẽ topology, xung kích/dạng áp-dòng rồi tính average; ghi rõ giả thiết dòng liên tục |
| Bài tập 5 | 36 | Kiểm tra gián đoạn/liên tục trước khi tính cho hai góc; dùng đúng mốc α và phần phụ lục được dẫn chiếu |
| Bài tập 6, các ý a/b/c/d | 36 | Tỉ số biến áp, áp ra, tác dụng Df và hài; phân biệt áp pha/áp dây và RMS/peak |
| Câu hỏi 1 | 36 | Vẽ các trường hợp L=0, hữu hạn, rất lớn; giải thích vì sao dạng dòng và áp thay đổi |
| Câu hỏi 2 | 36 | Tự giải thích chuyển mạch lưới bằng đường dòng |
| Câu hỏi 3 | 36-37 | Nêu miền α và ảnh hưởng của sức phản điện; không dùng một miền cho mọi tải |
| Câu hỏi 4 | 37 | Giải thích đường freewheel và năng lượng L khi nguồn đổi/ngắt |
| Câu hỏi 5 | 37 | Vẽ sáu xung/cặp dẫn, lý do xung kép và cách tạo xung thay thế |
| Câu hỏi 6 | 37 | Trình bày nguyên lý kích và ảnh hưởng tần số; đối chiếu sơ đồ ở IV.8 |
| Câu hỏi 7 | 37 | Phân loại ứng dụng và yêu cầu nguồn theo tải |
| Câu hỏi 8 | 37 | Đọc yêu cầu về vòng kín áp/dòng; lập sơ đồ chức năng do mình đề xuất và ghi là phần bổ sung. Tham chiếu “Hình 3.39” chưa tìm được sơ đồ tương ứng trong PDF này, nên chưa thể xác nhận lời giải đúng theo hình gốc |

## 4. Thứ tự học trong chương và mốc đi tiếp

1. **Nền dạng sóng (trang 1-6):** hoàn tất IV.1. Dùng IV.1.1.c để nối lại bài ắc quy đã giải; tính average/RMS/peak từ dạng sóng và kiểm tra bằng tích phân số. Đầu ra: bảng dòng-áp-công suất và danh sách giả thiết.
2. **Trạng thái điều khiển (6-12):** đọc IV.2 và IV.3.1-2. Đầu ra: ba dạng tải R/RL/RE và điều kiện dùng công thức average. Góc kích SCR là bài học điều khiển, không phải giao thức USB-C.
3. **Rating và không lý tưởng (12-20):** đọc phần còn lại IV.3. Đầu ra: bảng ứng suất, ripple, sụt áp và điều kiện chế độ. Chuyển các phương pháp sang budget rail/contact của dock.
4. **Ghép nguồn và đường năng lượng (20-27):** hoàn tất IV.4-7. Đầu ra: bản đồ chiều công suất, dòng vòng, freewheel và interlock. Đây là hiểu biết nền; dock v0 vẫn một nguồn cấp ra, chưa cho phép ghép hai dock.
5. **Tín hiệu và hệ thống (27-35):** hoàn tất IV.8-10. Đầu ra: sơ đồ chức năng đo-so sánh-driver-power stage, bảng phân biệt nguồn điện hóa/nguồn điện tử.
6. **Khóa kiến thức (35-37):** làm Bài 1-6 và trả lời Câu 1-8; ghi riêng lỗi/tham chiếu nguồn chưa giải quyết. Chỉ sau đó đánh dấu chương đã đọc hết.

Mỗi buổi chỉ cần 2-4 trang hoặc một nhóm mục, theo chu trình: hình/đường dòng -> định nghĩa -> phương trình -> ví dụ số -> một câu trả lời thiết kế. Một mục dày có thể cần nhiều buổi. Các phần phức tạp không cần tất cả đều trở thành mô phỏng phần cứng của dock.

## 5. Từ bài RE sang dock: ranh giới chuyển giao

| Bước mở rộng | Cái giữ lại từ Chương 4 | Cái phải thêm từ bài dock / tài liệu linh kiện |
|---|---|---|
| Cầu AC + R + E lý tưởng | KVL, khoảng dẫn, tích phân average/RMS, công suất | Chưa có điều khiển nạp, tải đồng hồ, SOC/nhiệt |
| Nguồn DC + R + điện áp pin thay đổi | Đường dòng và `I=(V_in−V_bat)/R` theo giả thiết | Dùng làm phản ví dụ: R không điều khiển dòng/áp hoặc kết thúc sạc |
| Bộ sạc DC có điều khiển | Cân bằng công suất, rating, điều kiện trạng thái | CC/CV, điều kiện tiền nạp/kết thúc, giới hạn pack, cảm biến nhiệt |
| Sạc + tải đồng hồ | Phân tích từng đường công suất và trạng thái | Power-path, input-current limit, ưu tiên tải, weak-source behavior |
| Dock mô-đun hoàn chỉnh | Dòng đỉnh/RMS, ripple, đặc tính theo tải, đường năng lượng | USB-C/PD, chống backfeed/hotplug, pin thực, PCB/EMI, serviceability và đo runtime |

Tham khảo trực tiếp các mô hình DC đã có: [lộ trình toán từ bài tập sang dock](../00-from-exercise-to-engineering.md). Đọc chương này cung cấp nền phân tích; nó không tự chứng minh sản phẩm đáp ứng PRD.

## 6. Kiểm tra hoàn tất và hạn chế nguồn

- [ ] Đã đọc liên tục trang PDF 1-37, gồm hình, ví dụ, bài tập và câu hỏi.
- [ ] Đã hoàn thành IV.1-10 và mọi mục con trong bảng, không bỏ phần ít dùng cho dock.
- [ ] Đã phân biệt các giả thiết tải R/RL/RE/RLE, gián đoạn/liên tục, lý tưởng/không lý tưởng.
- [ ] Đã giải Bài 1-6, trả lời Câu 1-8 hoặc ghi rõ câu bị thiếu tham chiếu nguồn.
- [ ] Đã tạo bảng “kiến thức dùng trực tiếp / kiến thức nền / kiến thức cần học thêm” cho dock.
- [ ] Đã gắn ít nhất một dự đoán toán học với phép đo dự kiến; chưa đánh dấu test phần cứng là đạt.

Các ô trên dành cho người học, hiện để trống; việc agent rà soát nguồn không có nghĩa người học đã hoàn thành chương.

Giữ nguyên lỗi đánh số của nguồn khi tra cứu: IV.3 lặp mục 5; IV.8 lặp mục 3 và chuyển từ tiểu mục a sang c. Nhiều tham chiếu công thức/hình còn mang 3.xx, và phần tóm tắt viết “chương ba”. Đặc biệt Câu 8 dẫn “Hình 3.39”, trong khi trang 34 có phương trình `<3.39>` của động cơ, chưa thấy sơ đồ vòng kín mang tên này. Không tự sửa số trong PDF hay invent một hình gốc còn thiếu.
