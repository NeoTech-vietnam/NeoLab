# NeoCam: đọc mạch nguồn XIAO ESP32-S3 Sense

Ngày ghi nhận: **2026-11-03**. Đây là phân tích từ tài liệu hãng, chưa phải kết quả đo hoặc sơ đồ nguồn hoàn chỉnh của NeoCam. Mục tiêu sản phẩm vẫn là [quay video có tiếng và xem trước qua Wi-Fi trong ít nhất 45 phút](standalone-video-audio-problem.md).

## Phạm vi tài liệu

Sơ đồ được đọc là [XIAO ESP32-S3 Sense V1.51, trang 4](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/new-res/202003753_XIAO%20ESP32S3%20Sense_V1.51_SCH_260611.pdf), ngày 2026-06-11. Chưa xác định revision của board người dùng đang có, do đó các mã linh kiện và điện trở dưới đây là của **sơ đồ V1.51**; cần đối chiếu revision thực tế trước khi áp dụng.

## Đường nguồn và sạc trên board

```text
USB-C, VBUS ──┬──> U4: SGM40567 ──> VBAT ──> pin 1 cell
              │      mạch sạc         │
              └──> D2 ──┐             │
                        ├──> VIN ──> U3 + L4 ──> VCC_3V3 ──> ESP32-S3,
VBAT ────────> Q1 ──────┘            buck                    đầu nối board Sense
```

Đây là sơ đồ chức năng rút gọn theo sơ đồ hãng, không thay thế sơ đồ nguyên lý trang 4. Q1 và D2 thuộc mạch chọn đường nguồn từ pin hoặc USB. U4 sạc pin khi có USB; U3 `SGM6029CYG/TR` là bộ hạ áp đồng bộ, đi cùng L4 `470 nH`, tạo đường `VCC_3V3`. Sơ đồ V1.51 còn có đèn báo sạc CHG0. [SGM6029 datasheet](https://www.sg-micro.com/rect/assets/3323f5dc-718f-4b81-b820-398ddb702989/SGM6029.pdf).

Việc có mạch chọn nguồn và mạch sạc chưa chứng minh hoạt động quay ổn định khi cắm hoặc rút nguồn, hay chất lượng chuyển nguồn không gián đoạn. Theo [hướng dẫn Seeed](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#battery-usage), khi chỉ dùng pin thì chân 5V không có điện; điện áp pin không được nối sẵn đến một GPIO để phần mềm đọc trực tiếp.

## Bài tính từ sơ đồ: dòng sạc đã đặt

Sơ đồ V1.51 dùng U4 `SGM40567-4.2XG/TR`, chân F nối GND, và R10 `220 kΩ` từ IREF xuống GND. Theo [datasheet SGM40567, trang 3](https://www.sg-micro.com/rect/assets/e95e555a-a8a2-4761-b86a-6554dff2d8ed/SGM40567.pdf), với dòng đặt không quá 400 mA:

```text
I_CHG [mA] = 24000 / R_IREF [kΩ]
           = 24000 / 220
           ≈ 109 mA, làm tròn 110 mA.
```

Đây là dòng sạc tối đa theo giá trị đặt danh định, không phải dòng sạc đo được trong toàn bộ quá trình; dòng thay đổi theo giai đoạn sạc và điều kiện IC. Đây cũng **không phải dòng NeoCam tiêu thụ khi quay**. Với biến thể `-4.2` và F nối GND, datasheet cho mức điện áp sạc đặt danh định 4,2 V; phải chọn cell có thông số điện áp sạc tương thích với mạch, kể cả dung sai.

## Ý nghĩa với yêu cầu 45 phút

Trong chế độ pin, điện áp pin không cố định tại 3,7 V danh định. U3 là **buck**, không phải bộ nâng áp. Vì vậy khi điện áp đầu vào giảm, phải xem xét khả năng duy trì VCC_3V3 có tính tới sụt áp và dòng tải. Việc không còn điều chỉnh đúng 3,3 V **không tự động đồng nghĩa** ESP32-S3 ngừng hoạt động ngay: các chân nguồn chính của chip được Espressif khuyến nghị trong dải 3,0–3,6 V, còn các giới hạn khác của cả hệ thống và hiện tượng sụt áp tức thời cần xét riêng. Nguồn: [ESP32-S3 datasheet, mục 5.2](https://documentation.espressif.com/esp32-s3_datasheet_en.pdf).

Gọi `I_pin_tb` là dòng trung bình tại pin trong đúng chế độ ghi hình có tiếng và đồng thời stream hình qua Wi-Fi, thì:

```text
45 phút = 0,75 giờ
Q_rút_từ_pin [mAh] = 0,75 × I_pin_tb [mA]
```

Chưa có `I_pin_tb`, cell hoặc dung lượng sử dụng được, nên chưa chọn được pin từ yêu cầu 45 phút. Không thay 109 mA dòng sạc vào công thức này. Mức 45 phút và các FPS vẫn là yêu cầu chưa chứng minh khả thi.

## Những việc thiết kế còn mở

- Đối chiếu revision của board thực tế; xác định cell 1S, điện áp cho phép, dung lượng, kích thước và các chức năng bảo vệ của cell/mạch.
- Phân tích khoảng điện áp pin, đường sụt áp Q1 và đặc tính buck; kiểm tra đầu ra đối với tải, nhất là khi Wi-Fi phát và khi pin yếu. Cần dữ liệu đúng điều kiện trước khi kết luận phải thay hay thêm bộ biến đổi.
- Xác định yêu cầu và phương án đo/báo pin, giới hạn nhiệt, sạc qua hai tiếp điểm sau lưng và hành vi khi đang quay mà kết nối nguồn sạc. Chưa chốt cực tính hoặc mức điện áp của giao diện đó ở đây.
- Chứng minh đồng thời chức năng ghi hình, âm thanh, lưu và stream; dòng tiêu thụ của chính chế độ này hiện chưa có trong hồ sơ.
