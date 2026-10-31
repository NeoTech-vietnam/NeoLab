# Kế hoạch chuẩn bị và phát triển NeoCam

Ngày: **2026-10-31**. Bắt đầu bằng XIAO ESP32S3 Sense đã có, rồi phát triển dần các [đặc điểm NeoCam](product-features.md). Tất cả các chặng dưới đây đang ở trạng thái **chưa thực hiện trên phần cứng trong hồ sơ này**. Các tiêu chí định lượng ghi dưới đây là đề xuất thử ban đầu, không phải thông số bán hàng.

**Framework đã chọn: ESP-IDF**, dùng từ P0 đến tích hợp camera, Wi-Fi, microSD và BLE Mesh. Camera dùng component `espressif/esp32-camera`; phiên bản ESP-IDF và component cần được chọn tương thích và lưu cùng cấu hình dự án.

## 1. Chuẩn bị vật tư và công cụ

| Hạng mục | Cần cho | Việc cần làm |
|---|---|---|
| XIAO ESP32S3 Sense và camera | P0 trở đi | Người dùng xác nhận có; kiểm tra revision, camera gắn, antenna và hư hỏng cơ khí. |
| Cáp USB-C có dữ liệu, máy tính, Wi-Fi nội bộ | P0–P1 | Xác nhận có, kiểm tra cổng serial và nguồn ổn định. Không coi cáp chỉ sạc là cáp nạp. |
| Công cụ lập trình | P0 | Dùng ESP-IDF theo lựa chọn người dùng. Ghi `idf.py --version`, toolchain, phiên bản `espressif/esp32-camera`, target `esp32s3`, cấu hình Flash/PSRAM và cổng serial. Chưa xác minh môi trường ESP-IDF hiện có trên máy. |
| microSD và đầu đọc | P2 | Kiểm tra có sẵn trước khi mua. Theo hướng dẫn Seeed: tới 32 GB, FAT32; thử mount và truy xuất thật. |
| Nút chụp, chỉ báo, dây thử | P2 | Kiểm tra pin map trước khi chọn GPIO, đặc biệt tài nguyên camera và SD. |
| Đồng hồ đo và thiết bị ghi công suất | P1, P3 | Đo USB giúp ước lượng tải; để chọn pin phải xét tải tại đường pin và dòng đỉnh. Dụng cụ không đủ nhanh thì ghi rõ chưa đo được đỉnh ngắn. |
| Pin và phần quản lý nguồn | P3 | Chọn sau khi đo và đọc schematic/revision; không mặc định pin đã đi kèm kit. |
| Node Mesh thứ hai | P4 | Mượn/tận dụng ESP32 tương thích trước; không mua đủ bộ NeoFilos ngay. |
| Vỏ mẫu, nam châm, tiếp điểm, tấm sạc thử | P5 | Làm theo kích thước thật; ưu tiên mẫu in 3D và gá thử đơn giản. |

## 2. Trình tự và tiêu chí kiểm chứng

### P0 Xác nhận board và môi trường

1. Chụp hai mặt board, ghi revision và camera đang gắn; kiểm tra lắp camera/antenna theo hướng dẫn hãng.
2. Đọc thông tin chip, Flash, PSRAM và sensor PID; không suy ra loại camera từ trang bán hàng.
3. Kiểm tra môi trường ESP-IDF hiện có; tạo dự án với target `esp32s3`, cấu hình Flash/PSRAM đúng board và cổng serial. Dùng firmware chẩn đoán nhỏ để kiểm tra build/nạp/log; ghi phiên bản toolchain, component, `sdkconfig.defaults` và log `idf.py build`, `idf.py -p <PORT> flash monitor`. Xem [hướng dẫn idf.py](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/tools/idf-py.html).

**Đầu ra:** bảng cấu hình thật, log khởi động và hướng dẫn nạp có thể làm lại. **Qua chặng khi:** board khởi động, đọc được camera và PSRAM, không còn lựa chọn pin map dựa trên phỏng đoán. Tham khảo [Seeed getting started](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/).

### P1 Chụp ảnh và xem hình qua Wi-Fi

