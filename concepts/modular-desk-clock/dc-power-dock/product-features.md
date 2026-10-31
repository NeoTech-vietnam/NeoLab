# Đặc điểm sản phẩm DC Power Dock

Ngày ghi nhận: **2026-10-30**. Người dùng đồng ý lưu danh sách này làm cơ sở phát triển dock của Neo Modular Desk Clock, dựa trên [concept đã chốt](../approved-concept.md) và [hình v7](../assets/neo-clock-hidden-support-dock-v7.png).

Danh sách gồm **ba đặc điểm đã chốt từ concept** và **bảy mục tiêu bổ sung được đồng ý ghi nhận để thiết kế tiếp**. Ngày 31-10-2026, người dùng chốt mục tiêu **công suất tối đa của đầu ra sạc không dây là 25 W**. Các điều kiện đo, dung lượng pin, kích thước, chuẩn sạc và linh kiện vẫn cần xác định.

## Ba đặc điểm đã chốt từ concept

| Mã | Đặc điểm | Trải nghiệm sử dụng |
|---|---|---|
| DOCK-F01 | Một mặt sạc không dây dùng chung | Khi gắn đồng hồ, dock cấp nguồn không dây cho cụm đồng hồ. Khi tháo cả đồng hồ và chân đỡ, người dùng đặt điện thoại tương thích lên cùng mặt đế để sạc. |
| DOCK-F02 | USB-C cấp điện ra bên hông | Có thể sạc thiết bị bằng cáp thay cho sạc không dây. Đây là chức năng đầu ra; cách bố trí nguồn đầu vào cần xác định riêng. |
| DOCK-F03 | Đế gọn, mặt trên phẳng | Màn hình nằm thấp sát dock, chân đỡ giấu phía sau và tháo theo đồng hồ. Dock không còn phần chân đỡ nhô lên cản điện thoại sau khi tháo. |

**Cập nhật DOCK-F01, 31-10-2026:** Mục tiêu định mức là sạc không dây **tối đa 25 W** cho một thiết bị nhận tương thích trong các điều kiện thiết kế và thử nghiệm sẽ xác định. Điểm đo theo chuẩn/profile, điều kiện đạt và duy trì mức này còn cần chốt; đây không phải công suất adapter, công suất pin dock hay cam kết công suất thực vào pin điện thoại. Mỗi thiết bị có thể sạc ở mức thấp hơn. Mục tiêu 25 W không tự xác định công suất cổng USB-C đầu ra.

Lộ trình học và ví dụ tính toán cho feature này: [Huỳnh Văn Kiểm và bài toán mặt sạc không dây](learning/01-f01-mat-sac-khong-day-hvk.md). Các giá trị trong bài toán là giả định, chưa phải thông số đã đo của dock.

Qi/Qi2 là hướng tương thích đang nghiên cứu, với iPhone và Samsung Galaxy đời mới có hỗ trợ sạc không dây là các nhóm thiết bị tham chiếu. **Qi2 25W** là một lựa chọn cần đánh giá, không phải chuẩn hoặc chứng nhận đã đạt của dock Neo. Bộ nhận trong chân đồng hồ cũng phải tương thích với bộ phát được chọn và đáp ứng tải đồng hồ.

**Giới hạn sử dụng: mỗi lần dock sạc hoặc cấp nguồn cho một thiết bị nhận điện.** Cụm màn hình cùng các module đang gắn được xem là một thiết bị. Chế độ sạc không dây và USB-C đầu ra dùng luân phiên; quy tắc chọn chế độ khi có nhiều yêu cầu kết nối cần được thiết kế.

## Bảy mục tiêu bổ sung

