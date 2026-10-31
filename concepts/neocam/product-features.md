# Đặc điểm NeoCam

Ngày ghi nhận: **2026-10-31**. Các mục dưới đây là mục tiêu phát triển từ trao đổi với người dùng; chưa được đánh dấu đạt trên phần cứng. [Ảnh concept](assets/neocam-features-v1.png) dùng đúng tám mã này để đối chiếu với kế hoạch thử.

## Tám đặc điểm ban đầu

| Mã | Đặc điểm và trải nghiệm | Phần cần thiết kế hoặc kiểm chứng | Chặng thử |
|---|---|---|---|
| **CAM-F01** | **Chụp ảnh:** người dùng yêu cầu chụp một bức ảnh; sản phẩm hướng tới nút chụp vật lý. | Thử bằng lệnh phần mềm trước, sau đó chọn nút/GPIO, chống dội và phản hồi thành công/thất bại. | P1, P2 |
| **CAM-F02** | **Pin riêng, dùng độc lập:** tháo khỏi đồng hồ vẫn có thể chụp và giữ ảnh. | Pin, bảo vệ, mạch sạc, cách đánh thức và lưu ảnh. Bản thử dùng microSD là phương án đề xuất. Thời lượng chưa chốt. | P2, P3 |
| **CAM-F03** | **Xem hình từ camera:** lúc đầu dùng trình duyệt qua Wi-Fi; đích tích hợp là màn hình NeoFilos. | Kích thước ảnh, tốc độ, độ trễ, xác thực truy cập và xử lý mất kết nối. Demo trình duyệt chưa chứng minh tích hợp màn hình. | P1, P4, P6 |
| **CAM-F04** | **Kết hợp module Neo:** có trải nghiệm hữu ích khi đi cùng AI, đèn, cảm biến hoặc âm thanh. | Cùng mạng được cấp quyền, khả năng và phiên bản tương thích; từng kịch bản phải có kiểm chứng riêng. | P4, P6 |
| **CAM-F05** | **Sạc qua hai tiếp điểm phía sau:** đặt vào tấm lưng sạc để nạp pin. | Hai pad nhận nguồn ở NeoCam, pogo có lò xo trên tấm sạc; cực tính, nguồn vào, chống chập, giới hạn dòng và mạch sạc trong module. Không cấp điện trực tiếp vào cell. | P5 |
| **CAM-F06** | **Gắn bằng nam châm:** gắn/tháo camera trên vị trí tương thích của đồng hồ hoặc sản phẩm Neo khác. | Nam châm riêng với tiếp điểm điện, định hướng chống lắp sai, lực giữ, độ trượt và đường antenna. Không mặc định mọi mặt kim loại hoặc mọi sản phẩm đều tương thích. | P5 |
| **CAM-F07** | **Nắp che và đèn báo:** che ống kính bằng tay, biết khi camera đang chụp/truyền. | Hành trình nắp, vùng che hoàn toàn, điều khiển chỉ báo, GPIO phù hợp. Nắp đóng vẫn có thể để camera lấy khung hình tối; chưa chốt cơ chế cắt nguồn cảm biến. | P2, P5 |
| **CAM-F08** | **Điều khiển qua BLE Mesh:** nhận yêu cầu chụp, báo trạng thái sẵn sàng/bận/lỗi/kết quả. | Provisioning, quyền, model/định dạng thông điệp, phản hồi, timeout, gửi trùng và hoạt động cùng Wi-Fi. Không dùng Mesh để stream hình. | P4 |

## Phạm vi theo phiên bản thử

**Nguyên mẫu V0 trên bàn:** camera chạy từ USB, chụp và xem hình qua Wi-Fi. Giữ ảnh/log để xác nhận phần camera trước. V0 chưa hoàn thành các mục tiêu pin, nam châm, pogo hay kết hợp module.

**Nguyên mẫu V1 độc lập:** thêm lưu ảnh và thao tác tại chỗ, đo tải, chọn pin phù hợp, kiểm chứng chụp không cần đồng hồ hoặc điện thoại. Việc xem ảnh sau đó vẫn có thể dùng thiết bị khác.

**Nguyên mẫu V2 kết hợp:** thêm Mesh và thử NeoCam với node thứ hai; tiếp tục thiết kế vỏ, nắp che và giao diện sạc. Mọi chỉ tiêu về thời lượng, độ trễ, nhiệt và cơ khí phải được cập nhật bằng số đo.

## Kịch bản với bốn module khác

| Cặp | Kịch bản kiểm chứng đề xuất | Điều kiện |
|---|---|---|
| NeoCam + đèn | NeoCam yêu cầu bật đèn, nhận xác nhận rồi chụp. | Chưa cam kết đồng bộ đèn flash; cần đo độ trễ và lượng sáng. |
| NeoCam + cảm biến | Số đo vượt ngưỡng thì yêu cầu NeoCam chụp và lưu ảnh. | Ngưỡng đã cấu hình, chống kích hoạt lặp và có nơi lưu ảnh. |
| NeoCam + âm thanh | Khi chụp xong hoặc lỗi, module âm thanh phát âm báo tương ứng. | Loa có âm báo cục bộ và nhận được sự kiện. Phát hiện chuyển động để báo động là bước mở rộng. |
| NeoCam + AI | Theo yêu cầu, gửi ảnh qua Wi-Fi để phân tích; trả trạng thái qua Mesh. | Nền tảng AI, đường ảnh và cách thể hiện kết quả cần chọn; AI cloud phụ thuộc Internet. |

Các cặp cần được thử sau thiết lập mạng, với chỉ hai module thực hiện kịch bản; không phụ thuộc màn hình/điện thoại duy trì điều khiển. Chức năng cần Internet phải nêu rõ điều kiện đó.

## Ý tưởng mở rộng chưa đưa vào bản đầu

- Phát hiện chuyển động và báo động qua module âm thanh.
- Ghi video: nghiên cứu dung lượng lưu, chất lượng, thời lượng và nhiệt.
- Webcam cho máy tính: cần chọn giao thức và chứng minh tương thích; không coi demo web qua Wi-Fi là webcam USB.
- Phân tích hình ảnh nâng cao với module AI; không hứa chạy mô hình AI lớn trên XIAO.

## Các thông số còn mở

Camera thực tế trên board, độ phân giải/FPS, số ảnh lưu, cách cấp quyền xem ảnh, thời lượng pin, dung lượng, kích thước vỏ, lực giữ nam châm, điện áp/dòng pogo và giá thành. [Kế hoạch phát triển](development-plan.md) quy định phép thử cần làm để chốt những lựa chọn này.
