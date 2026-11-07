# NeoCam — sơ đồ nguồn v0

Trạng thái: **sơ đồ nguyên lý để nghiên cứu và thảo luận**, chưa phải thiết kế đã xác nhận để chế tạo hay lắp pin thật. File KiCad là `neocam-power-v0.kicad_sch`. Cổng sạc USB-C, mạch sạc và bộ nguồn 3,3 V được giả định là các khối đã có trên XIAO ESP32-S3 Sense; không vẽ thêm bản sao của chúng.

## Phạm vi bản vẽ

- `BT1`: ký hiệu chức năng của **bộ pin lithium 1S đã có PCM**. Chưa chọn mã cell, PCM hoặc kích thước; ký hiệu không thể hiện chi tiết bên trong PCM.
- `J1`: ký hiệu đại diện cho hai điểm BAT+ và BAT− của XIAO; pin 1 trên ký hiệu là BAT+, pin 2 là BAT−. Đây là quy ước của bản vẽ, không phải xác nhận số chân hay footprint vật lý.
- `R1`, `R2`: mỗi điện trở 100 kΩ, dự kiến dung sai 1%, tạo bộ chia áp từ `VBAT_PACK`. `C1` = 100 nF nối song song R2 để lọc điểm `VBAT_SENSE`.
- `JP1`: solder jumper **mặc định hở**. Khi hở, mạch chia áp không được nối điện tới D0; đây là khoảng cách ly chờ đánh giá, không phải giải pháp tự động chống cấp ngược. Không đóng trước khi hoàn tất đánh giá các trạng thái bật/tắt và lỗi.
- `J2`: ký hiệu một chân **đại diện** cho D0 / GPIO1 / ADC1_CH0 của XIAO; không khẳng định board có đầu nối một chân riêng hay pin vật lý số 1. Chân này chưa được xem là đã dành riêng trong sản phẩm hoàn chỉnh.
- `PWR_FLAG` trên GND chỉ khai báo cho ERC rằng mạng tham chiếu có nguồn/đường về từ bên ngoài; không phải linh kiện vật lý hoặc bằng chứng mạch an toàn.

## Cơ sở đề xuất nhánh đo

[Seeed](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) ánh xạ D0 tới GPIO1; [Espressif](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html#adc) ánh xạ GPIO1 tới ADC1_CH0, khuyến nghị ADC1 và tụ 0,1 µF từ chân ADC xuống GND. Nếu JP1 được đóng sau khi thẩm định, C1 cần được đặt gần ADC theo hướng dẫn bố trí thực tế. Hướng ESP-IDF dự kiến là ADC1, attenuation 12 dB và hiệu chuẩn; không xem các giá trị này là kết quả đã đo.

Với điện trở danh định và bỏ qua dòng vào ADC:

```text
V_SENSE = V_PACK × R2 / (R1 + R2) = V_PACK / 2
I_chia   = V_PACK / (R1 + R2)
R_Thevenin = R1 || R2 = 50 kΩ
τ = R_Thevenin × C1 = 5 ms danh định
```

Để minh họa bằng một dữ liệu có điều kiện, SGM40567-4.2 của bản sơ đồ XIAO V1.51 đã xem có mức điện áp sạc 4,234 V ở điều kiện bảng datasheet được mô tả trong [phân tích pin/PCM](../battery-pcm-voltage-analysis.md). Không coi con số đó là giới hạn chắc chắn của mọi revision, nhiệt độ hoặc cell. Tại 4,234 V, chia áp lý tưởng cho 2,117 V và tiêu thụ 21,17 µA. Nếu hai điện trở riêng lẻ có dung sai ±1%, tỷ số chia nằm khoảng 0,495–0,505; điện áp tương ứng khoảng 2,096–2,138 V, trước sai số ADC, dòng rò và ảnh hưởng tải. Giá trị đó nằm trong vùng 0–2,9 V được Espressif công bố cho ATTEN=3 tại các điều kiện của hướng dẫn, nhưng **không chứng minh an toàn ở mọi trạng thái**, nhất là khi nguồn chip suy giảm.

## Các điểm còn mở trước khi dùng phần cứng

1. Xác nhận revision board đang có và cực tính pad theo tài liệu đúng revision; kiểm tra cấu hình pin và cách lắp/đấu thật.
2. Chọn cell và PCM theo điện áp sạc cho phép, dòng và thời gian, nhiệt độ sạc/xả, kích thước, dây và các chế độ lỗi. Một PCM không mặc nhiên giám sát nhiệt độ; bản vẽ chưa triển khai ngắt sạc theo nhiệt độ.
3. Phân tích điện áp/dòng chân ADC khi VDD bằng 0 hoặc đang lên/xuống, dòng cấp ngược, lỗi hở R2 và chuyển nguồn. JP1 đang hở nên ADC chưa hoạt động; cần thiết kế/thẩm định phương án phù hợp trước khi đóng hoặc thay bằng mạch bảo vệ tự động.
4. Đối chiếu các giới hạn điện áp, dung sai và sai số ADC; tính ngưỡng pin yếu theo toàn hệ và khả năng lưu video, không lấy ngưỡng PCM làm ngưỡng vận hành.
5. Dòng tải theo thời gian, hiệu năng quay/stream, nhiệt và mục tiêu 45 phút chưa có kết quả xác nhận. Sơ đồ chưa gồm giao diện sạc pogo sau lưng hoặc cơ chế quản lý đồng thời các nguồn sạc.
6. Chưa xác định footprint thật cho J1/J2/BT1, linh kiện mua cụ thể hay bố trí PCB. Không xuất Gerber hoặc coi sơ đồ này là thiết kế sản xuất.

## Kiểm tra sơ đồ

KiCad 10 `sch export netlist` xác nhận: `VBAT_PACK` nối BT1.1, J1.1, R1.1; `VBAT_SENSE` nối R1.2, R2.1, C1.1 và JP1.1; GND nối BT1.2, J1.2, R2.2 và C1.2; JP1.2 nối J2.1 trên một net riêng. `sch erc --exit-code-violations` báo 0 lỗi, 0 cảnh báo theo cấu hình hiện tại; cấu hình bỏ qua kiểm tra label toàn cục đơn lẻ, bốn điểm nối chung, vấn đề mô hình SPICE và footprint không khớp bộ lọc. ERC không xác nhận an toàn mạch, chức năng hoặc kết quả thực nghiệm. Chưa chạy mô phỏng hoặc đo phần cứng.
