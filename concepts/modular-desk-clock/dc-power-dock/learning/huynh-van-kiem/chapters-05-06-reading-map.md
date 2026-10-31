# Bản đồ đọc đầy đủ Chương 5 và 6 - Huỳnh Văn Kiểm

**Cập nhật DOCK-F01, 31-10-2026:** Phần nhận xét cũ về dock v0 không cần tạo AC được lập trước yêu cầu sạc không dây và không còn đúng cho bộ phát. Concept hiện tại cần AC cao tần cho cuộn TX; dùng Chương 6 để học cầu/nửa cầu và cộng hưởng, không xem sơ đồ trong sách là thiết kế Qi2. Mã F01–F08 trong bản đồ này là mã cũ, không trùng DOCK-F01–F10. Xem [bài toán mặt sạc hiện tại](../01-f01-mat-sac-khong-day-hvk.md).

Ngày rà soát: 2026-10-30. Mục đích: đọc hết hai chương và chuyển kiến thức thành bài toán thiết kế DC Power Dock. Đây là lộ trình đề xuất, không phải syllabus chính thức hay thiết kế đã nghiệm thu.

Nguồn cục bộ đã đọc:

- [Chương 5](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/Chuong5_Bo_bien_doi_ap_mot_chieu.pdf): **24 trang PDF**, số trang in cũng chạy 1-24.
- [Chương 6](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/Chuong6_Nghich_luu_doc_lap_va_bien_tan.pdf): **28 trang PDF**, số trang in cũng chạy 1-28.

Đã đọc nội dung trích xuất toàn bộ 52 trang, đọc lại các khoảng bị cắt đầu ra, và đối chiếu hình render ở các đầu mục quan trọng. Phông tiếng Việt cũ làm trích xuất sai dấu/ký hiệu; khi học phương trình hoặc đường dòng điện phải nhìn trang PDF gốc. Các khoảng trang chồng nhau khi một mục bắt đầu giữa trang.

Ký hiệu liên hệ: **D** = kiến thức dùng trực tiếp khi tính một khối của dock; **T** = nguyên lý chuyển giao, cần đổi mô hình; **N** = nền tảng phải đọc để hoàn thành giáo trình, chưa đưa mạch đó vào dock v0. F01 USB-C input; F02 sạc/power path; F03 pin; F04 nhiệt; F05 bảo vệ/nguồn ra; F06 kết nối cơ điện; F07 trạng thái; F08 bảo trì.

## 1. Chương 5 - Bộ biến đổi áp một chiều

Đây là chương gần mạch nguồn DC của dock nhất. Tuy nhiên, một bộ biến đổi áp không tự có CC/CV, kết thúc sạc, quản lý NTC hoặc power path. Đọc theo thứ tự trang 1 đến 24, không bỏ phần SCR; dành thời gian tính sâu hơn cho V.1, V.2 và V.4.