| Mã | Đặc điểm | Lợi ích và điều kiện cần thiết kế |
|---|---|---|
| DOCK-F04 | Pin bên trong dock | Mang cả cụm đồng hồ đi sử dụng; khi tháo đồng hồ, dock có thể làm pin dự phòng cho thiết bị tương thích. Dung lượng và thời lượng sẽ dựa trên tải đo được. |
| DOCK-F05 | Vừa cấp nguồn vừa nạp pin dock | Khi cắm nguồn ngoài đủ công suất, thiết bị hoạt động đồng thời pin trong dock được nạp. Cần phân bổ công suất giữa tải và việc nạp pin; giảm dòng nạp nếu nguồn không đủ. |
| DOCK-F06 | Rút dây nguồn mà đồng hồ vẫn chạy | Khi pin còn đủ năng lượng và tải nằm trong giới hạn, việc rút nguồn ngoài không làm đồng hồ khởi động lại. Cần kiểm chứng quá trình chuyển nguồn trên cụm đồng hồ thực tế. |
| DOCK-F07 | Đèn báo trạng thái nhỏ | Phân biệt đang nạp pin dock, đang cấp điện ra, pin yếu và lỗi; có khả năng giảm sáng để phù hợp dùng ban đêm. Cách hiển thị mức pin, màu và nhịp báo còn mở. |
| DOCK-F08 | Định vị dễ và chống trượt | Giúp đặt đồng hồ hoặc điện thoại đúng vùng sạc; giữ cả cụm đồng hồ vững trên bàn. Cơ cấu định vị, vật liệu mặt đế và chân chống trượt cần thiết kế. |
| DOCK-F09 | Tự bảo vệ | Giới hạn quá tải, giảm hoặc dừng khi quá nóng và xử lý vật kim loại lạ trong vùng sạc không dây. Ngưỡng bảo vệ, cách báo lỗi và phục hồi cần được xác định và kiểm chứng. |
| DOCK-F10 | Pin thay được, đáy mở bảo trì | Thay pin xuống cấp và tiếp cận các bộ phận cần sửa để kéo dài thời gian sử dụng dock. Cần thiết kế nắp đáy, cách cố định pin, đầu nối và quy trình thay thế. |

Nạp pin bên trong dock trong khi cấp nguồn cho một thiết bị là một chế độ quản lý nguồn của dock, vẫn giữ giới hạn một thiết bị nhận điện bên ngoài.

## Ranh giới giữa dock và đồng hồ

Dock quản lý **tổng nguồn cấp**, pin nội bộ, việc chọn đầu ra, trạng thái và bảo vệ ở cấp dock. Cụm màn hình phân phối nguồn và quản lý từng module đang gắn, sao cho tổng tải phù hợp khả năng cấp của dock.

Nếu cần hiển thị trạng thái dock trên màn hình hoặc phối hợp ngân sách nguồn, phải thiết kế giao tiếp trạng thái phù hợp. Khả năng truyền điện không dây không tự xác lập giao tiếp điều khiển từng module.

## Các thông số cần xác định tiếp

- Mốc đo và điều kiện đạt 25 W không dây, công suất liên tục và đỉnh; đo đồng hồ cơ bản trước, sau đó thêm từng module.
- Chuẩn/profile sạc không dây, phần nhận điện của đồng hồ, danh sách điện thoại tương thích và khả năng đạt các mức thấp hơn.
- Dung lượng, loại pin, thời lượng sử dụng và thời gian nạp pin dock.
- Cổng nhận nguồn, yêu cầu adapter, năng lực pin khi cấp công suất tối đa, việc nạp pin đồng thời và quy tắc chọn đầu ra.
- Kích thước, khối lượng, độ vững, định vị và khoảng cách cuộn dây, vật liệu, ốp điện thoại, cơ cấu tháo lắp và cách tiếp cận để bảo trì.
- Nhiệt độ, hiệu suất, phát hiện vật lạ, quá tải và hành vi giảm hoặc ngừng công suất khi cần.
- Mức ưu tiên triển khai từng tính năng và tiêu chí nghiệm thu đo được.

Các mã DOCK-F01 đến DOCK-F10 dùng để trao đổi và xây dựng yêu cầu chi tiết sau này. Chúng chưa thay thế các yêu cầu và phép kiểm chứng kỹ thuật cần cập nhật trong [PRD nháp](requirements.md) và [kế hoạch kiểm chứng](verification-plan.md).

## Tham khảo kỹ thuật

- [WPC về sạc Qi, quá nhiệt và phát hiện vật lạ](https://www.wirelesspowerconsortium.com/about-us/): cơ sở tham khảo cho DOCK-F09; không xác lập chứng nhận cho dock Neo.
- [WPC về Qi và Qi2 25W](https://www.wirelesspowerconsortium.com/standards/qi-wireless-charging/): phương án chuẩn cần khảo sát cho mục tiêu 25 W; không xác lập chứng nhận hay tương thích của dock Neo.
- [TI BQ25628 và quản lý đường nguồn](https://www.ti.com/product/BQ25628): ví dụ về khả năng quản lý tải, pin và chuyển nguồn liên quan DOCK-F05, DOCK-F06; chưa chọn linh kiện này cho sản phẩm.
