# Modular AI Desk Clock

**Cập nhật 2026-10-31 — NeoFilos:** Tên và kiến trúc module mới được mô tả trong [Thiết kế hệ thống NeoFilos](neofilos-system-design.md). Các quyết định mới về pin riêng, BLE Mesh và kết hợp hai module được ưu tiên khi khác với tài liệu lịch sử bên dưới; ngoại hình v7 và các mục tiêu dock vẫn được kế thừa trong phạm vi đã nêu.

**Concept đã chốt ngày 2026-10-30:** [Mốc sản phẩm và các quyết định hiện tại](approved-concept.md), theo [hình v7](assets/neo-clock-hidden-support-dock-v7.png). Màn hình sát dock với chân đỡ ẩn; dock dùng luân phiên cho đồng hồ, điện thoại sạc không dây hoặc thiết bị qua USB-C. Mốc này được ưu tiên khi khác với mô tả và kế hoạch kỹ thuật trước đó bên dưới.

**Đặc điểm dock:** [Danh sách 10 đặc điểm sản phẩm đã ghi nhận](dc-power-dock/product-features.md), gồm cách sạc, pin dự phòng, chuyển nguồn, báo trạng thái, định vị, bảo vệ và bảo trì.

**Trọng tâm hiện tại (2026-10-30):** Ưu tiên định nghĩa và kiểm chứng **DC Power Dock** trước khi tích hợp prototype đồng hồ đầy đủ ở mục 5. Tải đầu tiên là lõi đồng hồ cơ bản và màn hình; mục tiêu di động là mang cả cụm đồng hồ cùng đế để sử dụng trong vài giờ. Công suất và thời lượng cụ thể cần đo/chốt trong [hồ sơ kế hoạch DC Power Dock](dc-power-dock/README.md). Các thông số từ hình concept vẫn là giả thuyết thiết kế.

## 1. Giới thiệu concept

Modular AI Desk Clock là thiết bị đồng hồ để bàn tích hợp trợ lý AI, được thiết kế theo kiến trúc mô-đun. Người dùng có thể tháo lắp, thay thế và nâng cấp từng bộ phận theo nhu cầu.

Sản phẩm hướng đến vai trò một người bạn đồng hành trên bàn làm việc: hiển thị thời gian, hỗ trợ tập trung, nhắc việc và cho phép tương tác bằng cảm ứng, giọng nói hoặc cử chỉ.

Trung tâm của hệ thống là một **lõi xử lý AI dạng cartridge mỏng, có thể tháo rời**. Lõi được đặt phía sau màn hình để tạo hình dáng cân đối, gọn gàng cho đồng hồ.

Trong định hướng dài hạn, cùng một lõi có thể chuyển sang các bộ sản phẩm tương thích khác, chẳng hạn robot bốn bánh hoặc thiết bị đeo. Mỗi bộ sản phẩm cung cấp nguồn điện, cảm biến và cơ cấu riêng; lõi đảm nhiệm xử lý và phối hợp hoạt động.

### Các mô-đun dự kiến

| Mô-đun | Vai trò |
|---|---|
| Lõi xử lý | Xử lý ứng dụng, kết nối và điều phối các mô-đun |
| Màn hình cảm ứng | Hiển thị đồng hồ, thông tin và giao diện tương tác |
| Đế nguồn và pin | Cấp nguồn, sạc pin, giám sát năng lượng và bảo vệ |
| Micro và loa | Thu nhận giọng nói, phát âm thanh và thông báo |
| Camera | Hỗ trợ nhận biết sự hiện diện, vật thể và cử chỉ |
| Cảm biến | Đo ánh sáng, chuyển động hoặc hướng đặt để điều chỉnh hoạt động |

Các mô-đun có thể sử dụng vi điều khiển riêng để thực hiện chức năng cục bộ và trao đổi dữ liệu với lõi.

## 2. Bài toán cần giải quyết

### 2.1. Thiết bị tích hợp khó sửa chữa và nâng cấp