| Đánh dấu / ID đọc | Mục có thật trong PDF | Trang PDF | Lý do đọc, bài toán và đầu ra cần có | Liên hệ |
|---|---|---:|---|---|
| [ ] C5-00 | Mở đầu: định nghĩa bộ băm; PWM, điều chế tần số và hỗn hợp; cách dùng từ FORWARD/FLYBACK | 1 | Vẽ điện áp đóng/ngắt; định nghĩa D=t_on/T và f=1/T; tính điện áp trung bình một chu kỳ trước khi nói tới mạch lọc | D: F02/F05 |
| [ ] C5-11 | V.1 - Khảo sát loại FORWARD; 1. Một phần tư mặt phẳng tải | 1-3 | Vẽ đường dòng ON/OFF với tải RLE; giải phương trình L di/dt+Ri+E; tìm I_min, I_max, giá trị trung bình và ripple; giải thích vì sao cần diode đường dòng | D: F02/F05; T: F03 |
| [ ] C5-11A | Phần gần đúng T nhỏ hơn nhiều so với L/R; dòng liên tục/gián đoạn | 3-4 | So sánh nghiệm hàm mũ với xấp xỉ tam giác; xác định điều kiện I_min=0; chỉ dùng quan hệ áp trung bình trong miền hợp lệ | D: F02/F05 |
| [ ] C5-11B | Ví dụ in là “4.1”; Bài tập in là “4.1” | 4-5 | Tính lại ví dụ và suy ra biểu thức ripple từ nghiệm chính xác bằng khai triển; ghi riêng bất nhất L của ví dụ, xem mục cảnh báo bên dưới | D: F02/F05 |
| [ ] C5-11C | Bài tập in “4.2”: biện luận chế độ dòng theo E và α; “4.3”: thời gian có dòng t_X | 5 | Vẽ miền CCM/DCM; quét E và D; báo cả điều kiện giả thiết và kết quả. Không coi một điện áp pin cố định là toàn bộ chu kỳ sạc | D/T: F02/F03/F05 |
| [ ] C5-12 | V.1/2 - Hai phần tư I và II; ví dụ in “4.2” | 5-7 | Vẽ dòng dương, quanh 0 và âm; tính điện áp trung bình và ripple; giải thích năng lượng tái sinh và nguy cơ dẫn chéo khi hai khóa chuyển trạng thái | T: F02/F05 |
| [ ] C5-12B | Tiểu mục không đánh số “BBĐ tăng áp” | 7 | Chỉ rõ bên cấp và bên nhận năng lượng; biến đổi ký hiệu trong sách sang boost hiện đại. Bài chuyển giao: 1S giả định 3.0-4.2 V cần tạo rail 5 V thì buck đơn thuần không đủ | D/T: F03/F05 |
| [ ] C5-13 | V.1/3 - Bốn phần tư; điều khiển chung và điều khiển riêng | 7-9 | Lập bảng trạng thái cầu và dấu V/I/P; so sánh độ gợn của xung hai cực/một cực. Phân biệt khả năng đổi chiều với quyền được trả năng lượng về nguồn | N/T: F05 |
| [ ] C5-14 | V.1/4 - Sơ đồ hình 5.1.2.b1, hai phần tư I và IV; bài tập điều kiện V_o âm và dòng liên tục | 9-10 | Tính dấu áp khi dòng vẫn dương; vẽ đường freewheel/tái sinh. Ghi vì sao đây chưa phải kiến trúc cần thiết cho dock | N/T: F05 |
| [ ] C5-15A | V.1/5a - Sóng hài điện áp | 10 | Tích phân Fourier theo dạng sóng thực, xác định chu kỳ; đối chiếu CCM/DCM. Xuất phổ/ripple có ghi đơn vị và tần số lấy làm cơ bản | D/T: F05 |
| [ ] C5-15B | V.1/5b - Sóng hài dòng tải RLE | 10 | Dùng tổng trở từng bậc và nguyên lý xếp chồng; giải thích cuộn cảm giảm thành phần cao tần; không đồng nhất ripple của DC với THD lấy thành phần AC làm chuẩn | D/T: F02/F05 |
| [ ] C5-16 | V.1/6 - Làm việc song song; cải thiện dòng/áp bằng lệch pha và lọc đầu vào | 10-12 | Vẽ tổng hai dòng lệch pha; tính RMS nguồn và phổ; hiểu current sharing. Không suy ra được phép nối song song hai dock độc lập | N/T: F01/F05 |
| [ ] C5-20 | V.2 - Loại FLYBACK; hai pha tích/phóng năng lượng | 12-13 | Vẽ năng lượng trong L và C ở ON/OFF; dùng W_L=L I²/2 và W_C=C V²/2; so sánh điện áp và dòng qua khóa/diode | D/T: F02/F03/F05 |
| [ ] C5-20A | Bốn sơ đồ hình 5.2.1(a)-(d): đảo cực; tăng giảm; có biến áp; tăng áp | 13 | So sánh cực tính ra, dải điện áp và nhu cầu cách ly. Lập bảng buck/boost/buck-boost theo đầu vào, đầu ra thực của dock; chưa chọn topology chỉ từ hình đẹp | D/T: F02/F05 |
| [ ] C5-30 | V.3 - Mạch tắt SCR: nguyên lý tạo đường dòng tạm thời, t_q lớn hơn thời gian phục hồi | 13-14 | Giải thích vì sao SCR ở nguồn DC cần mạch tắt; so sánh với khóa điều khiển được trạng thái OFF. Ghi giới hạn chuyển giao sang MOSFET | N/T: F05 |
| [ ] C5-31 | V.3/1 - Ví dụ hai trạng thái bền dùng SCR | 14 | Vẽ cực tính C trước/sau kích; trình tự dẫn và điều kiện đầu. Đầu ra: một bảng trạng thái và một hình i/v theo thời gian | N |
| [ ] C5-32 | V.3/2 - Chuyển mạch cứng | 14-15 | Từ i=C dv/dt suy ra t_q=C V/I; tính thử một bộ giá trị và kiểm tra thứ nguyên, điện áp/dòng/điện tích | N/T: F05 |
| [ ] C5-33A | V.3/3 - BBĐ một phần tư dùng SCR; mở rộng cứng hai phần tư; Bài tập 5.4 | 15-17 | Đọc cả các hình 5.3.3(a,b,c), 5.3.4 và BT5.4; tính thời gian đảo cực LC, thời gian ON/OFF tối thiểu và các đường dòng; không xây mạch SCR này cho dock | N/T: F05 |
| [ ] C5-33B | Đầu mục cũng in “3.” - BBĐ hai phần tư dùng SCR, đảo lưu mềm | 17-20 | Theo dõi hình 5.3.5-5.3.9; giải dao động LC, điều kiện i_C>I_o để tắt, khảo sát lựa chọn L/C và năng lượng. Hiểu đánh đổi thời gian chuyển mạch với ứng suất | N/T: F05 |
| [ ] C5-40 | V.4 - Ứng dụng; hai nhóm nguồn DC và nguồn chỉnh lưu | 20 | Vẽ ranh giới adapter ngoài và dock DC; phân biệt sơ đồ cấp nguồn với mạch điều khiển tải | D/T: F01/F02 |
| [ ] C5-41 | V.4/1 - Nguyên lý điều khiển: PWM và so sánh có trễ | 20-21 | Vẽ sóng mang, mức đặt, hồi tiếp và xung; giải thích tần số cố định/biến thiên; lập bài kiểm tra step tải cho một rail DC | D: F02/F05; T: F07 |
| [ ] C5-42 | V.4/2 - Điều khiển động cơ DC và solenoid | 21-22 | Áp dụng dấu công suất, back-EMF, hãm tái sinh và vòng dòng vào tải động cơ; ghi rõ dock v0 chưa cấp động cơ. Đầu ra: so sánh tải RLE động cơ với pin và tải điện tử | N |
| [ ] C5-43A | V.4/3a - Nguồn DC: BBĐ thay ổn áp tuyến tính | 22 | Lập bảng tổn hao tuyến tính/chuyển mạch ở cùng điểm tải; hiểu lọc trước/sau và đường hồi tiếp | D: F02/F04/F05 |
| [ ] C5-43B | V.4/3b - Nguồn dùng biến áp cao tần | 22-23 | Vẽ các miền cách ly, đường năng lượng và feedback; đây là kiến thức adapter/upstream, không đưa điện lưới vào dock DC v0 | N/T: F01 |
| [ ] C5-44 | V.4/4 - Nghịch lưu | 23 | Giải thích điều kiện trung bình bằng 0 và thành phần cơ bản của AC; nối khái niệm sang Chương 6 | N |
| [ ] C5-50 | V.5 - Tóm tắt chương | 23 | Tự vẽ lại sơ đồ các họ biến đổi, ghi giả thiết từng công thức và phân biệt CCM/DCM; đánh dấu chỗ còn chưa chứng minh | D/T: F02/F05 |
| [ ] C5-B1 | Bài tập và câu hỏi 1, hình BT1.a/b | 23-24 | Tự dựng quan hệ điều khiển so sánh sóng mang tới áp trung bình; so sánh với mô phỏng. Học toán của PWM, không mặc định dùng mạch op-amp này làm charger | D/T: F02/F05 |
| [ ] C5-B2 | Bài tập và câu hỏi 2, gồm hai câu nhỏ | 24 | Tính duty để có dòng tải cho trước; tìm L theo giới hạn ripple với mọi duty. Đổi sang bộ số DC thấp áp có giả thiết trước khi thử nguyên mẫu | D: F02/F05 |

