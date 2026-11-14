# Kế hoạch thiết kế và phát triển firmware NeoCam

Ngày: **2026-11-14**. Phiên bản: **v0.1 — kế hoạch để tiếp tục trên máy khác**.

Trạng thái: đã tổng hợp các quyết định và kế hoạch từ trao đổi với người dùng; **chưa triển khai firmware, chưa hoàn tất D0–D2, chưa có kết quả nghiệm thu trên board**. Các thông số baseline dưới đây là lựa chọn thiết kế cần kiểm chứng, không phải hiệu năng sản phẩm đã đạt.

## Tiếp tục trên máy khác

Clone nhánh làm việc bằng HTTPS:

    git clone --branch neolab/neo-cam https://github.com/NeoTech-vietnam/NeoLab.git
    cd NeoLab

Nếu đã có clone, kiểm tra các thay đổi riêng trước khi chuyển nhánh hoặc cập nhật; sau đó dùng fetch, switch sang nhánh neolab/neo-cam và pull --ff-only. Không reset, clean hoặc ghi đè công việc chưa lưu.

Đọc tài liệu này trước. Điểm tiếp tục là **D0: lập register yêu cầu và ma trận tính năng–giới hạn**, tiếp đến D1 kiến trúc và D2 thiết kế chi tiết có rà soát độc lập. Chưa coi nội dung kế hoạch này là các tài liệu thiết kế đã hoàn thành và không đánh dấu tính năng đạt chỉ vì đã có một ví dụ của hãng.

Các đường dẫn triển khai trong tài liệu được tính từ root của repository; không phụ thuộc ổ đĩa, tên người dùng hay vị trí cài ESP-IDF của máy hiện tại. Workbook tham chiếu không được chép vào repository; Phụ lục A giữ tên nguồn, vị trí ô và các định nghĩa cần để tiếp tục mà không cần tệp trong Downloads của máy trước.

Nguồn nội bộ đọc kèm:

- [Đặc điểm NeoCam và CAM-F01–CAM-F08](product-features.md).
- [Hành vi quay thường độc lập](standalone-normal-recording-behavior.md).
- [Bài toán A/V và yêu cầu preview đồng thời](standalone-video-audio-problem.md).
- [Phân tích OV3660, ESP32-S3 và Wi-Fi](esp32s3-ov3660-wifi-analysis.md).
- [Kế hoạch phát triển trước đây](development-plan.md): phần P0–P6 có trước yêu cầu A/V đầy đủ. Tài liệu này cập nhật thứ tự phát triển phần mềm, vẫn giữ yêu cầu Wi-Fi/Mesh/pin ở các chặng sau.

## 1. Mục tiêu, yêu cầu đã chốt và phạm vi từng phiên bản

Thiết kế NeoCam theo trải nghiệm **Insta360 GO 3**, đối chiếu với giới hạn của **XIAO ESP32S3 Sense + OV3660**, ưu tiên xử lý ngay trên kit. Kiến trúc giữ **ApplicationLayer → RTE → BSW**, theo workbook tham chiếu; **ESP-IDF nằm sau các interface trừu tượng**.

Kế hoạch bao gồm thiết kế yêu cầu, kiến trúc, module, interface, luồng thực thi và kiểm chứng; sau đó triển khai từng chức năng qua các cổng nghiệm thu.

### 1.1. Các quyết định phải giữ nguyên

- Phần cứng đầu tiên: kit hiện có, OV3660, board Sense v1.1, microSD SanDisk Ultra 32 GB.
- ESP-IDF là framework đã chọn. Tham chiếu cấu trúc AUTOSAR không đồng nghĩa đã có AUTOSAR OS, RTE generator, MCAL hay chứng nhận tuân thủ chuẩn.
- USB trước: cấp nguồn, nạp, chẩn đoán và điều khiển; pin phát triển sau. USB ở bản đầu không phải webcam UVC hoặc USB mass storage.
- NeoCam phải chụp ảnh và ghi **video có âm thanh đồng bộ** lên microSD.
- Giữ mục tiêu **1280 × 720, 30 FPS và ít nhất 45 phút nội dung hình–tiếng phát lại được**. Hiện chưa có bằng chứng đạt.
- Quay tắt Wi-Fi/BLE được kiểm chứng trước. Giữ riêng mục tiêu sau đó: vừa ghi, vừa xem trước **720p/12 FPS không tiếng qua Wi-Fi Station** trong LAN; mục tiêu đồng thời cũng chưa được chứng minh.
- Hình ảnh qua Wi-Fi; BLE Mesh dành cho lệnh và trạng thái trong giai đoạn kết hợp module.
- Giữ các mã **CAM-F01–CAM-F08**; bổ sung yêu cầu A/V và các chế độ quay, không thay thế các yêu cầu cũ.
- Chức năng của Action Pod, app/Studio và phần cứng bổ sung phải được phân biệt với firmware chạy trên kit. Hậu kỳ trên host nằm ngoài bản firmware đầu.
- Cắm USB không tự bắt đầu quay. Không được suy ra camera/mic/thẻ đã bị cắt điện chỉ vì firmware đang không thu/ghi.

### 1.2. Chia phạm vi phát triển

