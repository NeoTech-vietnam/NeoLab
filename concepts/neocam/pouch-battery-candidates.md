# NeoCam: tìm nguồn pin LiPo dạng túi có PCM

Ngày đối chiếu: **2026-11-05**. Đây là nghiên cứu linh kiện và khả năng mua hàng, chưa phải BOM đã phê duyệt, báo giá xác nhận, kết quả đo pin hay xác nhận thời lượng NeoCam.

## Yêu cầu sàng lọc

- Pin lithium polymer dạng túi, cấu hình **1S**, điện áp danh định khoảng **3,7 V**, sạc đầy theo datasheet của bộ pin.
- Bộ pin có **PCM tích hợp**, không thay thế bằng cell trần hoặc pin RC chỉ có dòng xả cao.
- Mục tiêu tìm mua: **xả liên tục ít nhất 1,5 A** cho cả bộ pin gồm cell, PCM và dây.
- Có tài liệu nhận diện được hãng, model và phiên bản; đối chiếu với đúng mã bán hàng.
- Có kênh mua hoặc đặt nhập tại Việt Nam; ưu tiên giá thấp. Tồn kho ở hãng/phân phối nước ngoài không đồng nhất với tồn kho trong nước.
- Mục tiêu NeoCam đang xét: ghi 720p/30 FPS có tiếng và xem trước 720p/12 FPS không tiếng qua Wi-Fi trong 45 phút. Chức năng và thời lượng này chưa được chứng minh trên nguyên mẫu.

## Kết quả chính

**Chưa có mẫu nào được xác minh đủ toàn bộ điều kiện để chốt mua ngay.** Có hai bộ pin Jauch với tài liệu hãng và trang đặt hàng Việt Nam đáng đưa vào vòng xác nhận; giá cao hơn nhiều so với các pin generic trong nước. Giá ở đây là niêm yết số lượng một viên, chưa xác nhận chi phí giao hàng, thuế và khả năng vận chuyển lithium của đơn hàng cụ thể.

