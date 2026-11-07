# NeoCam: quay thường độc lập có âm thanh

Ngày: **2026-11-05**. Yêu cầu đề xuất **v0; chưa triển khai hoặc kiểm chứng**.

## Phạm vi và tham chiếu

NeoCam dùng pin riêng, XIAO ESP32-S3 Sense và ESP-IDF; lưu hình lẫn tiếng lên microSD, không cần điện thoại/module khác. Baseline đề xuất **Wi-Fi/BLE tắt**. **720p/30 FPS là mục tiêu chưa xác nhận**.

GO 3 có QuickCapture để bắt đầu quay nhanh; hãng công bố tự lưu khi tắt do pin yếu hoặc hết dung lượng. Đây là hành vi bên ngoài, không chứng minh thuật toán nội bộ. GO 3 dùng bộ nhớ tích hợp; NeoCam xét microSD. [Nút GO 3](https://onlinemanual.insta360.com/go3/en-us/operating-tutorials/step/button), [tự lưu và lưu trữ](https://onlinemanual.insta360.com/go3/en-us/operating-tutorials/storage/camera-storage).

Mục tiêu tham chiếu: tệp hoàn chỉnh chứa **ít nhất 45 phút hình-tiếng**, còn dự phòng kết thúc tệp. GO 3 công bố 45 phút ở 1080p/30 FPS, 25°C, Wi-Fi/màn hình tắt. [Nguồn hãng](https://onlinemanual.insta360.com/go3/en-us/specs/hardware). Yêu cầu **45 phút vừa ghi vừa xem trước Wi-Fi** trong [bài toán trước](standalone-video-audio-problem.md) vẫn được giữ riêng.

**Các trạng thái, nút và chỉ báo sau là đề xuất NeoCam**, không gán thành cách GO 3 thực hiện bên trong.

## Luồng đề xuất

**Nghỉ → Chuẩn bị → Ghi hình-tiếng → Hoàn tất tệp → Nghỉ.** Mỗi giai đoạn có nhánh lỗi.

| Trạng thái | Hành vi |
|---|---|
| **Nghỉ/tắt mềm** | Chờ nút; chưa chốt cách đánh thức, không mặc định 0 mA. |
| **Chuẩn bị** | Kiểm tra pin, thẻ, dung lượng và khả năng ghi; chuẩn bị camera, microphone, bộ đệm; tạo tên tệp duy nhất cả khi chưa có giờ/mạng. Báo đang chuẩn bị. |
| **Ghi hình-tiếng** | Chỉ báo đang quay khi đường thu/ghi sẵn sàng. Hình và tiếng dùng cơ sở thời gian chung. Quay thường không ghi đè tệp cũ. |
| **Hoàn tất tệp** | Dừng nhận dữ liệu mới; xử lý dữ liệu chờ, hoàn thiện cấu trúc tệp, flush/sync/close. Giữ nguồn MCU, bộ nhớ và thẻ. |
| **Đã lưu/nghỉ** | Chỉ báo thành công sau khi kết thúc không có lỗi được phát hiện, rồi nghỉ/tắt mềm. Lưu lỗi phải báo riêng. |

Chưa chốt màu/kiểu nháy LED. Cần phân biệt chuẩn bị, quay, pin yếu, đang lưu, đã lưu và lỗi; không mặc định có buzzer.

## Nút và tình huống bất thường

- Nhấn ngắn khi nghỉ: yêu cầu quay; nhận nút chưa có nghĩa đã bắt đầu ghi.
- Nhấn ngắn khi quay: dừng/lưu. Nhấn giữ để tắt: cũng lưu trước, không cắt nguồn ngay.
- Nhấn dừng lúc chuẩn bị: hủy yêu cầu, xử lý tệp đã mở rồi nghỉ.
- Đang lưu: không nhận lệnh tạo lượt quay mới hoặc cắt ngang; không xếp hàng để tự quay lại.
- Thiếu/lỗi thẻ hoặc thiếu dung lượng: từ chối quay. Gần đầy khi quay: tự kết thúc đủ sớm, chừa dung lượng cho dữ liệu chờ và metadata.
- Giới hạn định dạng/hệ thống tệp: chia đoạn trước giới hạn. Độ dài đoạn và gián đoạn cho phép còn mở, chưa hứa không mất khung.
- Cắm USB không tự bắt đầu quay. Vừa sạc vừa quay/chuyển nguồn cần phân tích riêng.

## Xử lý pin yếu

Ba quyết định riêng:

1. **Không cho bắt đầu:** nguồn không đáp ứng khởi động, quay và dự phòng lưu.
2. **Cảnh báo:** tiếp tục quay, báo pin yếu.
3. **Buộc dừng:** tự hoàn tất tệp rồi nghỉ; **không chờ PCM cắt**.

Chưa chốt volt, phần trăm hoặc giây. Xét điện áp có tải, sụt áp, nhiệt độ, lão hóa, nhiễu và hysteresis. Không để lọc nhiễu bỏ qua sụt áp nguy hiểm. Điện áp hồi sau khi dừng không được làm máy tự quay lại.

\[
E_{\text{còn dùng được trước mất ổn định}}
\ge E_{\text{hoàn tất tệp, bất lợi}}+E_{\text{dự phòng}}
\]

Tính cả dữ liệu chờ, MCU/thẻ, chỉ báo và thời gian ghi bất lợi. Rút pin, PCM ngắt bất ngờ, reset hoặc thẻ hỏng/rút có thể gây mất dữ liệu; không hứa cứu được mọi tệp. Flush/sync/close không tự bảo đảm chịu mọi mất nguồn.

## Nghiệm thu và phép tính tiếp theo

Các tình huống sau **chưa chạy thử**: nhấn quay tạo tệp có hình-tiếng; nhấn dừng/giữ tắt phải lưu trước; pin yếu phải cảnh báo rồi tự lưu; gần đầy phải lưu trong phần dự phòng; lỗi ghi không được báo thành công.

Tiếp theo: ngân sách dòng trung bình/đỉnh và thời gian từng giai đoạn; thời gian bắt đầu, độ lệch hình-tiếng; tốc độ ghi/bộ đệm; năng lượng kết thúc; đường đo pin và ngưỡng vận hành. Dòng cấp nguồn dự trù không tự thành dòng trung bình tính thời lượng. Các trị số thiếu dữ liệu để kiểm chứng sau; chưa yêu cầu lập trình/chạy thử ngay.
