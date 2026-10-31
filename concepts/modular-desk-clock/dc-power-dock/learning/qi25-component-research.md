# Nghiên cứu nguồn linh kiện sạc không dây cho DOCK-F01

Ngày kiểm tra: **31-10-2026**. Tài liệu ghi các ví dụ có nguồn chính hãng cho mặt sạc dùng chung với mục tiêu **tối đa 25 W**; đây là đầu vào để so sánh, không phải danh sách linh kiện đã chọn, kết quả đo hay bằng chứng dock Neo đã tương thích/chứng nhận. Điểm đo và điều kiện xác định mức 25 W của Neo vẫn chưa được khóa.

## Một bộ phát cho các bộ nhận dùng luân phiên

Khối phát nằm trong dock: nguồn DC phù hợp → điều khiển và công suất phát → mạng cộng hưởng, cuộn phát và lớp vật liệu từ. Bộ nhận của đồng hồ cần cuộn nhận, mạch nhận/chỉnh lưu/điều khiển và các đường DC phù hợp cho tải; điện thoại tương thích đã có bộ nhận riêng. Đây là hai trường hợp dùng luân phiên, với hình học, công suất và chế độ tải khác nhau.

Nếu cụm đồng hồ có pin riêng thì mạch nhận điện không tự thay thế mạch sạc/quản lý pin đó. Pin trong dock và pin của cụm đồng hồ là hai quyết định khác nhau.

## Khối phát 25 W để khảo sát tích hợp: Maxic MT201

