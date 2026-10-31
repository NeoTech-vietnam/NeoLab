# Chương 1 — bản đồ đọc đủ mục và liên hệ dock

Nguồn: `Chuong1_Modau.pdf`, **8 trang PDF**, đã đọc toàn văn và xem bản render cả 8 trang; đối chiếu riêng trang 3 và 8. Trang trong bảng là số trang PDF, bắt đầu từ 1. Ngày rà soát: 2026-10-30. Đây là bản đồ học, không phải kết quả mô phỏng hay kiểm chứng phần cứng.

File gốc: [Chương 1](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/Chuong1_Modau.pdf).

Mã C1 dưới đây giúp đánh dấu tiến độ; giữ nguyên ký hiệu mục I.x của tài liệu. Mỗi dòng đều cần đọc. Phần có mạch chưa cần triển khai trong dock vẫn cần một đầu ra học tập.

| ID | Mục gốc và trang PDF | Vì sao đọc / liên hệ sản phẩm | Bài làm và điều kiện đi tiếp |
|---|---|---|---|
| C1-01 | I.1 — Các khái niệm, tr.1 | Phân loại AC/DC, DC/DC, DC/AC; tách mạch công suất và điều khiển. Dock nhận DC; adapter ngoài chịu trách nhiệm AC/DC. | Vẽ sơ đồ năng lượng dock, ghi ranh giới từng khối và loại chuyển đổi; liên hệ F01/F02/F05. |
| C1-02 | I.2 — Ngắt điện bán dẫn, tr.1–2 | Hiểu mô hình diode, SCR, transistor và điều kiện dẫn/ngắt; nền tảng chọn mô hình trước khi chọn linh kiện. | Vẽ bảng ON/OFF của ba phần tử và chiều dòng; giải thích vì sao không áp một mô hình lý tưởng cho mọi linh kiện thật. |
| C1-03 | I.3.1 — Đặc trưng áp, dòng, tr.2–3 | Trung bình, RMS, đỉnh phục vụ lượng điện nạp, nhiệt và ứng suất; F02/F05. | Tự tính ba đại lượng của xung chữ nhật; so với bài nạp ắc quy đã có. Viết rõ chu kỳ lấy trung bình. |
| C1-04 | I.3.2 — Sóng hài bậc cao, hệ số hình dáng, tr.3 | Đọc ripple của nguồn chuyển mạch và tác dụng của lọc; F02/F05. | Phân biệt DC, ripple RMS, ripple đỉnh-đỉnh và thành phần cơ bản; lập bảng ký hiệu/định nghĩa theo giáo trình. |
| C1-05 | I.3.3 — Công suất và hệ số công suất, tr.3–4 | Định nghĩa công suất bằng trung bình v(t)i(t); phân biệt W, VA và hiệu suất. PF của nguồn AC thuộc adapter ngoài. | Tính P, S, PF cho ví dụ nguồn sin/dòng xung; với dock lập budget công suất đầu vào–tải–sạc–tổn hao. |
| C1-06 | I.3.4.a — Tổ hợp mạch tuyến tính theo trạng thái ngắt điện, tr.4 | Chia mạch theo diode/MOSFET dẫn-ngắt để viết đúng phương trình. | Với mạch RL có diode hồi dòng: vẽ từng mạch tương đương và điều kiện chuyển trạng thái. |
| C1-07 | I.3.4.b — Giải trực tiếp quá trình quá độ bằng PTVP/Laplace, tr.4–5 | Dòng cuộn cảm không đổi tức thời; liên hệ khởi động và chuyển nguồn F02/F05. | Giải đáp ứng RL từ điều kiện đầu; so sánh đồ thị tay với mô phỏng dự kiến. |
| C1-08 | I.3.4.c — Chu kỳ tựa xác lập bằng PTVP/Laplace, tr.5–6 | Tách startup và trạng thái lặp lại; nền tảng ripple của converter. | Đặt i(0)=i(T) cho một chu kỳ; chỉ rõ cửa sổ đo steady-state trong kế hoạch mô phỏng. |
| C1-09 | I.3.4.d — Nguyên lý xếp chồng, tr.6–7 | Tách thành phần DC và hài khi phân tích tải tuyến tính; không áp xếp chồng tùy ý qua phần tử đóng-ngắt phi tuyến. | Làm lại ví dụ RC bằng thành phần DC và ít nhất một hài; giải thích giới hạn xấp xỉ. |
| C1-10 | I.3.4.e — Biến đổi Laplace rời rạc, tr.7 | Nhận diện hướng phân tích theo mẫu/chu kỳ. Tệp chỉ nêu tên phương pháp ở mục này. | Viết một ghi chú phân biệt mô hình liên tục, mô hình lấy mẫu và bước thời gian mô phỏng; chưa coi đây là khóa học điều khiển số đầy đủ. |
| C1-11 | I.3.4.f — Mô hình toán trên máy tính và mô hình thực nghiệm, tr.7 | Kết nối toán, QSPICE và phép đo; nền tảng V-model cho mọi feature. | Viết một procedure có giả thiết, sơ đồ, bước thời gian, đại lượng đo, giá trị dự đoán và tiêu chí sai lệch. |
| C1-12 | I.4 — Tóm tắt, tr.7 | Tự kiểm tra công thức và mô hình ngắt điện. | Tóm tắt một trang từ trí nhớ rồi đối chiếu nguồn. |
| C1-13 | I.5 — Bài 1, tr.7 | Nguồn DC–RL, đóng rồi mở khóa, diode hồi dòng; liên hệ năng lượng cảm và ứng suất khóa. | Giải i_L lúc đóng/ngắt; kiểm tra liên tục của dòng và đường thoát năng lượng. |
| C1-14 | I.5 — Bài 2.a–b, tr.7 | RLC và điều kiện đầu khác nhau; liên hệ inrush/overshoot. | Vẽ i_L và v_C cho hai điều kiện đầu; không tự giả sử tụ luôn bằng 0 V. |
| C1-15 | I.5 — Bài 3, tr.8 | Thêm diode nối tiếp làm thay đổi các trạng thái dẫn; liên hệ chặn ngược F01/F05. | Liệt kê điều kiện diode dẫn/ngắt, chỉ ra đoạn đáp ứng bị thay đổi so với Bài 2. |
| C1-16 | I.5 — Bài 4, tr.8, gồm điều kiện a/b kế thừa và c bổ sung | Diode song song ngược và năng lượng ban đầu; luyện giải mạch theo trạng thái. | So sánh với Bài 2/3 và kiểm tra dấu áp/dòng; tính trường hợp c được bổ sung trong đề. |
| C1-17 | I.6 — Tạp chí và website tổ chức KHKT, tr.8 | Học cách tìm nguồn sơ cấp khi giáo trình không đủ. Phần này có danh sách nằm trong ảnh nên trích xuất text đơn thuần bỏ sót. | Đọc đủ danh sách; ghi nhận nhóm IEEE/journal/conference và chọn một nguồn sơ cấp để tra khi cần. Địa chỉ năm 2004–2005 không mặc nhiên là URL hiện hành. |