**Điều kiện hoàn thành Chương 5:** mỗi dòng được đọc và có ít nhất một hình/tính toán/so sánh; tự suy ra quan hệ áp trung bình và ripple của mô hình một phần tư; nhận ra miền công thức mất hiệu lực; viết được lý do chọn họ biến đổi cho đầu vào/đầu ra dock. Điều này chưa đủ xác nhận bộ sạc Li-ion hoặc thiết kế PCB.

### Các điểm cần đọc có phê bình ở Chương 5

1. Sách dùng **ΔI=(I_max-I_min)/2** ở trang 3. Nhiều datasheet dùng ripple peak-to-peak. Ghi `ΔI_pp=2ΔI_sách` trước khi so sánh hay tính L.
2. Trang 4, ví dụ in “4.1” ghi L=0.001 H trong dữ kiện nhưng phần thế số dùng L=0.01 H: τ=0.002 s với R=5 Ω và ripple nửa biên độ 0.105 A. Với L=0.001 H, τ=0.0002 s và xấp xỉ ripple là 1.05 A. Học viên phải xác nhận bộ dữ kiện rồi tính lại, không chép kết quả.
3. Cách nhóm FORWARD/FLYBACK ở đây rộng hơn tên topology hiện đại: boost cũng nằm trong nhóm truyền năng lượng qua cuộn dây của sách. Đọc hình và hai trạng thái dòng thay vì đồng nhất mọi chữ FLYBACK với mạch flyback biến áp cách ly.
4. Ví dụ/bài tập có nhãn “4.x” trong chương 5, và V.3 có hai đầu mục cùng số 3. ID C5-33A/B là mã lộ trình mới, không sửa số của tác giả.
5. Hai/nhiều phần tư và song song bộ biến đổi là kiến thức về dấu năng lượng và chia tải, không phải quyền backfeed vào USB-C hoặc nối song song các dock. Các chức năng đó cần hợp đồng và bảo vệ riêng.