| Nhóm | Nội dung | Điều kiện |
|---|---|---|
| **USB-Core** | Chẩn đoán; chụp JPEG; quay hình–tiếng; cấu hình; trạng thái; dừng/lưu; chia đoạn; xử lý lỗi. | Nền tảng bắt buộc trước mọi tính năng nâng cao. |
| **USB-Modes** | Hẹn giờ chụp; interval photo; timelapse; hẹn lịch khi còn cấp nguồn; loop recording; lưu cấu hình. | Các chế độ là baseline roadmap đề xuất; dùng lại đường thu/ghi đã kiểm chứng. |
| **Wireless** | Xem trước bằng trình duyệt; danh sách/tải tệp; điều khiển và cấu hình qua mạng nội bộ. | Đo riêng tác động lên ghi hình, bộ nhớ và nguồn. |
| **Neo integration** | BLE Mesh; node thứ hai; lệnh có quyền, chống thực hiện lặp; tích hợp màn hình và các module Neo. | Dùng chung hợp đồng lệnh/trạng thái của bản USB. |
| **Power/HMI** | Nút, chỉ báo, QuickCapture bằng nút, ngủ/đánh thức, đo pin và dừng khi pin yếu. | Cần interface phần cứng và số đo thật. |
| **Thử nghiệm** | Pre-record, HDR, xử lý âm thanh, voice control, slow motion độ phân giải thấp, chống rung cơ bản. | Mỗi tính năng phải có báo cáo khả thi trước khi trở thành tính năng hỗ trợ. |

Bản đầu không đặt mục tiêu ngang GO 3 về 2.7K, FlowState, FreeFrame hoàn chỉnh hay stereo. Các khả năng đó được ghi rõ trong bảng giới hạn và hướng mở rộng phần cứng/xử lý.

### 1.3. Các giới hạn phải đưa vào register yêu cầu

- OV3660 tối đa 2048 × 1536, fixed-focus. Các FPS trong datasheet là giới hạn sensor theo cấu hình quy định, không phải FPS của pipeline XIAO thu âm/ghi thẻ.
- ESP32-S3 có ngân sách CPU/RAM hữu hạn; 8 MB PSRAM không đồng nghĩa toàn bộ 8 MB đều dành cho pre-record hay xử lý ảnh.
- Kit stock đang xét có một microphone, không có IMU, màn hình hoặc loa. Không cam kết stereo, gyro-assisted stabilization hay giao diện Action Pod bằng firmware đơn thuần.
- JPEG từ sensor là đường khởi đầu; software H.264 trên S3 có tồn tại nhưng không có bằng chứng đáp ứng mục tiêu 720p/30 của NeoCam.
- Chậm tốc độ phát không tạo thêm chi tiết chuyển động như quay high-FPS thật; crop không tạo được góc nhìn chưa thu.
- Không hứa tự hoàn tất tệp sau rút USB khi chưa có nguồn dự phòng. FAT32/card metadata cũng có thể bị ảnh hưởng, không chỉ tệp đang ghi.
- Giữ nút/LED/nắp che, nam châm và pogo trong roadmap gốc; không dùng các mục này để yêu cầu sửa kit trước USB-Core. GPIO21 dùng cho SD CS, không nháy độc lập làm đèn trạng thái.

## 2. Kiến trúc package và phân công trách nhiệm

Dùng định nghĩa trong workbook làm nguồn chuẩn cho tên nhóm và trách nhiệm tham chiếu. Mỗi package được đánh dấu **thích nghi từ tham chiếu** hoặc **bổ sung NeoCam**; không coi catalogue là mã nguồn có thể đưa trực tiếp vào firmware. Không thay khung này bằng hệ lớp L0–L3 khác.

### 2.1. ApplicationLayer và RTE

| Nhóm | Package | Trách nhiệm được giao |
|---|---|---|
| **ASV** | CamSupv — mới | Quản lý tác vụ hiện tại: nghỉ, chụp, quay, chẩn đoán; cấp quyền sử dụng tài nguyên và thu hồi sau khi cleanup hoàn tất. |
| **ASW** | CamHmi — thích nghi trách nhiệm HmiMngt | Kiểm tra ý nghĩa lệnh, hiển thị trạng thái và kết quả; hỗ trợ USB trước, mạng/nút sau. |
| **ASW** | CamRec — mới | Chụp ảnh và các chế độ thu/ghi; sở hữu hành vi quay, đồng bộ, chia đoạn và kết thúc phiên. |
| **ASW** | CamDiag — mới | Chẩn đoán thiết bị, phép thử chủ động và thống kê phục vụ kiểm chứng. |
| **DE** | CamDev — mới | Biểu diễn camera bằng cấu hình, khung JPEG, thời gian thu và trạng thái. |
| **DE** | MicDev — mới | Biểu diễn microphone bằng PCM, số mẫu, thời gian và trạng thái. |
| **DE** | MediaDev — mới | Biểu diễn trạng thái, khả năng và dung lượng của phương tiện lưu trữ. |
| **RTE** | NeoCamRte — hiện thực theo trách nhiệm RTE | Interface, kết nối, sự kiện, ánh xạ runnable vào ngữ cảnh chạy và hợp đồng trao đổi buffer. |

Phân rã nội bộ CamRec:

| Module | Trách nhiệm |
|---|---|
| PhotoCtrl | Một lượt chụp và lưu ảnh. |
| RecCtrl | **Nơi duy nhất sở hữu S0–S5 của luồng quay.** |
| CaptureSchedule | Hẹn giờ, interval, timelapse và lịch bắt đầu hợp lệ. |
| AVSync | Mốc thời gian chung, nhịp video và thống kê khung thu/rơi/lặp. |
| SegmentCtrl | Chuyển đoạn và chính sách loop. |
| FinalizeCtrl | Điều phối dừng thu, xử lý dữ liệu chờ và hoàn tất phiên. |
| RecStats | FPS thực, mẫu âm thanh, áp lực buffer, thời gian ghi và kết quả phiên. |

CamSupv quản lý **quyền thực hiện tác vụ**; RecCtrl quản lý **tiến trình quay**. Không duy trì hai máy trạng thái S0–S5 độc lập. Trạng thái quay không được diễn giải thành trạng thái nguồn PMM.

### 2.2. BSW và backend ESP-IDF

| Nhóm | Package | Trách nhiệm |
|---|---|---|
| **SysSrv** | EbOs, ESM, WdgS | Trừu tượng thực thi, lifecycle nền tảng và giám sát. FreeRTOS/watchdog nằm trong implementation. |
| **SysSrv** | LocalTime — mới | Thời gian đơn điệu, deadline và timestamp; tách khỏi đồng bộ thời gian mạng của catalogue. |
| **Infrastruc** | BootCtrl, phần cần của BswLib, CUCEL | Startup, main, reset, kiểu nền tảng và tích hợp bộ nhớ/compiler. |
| **Infrastruc** | MediaFormat — mới | Đóng gói backend AVI/MJPEG/PCM, giữ riêng handle và lỗi của esp_muxer. |
| **MemStack** | MediaFs — mới | Mount, tên tệp, file handle, dung lượng, ghi/seek/sync/close và tệp chưa hoàn tất. |
| **MemStack** | SettingsStore — bổ sung ở USB-Modes | Lưu cấu hình có phiên bản; backend NVS được che sau interface. |
| **ComStack** | UsbConsole — mới | Transport và console USB. Wi-Fi/Mesh được bổ sung ở chặng tương ứng. |
| **CDD** | CamIo, MicIo — mới | Tích hợp camera và microphone đặc thù của kit. |
| **IoServices** | IoExtDev/SdCardIo — bổ sung | Khởi tạo, truy cập và trạng thái SD bên dưới filesystem. |
| **Mcal** | Các binding cần thiết | Driver MCU của ESP-IDF; các phần mở rộng theo chip phải được ghi rõ. |

Quy tắc phụ thuộc:

- ASV, ASW và DE sử dụng interface dự án qua RTE.
- Header công khai không chứa esp_err_t, FreeRTOS handle, camera_fb_t, FILE* hoặc muxer handle.
- Chỉ implementation BSW biết ESP-IDF và các thư viện backend.
- BSW có interface nội bộ giữa các package; ví dụ MediaFormat gọi MediaFs trực tiếp, không vòng qua RTE.
- Mỗi package logic không mặc nhiên trở thành một task hoặc một ESP-IDF build component.
- Không đặt AVI vào BswLib, không coi MediaFs là NvM hiện hữu, không coi UsbConsole là CAN/COM hiện hữu. Đây là các bổ sung có tên và phạm vi riêng.

### 2.3. STConf

- CONFIG: board profile và cấu hình vận hành mặc định.
- ConfAr: định nghĩa interface, kết nối, runnable/event mapping và cấu hình adapter.
- GLOBAL: kiểu dữ liệu, mã lỗi và phiên bản hợp đồng.
- TOOLCONF: toolchain, build, nạp và debug tái lập được.

Áp dụng mẫu ConfAr/BswIfWrp cho ranh giới với ESP-IDF. Workbook định nghĩa đây là wrapper giữa AUTOSAR và phần BSW không tuân thủ AUTOSAR. Bản đầu dùng hiện thực interface và kết nối tĩnh của dự án; không giả định có sẵn ARXML, generator hoặc RIPS cần triển khai đầy đủ.

## 3. Thiết kế chi tiết phải hoàn thành

Các thông số dưới đây là **baseline kỹ thuật đề xuất của kế hoạch**, được phân biệt với yêu cầu sản phẩm đã có.

### 3.1. Hợp đồng điều khiển và trạng thái

| Hợp đồng | Nội dung tối thiểu |
|---|---|
| Command | ID yêu cầu, nguồn yêu cầu, thao tác và tham số đã kiểm tra. |
| Status | Tác vụ hiện tại, giai đoạn quay, profile, tệp/đoạn hiện tại, dung lượng và lỗi gần nhất. |
| OperationResult | Phân biệt nhận yêu cầu, bắt đầu thực sự, hoàn tất, hủy và lỗi. |
| MediaDescriptor | Token buffer, ID phiên, định dạng, độ dài, sequence và timestamp. |
| AudioDescriptor | Thêm chỉ số mẫu đầu và số mẫu. |
| Error | Mã dự án như busy, tham số sai, thiếu thẻ, thiếu chỗ, timeout, I/O và hết bộ nhớ. Mã lỗi native được giữ trong chẩn đoán BSW. |

Nhóm lệnh USB đầu tiên:

- Thông tin, trạng thái và chẩn đoán.
- Chụp một ảnh.
- Bắt đầu/dừng quay.
- Chọn và đọc cấu hình.
- Hủy tác vụ đang chuẩn bị hoặc chờ lịch.