1. Tích hợp [component `espressif/esp32-camera`](https://github.com/espressif/esp32-camera) và tham khảo ví dụ ESP-IDF của component; cấu hình pin map XIAO ESP32S3 Sense cùng PSRAM theo phần cứng thật. Đề xuất bắt đầu ảnh JPEG 640×480 rồi tăng theo kết quả.
2. Cấp nguồn USB, kết nối Wi-Fi nội bộ, xem từ trình duyệt trên máy tính/điện thoại. Ghi kích thước ảnh, tốc độ và lỗi; chưa cần đồng hồ NeoFilos.
3. Chụp 10 ảnh liên tiếp và thử xem hình khoảng 10 phút; quan sát reset, mất ảnh, dòng điện và nhiệt. Lưu ít nhất một ảnh gốc, log và ghi nhận điều kiện thử.

**Qua chặng khi:** 10 ảnh đều mở được và phiên xem hình hoàn tất không reset ngoài dự kiến. Nếu lỗi, lưu lỗi và sửa trước khi tăng chất lượng ảnh. Đây là kiểm tra chức năng ban đầu, chưa chứng minh độ bền hoặc nhiệt trong vỏ. [Tài liệu camera Seeed](https://wiki.seeedstudio.com/xiao_esp32s3_camera_usage/) dùng để đối chiếu phần cứng; firmware nguyên mẫu được phát triển bằng ESP-IDF.

### P2 Chụp và lưu ảnh bằng thao tác tại chỗ

1. Thử ghi/đọc ảnh microSD, sau đó thêm nút chụp và chỉ báo. Chọn chân từ pin map và schematic đúng board; không tùy tiện dùng LED sẵn vì có thể dùng chung tài nguyên với SD.
2. Quy định trạng thái: sẵn sàng, đang chụp, thành công, thiếu thẻ hoặc lỗi ghi. Xác định trạng thái camera phải bật chỉ báo.
3. Thử 10 lần nhấn, nhấn giữ, thẻ thiếu và thẻ không ghi được; ghi dung lượng tệp và cách xem lại.

**Qua chặng khi:** một thao tác hợp lệ tạo một ảnh truy xuất được; lỗi có phản hồi và không làm treo thiết bị. Ánh sáng chỉ báo và nắp che là hai cơ chế riêng cần kiểm chứng. Chưa gọi là dùng pin độc lập khi vẫn cấp nguồn USB.

### P3 Đo tải và chọn pin

Lập bảng tải cho: khởi động, chờ, chụp, ghi SD, xem Wi-Fi liên tục và ngủ. Ghi vị trí đo, điện áp, dòng trung bình, dòng đỉnh đo được, nhiệt, thời gian và độ sáng/độ phân giải liên quan. Ghi riêng số đo USB và số đo tại đường pin.

Tính gần đúng theo năng lượng:

`Dung lượng (mAh) ≈ 1000 × P_tải_trung_bình (W) × thời_gian (h) / [V_pin_danh_định (V) × hiệu_suất × phần_dung_lượng_sử_dụng]`

Hiệu suất và phần dung lượng sử dụng phải ghi là giả định hoặc kết quả đo. Chốt thời lượng mong muốn theo chế độ sử dụng trước khi mua pin; sau đó kiểm tra dòng đỉnh, dòng sạc cho phép của cell và kích thước. Cell lithium phù hợp, bảo vệ và mạch sạc là các thành phần khác nhau cần đối chiếu với board.

**Thử:** chạy bằng pin, rút USB, chụp/lưu/xem ảnh, đo thời lượng và hành vi khi pin yếu. Chỉ thử sạc trong lúc chạy sau khi xác minh đường nguồn hỗ trợ tình huống đó. Chân BAT không phải nơi cấp pogo 5 V; dòng sạc và cách bảo vệ phải đọc từ schematic/revision. Không giả định XIAO đã có sẵn chức năng báo phần trăm pin. Xem [Seeed battery usage](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#battery-usage).

**Qua chặng khi:** pin được chọn có căn cứ tải/dòng/kích thước, chạy độc lập theo thời lượng đã thống nhất và có số đo. Chưa chọn cố định 500 mAh hay 1.000 mAh ở bước concept.

### P4 Điều khiển BLE Mesh và kết hợp node thứ hai

1. Dùng ví dụ [ESP-BLE-MESH cho ESP32-S3](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/esp-ble-mesh/ble-mesh-index.html), kiểm tra gia nhập/cấu hình mạng và quyền điều khiển trước khi ghép firmware camera.
2. Đề xuất thông điệp ứng dụng: `capture_request(request_id)` và `capture_result(request_id, status)`; định nghĩa ID, phiên bản, timeout và chống thực hiện chụp lặp khi gửi lại. Đây là phác thảo, chưa phải đặc tả giao thức Neo chính thức.
3. Thử yêu cầu từ node thứ hai, restart, mất liên lạc và lỗi lưu ảnh; sau thiết lập thử khi không có điện thoại duy trì kết nối.
4. Kiểm tra camera Wi-Fi và Mesh cùng hoạt động: số lệnh thành công, độ trễ, lỗi và bộ nhớ. [FAQ Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/esp-ble-mesh/ble-mesh-faq.html) hiện nêu Wi-Fi STA cho coexistence; không mặc định chế độ SoftAP dùng đồng thời được.

**Qua chặng khi:** node được cấp quyền chụp và nhận kết quả tương ứng; lỗi/mất node được xử lý. Mục tiêu sơ bộ 20 yêu cầu được kết thúc bằng thành công hoặc lỗi xác định, không chụp trùng request. Ghi độ trễ thực tế trước khi chốt giới hạn. Chọn cơ chế ngủ/đánh thức và vai trò Friend theo kịch bản; deep sleep không duy trì nhận lệnh tức thời.

### P5 Vỏ nam châm và sạc hai tiếp điểm sau lưng

1. Xếp board, camera, antenna, pin, dây và mạch cần thêm; dành chỗ thao tác nút, mở vỏ/thay pin, thẻ và USB. Kiểm tra nhiệt trước khi đóng kín.
2. Thiết kế nắp che thực sự che được lens; chỉ báo nhìn thấy từ phía người dùng. Không coi nắp che tự cắt điện camera.
3. Làm gá nam châm và định hướng; thử trọng lượng thật, gắn/tháo và trượt. Số lượng, lực giữ và chu kỳ thử cần chốt theo kích thước/khối lượng.
4. Hai pad phẳng nằm **mặt sau NeoCam**; pogo nằm ở tấm sạc. Định nghĩa cực tính, vùng tiếp xúc và dung sai; nam châm tách khỏi đường điện. Nguồn pogo đi vào mạch quản lý sạc, không đi thẳng vào cell.
5. Thử có giới hạn dòng trên gá trước khi lắp pin; kiểm tra lắp sai, chập tiếp điểm, cắm/rút, dòng sạc và nhiệt. Thiết kế đường nguồn tránh cấp ngược khi USB và pogo cùng xuất hiện.

**Qua chặng khi:** bản vẽ/giao diện khớp với prototype, sạc đúng đường nguồn và không làm ảnh hưởng việc chụp/truyền; số đo và lỗi cơ khí được ghi nhận. Hình concept không dùng thay bản vẽ chế tạo.

### P6 Tích hợp và tối ưu giá

Thử lần lượt bốn cặp trong [CAM-F04](product-features.md), rồi hiển thị ảnh trên màn hình NeoFilos. Không dùng demo trình duyệt để đánh dấu tích hợp đồng hồ đã xong.

Lập BOM với mã hàng, số lượng, đơn giá VND, nhà cung cấp, ngày báo giá, công lắp, vỏ, pin, nguồn và hao hụt. Giữ hai tổng: chi phí mua thêm khi đã có XIAO và chi phí đầy đủ để làm lại một NeoCam. Ghi riêng phí cloud nếu dùng. So sánh giữ board có sẵn với PCB riêng chỉ sau khi biết tải và chức năng; quy mô 5 bộ không dùng giá lô 10.000 bộ.

## 3. Nhịp làm đề xuất

Theo nhịp học đã nêu **6–8 giờ/tuần**, chia thành buổi đọc/cấu hình, buổi thử và buổi ghi kết quả. Thứ tự P0–P6 là phụ thuộc kỹ thuật; chưa cam kết mỗi chặng chỉ một tuần. Chỉ mua pin/vỏ/tiếp điểm sau khi có dữ liệu cần thiết, ưu tiên tận dụng thiết bị hiện có.

## 4. Hồ sơ cần lưu khi thực hiện

Firmware thử, cấu hình và log build/flash cần có phiên bản đi kèm. Theo [kế hoạch repo NeoLab](../../docs/Repo%20Plan.md), đề xuất khi bắt đầu viết firmware thật sẽ dùng `products/neocam/firmware/`; chưa tạo mã nguồn sản phẩm trong thư mục concept.

Kết quả đo thực tế dự kiến lưu cùng phiên thử ở `products/neocam/tests/test-reports/`: board/revision, sensor PID, nguồn cấp, firmware, điều kiện, kết quả và tệp đính kèm. Các thư mục này chưa có báo cáo thực nghiệm. Đánh dấu từng CAM-Fxx theo chặng đã kiểm chứng; không chuyển sang “đạt” chỉ vì có hình hoặc ví dụ của hãng.

## 5. Lựa chọn cần chốt theo tiến độ

- P0: loại camera thực tế, phiên bản ESP-IDF/component và cấu hình board; framework ESP-IDF đã được chọn.
- P1: chất lượng ảnh tối thiểu và cách xem ảnh.
- P2: lưu ảnh, nút và chỉ báo.
- P3: thời lượng theo chế độ dùng, cell, sạc và nguồn.
- P4: provisioning, quyền và đường ảnh tách khỏi điều khiển.
- P5: kích thước, vật liệu vỏ, pin có thể thay, nam châm và pogo.
- P6: giá mục tiêu NeoCam và phạm vi ngân sách một bộ NeoFilos.