## 2. Chương 6 - Nghịch lưu độc lập và biến tần

Đọc đủ 28 trang để hoàn thành bức tranh biến đổi năng lượng, mạch chuyển mạch và điều chế. Dock không có yêu cầu cấp điện AC lưới ra ngoài hoặc điều khiển động cơ; bộ phát sạc không dây DOCK-F01 tạo AC cao tần bên trong. Giá trị chuyển giao gồm cầu/nửa cầu, cộng hưởng, PWM, đường dòng, switching loss, nhiễu, lọc, trình tự và dự phòng nguồn.

| Đánh dấu / ID đọc | Mục có thật trong PDF | Trang PDF | Lý do đọc, bài toán và đầu ra cần có | Liên hệ |
|---|---|---:|---|---|
| [ ] C6-00 | Mở đầu: định nghĩa nghịch lưu độc lập; ba nhóm ứng dụng theo đầu ra/tần số | 1 | Vẽ bản đồ DC-AC, phân biệt với nghịch lưu phụ thuộc; ghi ứng dụng cần AC thật và ứng dụng chỉ cần DC dự phòng | N/T: F02 |
| [ ] C6-11A | VI.1/1a - Nghịch lưu song song | 1-2 | Theo hình 6.1.1/6.1.2 vẽ đường nạp/xả C và vai trò L nguồn; tính năng lượng và điều kiện dòng không đổi tức thời | N/T: F05 |
| [ ] C6-11B | VI.1/1b - Nghịch lưu nối tiếp | 2 | Mô hình RLC, nhận ra dòng về 0 và chuyển mạch tải; so sánh series/parallel bằng một bảng | N |
| [ ] C6-12A | VI.1/2a - Nghịch lưu nguồn dòng | 2-3 | Giải thích nguồn có tổng trở lớn và đầu vào L lớn; vẽ nguồn dòng lý tưởng so với nguồn thực | N/T: F05 |
| [ ] C6-12B | VI.1/2b - Nghịch lưu nguồn áp | 3 | Giải thích nguồn trở kháng thấp, tụ bus và diode đường trả năng lượng; phân biệt rail DC với nguồn có thể nhận năng lượng | T: F02/F05 |
| [ ] C6-21A | VI.2/1 - Một pha, tải R và L nguồn rất lớn | 3-4 | Viết phương trình RC, áp dụng đối xứng chu kỳ; làm bài tập không số ở trang 4 về t_q và dòng nguồn I bằng cân bằng công suất | N/T: F05 |
| [ ] C6-21B | VI.2/1 - Trường hợp RL thực; diode chặn | 4-5 | So sánh waveform lý tưởng/thực và điều kiện đảm bảo chuyển mạch; ghi phần nào bị mất khi giả thiết L vô cùng không còn đúng | N/T: F05 |
| [ ] C6-21C | VI.2/1 - Tính gần đúng bằng thành phần cơ bản, vector; bài tập 500 V ở trang 6 | 5-6 | Tính C/t_q, áp RMS, công suất và dòng nguồn theo giả thiết; đối chiếu với một mô hình số. Bài cao áp chỉ phân tích/mô phỏng trong lộ trình này | N |
| [ ] C6-22 | VI.2/2 - Ba pha, logic hai khóa cùng dẫn, mạch chuyển mạch cụ thể | 6-7 | Lập bảng thứ tự sáu trạng thái và đường dòng; so sánh một/ba pha; nêu nguy cơ điện áp tăng khi không tải của mô hình nguồn dòng | N/T: F05 |
| [ ] C6-31 | VI.3/1 - Nguồn áp một pha, cầu/nửa cầu/đẩy kéo | 7-8 | Vẽ trạng thái áp và dòng tải, giải RL bằng hàm mũ; xác định đường freewheel và nguyên tắc tránh đóng sai khóa | T: F05 |
| [ ] C6-32 | VI.3/2 - Nguồn áp ba pha, sóng sáu nấc, áp dây/pha/trung tính | 8-9 | Từ bảng trạng thái suy ra v_AB, v_A và v_Nn; đây là bài toán mạng điện và logic trạng thái, không phải rail của dock | N |
| [ ] C6-33A | VI.3/3a - Đa bậc dùng nguồn nhiều cấp/diod kẹp | 10 | Lập bảng mức điện áp và trạng thái hợp lệ; so sánh ứng suất linh kiện với độ phức tạp | N |
| [ ] C6-33B | VI.3/3b - Đa bậc dùng cell nghịch lưu một pha | 10-11 | Chứng minh 2N+1 mức theo mô hình; vẽ nguồn nào phải cách ly. Không gọi việc ghép cell nghịch lưu là ghép module dock | N |
| [ ] C6-33C | VI.3/3c - Đa bậc ghép biến áp đầu ra; bảng tổng hợp 12 nấc | 11 | Cộng các xung lệch pha theo bảng và vẽ dạng sóng; giữ dấu/cuộn biến áp rõ ràng | N |
| [ ] C6-34 | VI.3/4 - Tính gần đúng nguồn áp bằng thành phần cơ bản và xếp chồng | 11-12 | Tính tải ở từng hài; đối chiếu chỉ dùng hài 1 với tổng nhiều hài. Lập điều kiện được bỏ qua hài cao | T: F05 |
| [ ] C6-41 | VI.4/1 - Phân tích sóng hài điện áp; Bài tập 6.1 gồm hai câu | 12 | Tự tích phân Fourier dạng xung; kiểm tra đối xứng và bậc bị triệt; đối chiếu áp dây/pha. Chú ý căn 3 trong PDF, không dùng chuỗi trích xuất mất căn | T: F05 |
| [ ] C6-42A | VI.4/2a - Thay đổi áp nguồn | 13 | Phân biệt điều khiển biên độ nguồn DC với điều chế đầu ra; viết nguồn vào thay đổi tác động thế nào tới đầu ra | T: F01/F05 |
| [ ] C6-42B | VI.4/2b - Điều chế một/nhiều xung, hoàn toàn/không hoàn toàn | 13-15 | Vẽ trạng thái v/i khi nhánh cầu không dẫn; làm Ví dụ 6.1 (RL), Bài 6.2 (độ rộng giây cố định) và 6.3 (hài theo lệch pha) | T: F02/F05 |
| [ ] C6-43A | VI.4/3a - Các cách hạn chế sóng hài; bài phân tích 12 nấc | 15 | So sánh đa bậc/PWM và nơi phổ được dời tới; tính một phổ, ghi RMS/nhiệt/nhiễu có thể tăng dù giảm một số hài | T: F04/F05 |
| [ ] C6-43B | VI.4/3b - Chỉ tiêu PWM: chỉ số điều chế m và THD | 15-16 | Ghi đúng định nghĩa m của sách; tính THD với thành phần cơ bản và phân biệt với ripple factor trên rail DC | T: F05 |
| [ ] C6-43C | VI.4/3c - SPWM | 16-17 | Tạo sóng sin tham chiếu, tam giác mang và xung; so sánh phổ với six-step; tách tần số sóng mang khỏi tần số thành phần mong muốn | T: F05 |
| [ ] C6-43D | VI.4/3d - SPWM cải tiến | 17-18 | Đọc bổ sung hài thứ tự không và phạm vi tuyến tính; so sánh sử dụng bus và số lần chuyển mạch | N/T: F05 |
| [ ] C6-43E | VI.4/3e - Điều chế theo mẫu, đối xứng/không đối xứng | 18-19 | Tính độ rộng một vài xung từ mẫu; xét độ phân giải thời gian và cập nhật theo chu kỳ | T: F05/F07 |
| [ ] C6-43F | VI.4/3f - SVM/SVPWM; vector trạng thái, thời gian và giới hạn điều chế | 19-21 | Vẽ lục giác, phân tích một vector giữa hai vector lân cận, kiểm tra t_a+t_b+t_0=T_C và thời gian không âm | N |
| [ ] C6-43G | VI.4/3g - Triệt hài chọn trước, ví dụ loại hài 5/7 | 21-22 | Lập hệ phương trình góc chuyển mạch, kiểm tra các hài còn lại và bài toán tối ưu có trọng số; ghi đánh đổi không chỉ nhìn hai hài đã triệt | N/T: F05 |
| [ ] C6-43H | VI.4/3h - Điều chế bằng so sánh có trễ | 22 | Vẽ band sai số và tác động chuyển khóa; so sánh chất lượng bám với tần số thay đổi; ghi lợi/hại cho EMI | T: F02/F05 |
| [ ] C6-C1 | Đầu mục in “V.3 Mạch điều khiển nghịch lưu” /1 - Logic ba pha | 22-23 | Đọc CD4017 và bài SR flip-flop; lập bảng trạng thái hợp lệ và reset an toàn. Chuyển giao tư duy trình tự/fault-state, không yêu cầu dock dùng logic ba pha | N/T: F07 |
| [ ] C6-C2 | Cùng mục /2 - Tạo tham chiếu sin bằng ROM và DAC | 23-24 | Tính f_sample=N f_out; phân biệt tham chiếu và sóng mang; đọc quan hệ đồng bộ trong hình | N/T: F07 |
| [ ] C6-C3 | Cùng mục /3 - Điều khiển bằng chương trình ROM; ví dụ dung lượng/tần số | 24-25 | Tính bit địa chỉ, trang/trạng thái, tần số PC; giải thích cập nhật tại ranh giới chu kỳ tránh trạng thái không kiểm soát | N/T: F07 |
| [ ] C6-C4 | Cùng mục /4 - Dùng vi xử lý | 25 | Tách tính toán/điều khiển, cảm nhận, giao diện và ghi lỗi; lập bảng chức năng supervisor có thể cần ở dock, bảo vệ thiết yếu vẫn phải có thiết kế độc lập phù hợp | T: F04/F05/F07 |
| [ ] C6-F | Đầu mục in “V.6 Biến tần”: trực tiếp, trung gian DC, U/f, vector có/không cảm biến | 25-27 | Vẽ hai kiến trúc; tính tốc độ đồng bộ và giải thích U/f. Phần này mở rộng năng lực mechatronics; chưa thêm motor/AC vào yêu cầu dock | N |
| [ ] C6-A1 | Đầu mục in “V.7 Ứng dụng” /1 - Nguồn tần số cao | 27 | Nối cộng hưởng, tần số và ứng dụng gia nhiệt; giải thích vì sao ứng dụng quyết định yêu cầu khác nhau | N |
| [ ] C6-A2 | Cùng mục /2 - Nguồn xung sử dụng nghịch lưu | 27-28 | Vẽ đường DC-cao tần-biến áp-chỉnh lưu-lọc-hồi tiếp; nhận diện push-pull và thời gian chuyển khóa | N/T: F01/F05 |
| [ ] C6-A3 | Tiểu mục in “23.” - Nguồn AC không gián đoạn (UPS) | 28 | So sánh on-line/off-line, ranh giới chuyển nguồn và thời gian mất áp; chuyển giao sang kiểm tra adapter-to-battery của dock bằng rail/reset thực, không dùng nguyên thời gian ms của UPS làm tiêu chí clock | T: F02/F03/F05 |