| Bộ pin | Dung lượng theo tài liệu | Dòng xả tối đa của bộ pin trong tài liệu | PCM và kích thước | Niêm yết Việt Nam | Điểm còn mở |
|---|---|---|---|---|---|
| **Jauch LP503759JU + PCM + 2 WIRES 70MM**, Jauch No. 246517 | 1.300 mAh tối thiểu, 1.350 mAh điển hình, đo ở 0,2C đến 3,0 V | **2C = 2,7 A** ở bản 10/2022 | PCM: phát hiện quá dòng 3-6 A; bộ pin tối đa **62 × 38 × 5,4 mm** khi giao, dày tối đa **5,8 mm sau chu kỳ sử dụng** | **299.000đ**, [Xpart](https://xpart.vn/vi/product/jauch-quartz/lp503759ju-pcm-2-wires-70mm-15582583/p/15626651) | Cần xác nhận hàng giao đúng revision, dòng xả liên tục, tồn kho, thời gian và phí nhập |
| **Jauch LP103450JH + PCM + PTC + 2 WIRES 70MM**, Jauch No. 246520 | 1.900 mAh tối thiểu, 1.920 mAh điển hình ở tài liệu 10/2022 | **1C = 1,9 A** ở tài liệu PTC 10/2022 | Bản PTC: quá dòng PCM 4-10 A; PTC Ihold 4,5 A/Itrip 9 A. Listing DigiKey ghi **52 × 34,5 × 10,8 mm**; bản hãng 08/2024 có cấu trúc bảo vệ khác, xem bên dưới | **432.000đ**, [Xpart](https://xpart.vn/vi/product/jauch-quartz/lp103450jh-pcm-ptc-2-wires-70mm-15582693/p/15626654) | Link datasheet PTC của listing hiện trả 404 khi tải trực tiếp; cần tài liệu hiện hành đúng biến thể hàng giao |

Nguồn chính: [datasheet Jauch LP503759JU 10/2022](https://www.jauch.com/downloadfile/5bf52a0235b7eda183983c1995e3fcf49/1300mah_-_lp503759ju_1s1p_2_wire_70mm.pdf), [tài liệu PTC LP103450JH được công cụ tìm kiếm lập chỉ mục](https://www.jauch.com/downloadfile/5bf5298855bac29715e11c4887bb44ba3/1900mah_-_lp103450jh_1s1p_2_wire_70mm.pdf), [listing DigiKey đúng mã LP103450JH+PCM+PTC](https://www.digikey.com/en/products/detail/jauch-quartz/LP103450JH-PCM-PTC-2-WIRES-70MM/9560994).

### Phân biệt mức xả tối đa và xác nhận liên tục

Hai datasheet Jauch dùng nhãn **“Max. Discharging Current”**. Không thấy thời gian xung đi kèm, nhưng cũng không nên tự viết lại thành bảo đảm chạy liên tục 45 phút ở dòng đó. Trước khi chốt mẫu theo yêu cầu người dùng, cần xác nhận hãng hoặc nhà cung cấp rằng **bộ pin đúng revision cho phép xả liên tục ≥1,5 A trong điều kiện nhiệt độ áp dụng**. Không dùng ngưỡng ngắt quá dòng 3-6 A hoặc 4-10 A để thay cho thông số này.

### Đối chiếu listing, revision và việc mua tại Việt Nam

- Link PDF nhúng trong listing **LP503759JU** là [cdn.xpart.vn/r/51236679](https://cdn.xpart.vn/r/51236679), chuyển đến đúng tài liệu hãng 10/2022 Rev. 1.1 ghi **2C/2.700 mA**. Đây là bằng chứng listing có liên kết đến datasheet phù hợp, chưa phải bằng chứng lô hàng thực tế đã được kiểm tra.
- Một [bản datasheet Jauch cũ do BCE lưu](https://www.bce.it/wp-content/uploads/2019/11/LP503759JU-1300mah_-_lp503759ju_1s1p_2_wire_70mm_BCE.pdf) của **cùng tên LP503759JU** chỉ ghi **1C**, tương đương khoảng 1,35 A, không đạt tiêu chí tìm mua. Vì vậy phải khớp revision, không chỉ khớp tên ngắn của cell.
- Link PDF nhúng của **LP103450JH PTC** là [cdn.xpart.vn/r/51236700](https://cdn.xpart.vn/r/51236700), trỏ đến tài liệu PTC đã nêu nhưng đường dẫn đích hiện trả 404. Nội dung còn được công cụ tìm kiếm lập chỉ mục không thay thế một tài liệu hiện hành do bên bán cung cấp.
- [Bản hãng LP103450JH 08/2024](https://www.jauch.com/downloadfile/66cedf6fabf401098a2fc8d8447d4ce20/matd_246520_lp103450jhpcm2ic2_wires_70mm.pdf) tải được, ghi **1C**, dung lượng tối thiểu 1.900 mAh, **Second IC** thay vì PTC; PCM phát hiện quá dòng xả 2-7 A. Bản vẽ bộ pin ghi dài tối đa **52,0 mm**, rộng **34,5 mm**, dày **10,0 mm khi giao / 11,0 mm sau chu kỳ sử dụng**, dây 70±5 mm AWG24. Không gán cấu trúc PCM này cho listing có hậu tố PTC khi chưa được xác nhận.
- Dữ liệu tham chiếu DigiKey trên các listing Xpart có mốc cập nhật **16/05/2026**, thời gian giao cần liên hệ. Không coi số lượng hiển thị là tồn kho Việt Nam thời điểm 05/11/2026. [Điều khoản Xpart](https://xpart.vn/vi/terms) đề cập VAT 10%; cần xác nhận tổng tiền đã gồm VAT, vận chuyển và chi phí nhập của báo giá cụ thể.
- Listing LP503759JU ghi kích thước **62 × 38,1 × 6 mm**, khác nhẹ so với bản vẽ datasheet 62 × 38 × 5,4 mm khi giao / dày 5,8 mm sau chu kỳ sử dụng. Khi dựng bố trí sơ bộ nên dành ít nhất khoảng bao **62 × 38,1 × 6 mm** cho pin, rồi bổ sung dung sai và khoảng hở lắp ráp; chưa coi kích thước nhỏ hơn trong tên cell là kích thước pack.
- Trang [DigiKey LP103450JH](https://www.digikey.com/en/products/detail/jauch-quartz/LP103450JH-PCM-PTC-2-WIRES-70MM/9560994) thông báo sản phẩm chứa lithium và **không có vận chuyển hàng không tại thời điểm kiểm tra**. Kênh nhập của Xpart cần được xác nhận riêng; không suy ra có thể giao lithium về Việt Nam chỉ từ việc có nút mua hoặc giá VND.

## Các mẫu giá rẻ và các mẫu chưa đạt điều kiện

| Mẫu / nguồn | Giá quan sát | Dữ liệu hiện có | Lý do chưa chọn |
|---|---:|---|---|
| **LiPo 523450 1.000 mAh** tại [Linh Kiện Thành Công](https://linhkienthanhcong.com/collections/all?page=29&sortby=price-desc) | **50.000đ** | Listing mô tả có mạch bảo vệ, kích thước khoảng **50 × 34 × 5,2 mm** | Chưa có datasheet hãng/bộ pin và dòng xả liên tục ≥1,5 A; tên 523450 chỉ mô tả kích thước, không xác định duy nhất cell/PCM |
| **LiPo 102040 800 mAh** tại [Thế Giới IC](https://www.thegioiic.com/pin-sac-lipo-102040-3-7v-800mah) | **33.060đ**, hết hàng khi đối chiếu | Người bán ghi bảo vệ và mức xả 2C; kích thước khoảng **40 × 20 × 10 mm** | Chưa có datasheet hãng cho đúng bộ pin, chưa xác nhận 2C là liên tục; tình trạng hết hàng |
| **LiPo đủ loại tại LKCG** | Tùy biến thể | [Trang sản phẩm](https://lkcg.vn/pin-li-po-lithium-3-7v-du-loai) ghi có PCM | Tiêu đề “DATASHEET” không có tài liệu tải kèm; không có định mức dòng liên tục để đối chiếu |
| **LiBa 1.200 mAh** | Trang hiện giá 1đ, cần báo giá | [Thông số người bán](https://www.liba.com.vn/pin-lithium-polyme-lipo-37v-1200mah-103040-liba3.html?lang=EN) ghi xả liên tục **0,5C = 0,6 A**, tối đa 1C; tùy chọn PCM | Không đạt 1,5 A liên tục; giá 1đ không phải báo giá thực tế |
| **DFRobot FIT0120 1.000 mAh** | Đã ngừng bán | [Hãng](https://www.dfrobot.com/index.php?product_id=437&route=product%2Fproduct) công bố bảo vệ và **1C liên tục = 1 A** | Không đạt dòng, sản phẩm retired |
| **RS PRO 144-9405 1.800 mAh** | Chưa xác minh giá Việt Nam | [Datasheet hãng](https://docs.rs-online.com/42f4/A700000008874152.pdf) có PCM, nhưng dòng liên tục **0,85 A** ở 0-60°C | Không đạt dòng; không dùng ngưỡng PCM 5-9 A để thay thế |

### Jauch LP603048JK: không dùng làm lựa chọn ưu tiên

[Xpart LP603048JK + PCM + 2 WIRES 70MM](https://xpart.vn/vi/product/jauch-quartz/lp603048jk-pcm-2-wires-70mm-15582565/p/15626646) niêm yết **251.000đ**. Tài liệu đối chiếu của lượt nghiên cứu ghi khoảng 820 mAh tối thiểu / 850 mAh điển hình, mức xả tối đa 2C, tương đương **1,64 A nếu dùng dung lượng tối thiểu**. Tuy nhiên, ngưỡng phát hiện quá dòng PCM **1,5-3 A** có thể chạm mức yêu cầu 1,5 A ở đầu dung sai. Không coi đây là ứng viên đã đáp ứng ổn định tiêu chí 1,5 A; cần dữ liệu phối hợp cell/PCM và đúng biến thể trước khi xét tiếp.

## Các lựa chọn kỹ thuật có datasheet nhưng chưa có nguồn mua Việt Nam xác nhận

- **LiPol LPHD7425048 1.000 mAh**: [bản datasheet hãng được lưu trên TI E2E](https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/196/LPHD7425048-1000mAh.pdf) mô tả bộ pin 1S1P có PCM và NTC, dòng xả chuẩn 6 A/tối đa 10 A, kích thước khoảng **49 × 25 × 7,4 mm**. Đây là ứng viên kỹ thuật đáng nghiên cứu về bố trí, nhưng chưa tìm được listing Việt Nam, giá hoặc khả năng giao hàng được xác nhận. Không dùng nó làm lời khẳng định có thể mua ngay.
- **RS PRO 125-1266 / SR674361P 2.000 mAh**: [datasheet hãng](https://docs.rs-online.com/83b7/A700000008874172.pdf) ghi **2 A liên tục**, có PCM với HY211-GB và SM8205A, bộ pin **63 × 43,5 × 7 mm**. [RS UK](https://uk.rs-online.com/web/p/speciality-size-rechargeable-batteries/1251266) có hàng và niêm yết £18,60 chưa VAT tại thời điểm nghiên cứu; chưa có giá giao Việt Nam và sản phẩm không xuất hiện đúng mã trong tìm kiếm RS Vietnam. Kích thước quá lớn cho định hướng NeoCam hiện tại.
- **Kênh EEMB tại Việt Nam**: [linecard EPI Vietnam](https://epi-tech.com.vn/wp-content/uploads/2023/08/EPI-Linecard-2023-version.pdf) liệt kê EEMB trong nhóm battery. Đây là đầu mối có cơ sở để yêu cầu một bộ pin theo thông số; chưa có báo giá, tồn kho hoặc mã cell/PCM đã được xác nhận cho NeoCam.
- Không mặc định một **EEMB LP103454** bán trên marketplace đáp ứng chỉ vì trang danh mục cell ghi tối đa 2C. Các tài liệu/biến thể của pack có thông số khác nhau; cần đúng tài liệu bộ pin. [Bản vẽ pack EEMB được cung cấp trên Amazon](https://m.media-amazon.com/images/I/813y0IfZwAL.pdf) là ví dụ phải đối chiếu cả giới hạn tải của PCM, không chỉ dung lượng cell.

## Đối chiếu mục tiêu 45 phút

Để so sánh mua hàng, dùng **ngân sách thiết kế sơ bộ 1 A tại pin**, không phải dòng NeoCam đã được chứng minh. Mức tìm mua 1,5 A là khả năng cấp dòng yêu cầu, không phải dòng luôn bị rút.

    Q_dùng_được ≥ I_pin_trung_bình × thời_gian
                ≥ 1 A × 0,75 h
                ≥ 750 mAh

- LP503759JU cần dùng được ít nhất **750/1300 = 57,7%** dung lượng tối thiểu.
- LP103450JH cần dùng được ít nhất **750/1900 = 39,5%** dung lượng tối thiểu.
- Pin generic 1.000 mAh cần ít nhất **75%** dung lượng danh định; pin 800 mAh cần **93,75%**. Những tỷ lệ đó không xác nhận pin đạt 45 phút.

Dung lượng Jauch nêu trên được xác định đến **3,0 V** với dòng kiểm tra 0,2C. Nếu nguồn XIAO/camera cần dừng ở điện áp tải cao hơn, ví dụ mốc thiết kế sơ bộ khoảng 3,6 V, lượng điện dùng được trước thời điểm dừng nhỏ hơn dung lượng danh định. Chưa có đủ đường cong xả cho đúng bộ pin/điều kiện để suy ra thời lượng NeoCam. Không báo 78 hoặc 114 phút như thời lượng thực tế bằng cách chia dung lượng tối thiểu cho 1 A.

## Kết luận lựa chọn hiện tại

1. **LP503759JU 299.000đ** là ứng viên có liên kết datasheet từ listing rõ nhất trong các mẫu tìm được. Cần xác nhận đúng revision 2C, định mức liên tục, khả năng nhập lithium và tổng chi phí giao trước khi chốt.
2. **LP103450JH 432.000đ** có nhiều dự phòng dung lượng hơn, nhưng cần khôi phục đúng datasheet PTC hoặc xác nhận biến thể hiện hành; không trộn thông số PTC và Second IC.
3. Hai mẫu Jauch rộng ít nhất **34,5-38 mm**, đã vượt chiều rộng vỏ tham khảo khoảng **25,6 mm** của mục tiêu giống phần camera GO 3, trước khi tính bo và vỏ. Pin túi không đồng nghĩa tự động vừa hình dáng mục tiêu.
4. Hướng ưu tiên chi phí thấp còn mở là lấy **đúng model pouch nhỏ có hồ sơ từ nhà cung cấp trong nước**, hoặc xin báo giá bộ pin từ EPI/EEMB. Pin generic 33.000-50.000đ hiện mới là đầu mối mua hàng, chưa là linh kiện được chứng minh đáp ứng.

Thông tin cần có để đóng vòng chọn mua: mã đầy đủ + revision bộ pin; PDF cell và bộ pin/PCM; xác nhận xả liên tục ≥1,5 A; ngưỡng và trễ bảo vệ; điện trở đường dòng; dung lượng tối thiểu và đường cong xả; kích thước tối đa gồm PCM/vỏ bọc và phần tăng chiều dày sau sử dụng; dây/đầu nối và cực tính; tồn kho, giá một/năm viên, phí nhập và thời gian giao lithium. Chưa gửi yêu cầu báo giá hoặc liên hệ người bán trong lượt nghiên cứu này.
