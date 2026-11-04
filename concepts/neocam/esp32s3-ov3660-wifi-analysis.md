# NeoCam: OV3660 → ESP32-S3 → Wi-Fi

Ngày: **2026-11-03**. Trạng thái: **phân tích tài liệu nhà sản xuất và tính toán; chưa triển khai hoặc chạy phép thử**.

Ưu tiên theo người dùng: trước tiên đọc datasheet và tìm hiểu đường xử lý, không vội kiểm thử. NeoCam dự kiến dùng XIAO ESP32-S3 Sense với sensor OV3660, phát triển bằng ESP-IDF.

## 1. Luồng dữ liệu đề xuất

```text
Ánh sáng
   ↓
OV3660: cảm biến + xử lý ảnh + bộ nén
   ↓  DVP: dữ liệu song song + PCLK/HREF/VSYNC
ESP32-S3: camera controller + DMA
   ↓  bộ đệm khung hình trong RAM/PSRAM
Ứng dụng ESP-IDF
   ↓  HTTP → TCP/IP/lwIP → Wi-Fi driver
Wi-Fi 2,4 GHz → điện thoại/máy tính
```

**Phương án kết nối đã chọn:** NeoCam làm Wi-Fi Station, cùng mạng nội bộ với điện thoại, để điện thoại không phải chuyển sang Wi-Fi riêng của NeoCam. Phương án cần mạng Wi-Fi tương thích cho phép hai thiết bị liên lạc; bản thân việc xem cục bộ không cần Internet. Sơ đồ xử lý và giao thức stream ở trên vẫn là kiến trúc có thể triển khai, chưa phải firmware đã hoàn thành hay kết quả chạy; SoftAP chưa được chọn cho bài toán này.

## 2. OV3660 và giao diện camera

Datasheet OV3660 mô tả ISP và **compression engine** nằm trong cảm biến, cùng cổng video song song DVP. Driver chính thức `esp32-camera` hỗ trợ OV3660 và cấu hình `PIXFORMAT_JPEG`; code driver thiết lập các thanh ghi sensor cho chế độ JPEG. Do đó có thể chọn đường ảnh JPEG được nén từ sensor, không mặc định ESP32-S3 phải nén lại ảnh thô bằng CPU. Chưa kết luận chất lượng hay FPS đạt được từ đường này.

Datasheet ESP32-S3 v2.2, mục 4.2.1.4, nói camera controller nhận sensor DVP qua bus 8–16 bit, xung tối đa 40 MHz. Trên XIAO Sense, tài liệu Seeed liệt kê 8 đường dữ liệu camera cùng PCLK, HREF và VSYNC. 40 MHz là giới hạn khối giao tiếp trong datasheet, không phải PCLK của board trong một cấu hình cụ thể hay FPS stream.

