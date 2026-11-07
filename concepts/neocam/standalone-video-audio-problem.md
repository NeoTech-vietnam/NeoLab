# Bài toán NeoCam: quay video độc lập có âm thanh

Ngày ghi nhận: **2026-11-03**. Trạng thái: **yêu cầu và bài toán đang xác định; chưa có kết quả thực nghiệm cho chế độ này trong hồ sơ NeoCam**.

**Cập nhật 2026-11-06:** đây là hồ sơ mục tiêu đồng thời có Wi-Fi. Chế độ ưu tiên hiện tại đã được tách thành [quay độc lập có tiếng 45 phút, radio tắt](standalone-normal-recording-behavior.md). [Hồ sơ nguồn v0.5](power-design-review.md) và [thiết kế chuyển USB/pin](usb-battery-switchover-design.md) tổng hợp mạch KiCad, tính toán và lựa chọn hiện hành; các câu “chưa có sơ đồ” bên dưới mô tả mốc 2026-11-03.

## Mục tiêu người dùng

NeoCam có pin riêng, có thể hoạt động độc lập mà không cần gắn vào màn hình hoặc module Neo khác. NeoCam cần **quay và lưu video có cả hình ảnh và âm thanh**, để khi lấy video ra xem lại có thể nghe được tiếng. Người dùng cũng cần **xem hình trực tiếp trên điện thoại đồng thời khi đang ghi**; luồng xem trực tiếp này **không cần âm thanh**.

Nguyên mẫu sử dụng **Seeed Studio XIAO ESP32S3 Sense** mà người dùng đã có; firmware dự kiến phát triển bằng **ESP-IDF**. XIAO Sense có camera, microphone số và khe microSD. Sử dụng microSD để lưu là phương án cần thiết kế và kiểm chứng; định dạng tệp và cách ghi chưa chốt.

## Yêu cầu và điều kiện đã ghi nhận ngày 2026-11-03

| Mục | Nội dung và trạng thái |
|---|---|
| Chất lượng hình xem trước | Người dùng đã chọn **mục tiêu sơ bộ 1280 × 720 (720p), 12 FPS tại điện thoại**, phát hình không tiếng qua Wi-Fi. Đây là yêu cầu mục tiêu, chưa phải hiệu năng đã đạt. |
| Chất lượng video lưu | Người dùng đã chọn **mục tiêu sơ bộ 1280 × 720 (720p), 30 FPS**, có âm thanh khi phát lại. Đây là yêu cầu mục tiêu, chưa được chứng minh khả thi trên XIAO ESP32-S3 Sense + OV3660 khi đồng thời stream. Codec, định dạng tệp, chất lượng nén và thông số âm thanh chưa chốt. |
| Âm thanh và chế độ đồng thời | **Đã xác định:** tệp video ghi lại có hình và tiếng; trong lúc ghi có thể xem hình trực tiếp trên điện thoại; luồng xem trực tiếp không cần gửi tiếng. Định dạng tệp và cách đồng bộ hình-tiếng chưa chốt. |
  | Kết nối xem hình trên điệnthoạiq  | **Đã chọn phương án Wi-Fi Station:** NeoCam tham gia mạng Wi-Fi sẵn có, điện thoại giữ kết nối với mạng của mình và xem hình qua mạng nội bộ. Đường đi này cần mạng tương thích cho phép hai thiết bị liên lạc, không cần Internet chỉ để xem cục bộ và không cần module Neo khác. Cấu hình mạng qua BLE là một phương án chuẩn bị, chưa chốt giao thức hay firmware. |