**Điều kiện hoàn thành Chương 6:** có ghi chú cho từng dòng, bảng phân loại các nguồn dòng/áp và topology, một bộ dạng sóng điều chế kèm phổ, một bài tính trạng thái/thời gian và một so sánh nguồn dự phòng AC với power path DC. Các phần ba pha, SVPWM, biến tần và gia nhiệt vẫn được đọc hết nhưng không trở thành feature của dock v0.

### Giới hạn và lỗi đánh số của nguồn

- Sau VI.4, PDF in **V.3** cho mạch điều khiển, **V.6** cho biến tần, **V.7** cho ứng dụng; đây là nhãn nguyên bản đã nhìn trên render. Không tự coi có các mục VI.5/VI.6/VI.7 mang số đã sửa.
- Mục UPS trang 28 in **23.**; ID C6-A3 chỉ là mã đọc của lộ trình. Một số hình còn số “5.x” trong chương 6. Tìm theo tiêu đề/nội dung và trang PDF thay vì chỉ số hình.
- Các phát biểu về “hiện nay”, giới hạn công suất/linh kiện và phần cứng ROM/vi xử lý phản ánh giáo trình cũ. Đọc để hiểu nguyên lý, không dùng làm khuyến nghị chọn linh kiện hiện hành.
- THD của nghịch lưu dùng một thành phần cơ bản AC làm chuẩn; ripple DC, switching ripple và reset-droop của dock cần định nghĩa riêng.

