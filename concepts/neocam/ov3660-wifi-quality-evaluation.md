# NeoCam: đánh giá chất lượng OV3660 qua Wi-Fi

Ngày: **2026-11-03**. Trạng thái: **xác định yêu cầu và phương pháp; chưa chạy firmware, chưa stream và chưa có số đo của board người dùng**.

**Ưu tiên hiện tại theo người dùng:** đọc datasheet và phân tích cách ESP32-S3 xử lý hình qua Wi-Fi trước; chưa tiến hành phép thử. Xem [phân tích ESP32-S3 và OV3660](esp32s3-ov3660-wifi-analysis.md). Phần phương pháp thử phía dưới được giữ để tham khảo sau.

## Phạm vi và ý nghĩa

Người dùng cho biết board XIAO ESP32S3 Sense của mình gắn **OV3660**. Yêu cầu là **vừa ghi và lưu hình kèm tiếng, vừa phát hình không tiếng qua Wi-Fi để xem trên điện thoại**. Phân tích luồng hình truyền qua Wi-Fi cần được đặt trong chế độ hoạt động đồng thời này; chưa có bằng chứng hệ thống đã hoạt động.

Không lấy chất lượng, tốc độ khung hình hay công suất của stream Wi-Fi làm kết quả đo cho chế độ quay độc lập ghi thẻ. Bản thân hình ảnh phát qua Wi-Fi cũng chưa được thử trên board của người dùng.

## Giới hạn do nhà sản xuất công bố cho sensor

Datasheet OV3660 bản preliminary 1.3 do Seeed cung cấp công bố mảng ảnh 2048 × 1536 và tốc độ truyền ảnh tối đa của **sensor**, trong cấu hình được nêu trong datasheet:

| Cấu hình sensor | Tốc độ tối đa trong datasheet | Tình trạng trên XIAO và Wi-Fi |
|---|---:|---|
| VGA 640 × 480 | 60 fps | Chưa đo |
| 720p 1280 × 720 | 45 fps | Chưa đo |
| 1080p 1920 × 1080 | 20 fps | Chưa đo |
| 2048 × 1536 | 15 fps | Chưa đo |

Nguồn: [OV3660 datasheet, trang đầu và bảng 2-2](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/res/OV3660_datasheet.pdf). Các giá trị trên **không phải cam kết FPS của XIAO, firmware ESP-IDF hoặc luồng Wi-Fi**. Chưa biết board của người dùng đạt cấu hình nào, tốc độ nào hay hình có đáp ứng nhu cầu.