[Danh mục WPC cho Maxic MT201, Qi ID 24250](https://jpsapi.wirelesspowerconsortium.com/products/qi/24250) ghi rõ: Qi v2.2.1, profile **MPP25**, potential load power **25.0 W**, loại PTx; mục “subsystem for integration in other products” là **Yes**, còn mục “evaluation module” là **No**. Đây là chứng nhận cho đúng MT201 trong hồ sơ, không chứng nhận một dock Neo hoàn chỉnh.

[Thông báo của Maxic về MT201 và MT5820](https://www.maxictech.com/en/about/news/107.html) nêu MT201 chứa MT5820, kích thước module đường kính 56,53 mm và dày 8,0 mm; công ty mô tả các profile BPP/EPP/MPP và các cấu hình nguồn vào 5 V/3 A, 9 V/3 A và 15 V/3 A. Các giá trị này là công bố của hãng, không chứng minh mức 25 W ở từng nguồn vào; hồ sơ WPC cụ thể chỉ liệt kê profile MPP25. Cần hỏi hãng tài liệu về profile, coil, che chắn, firmware, FOD, điều kiện 25 W, đầu vào, giới hạn tích hợp, giá và khả năng mua. Tình trạng tồn kho, giá và thời gian giao chưa được xác minh.

**Vai trò phù hợp ở giai đoạn này:** ứng viên subsystem để liên hệ lấy tài liệu và thử nghiệm, nhằm đánh giá hành vi nguồn, nhiệt, định vị điện thoại và tải đồng hồ. Trước khi sử dụng, kiểm tra kích thước, điều kiện tích hợp, firmware, nhu cầu adapter, quyền sử dụng và tương thích thực tế của các bộ nhận. Không tuyên bố chứng nhận của MT201 tự động chuyển thành chứng nhận cho Neo; xem [WPC: linh kiện và subsystem](https://www.wirelesspowerconsortium.com/knowledge-base/testing-and-certification/qi-components-and-subsystems/).

## Board nhận để học và thử: TI BQ51013C-Q1EVM

[TI BQ51013C-Q1EVM](https://www.ti.com/tool/BQ51013C-Q1EVM) là **board nhận**, BPP 5 W, ngõ ra danh định 5 V tới 1 A; cần bộ phát phù hợp bên ngoài. TI nêu cuộn nhận của board là **48 mm × 32 mm**, và lớp vỏ bên dưới cuộn tới mặt giao tiếp dày **2 mm**. Bản thân board, cuộn và gá có thể lớn hơn hoặc dày hơn kích thước cuộn: phải xem bản vẽ board trước khi kết luận lắp được trong chân đồng hồ.

[TI BQ51013C-Q1](https://www.ti.com/product/BQ51013C-Q1) là IC nhận; trang hãng ghi **ACTIVE** tại thời điểm kiểm tra, AC–DC/chỉnh lưu và giao tiếp Qi v1.3, khả năng đến 5 W. Điều này chỉ làm nó thành **ứng viên khảo sát** nếu tải đồng hồ sau đo, dòng khởi động, nhiệt, kích thước và điều kiện tích hợp phù hợp; không phải nguồn 25 W cho điện thoại và không tự chứng minh bộ RX có thể sạc pin riêng của đồng hồ.

**Ghép thử chưa xác nhận:** nhà sản xuất MT201 mô tả hỗ trợ BPP, nhưng chưa có thử nghiệm MT201 với BQ51013C-Q1EVM trong workspace; WPC chỉ liệt kê MPP25 trên hồ sơ MT201. Cần xác nhận profile/phiên bản với hãng và thử thực tế cả bắt tay, dòng thấp, tải đỉnh, khoảng cách, vị trí và FOD trước khi tuyên bố cặp này làm việc ổn định.

## IC phát để đọc nguyên lý: Infineon WLC1125

[Datasheet Infineon WLC1125, Rev. **, 14-10-2025, trang 6](https://www.infineon.com/assets/row/public/documents/24/49/infineon-wlc1125-wireless-charging-ic-wlc-transmitter-25-w-datasheet-en.pdf) trình bày hệ phát 25 W Qi2.x với nguồn Type-C PD sink hoặc DC, buck-boost, nghịch lưu cầu toàn phần, cuộn MPP1, giải điều chế ASK và IC xác thực qua I²C. Trang 34 mô tả FOD theo hệ số Q/tần số, tổn hao công suất và quá nhiệt khi có thermistor. Đây là ví dụ hữu ích để học sự liên hệ giữa converter, công tắc, cộng hưởng và điều khiển.

**Không đề xuất chọn mã này cho thiết kế mới:** tại thời điểm kiểm tra, Infineon ghi cả [WLC1125-68LQXQ](https://www.infineon.com/part/WLC1125-68LQXQ) và [WLC1125-68LQXQT](https://www.infineon.com/part/WLC1125-68LQXQT) là end-of-life/discontinued. Trang gợi ý tìm phương án thay thế, nhưng mã và tình trạng thay thế chưa được xác minh. Không suy ra một IC đơn lẻ hay sơ đồ minh họa thay thế được firmware, IC xác thực, cuộn, FOD và kiểm thử hệ thống.

## Không nhầm 50 W độc quyền với Qi2 25W

ST mô tả [giải pháp STWBC2-HP](https://www.st.com/content/dam/specialevents-assets/mwc-shanghai-2023/experts/100w-wireless-charging-solution-based-on-stwbc2-hp-and-stwlc99-rayna-wang.pdf): Qi v1.3 EPP tới 15 W, còn mức cao hơn dùng giao thức **ST SuperCharge (STSC) độc quyền** với bộ nhận thích hợp. [Thông báo ST về bộ board 50 W](https://newsroom.st.com/media-center/press-item.html/n4630.html) cũng nói 50 W dùng STSC. Vì vậy board ST 50 W có thể minh họa mạch điện tử công suất, nhưng con số 50 W đó không chứng minh tính năng **Qi2 25W** cho iPhone/Samsung của Neo.

Một linh kiện rời trong giải pháp ST này là MOSFET **STL20N6F7**. [Datasheet ST, trang 4](https://www.st.com/resource/en/datasheet/stl20n6f7.pdf) cho biết RDS(on) điển hình 4,6 mΩ, tối đa 5,4 mΩ **tại VGS = 10 V, ID = 10 A, TC = 25 °C**; VDS định mức 60 V. Các điều kiện này phải đi kèm thông số: không tự áp giá trị RDS(on) ở 10 V cho một mạch điều khiển cổng 5 V hoặc nhiệt độ khác. Đây là ví dụ học đọc datasheet trong một thiết kế khác, chưa phải MOSFET đã chọn hay đã kiểm chứng cho Neo.

Một ví dụ vật liệu cuộn là [TDK WCT38466-N0E0SST101 / MP-A28](https://product.tdk.com/en/techlibrary/productoverview/mp-a28.html), cuộn phát có lớp từ mỏng; chính TDK mô tả ứng dụng cuộn này là **15 W**. Nó giúp tìm hiểu cấu tạo và tổn hao AC, nhưng chưa đủ cơ sở chọn cho cấu hình Qi2 25 W hoặc thay cuộn của subsystem MT201.

## Quy tắc chọn linh kiện sau khi tính

1. Xác định tải đầu RX của đồng hồ: điện áp, dòng trung bình, đỉnh khởi động, mức thấp nhất, thời gian đỉnh và giới hạn nhiệt; kiểm tra đồng thời danh sách điện thoại mục tiêu.
2. Chọn TX và RX theo **đúng profile**, mức công suất tại đúng biên đo, điều kiện nguồn và thông số ghép cuộn. Qi2 25W là ứng viên được WPC mô tả cho Qi v2.2.1; lựa chọn chuẩn cuối cùng và điều kiện chứng nhận sản phẩm chưa chốt. [WPC về Qi2 25W](https://www.wirelesspowerconsortium.com/standards/qi-wireless-charging/)
3. Theo bản tham chiếu của nhà sản xuất để chọn cuộn/ferrite, tụ cộng hưởng và cầu công suất: kiểm tra điện áp đỉnh trên tụ, dòng RMS, điện trở AC, nhiệt, kích thước và dung sai. Dòng ở cuộn cộng hưởng không thể suy thẳng bằng dòng DC đầu vào; cần tính/mô phỏng và đo theo topology thật.
4. Tính nguồn đầu vào với biên đo và hiệu suất nêu rõ; xét thêm nạp pin dock đồng thời và chế độ chạy từ pin. Các cấu hình nguồn của MT201 không chứng minh 25 W trong mọi chế độ.
5. Xác nhận authentication, firmware và vật liệu cấp nguồn; làm rõ bảo vệ vật lạ, nhiệt, quá tải, cơ cấu giữ, căn cuộn và cách tháo đồng hồ. Đo với các receiver, ốp, vị trí và điều kiện sử dụng đại diện.
6. Chốt bộ linh kiện sau khi có chứng cứ: tài liệu thiết kế, khả năng mua, tương thích thực tế, đo điện/nhiệt, và điều kiện dùng subsystem/certification. Chưa có kiểm thử phần cứng hoặc xác nhận mua hàng trong tài liệu này.