## 3. Chuyển hai chương thành các bước thiết kế dock

| Bước | Đầu ra đề xuất | Điều kiện mới được đi tiếp |
|---|---|---|
| H5-A - Sau V.1/1 và các bài CCM/DCM | Worksheet D/f/L, I_min/I_max/I_rms và ripple cho một mô hình thấp áp có nguồn giới hạn | Tự kiểm tra giả thiết và quy ước ripple; giải thích mọi đường dòng ON/OFF |
| H5-B - Sau V.2 và V.4/3 | Bảng so sánh rail pin, rail clock, input; buck/boost/buck-boost; loss và nhiệt | Có load profile, dải pin và voltage budget; chưa biến 5 V/10 W giả định thành rating |
| H5-C - Sau V.4/1 | Sơ đồ điều khiển chức năng CC/CV và giới hạn nguồn/nhiệt; phương án controller dựa datasheet | Phân biệt converter với charger; xác định ngưỡng pack và hành vi load sharing trước khi vẽ PCB |
| H6-A - Sau VI.3/VI.4 | Báo cáo waveform/phổ, lựa chọn cách đo ripple, hot-loop/feedback và reset timing | Phân biệt dự đoán lý thuyết, mô phỏng mạch và số đo; đặt giới hạn kiểm chứng |
| H6-B - Sau điều khiển/UPS | State diagram input-present, battery, low-energy, fault; kịch bản plug/unplug | Có phối hợp với carrier và phép đo rail/reset; không lấy một đèn sáng làm bằng chứng seamless |
| Tích hợp thực tế | Evaluation board rồi schematic, BOM, PCB, test plan phù hợp | Đủ yêu cầu nguồn/pack/thermal; review thiết kế; đo phần cứng theo PRD trước khi tuyên bố ổn định |