[Espressif esp32-camera](https://github.com/espressif/esp32-camera) hỗ trợ OV3660, JPEG và ví dụ JPEG HTTP stream với ESP-IDF. Tài liệu lưu ý tác động của PSRAM, định dạng ảnh và Wi-Fi; cần xác định cấu hình và phiên bản thực tế khi chạy. Việc driver có tên độ phân giải không chứng minh cấu hình đó stream thành công.

## Mục tiêu xem trước đã chọn

Người dùng đã chọn mục tiêu sơ bộ: **1280 × 720 / 12 FPS tại điện thoại**, hình xem trực tiếp không tiếng trong khi NeoCam ghi video có tiếng. Đây là yêu cầu thiết kế, chưa phải hiệu năng đã đạt; mức FPS tối thiểu và cách đánh giá tính ổn định chưa chốt. Video lưu có mục tiêu riêng **720p / 30 FPS**, cũng chưa được chứng minh đạt đồng thời.

Hai cấu hình tham khảo trong [ví dụ stream JPEG của Espressif](https://github.com/espressif/esp-webrtc-solution/blob/main/solutions/local_jpeg_stream/README.md) là **1280 × 720 / 12 FPS** và **640 × 480 / 25 FPS** cho OV3660. Ví dụ chọn 720p/12 FPS làm mặc định cho một cấu hình ESP32-S3 trên board Korvo; 640 × 480/25 FPS được nêu là tùy chọn camera. Đây là cấu hình tài liệu, không phải kết quả đo đầu ra XIAO hoặc cam kết khi vừa ghi hình, thu tiếng và stream.

Nhịp danh nghĩa `1 / 12` giây xấp xỉ `83,3 ms/khung`; con số này không phải độ trễ đầu-cuối. Với stream MJPEG, `R_ảnh = 8 × f × S̄`; kích thước JPEG trung bình `S̄` chưa biết nên chưa thể tính bitrate số hoặc công suất thực tế. Việc tạo profile xem trước khác profile ghi cần thiết kế, không mặc định sensor xuất hai profile đồng thời.

## Phương pháp đề xuất khi bắt đầu thử

1. Ghi board/revision, PID sensor, phiên bản ESP-IDF và `esp32-camera`, cấu hình PSRAM, nguồn cấp, mạng 2,4 GHz và thiết bị nhận. Không lưu mật khẩu Wi-Fi vào hồ sơ.
2. Dùng firmware ESP-IDF để phát luồng JPEG qua HTTP trên Wi-Fi. Lần đầu chỉ đánh giá hình; âm thanh và ghi thẻ sẽ được kiểm chứng riêng sau. Tùy điều kiện, dùng nguồn USB ổn định; số đo ở USB nếu có không đồng nhất với dòng pin.
3. Đề xuất lần lượt kiểm tra VGA, 720p, 1080p và 2048 × 1536. Giữ cảnh, khoảng cách, ánh sáng và các cài đặt so sánh được; ghi giá trị JPEG quality, frame buffer và mọi thay đổi. Nếu cấu hình không chạy, ghi lỗi thay vì coi là đạt.
4. Với mỗi cấu hình, đề xuất ba lần quan sát, mỗi lần 60 giây. Đây là **thời lượng và số lần thử đề xuất**, không phải chỉ tiêu sản phẩm. Lưu hình gốc/đoạn ghi màn hình, log và điều kiện thực tế; kiểm tra hình tĩnh có chi tiết và cảnh có chuyển động.
5. Ghi số khung JPEG nhận hoàn chỉnh, thời gian thực tế, tổng byte, lỗi/ngắt, nhận xét độ nét, màu sắc, hiện tượng nén và độ mượt. Ghi nhiệt độ môi trường nếu đo được. Xác định phương tiện đếm và giới hạn phép đo để tránh coi một khung hiển thị lặp là khung mới.

Với `N` là số khung hoàn chỉnh nhận được trong `T` giây, và `B_i` là số byte JPEG của khung thứ `i`:

`FPS_nhận = N / T`

`R_JPEG_trung_bình = 8 × Σ B_i / T` bit/s

Tốc độ này chỉ tính payload JPEG theo định nghĩa phép đo; không mặc định đã bao gồm overhead giao thức. FPS tại thiết bị nhận không mặc định bằng FPS cảm biến, và FPS trung bình một mình không chứng minh không có khựng. Muốn đánh giá độ ổn định phải ghi thêm khoảng thời gian giữa các khung, lỗi và điều kiện mạng.

## Dữ liệu cần để chốt yêu cầu NeoCam

Chất lượng nén, điều kiện chấp nhận của hai profile và thời lượng quay mong muốn còn mở. Dữ liệu của chế độ ghi hình và tiếng cùng lúc stream hình không tiếng là đầu vào quan trọng để thiết kế nguồn; chưa suy ra dòng chế độ đó từ phép đo của một tác vụ khác.

| Kết quả | Trạng thái |
|---|---|
| Ảnh/video stream thực tế | Chưa đo |
| Độ phân giải/FPS tại thiết bị nhận | Chưa đo |
| Tốc độ dữ liệu và độ ổn định | Chưa đo |
| Ghi âm thanh vào video đồng thời với stream hình không tiếng | Yêu cầu đã xác định; chưa kiểm chứng |
| Dòng và thời lượng quay độc lập | Chưa đo |