## Điểm cần đọc có phản biện

- Trang 3 in quan hệ chu kỳ `T=ω/2π`. Kiểm tra đơn vị cho thấy phải dùng `ω=2π/T`, tức `T=2π/ω=1/f` cho các bài tính; ghi chú sai khác trong sổ, không sửa file nguồn.
- Giáo trình định nghĩa hệ số hình dáng DC bằng `V_avg/V_rms`. Nhiều tài liệu khác dùng tỷ số nghịch đảo. Ghi rõ định nghĩa trước khi đối chiếu kết quả.
- Công thức được tài liệu gọi là “THD đối với ngõ ra DC” có dạng tỷ số ripple RMS/DC. Khi trao đổi thiết kế, ghi tên đại lượng và công thức; không đồng nhất nó với THD so với thành phần cơ bản của tín hiệu AC.
- `PF=P/(V_rms I_rms)` là định nghĩa cần giữ. `cosφ` đơn lẻ không mô tả đủ dòng méo; đọc tiếp biểu thức có thành phần cơ bản ở trang 4.
- I.6 có nội dung trong ảnh thực sự ở trang 8; đã xem render để xác nhận, không đánh dấu mục này trống chỉ vì text extractor không đọc được.

## Đầu ra của chặng Chương 1

Một sơ đồ chức năng dock, bảng đơn vị/ký hiệu, một trang trung bình–RMS–đỉnh–công suất, bốn nhóm bài I.5 được lập mô hình, và một kế hoạch mô phỏng có kỳ vọng định lượng. Có thể đọc tiếp Chương 2 khi tự giải thích được đường dòng và điều kiện chuyển trạng thái; các phép mô phỏng và đo thực vẫn phải ghi nhận riêng khi được thực hiện.