Nguồn: [OV3660 datasheet, mục 2.2 và 5.1](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/res/OV3660_datasheet.pdf), [ESP32-S3 datasheet, mục 4.2.1.4](https://documentation.espressif.com/esp32_s3_datasheet_en.pdf), [Seeed pin map Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/), [driver OV3660 của Espressif](https://github.com/espressif/esp32-camera/blob/master/sensors/ov3660.c).

## 3. Bộ nhớ và firmware

Datasheet ESP32-S3 công bố **512 KB SRAM trên chip**. Seeed liệt kê XIAO ESP32-S3 Sense dùng ESP32-S3R8 với **8 MB PSRAM**; Espressif xác định R8 có PSRAM Octal SPI trong package, khác SRAM trên die. Driver camera nêu PSRAM và số frame buffer ảnh hưởng cách thu; DMA trực tiếp vào PSRAM phụ thuộc cấu hình. Không mặc định cả 8 MB trống cho ảnh.

Ứng dụng lấy frame từ driver, sau đó có thể gửi qua HTTP. Ví dụ JPEG HTTP stream của Espressif gửi nhiều ảnh JPEG dạng multipart; HTTP/lwIP và Wi-Fi driver chia dữ liệu thành gói để truyền. Luồng đó không tự là tệp video có âm thanh, cũng không mặc định là H.264.

Nguồn: [ESP32-S3 datasheet, bảng 1-1 và mục 4.1.2.1](https://documentation.espressif.com/esp32_s3_datasheet_en.pdf), [Seeed XIAO Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/), [esp32-camera: bộ đệm và JPEG HTTP stream](https://github.com/espressif/esp32-camera), [ESP-IDF: Wi-Fi datapath](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/wifi-driver/wifi-performance-and-power-save.html).

## 4. Wi-Fi: tốc độ PHY và kết quả công bố

ESP32-S3 datasheet v2.2: Wi-Fi 802.11b/g/n, băng tần 2,4 GHz, kênh 20/40 MHz, 1T1R, tốc độ lên tới **150 Mbit/s**. Đây là tốc độ danh nghĩa ở lớp vô tuyến, không phải tốc độ dữ liệu JPEG tại ứng dụng.

Espressif công bố kết quả **TCP TX** bằng `iperf` với cấu hình hãng: **20 Mbit/s trong phòng lab qua không khí**, **83 Mbit/s trong buồng chắn**. Đây là kết quả tốt trong điều kiện của hãng, không phải tốc độ bảo đảm hoặc mức trần tuyệt đối cho NeoCam; cũng không phải phép đo khi camera và microphone cùng hoạt động. Wi-Fi stack, buffer, PSRAM, giao thức và điều kiện vô tuyến đều liên quan.

Nguồn: [ESP32-S3 datasheet, mục Features](https://documentation.espressif.com/esp32_s3_datasheet_en.pdf) và [Espressif Wi-Fi Performance, bảng ESP32-S3 Wi-Fi Throughput](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/wifi-driver/wifi-performance-and-power-save.html).

## 5. Phép tính giới hạn dữ liệu cho một luồng JPEG

Với `f` khung/giây và `S̄` byte JPEG/khung trung bình, tốc độ payload ảnh trung bình là:

`R_JPEG = 8 × f × S̄` bit/s.

Ví dụ đối chiếu có điều kiện: lấy **1080p/20 fps** là giới hạn sensor OV3660 công bố, và **20 Mbit/s TCP TX** là một kết quả iperf qua không khí do Espressif công bố. Đây là **hai nguồn và hai điều kiện độc lập**, không phải một phép đo stream của NeoCam. Nếu dùng 20 Mbit/s làm ngân sách TCP và đặt mục tiêu 20 fps cho một máy nhận:

`S̄ ≤ 20.000.000 / (8 × 20) = 125.000 byte/khung ≈ 125 kB/khung`.

Đây là giới hạn trung bình lý tưởng dành toàn bộ ngân sách TCP cho dữ liệu JPEG; HTTP/multipart phải dùng một phần nên ngân sách JPEG thực tế nhỏ hơn. Kích thước JPEG thay đổi theo nội dung và thiết lập nén. Thỏa điều kiện trung bình này chỉ là điều kiện cần về dữ liệu theo phép tính, không đủ để chứng minh FPS, độ mượt, bộ đệm hoặc chất lượng ảnh. Kết quả không có nghĩa XIAO đạt 1080p/20 fps.

## 6. Vấn đề còn mở cho thiết kế

- Mục tiêu sơ bộ đã chọn: **video lưu 720p/30 FPS có tiếng; xem trước 720p/12 FPS không tiếng** đồng thời, trong ít nhất 45 phút bằng pin. Mức nén/chất lượng hình và tiêu chí chấp nhận chưa chốt; hệ thống chưa được chứng minh đạt mục tiêu.
- Kích thước JPEG, throughput khi chạy camera đồng thời với Wi-Fi, FPS cuối, dung lượng buffer và độ trễ của NeoCam: chưa có kết quả riêng.
- Người dùng đã xác định: vừa ghi video có tiếng, vừa stream hình không tiếng lên điện thoại. Đồng bộ hình-tiếng trong tệp, lưu thẻ và chế độ độc lập dùng pin chưa có kiến trúc hoàn chỉnh.
- Hiện chưa có cơ sở chuyển tốc độ vô tuyến hoặc công suất ở một phép đo khác thành dòng pin khi quay độc lập.

## 7. Hai mục tiêu người dùng đã chọn

- Video lưu: **1280 × 720, 30 FPS**, có âm thanh khi phát lại.
- Hình xem trước trên điện thoại đồng thời: **1280 × 720, 12 FPS**, không gửi âm thanh.
- Đây là yêu cầu sơ bộ, chưa phải hiệu năng đã đạt; codec, container và thiết lập chất lượng chưa được chốt.

Nếu một thiết kế dùng chung 30 khung hình/giây đã thu cho cả hai nhánh, thì nhánh preview 12 FPS cần chọn trung bình `12 / 30 = 0,4`, tức hai trong mỗi năm khung; cách lấy mẫu và đồng bộ cần được thiết kế. `1 / 30 ≈ 33,3 ms` là chu kỳ khung danh nghĩa khi nhịp đều, không phải độ trễ tổng thể. Luồng ghi vẫn cần xử lý âm thanh; không tự suy ra dòng điện từ các FPS.

Cơ sở để phân tích tính khả thi, không phải cam kết của NeoCam:

1. [OV3660 datasheet, bảng 2-2](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/res/OV3660_datasheet.pdf) nêu 720p / 45 FPS cho một chế độ cảm biến, với system clock 54 MHz. Thông số này không chứng minh XIAO, chế độ JPEG hoặc tác vụ ghi âm và stream đồng thời đạt 30 FPS.
2. [Espressif — ví dụ local JPEG stream](https://github.com/espressif/esp-webrtc-solution/blob/main/solutions/local_jpeg_stream/README.md) đặt cấu hình tham khảo ESP32-S3 + OV3660 720p / 12 FPS trên board Korvo, không phải kết quả trên NeoCam.
3. [Espressif — camera và Wi-Fi](https://docs.espressif.com/projects/esp-techpedia/en/latest/esp-friends/solution-introduction/camera/dvp-mipi-csi-camera-solution.html) báo cáo ví dụ ESP32-S3 stream MJPEG 720p khoảng 20 FPS; đoạn đó không nêu đầy đủ sensor/điều kiện và không chứng minh kịch bản ghi video, thu tiếng, stream đồng thời. Không coi 20 FPS là giới hạn tuyệt đối.