| Thời lượng quay và xem trước đồng thời | Người dùng chọn **mục tiêu ít nhất 45 phút liên tục**: ghi 720p/30 FPS có tiếng và đồng thời stream hình 720p/12 FPS không tiếng qua Wi-Fi. Đây là yêu cầu mục tiêu, chưa phải kết quả thực nghiệm hay thời lượng đã được chứng minh. Thời lượng khi chỉ ghi mà không xem trước chưa chốt riêng. |
| Kích thước | Mục tiêu theo phần camera tháo rời của **Insta360 GO 3**: **25,6 × 54,4 × 23,2 mm** (rộng × cao × dày), theo [thông số Insta360](https://onlinemanual.insta360.com/go3s/en-us/specs/hardware). Chưa chứng minh NeoCam lắp vừa; mức dung sai hoặc tối đa chưa được chốt. |
| Nhiệt độ môi trường | Cần xác định và kiểm chứng sau; **chưa có dải nhiệt độ vận hành của NeoCam**. Không lấy thông số nhiệt độ của riêng chip làm chứng nhận cho sản phẩm hoàn chỉnh. |
| Nước | Hiện tại **không yêu cầu chống nước**; yêu cầu và giải pháp bảo vệ liên quan đến nước xem xét sau. Chưa có cấp IP được công bố. |
| Giới hạn khác | Khối lượng, giá NeoCam, cách lưu, nút điều khiển, mức nhiệt khi chạm và điều kiện sạc trong lúc quay: **chưa chốt**. Đây là các mục cụ thể cần quyết định, không phải thông số đã được người dùng chấp nhận. |

Chi tiết thông số sensor, cách đánh giá stream và bằng chứng: [Đánh giá chất lượng OV3660 qua Wi-Fi](ov3660-wifi-quality-evaluation.md). Stream không tiếng đồng thời với ghi video có tiếng trong ít nhất 45 phút là **yêu cầu**, chưa phải chức năng đã hoạt động. Kết nối cục bộ qua Wi-Fi chung cần có mạng phù hợp; chưa có phương án xem hình khi không có mạng đó.

### Ý nghĩa mục tiêu thời lượng

- Chế độ đã chọn: dùng pin, ghi hình và âm thanh liên tục, đồng thời xem trước hình qua Wi-Fi trong tối thiểu 45 phút. Điều kiện pin sạc đầy lúc bắt đầu và không có nguồn sạc ngoài là đề xuất để định nghĩa một chu kỳ, chưa phải kết quả đã đạt.
- Chế độ chỉ ghi độc lập không xem trước: vẫn thuộc mục tiêu NeoCam, nhưng thời lượng riêng và trạng thái Wi-Fi/BLE chưa được người dùng chốt. Không mặc định dòng hay thời lượng của chế độ này từ chế độ có xem trước.
- `45 phút = 0,75 giờ`. Nếu `I_pin_tb` là dòng trung bình tính bằng mA tại đường pin trong đúng chế độ và khoảng thời gian này, thì điện lượng rút ra là `Q = I_pin_tb × 0,75` mAh. `I_pin_tb`, dung lượng pin dùng được và hiệu suất chưa biết; không suy ra dung lượng cell hoặc khẳng định một cấu hình đã đạt.
- [Insta360 GO 3](https://onlinemanual.insta360.com/go3s/en-us/specs/hardware) công bố pin camera 310 mAh, thời lượng 45 phút khi quay 1080p/30 FPS tại 25°C trong phòng lab, Wi-Fi và màn hình tắt. Đây là số liệu của GO 3 trong điều kiện riêng, **không phải thời lượng hay dung lượng đã xác định của NeoCam khi bật Wi-Fi**.

## Bài toán nguồn cần giải

Xác định và thiết kế hệ thống nguồn để NeoCam đáp ứng chế độ quay hình và tiếng độc lập, với thời lượng và các điều kiện hoạt động sẽ được xác định. Người dùng đã nêu hướng dùng **pin lithium một cell 3,7 V danh định**; cell, dung lượng và mạch cụ thể chưa được lựa chọn.

Cần lần lượt:

1. Xác định yêu cầu đầu ra: thời lượng quay mong muốn, điều kiện sử dụng, kích thước và các giới hạn sản phẩm cần thiết.
2. Kiểm tra đúng revision, sơ đồ nguyên lý và camera của board đang có; xác định những phần nguồn, sạc và bảo vệ board đã có, cùng những gì còn cần bổ sung.
3. Xây dựng sơ đồ khối và sơ đồ mạch phù hợp, tính toán dựa trên yêu cầu, tài liệu linh kiện và dữ liệu có nguồn gốc rõ ràng; lựa chọn linh kiện theo các điều kiện thiết kế.
4. Xác định cách kiểm chứng: ghi hình và tiếng cùng lúc, lưu và đọc lại được tệp; đo điện áp, dòng theo thời gian, thời lượng và nhiệt theo điều kiện thử được ghi nhận.
5. Đối chiếu kết quả với yêu cầu, ghi lại sai số và giới hạn dụng cụ đo, sau đó điều chỉnh thiết kế nếu cần.

Đây là trình tự công việc cần thực hiện, chưa phải thiết kế hoặc kết quả đo đã hoàn thành. Việc lựa chọn mạch cần xét dải điện áp và khả năng cấp dòng của pin, cũng như điều kiện thực tế của board.

## Những điều chưa chốt hoặc chưa được chứng minh

- Thời lượng thực tế của cả hai chế độ, mục tiêu thời lượng chỉ ghi, độ phân giải và FPS thực tế khi ghi và stream, định dạng tệp, chất lượng âm thanh và kích thước/dung lượng pin.
- Cách điều khiển quay, cách báo trạng thái, xử lý lỗi lưu và khả năng vừa sạc vừa quay.
- Firmware ESP-IDF quay đồng thời hình và tiếng, lưu được tệp và stream hình không tiếng cùng lúc: **chưa có bằng chứng chạy trong hồ sơ này**.
- Dòng trung bình, dòng đỉnh, công suất, nhiệt và thời lượng khi quay của NeoCam: **chưa có số đo của chế độ này**. Không kết luận đây là chế độ có dòng lớn nhất.
- Sơ đồ mạch nguồn hoàn chỉnh, thông số linh kiện và BOM: **chưa chốt**.

Không suy ra dòng tiêu thụ của chế độ quay hình và tiếng bằng cách cộng các số đo của những tác vụ khác nhau. Khi dùng số liệu hãng, phải ghi nguồn, điểm đo và điều kiện; phân biệt rõ số hãng công bố với kết quả đo NeoCam.

## Tài liệu liên quan

- [Đặc điểm NeoCam](product-features.md) và [kế hoạch phát triển](development-plan.md).
- [Seeed Studio: XIAO ESP32-S3 Sense và sử dụng pin](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/).
- [Espressif: hướng dẫn đo dòng tiêu thụ ESP32-S3](https://docs.espressif.com/projects/esp-idf/en/v5.2/esp32s3/api-guides/current-consumption-measurement-modules.html).