Các lệnh chụp có hẹn giờ, interval, timelapse, loop, đặt giờ và lưu cấu hình được bổ sung ở USB-Modes. Mọi nguồn điều khiển sau này phải dùng cùng đường kiểm tra và tiếp nhận yêu cầu.

Phản hồi nhận/busy/lỗi có mục tiêu ban đầu **≤250 ms**. Hoàn thành thao tác là sự kiện riêng. Chẩn đoán chủ động sử dụng tài nguyên chỉ chạy khi được CamSupv cấp quyền; đọc trạng thái thụ động vẫn hoạt động khi đang quay.

### 3.2. Trạng thái quay và xử lý lệnh

| Trạng thái | Hành vi và quy tắc |
|---|---|
| **S0 — Nghỉ** | MCU còn thức để giữ USB; không tự quay khi cắm nguồn. |
| **S1 — Chuẩn bị** | Kiểm tra thẻ, dung lượng, profile, bộ nhớ và thiết bị; chuẩn bị tệp. STOP sẽ hủy và cleanup. |
| **S2 — Quay** | Thu/ghi hình–tiếng; chỉ công bố đang quay khi đường dữ liệu đã sẵn sàng. |
| **S3 — Hoàn tất** | Chốt biên dừng, dừng nhận dữ liệu mới, xử lý dữ liệu chờ và hoàn tất tệp. START mới bị từ chối, không xếp hàng. |
| **S4 — Đã lưu** | Chỉ phát sau khi container và các thao tác tệp bắt buộc thành công. |
| **S5 — Lỗi** | Giữ nguyên nguyên nhân lỗi, cleanup trong khả năng còn lại; chỉ thử lại bằng yêu cầu mới. |

- STOP lặp trong S3 có tính idempotent, không khởi động lại deadline.
- Hủy S1 khi chưa có nội dung được ghi không được báo “đã lưu video”.
- Timeout chuẩn bị ban đầu **10 giây**, hoàn tất **15 giây**.
- Timeout không cho phép cưỡng bức giải phóng buffer hay hủy task đang thực hiện I/O.
- CamSupv chỉ trả quyền tài nguyên sau khi các worker xác nhận đã ngừng sử dụng.
- Nếu không thể quiesce đúng hạn: giữ lỗi, từ chối tác vụ mới và tiếp tục trả trạng thái; không reset vào vòng lặp tự thử lại.

### 3.3. Đường dữ liệu và đồng bộ hình–tiếng

Luồng chính:

    CamDev → JPEG có timestamp ─┐
                               ├→ buffer hữu hạn → AVSync
    MicDev → PCM có số mẫu ─────┘
                                      ↓ qua RTE
                                MediaFormat → MediaFs → SD

- Dùng số mẫu âm thanh làm mốc timeline.
- Nội bộ dùng số đếm 64 bit; chuyển sang đơn vị backend tại ranh giới BSW.
- Video được đưa vào các thời điểm phát cố định theo profile.
- Thiếu khung tại một thời điểm: lặp khung gần nhất và tăng bộ đếm riêng.
- Khung dư được bỏ có thống kê.
- Mất mẫu âm thanh, tràn buffer hoặc lỗi thu/ghi phải gây dừng có kiểm soát; không chèn im lặng để che mất dữ liệu.
- Báo riêng khung thu được, khung mới đã ghi, khung lặp và khung bị bỏ. FPS trong header AVI không chứng minh FPS thu thực.
- Timestamp camera lấy từ thời điểm thu do driver cung cấp, được chuyển đổi bên trong BSW; không thay bằng lúc consumer lấy ra khỏi queue.

Camera producer sao chép JPEG sang pool rồi trả framebuffer cho driver. Quyền sở hữu chỉ chuyển sau khi enqueue thành công; bên nhận giải phóng đúng một lần. ID phiên và generation của token ngăn completion cũ truy cập buffer của phiên mới. Payload không được chép vào một generic event bus; sự kiện điều khiển và dữ liệu media có đường riêng.

### 3.4. Thực thi, bộ nhớ và thời gian

Năm vai trò thực thi ban đầu:

| Vai trò | Công việc |
|---|---|
| Controller | Chạy điều phối tác vụ, trạng thái và sự kiện điều khiển. |
| Camera producer | Thu và chuyển JPEG vào pool. |
| Audio producer | Thu PCM và đếm mẫu. |
| Media writer | Thực hiện tuần tự mux, ghi, chuyển đoạn và hoàn tất. |
| Console | Đọc lệnh và xuất phản hồi có giới hạn. |

Task được ánh xạ qua RTE/BSW; ASW không tự tạo FreeRTOS task. SDK có thể tạo thêm task nội bộ. Chưa pin task ứng dụng vào core trước khi có số đo.

STOP và lỗi nghiêm trọng có đường báo riêng, hoạt động ngay cả khi queue media đầy. Không ghi log theo từng frame trên đường thu/ghi. ISR/callback native chỉ báo thức hoặc enqueue; logic ứng dụng chạy trong task context.

Ngân sách thử ban đầu cho profile 720p:

| Tài nguyên | Giới hạn thiết kế ban đầu |
|---|---:|
| Pool JPEG | 16 slot × 256 KiB |
| Framebuffer camera | Ngân sách dự kiến khoảng 512 KiB; xác nhận cấp phát thật |
| JPEG giữ lại để lặp | 256 KiB |
| Queue PCM | Khoảng 64 KiB |
| Dự phòng index AVI | 512 KiB; phải kiểm tra bằng heap trace |
| SD staging | 16 KiB, bộ nhớ nội phù hợp DMA |