Feature 06 và 08 còn đòi hỏi kiến thức tiếp điểm, cơ khí, dung sai, strain relief và bảo trì ngoài hai chương. Feature 03 còn cần electrochemistry/pack data và usable Wh. Không ép một đoạn về ghép cell nghịch lưu hoặc module converter thành bằng chứng tái sử dụng NeoVerse.

## 4. Tài liệu hãng cần ghép vào khi chuyển sang thiết kế thật

Các nguồn dưới được mở kiểm tra ngày 2026-10-30. Chúng bổ sung chủ đề mà bản đồ Chương 5/6 không cung cấp đủ để hoàn tất dock. Tên IC chỉ minh họa chức năng, không là BOM được chọn.

| Chỗ cần bổ sung | Đọc thêm và lý do |
|---|---|
| USB-C attachment/current qualification | [TI TUSB320 documentation](https://www.ti.com/product/TUSB320): ví dụ chức năng CC nhận attach/orientation/current mode. CC detection không phải PD negotiation; muốn PD phải có đặc tả/controller và source profile tương ứng. F01. |
| Sạc Li-ion, power path, input limit, NTC và trạng thái | [TI BQ24074 documentation](https://www.ti.com/product/BQ24074): đọc charge-cycle, DPPM, NTC và fault/status. Đây là họ sạc tuyến tính ví dụ, không khẳng định đáp ứng load/thermal dock hay tạo rail 5 V. F02/F03/F04/F07. |
| Tính power stage thực | [TI Basic Calculation of a Buck Converter's Power Stage](https://www.ti.com/lit/an/slva477b/slva477b.pdf): đọc switch current, L, diode, input/output capacitors và setting. Bản được trả về hiện là Rev. C; tài liệu giới hạn ở power stage CCM, không thay thế tính compensation hoặc thiết kế boost. F02/F05. |
| PCB/EMI, hồi tiếp, lọc và nhiệt | [TI AN-1149 Layout Guidelines for Switching Power Supplies](https://www.ti.com/lit/an/snva021c/snva021c.pdf): kiểm tra đường công suất/feedback, vị trí tụ, ground và tản nhiệt; tiếp theo phải đọc layout của IC thực sự chọn. F04/F05. |

## 5. Trạng thái bao phủ và bằng chứng

- Bao phủ trang: Chương 5 **1-24**, Chương 6 **1-28**, không có khoảng trang bỏ qua. Tất cả phần giới thiệu, mục lớn/nhỏ, ứng dụng, tóm tắt và bài tập đã có hàng đọc hoặc được nêu rõ bên trong hàng tương ứng.
- Đã dùng trích xuất toàn bộ nội dung và render kiểm tra tiêu đề/hình tại các trang quan trọng. Đã kiểm tra bằng mắt đặc biệt V.2/V.3/V.4/V.5 của Chương 5 và VI.4, các đầu mục sai số cuối Chương 6.
- Ô đánh dấu đang trống vì đây là việc **người học cần hoàn thành**, không phải khẳng định người học đã đọc hay làm bài.
- Chưa thực hiện các bài toán minh họa trong bản đồ, chưa chạy QSPICE mới, chưa xây/đo phần cứng và chưa nghiệm thu tính năng.