Ở thiết bị nguyên khối, hỏng màn hình, pin hoặc một bộ phận nhỏ có thể làm giảm giá trị sử dụng của toàn bộ sản phẩm. Nâng cấp tính năng thường đòi hỏi thay cả thiết bị.

**Hướng giải quyết:** tách các bộ phận theo chức năng, thiết kế cơ cấu tháo lắp và cung cấp khả năng thay thế từng mô-đun.

### 2.2. Chức năng chưa phù hợp với từng người dùng

Nhu cầu sử dụng trên bàn làm việc khác nhau. Có người chỉ cần đồng hồ và bộ đếm tập trung; người khác cần nhắc việc, điều khiển bằng giọng nói hoặc nhận biết sự hiện diện.

**Hướng giải quyết:** xây dựng cấu hình cơ bản hữu ích, sau đó bổ sung mô-đun theo nhu cầu thay vì bắt buộc mọi người dùng sử dụng cùng một cấu hình.

### 2.3. Lãng phí phần cứng giữa các thiết bị

Các sản phẩm thông minh thường lặp lại bộ xử lý và kết nối dù có những nhu cầu tính toán tương tự.

**Hướng giải quyết:** nghiên cứu lõi xử lý dùng chung giữa các bộ sản phẩm tương thích. Khả năng tái sử dụng phải được kiểm chứng về công suất, giao tiếp, kích thước và hiệu năng.

### 2.4. Tiêu thụ năng lượng không cần thiết

Màn hình, camera, micro và xử lý AI có thể tiêu thụ điện ngay cả khi người dùng không tương tác.

**Hướng giải quyết:** phân chia chế độ hoạt động, điều khiển nguồn từng mô-đun và chỉ kích hoạt chức năng khi cần.

### 2.5. Quyền riêng tư trong không gian cá nhân

Thiết bị có camera và micro cần giúp người dùng hiểu rõ khi nào cảm biến đang hoạt động và dữ liệu được xử lý ở đâu.

**Hướng giải quyết:** ưu tiên xử lý cục bộ khi phù hợp, có chỉ báo hoạt động và cơ chế tắt cảm biến rõ ràng; camera dự kiến có màn che vật lý.

## 3. Giá trị cốt lõi

- **Hữu ích hằng ngày:** chức năng đồng hồ và hỗ trợ tập trung phải hoạt động tốt ngay cả khi chưa sử dụng AI.
- **Có thể sửa chữa:** pin và các bộ phận hao mòn có thể tiếp cận để thay thế.
- **Nâng cấp từng phần:** bổ sung chức năng thông qua mô-đun tương thích.
- **Tiết kiệm năng lượng:** tài nguyên hoạt động theo nhu cầu thực tế.
- **Tôn trọng quyền riêng tư:** trạng thái cảm biến và việc sử dụng dữ liệu phải minh bạch.
- **Tái sử dụng lâu dài:** hướng đến kéo dài tuổi thọ của lõi và các mô-đun.

## 4. Định hướng thiết kế sản phẩm

### 4.1. Hình dáng và bố trí

Mặt trước tập trung vào màn hình với giao diện dễ đọc. Lõi mỏng được bố trí phía sau màn hình; đế nguồn tạo điểm tựa vững và góc nhìn phù hợp.

Các mô-đun bổ sung cần hòa vào tổng thể, hạn chế tạo cảm giác các khối linh kiện nhô ra thiếu cân đối.

### 4.2. Vật liệu và khả năng tháo lắp

Ưu tiên vỏ kim loại bền, có thể tái chế, chẳng hạn nhôm, kết hợp vật liệu cách điện tại những vị trí cần thiết.

Sử dụng vít, ngàm và đầu nối có thể tháo để hỗ trợ bảo trì. Thiết kế phải đồng thời đáp ứng độ bền, quản lý nhiệt, cách điện và khả năng kết nối không dây.

### 4.3. Giao tiếp mô-đun

Định hướng ban đầu là đầu nối pogo nhỏ gọn, có khóa định hướng và cơ cấu giữ chắc chắn.

Phương án bốn tiếp điểm là giả thuyết thiết kế cần kiểm chứng. Số chân và giao thức chỉ được chốt sau khi đánh giá:

- Dòng điện liên tục, dòng đỉnh và sụt áp tiếp xúc.
- Băng thông của từng mô-đun.
- Cách kết nối nhiều mô-đun đồng thời.
- Độ bền tiếp điểm và khả năng tháo lắp.
- Bảo vệ khi cắm/rút và xử lý lỗi kết nối.

### 4.4. Kiến trúc nguồn

Đế nguồn chịu trách nhiệm cấp điện, quản lý pin và bảo vệ. Lõi và các mô-đun nhận các đường nguồn phù hợp thông qua giao diện đã xác định.

Prototype ban đầu ưu tiên nguồn DC ngoài phù hợp để tập trung kiểm chứng sản phẩm. Khối AC/DC công suất cao đang nghiên cứu là một hướng phát triển riêng; việc tích hợp vào đế cần được đánh giá sau.

## 5. Phạm vi prototype đầu tiên

Prototype đầu tiên tập trung vào cấu hình:

**Lõi tháo rời + màn hình cảm ứng + đế nguồn.**

### Chức năng chính

- Hiển thị thời gian và ngày tháng.
- Báo thức, hẹn giờ và bộ đếm tập trung.
- Điều chỉnh độ sáng.
- Nhận biết kết nối giữa lõi và phần thân.
- Hiển thị trạng thái nguồn hoặc pin nếu được trang bị.
- Chuyển sang chế độ tiết kiệm điện khi phù hợp.

### Mục tiêu kiểm chứng

- Đồng hồ có thực sự hữu ích trong sử dụng hằng ngày?
- Kích thước lõi có phù hợp với hình dáng sản phẩm?
- Thao tác tháo lắp có dễ hiểu và chắc chắn?
- Giao tiếp giữa các mô-đun có ổn định?
- Công suất, nhiệt độ và thời lượng pin có đáp ứng yêu cầu sử dụng?

Camera, nhận dạng cử chỉ và tương tác AI nâng cao được bổ sung sau khi nền tảng cơ bản hoạt động ổn định.

## 6. Định hướng phát triển

| Giai đoạn | Trọng tâm | Kết quả mong muốn |
|---|---|---|
| 1. Xác định nhu cầu | Tình huống sử dụng và yêu cầu sản phẩm | Phạm vi rõ ràng, có tiêu chí đánh giá |
| 2. Prototype đồng hồ | Lõi, màn hình và nguồn DC | Kiểm chứng chức năng và hình dáng |
| 3. Nguồn và cơ khí | Pin, quản lý năng lượng, tiếp điểm và tháo lắp | Hoạt động ổn định, dễ bảo trì |
| 4. Tương tác AI | Micro, loa, cảm biến; camera khi có nhu cầu rõ ràng | Chức năng AI mang lại giá trị thực tế |
| 5. Mở rộng hệ sinh thái | Thử lõi trên một bộ sản phẩm khác | Chứng minh khả năng tái sử dụng lõi |

## 7. Những điểm chưa chốt

Các nội dung sau cần được tính toán, thử nghiệm hoặc xác nhận trước khi trở thành thông số sản phẩm:

- Kích thước chính thức và nền tảng xử lý của lõi.
- Phân chia xử lý AI cục bộ và dịch vụ bên ngoài.
- Giao thức và số tiếp điểm của đầu nối.
- Công suất nguồn, dung lượng pin và thời lượng hoạt động.
- Kích thước màn hình và cấu hình mô-đun.
- Giá thành mục tiêu và chi phí thay thế.
- Khả năng tương thích với robot hoặc thiết bị đeo.

## 8. Tầm nhìn

Modular AI Desk Clock là sản phẩm đầu tiên để kiểm chứng một nền tảng thiết bị thông minh có thể tháo lắp, sửa chữa và mở rộng.

Ưu tiên trước mắt là tạo ra một chiếc đồng hồ để bàn hữu ích, ổn định và có thiết kế cân đối. Khả năng dùng chung lõi giữa nhiều sản phẩm sẽ được phát triển dựa trên kết quả kiểm chứng đó.