Đây là ngân sách cần kiểm chứng, không phải bằng chứng đạt 30 FPS. Cấp phát pool dự án trong S1; khung vượt giới hạn phải báo lỗi, không cắt ngắn hoặc tự đổi chất lượng. Thư viện backend có thể còn cấp phát động; phải đo, không tuyên bố toàn bộ SDK/muxer không cấp phát trong lúc chạy.

Độ sâu queue được tính từ FPS và thời gian nghẽn dự kiến. Ngân sách 16 slot tương ứng khoảng 500 ms ở 30 FPS, có thêm một slot dự phòng. Sau cấp phát phải còn headroom đo được; baseline ban đầu giữ ít nhất **512 KiB PSRAM chưa cam kết**.

Phát hiện camera stall không đồng nghĩa driver đã dừng ngay: driver camera được ghim có lời gọi chờ framebuffer tới 4 giây. Cleanup phải chờ đúng vòng đời driver. Nguồn: [esp32-camera v2.1.9](https://raw.githubusercontent.com/espressif/esp32-camera/v2.1.9/driver/esp_camera.c). Cũng theo mã nguồn này, ESP32-S3 dùng LCD_CAM tạo XCLK; không mặc định chiếm LEDC chỉ vì generic camera configuration có các trường LEDC.

### 3.5. Định dạng, lưu trữ và các chế độ

**Profile ban đầu**

- Bring-up: VGA, 5 FPS, dùng để kiểm chứng đường dữ liệu.
- Profile đích: 1280 × 720, 30 FPS.
- JPEG chất lượng khởi đầu 12, được ghi rõ trong profile.
- Audio: PCM mono 16 kHz, 16 bit; thu khối 20 ms, ghép packet khoảng 100 ms.
- Video: AVI MJPEG + PCM, có index cuối tệp.
- Thử ảnh độ phân giải cao riêng khi nghỉ; không đổi sensor sang chụp ảnh lớn giữa phiên quay.

**Quay thường**

- Tên dựa trên counter phiên/đoạn, không yêu cầu giờ mạng; tạo độc quyền và kiểm tra tồn tại.
- Dùng tên tạm trong lúc ghi; chỉ đổi tên hoàn chỉnh sau khi hoàn tất thành công.
- Chia đoạn sau **5 phút hoặc trước giới hạn 512 MiB**.
- Bắt đầu chuyển theo kích thước ở khoảng 510 MiB để chừa phần hoàn tất; writer giữ chặn cứng 512 MiB. Theo dõi file extent, không chỉ vị trí seek hiện tại.
- Tiếp tục thu qua chuyển đoạn trong giới hạn buffer; đo và báo mọi gián đoạn.
- Mặc định cho bắt đầu khi còn ít nhất **128 MiB**; yêu cầu dừng an toàn khi xuống **64 MiB**. Đồng thời xác nhận phần dự phòng lớn hơn dữ liệu chờ, index và metadata.
- Không tự format, ghi đè, xóa hoặc tự nối tiếp phiên sau reset.
- Tệp chưa hoàn tất được giữ và đánh dấu để chẩn đoán.
- MediaFs giữ lỗi short-write/seek/flush/sync/close độc lập với mã trả về của muxer. Một lỗi đã xuất hiện không bị xóa bởi bước thành công tiếp theo.

**USB-Modes**

- Hẹn giờ/interval dùng LocalTime.
- Lịch theo ngày giờ chỉ được nhận khi đồng hồ hợp lệ; mất mốc giờ phải báo rõ.
- Timelapse tạo nội dung ngay trên kit; profile ghi rõ khoảng thu và FPS phát.
- Loop bật riêng; mặc định thử cửa sổ 5 phút, đoạn tối đa 1 phút.
- Loop chỉ xóa đoạn đã hoàn tất, không được bảo vệ và thuộc phiên loop hiện tại; không xóa ảnh, quay thường hoặc tệp khác.
- Cấu hình lưu chỉ được cập nhật khi nghỉ; kiểm tra phiên bản và giá trị khi nạp.

Rút USB đột ngột vẫn có thể làm hỏng tệp hoặc filesystem. Dung lượng dự phòng trên SD không thay thế năng lượng dự phòng.

## 4. Các chặng công việc và sản phẩm giao

| Chặng | Công việc | Đầu ra và cổng chuyển chặng |
|---|---|---|
| **D0 — Tổng hợp yêu cầu** | Lập ma trận toàn bộ mong muốn, nguồn, giới hạn kit và trạng thái: đã chốt/đề xuất/thử nghiệm. Giữ CAM-F01–08 và thêm nhóm A/V. | Mỗi yêu cầu có ID, phiên bản đích và phương pháp kiểm chứng; không còn tính năng bị bỏ quên hoặc nhầm là đã đạt. |
| **D1 — Kiến trúc** | Lập registry package, đối chiếu workbook, đánh dấu phần thích nghi/mới; vẽ dependency, luồng điều khiển và dữ liệu. | Rà soát độc lập vị trí package, quyền sở hữu và ranh giới ESP-IDF. |
| **D2 — Thiết kế chi tiết** | Hoàn thành interface, bảng state/event, sequence start/stop/error/rollover, task mapping, ngân sách RAM/CPU/I/O và kế hoạch thử. | Không còn mâu thuẫn về trạng thái, buffer, file handle, cancellation và lỗi. |
| **D3 — Nền tảng và ngoại vi** | Build/nạp tái lập; xác nhận board/PID/pin map; USB, thời gian, PSRAM, camera, mic và SD qua interface thiết kế. | Log phần cứng thật; JPEG đọc được, WAV nghe được, ghi/đọc SD khớp. Cập nhật ngân sách D2 bằng số đo. |
| **D4 — USB-Core** | Tích hợp chụp và quay; VGA ngắn → clip dài hơn → profile 720p; hoàn tất/lỗi/chuyển đoạn. | Qua kiểm thử chức năng, saturation, hủy/dừng và lỗi I/O trước khi chạy dài. |
| **D5 — Chế độ thông minh** | Hẹn giờ, interval, timelapse, loop và lưu cấu hình. | Qua kiểm thử lịch, dừng/hủy, dung lượng và phạm vi xóa của loop. |
| **D6 — Wi-Fi** | STA trên LAN; preview MJPEG, trạng thái và tải tệp khi nghỉ; sau đó thử đồng thời ghi/preview. | Client chậm không chặn recorder; đo riêng mục tiêu preview 720p/12 và ghi 720p/30 có tiếng. |
| **D7 — Mesh và module Neo** | Provisioning, quyền điều khiển, ID yêu cầu, chống lặp, mất kết nối và coexistence. | Thử node thứ hai; ảnh vẫn qua Wi-Fi. Tích hợp màn hình/module có nghiệm thu riêng. |
| **D8 — Nút, pin và nguồn** | Nút/LED phù hợp GPIO; QuickCapture bằng nút; cảm biến nguồn, PMM, ngủ/đánh thức, dừng pin yếu. | Ngưỡng dựa trên số đo và năng lượng hoàn tất; kiểm chứng pin 45 phút và chuyển nguồn riêng. |
| **D9 — Thử nghiệm nâng cao** | Pre-record, HDR, voice, xử lý âm thanh, slow motion thấp và chống rung. | Báo cáo khả thi có ngân sách/tác động lên baseline; chỉ đưa vào hỗ trợ sau khi qua gate riêng. |

D6–D9 giữ trong roadmap, không được dùng để trì hoãn nền tảng USB-Core. Các thử nghiệm không đạt phải được cách ly khỏi cấu hình đã hỗ trợ.

### 4.1. Hồ sơ thiết kế cần giao

Gộp thành bốn nhóm tài liệu:

1. **Yêu cầu và ma trận tính năng–giới hạn GO 3/kit.**
2. **Kiến trúc, package, interface và thiết kế hành vi/tài nguyên.**
3. **Kế hoạch kiểm chứng và traceability yêu cầu → thiết kế → test → bằng chứng.**
4. **Hướng dẫn build/nạp/vận hành và báo cáo thử.**

Vị trí dự kiến từ root repository:

- products/neocam/firmware/ — mã nguồn khi bắt đầu triển khai thật.
- products/neocam/tests/test-reports/ — bằng chứng chạy thật theo phiên thử.

Không tạo trạng thái “đã triển khai/đạt” cho các thư mục hoặc artefact chưa tồn tại. Giữ thay đổi phần cứng và tài liệu nguồn không liên quan ngoài commit phần mềm hiện tại.

### 4.2. Môi trường tái lập trên máy mới

Ghim baseline **ESP-IDF 5.5.6, esp32-camera 2.1.9, esp_muxer 1.2.4**, xác nhận tổ hợp ở D3 và lưu lock/configuration. SDK/toolchain cài riêng trên máy mới; không sao chép đường dẫn cài đặt hoặc biến môi trường của máy cũ vào dự án.

- Cài đúng release/toolchain và mở shell đã export môi trường ESP-IDF của máy mới.
- Kiểm tra phiên bản, target esp32s3, Flash/PSRAM thật và cổng USB thực tế.
- Khi firmware được tạo ở D3, lưu dependency manifest, lockfile, sdkconfig.defaults, partition configuration và hướng dẫn build/nạp.
- Lưu log build/flash và thông tin sensor/PSRAM cùng commit/configuration của từng phiên thử.
- Lệnh build/flash chỉ được thực hiện khi project thật đã tồn tại. Việc checkout kế hoạch hiện tại chưa tạo ra một project có thể nạp.

Nguồn baseline: [ESP-IDF 5.5.6](https://github.com/espressif/esp-idf/releases/tag/v5.5.6), [camera 2.1.9](https://components.espressif.com/components/espressif/esp32-camera/versions/2.1.9/readme), [muxer 1.2.4](https://components.espressif.com/components/espressif/esp_muxer/versions/1.2.4/readme?language=en).

## 5. Kiểm chứng, nghiệm thu và quản lý giới hạn

### 5.1. Các cổng nghiệm thu bắt buộc

| Cổng | Điều kiện |
|---|---|
| **Thiết kế hoàn chỉnh** | Yêu cầu có nguồn và test; package/interface/state/resource được rà soát độc lập; mọi bất định hiệu năng được ghi thành phép thử. |
| **Đúng kiến trúc** | ASW/DE và header RTE công khai build được với stub chuẩn C, không phụ thuộc ESP-IDF/FreeRTOS/muxer/POSIX file types. |
| **Đúng điều khiển** | 20 chu kỳ bắt đầu/dừng; hủy S1; STOP lặp S3; START khi bận; sự kiện cũ; queue đầy; timeout và giải phóng đúng một lần. |
| **Đúng dữ liệu** | 10 ảnh và 10 clip 60 giây hợp lệ; toàn bộ media giải mã được; AVI phát/tua được; không mất mẫu âm thanh hoặc báo lưu sai. |
| **Đồng bộ** | Ngưỡng thử ban đầu: lệch hình–tiếng **≤100 ms** gần đầu và cuối mỗi đoạn, kiểm tra bằng dấu hiệu hình/âm thật. |
| **Đúng lưu trữ** | Thử thiếu/thẻ lỗi/gần đầy, short-write, seek/sync/close lỗi, chuyển đoạn theo thời gian/kích thước và tên trùng. |
| **Chạy dài** | Chạy soak 60 phút cho mỗi profile được công bố hỗ trợ; có ít nhất 45 phút nội dung hình–tiếng hoàn chỉnh, mọi đoạn hợp lệ và số liệu tài nguyên. |
| **Đạt chất lượng đích** | Chứng minh 720p/30 bằng khung thu mới và timestamp; báo riêng khung lặp/rơi. Profile thấp hơn không được đánh dấu thay cho mục tiêu này. |
| **Tính năng đồng thời** | Wi-Fi, Mesh, voice hoặc pre-record phải qua lại các phép thử ghi hình trong đúng tổ hợp được hỗ trợ. |

Trong thử chạy dài phải lưu: firmware/configuration, board/thẻ/nguồn, kích thước JPEG, FPS thực, số mẫu audio, độ lệch A/V, queue peak, heap/stack watermark, SD latency, thời gian hoàn tất và gián đoạn chuyển đoạn.

Loop/fault injection phải dùng dữ liệu thử trong phạm vi đã chọn, không format hay xóa ảnh/video riêng trên thẻ của người dùng. Các ngưỡng thử mới trong kế hoạch phải được ghi là đề xuất kỹ thuật khi lập register D0; không diễn giải chúng thành thông số do người dùng đã đo hoặc đã nghiệm thu.

### 5.2. Giới hạn phải giữ trong kết luận

- Thiết kế đã review hoặc build thành công chưa chứng minh chức năng trên board.
- Quay 45 phút qua USB chưa chứng minh thời lượng pin, sạc, chuyển USB/pin không reset hoặc nhiệt trong vỏ.
- Không coi crop hoặc phát chậm là tương đương FreeFrame, FlowState hay quay high-FPS.
- Voice phải dùng lại microphone stream; preview dùng lại camera producer; không để tính năng mới tự khởi tạo lại ngoại vi đang quay.
- PMM chỉ được mở rộng sau khi có thiết kế nguồn/đánh thức thật. Nắp che vật lý không mặc nhiên tạo tín hiệu để firmware biết hoặc cắt camera.
- Khi không đạt mục tiêu, giữ nguyên yêu cầu và lập báo cáo chênh lệch cùng phương án sửa, tối ưu hoặc thay phần cứng; không âm thầm hạ thông số.

**Điểm bắt đầu là D0–D2: hoàn thiện bộ thiết kế có truy vết và review. Sau đó D3 cung cấp số đo để xác nhận thiết kế trước khi tích hợp USB-Core.**

## Phụ lục A. Nguồn tham chiếu và ý nghĩa cần mang sang máy mới

### A.1. Workbook kiến trúc

Nguồn người dùng cung cấp: **RefArch_MDG1evo_34010-RefArch_MDG1evo-34010-14.xlsx**, hình nhúng có nhãn **RefArch_MDG1evo SW-Architecture V1.12.0**.

Workbook được dùng làm reference package taxonomy và định nghĩa. Không đưa file Excel gốc, dữ liệu người phụ trách/liên hệ, hoặc đường dẫn máy cá nhân vào repository. Các vị trí sau đủ để kiểm tra lại khi người dùng cung cấp workbook trên máy mới; nội dung kế hoạch không cần truy cập trực tiếp file gốc để tiếp tục D0.

Sheet Architecture là view theo cây; FC-abc-Sorted là view sắp xếp với metadata bổ sung. Mỗi view có 35.762 dòng dữ liệu, không cộng hai view thành số component khác nhau. Cột A–E là chuỗi AC; F/G/H/I lần lượt có nhãn BC/MC/GC/FC; J/K là longname Đức/Anh. Workbook không có glossary hoàn chỉnh cho mọi viết tắt, nên không tự mở rộng các tên chưa được định nghĩa.

| Vị trí trên Architecture | Định nghĩa nguồn | Áp dụng/giới hạn cho NeoCam |
|---|---|---|
| B3/K3 | ASV — Application Supervisor | Giữ nhóm; CamSupv là package mới của NeoCam. |
| B951/K951 | ASW — Application Software | Chính sách chụp/quay và tương tác người dùng. |
| B23493/K23493 | DE — Device Encapsulation | Camera/mic/media ở phía ứng dụng; không chuyển DE thành driver MCU. |
| F23:K25 | PMM — Power Mode Manager; coordinator functions | Nguồn cho bước power/sleep; không đồng nhất với S0–S5 của recorder. |
| F6466:K6504 | HmiMngt — Hmi device management, dưới Veh | Thích nghi trách nhiệm thành CamHmi, không sao chép chức năng xe hoặc giữ Veh không liên quan. |
| D27715:K27774 | HmiDev; HmiActr và HmiSnsr | Tham khảo cho nút/LED về sau; chưa cần cho USB console thuần. |
| F34258:K34273 | BootCtrl; BswLib chứa main/memory layout/reset/trap/error support | Chỉ thích nghi phần nền phù hợp; không đặt AVI vào BswLib. |
| F34673:K34763 | Mcal — Microcontroller Abstraction Layer | Dùng driver/binding ESP-IDF làm implementation; không tuyên bố tuân thủ chuẩn chỉ từ tên. |
| F34765:K34801 | Mem — NvM/flash/EEPROM/memory-interface portfolio | MediaFs là bổ sung filesystem/microSD, không phải package đã có. |
| B28924/K28924 | ComStack — vehicle network communication (CAN, LIN and FlexRay) | UsbConsole là bổ sung mới; không đổi nghĩa CAN/COM hiện hữu. |
| F35363:K35399 | EbOs, OS Shell, ESM/EcuM | Thích nghi vai trò thực thi và lifecycle; FreeRTOS không được gọi là AUTOSAR OS tương đương đầy đủ. |
| F35451:K35456 | TimeServices — network time synchronization | LocalTime đơn điệu là trách nhiệm mới, riêng biệt. |
| F35473:K35482 | Rte và các plugin/timer-support entries | Có RTE trong catalogue dù A–E trống ở những dòng này; không bỏ sót khi đọc cây. |
| G35521:K35525 | BswIfWrp — interface wrapper between AUTOSAR and non-AUTOSAR BSW | Pattern trực tiếp cho abstraction ESP-IDF; không phải wrapper ESP-IDF đã tồn tại. |
| A35483:K35763 | STConf — Software and Tool Configuration | Giữ CONFIG, ConfAr, GLOBAL, TOOLCONF theo vai trò cần thiết. |
| G35735:K35758 | ToolConfig, gồm RteGenr configuration | Có definition tooling không chứng minh tool/generator đã được cài hoặc chạy. |
| B7061:K23492 | Coop — Cooperation/customer integration/delivery | Không dùng Coop để đại diện Wi-Fi/Mesh giữa các module Neo. |

Các package động cơ, immobilizer, high-voltage BMS, OBD và customer-specific delivery không được đưa vào chỉ để điền đủ các ô của hình. SIA là Security and Immobilizer Authority; SysVar là System Variability, không phải generic settings manager.

### A.2. Chuẩn trải nghiệm GO 3 và tài liệu kỹ thuật

- [GO 3 specifications](https://www.insta360.com/specs/go3): các mode GO 3, 2.7K/30, slow motion 1080p/120, H.264/MP4; không trộn tính năng GO 3S/GO Ultra vào baseline.
- [GO 3 QuickCapture](https://onlinemanual.insta360.com/go3/en-us/operating-tutorials/capture-preview/shooting-function/quickcapture): thao tác từ trạng thái tắt là yêu cầu khác với một lệnh start trên USB.
- [GO 3 loop recording](https://onlinemanual.insta360.com/go3/en-us/camera/looprecording): phân biệt loop với quay thường không ghi đè.
- [GO 3 pre-recording](https://onlinemanual.insta360.com/go3/en-us/operating-tutorials/capture-preview/shooting-function/pre-recording): lịch sử 10/15/30 giây của GO 3 không phải khả năng đã xác định của PSRAM NeoCam.
- [GO 3 FreeFrame/postprocessing](https://www.insta360.com/blog/tips/insta360-go-3-pro-best-settings.html): phân biệt xử lý camera với phần app/Studio.
- [Seeed XIAO ESP32S3 Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/): wiki hiện nay có nhiều revision/sensor; PID và board người dùng mới là baseline của NeoCam.
- [OV3660 datasheet](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/res/OV3660_datasheet.pdf): giới hạn sensor, không phải kết quả đo cả hệ thống.
- [Espressif software H.264](https://components.espressif.com/components/espressif/esp_h264/versions/1.5.0/readme?language=en): có encoder phần mềm trên S3, nhưng benchmark không chứng minh mục tiêu 720p/30.

## Phụ lục B. Checklist cho phiên làm việc tiếp theo

1. Kiểm tra trạng thái Git và đọc phần 1 cùng các nguồn nội bộ; không ghi đè công việc phần cứng/tài liệu khác.
2. Tạo register D0, giữ rõ nguồn yêu cầu và trạng thái chưa kiểm chứng; ánh xạ CAM-F01–08 và A/V.
3. Chốt registry D1 theo ApplicationLayer/RTE/BSW/STConf, phân biệt package tham chiếu và package mới.
4. Soạn interface/state/sequence/resource design D2 và test traceability; thực hiện review độc lập.
5. Chỉ bắt đầu D3 sau khi phạm vi và ownership đã nhất quán; số đo D3 phải phản hồi lại thiết kế.
6. Không tự đưa pin, Wi-Fi, Mesh hoặc mode thử nghiệm vào USB-Core; cũng không xóa chúng khỏi roadmap.
7. Chưa ghi nhận cổng nào là PASS nếu chưa có artefact/bằng chứng phù hợp. Tiếp tục từ D0, không giả định firmware đã có vì đọc thấy các tên package trong kế hoạch.
