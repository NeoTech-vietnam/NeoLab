# NeoCam: thẩm định lựa chọn pin, PCM và đường dòng

Ngày đối chiếu: **2026-11-06**. Hồ sơ thiết kế tham chiếu cho bo nguồn phụ trợ giữ nguyên XIAO; chưa có thử nghiệm pin/NeoCam, chưa đặt mua hay liên hệ nhà cung cấp.

## 1. Quyết định kỹ thuật đề xuất

- Dùng **bộ pin DNK102050 3,7 V 1000 mAh có PCM** làm **mẫu tham chiếu điện** cho cấu hình có giới hạn thiết kế đường pin **1 A liên tục**. Đây không phải xác nhận mua được ở Việt Nam hoặc đã đạt 45 phút. Nếu yêu cầu sản phẩm vẫn bắt buộc 1,5 A liên tục, không dùng mẫu này làm linh kiện đã đạt.
- Giữ **LiPo 102040 800 mAh generic** làm ứng viên giảm giá/kích thước; chưa đưa vào BOM có định mức được xác nhận khi chưa có hãng/model và datasheet đúng pack.
- Dùng connector có khóa cơ **JST PH 2 chân**, dây AWG24; quy ước bản vẽ **chân 1 = P+, chân 2 = P-**. Đây là quy ước NeoCam, không phải quy tắc cực tính áp dụng cho mọi dây pin mua sẵn.
- Có thể đặt cầu chì thứ cấp **Littelfuse 046701.5NR** trong đường P+ gần đầu nối. Cầu chì không thay PCM, không tạo giới hạn dòng 1,5 A tức thời và không chứng minh bảo vệ cell 1 A trong mọi dạng lỗi.

## 2. Dữ liệu đúng pack tham chiếu

[Datasheet DNK102050 của DNK POWER](https://www.dnkpower.com/wp-content/uploads/2023/11/DNK102050-3.7V-1000mAh-Lipo-Battery-Specification.pdf) ghi:

| Đại lượng | Dữ liệu hãng |
|---|---|
| Điện áp/dung lượng danh định | 3,7 V / 1000 mAh |
| Dòng xả liên tục tối đa | 1C = **1 A** |
| Xung | 3C = **3 A trong 10 ms** |
| Dòng sạc khuyến nghị / tối đa | 0,2C = 200 mA / 0,5C = 500 mA |
| Điện áp sạc | 4,2 V; tài liệu này không cho đầy đủ dung sai sạc cho phép |
| Bảo vệ quá sạc / quá xả | 4,275 +/- 0,050 V / 2,50 +/- 0,050 V |
| Bảo vệ quá dòng | 3-5 A; có bảo vệ ngắn mạch; thiếu độ trễ chi tiết |
| Nhiệt độ sạc / xả | 0-45 C / -20 đến 60 C |
| Kích thước pack / khối lượng | 52 x 20 x 10 mm / khoảng 16,7 g |
| Trở kháng trong | <60 milliohm, chưa đủ điều kiện đo để coi là ESR DC toàn dải nhiệt/SOC |

Không suy ra ngưỡng PCM 3-5 A là cell được phép xả 3-5 A liên tục. Datasheet không nêu model IC/MOSFET trong PCM; không tự gán DW01A/FS8205A vào pack này. Dùng pack có sẵn giúp tránh phải tự phối hợp cell với một PCM chưa được nhà sản xuất pack xác nhận.

## 3. Bao thiết kế dòng: điều kiện kiểm tra, không phải dự báo dòng quay

Mỗi mức phân bổ sau phải được ghi là **lựa chọn thiết kế**, không phải maximum tải hãng công bố:

- `I_3V3_alloc`: mức tổng phân bổ cho nhánh 3,3 V, phải gồm MCU, bộ nhớ, mic, SD và LED. Nếu lấy 0,600 A theo mức đường nguồn tham chiếu thì vẫn cần chứng minh bo XIAO đáp ứng ở điện áp/nhiệt độ làm việc.
- `I_cam_alloc`: dòng vào hai LDO camera. Dòng tham chiếu OV3660 là khoảng 0,098 A ở điều kiện datasheet; chọn 0,150 A làm ngân sách có dự phòng là **quyết định thiết kế**, chưa phải maximum cảm biến.
- `eta_min_design`: tiêu chí hiệu suất thấp nhất cần xác nhận ở đúng tải/điện áp, không tự gán đường cong typical thành giới hạn bảo đảm. Hồ sơ tổng dùng **eta >= 0,85** làm tiêu chí thiết kế, không phải bảo đảm từ datasheet.

**Sửa phép tính ngày 2026-11-06:** không thay VIN bằng điện áp pack trong mẫu số. Q1 bo chính không phải MOSFET trở nhỏ vài chục milliohm. [LP0404N3T5G, LRC](https://www.lrc.cn/Upload/PDF/Product/MOS/LP0404N3T5G.pdf) có RDS(on) max **0,67 ohm tại VGS=-2,5 V, ID=-660 mA, 25 C**. Xem [thẩm định bo chính](mainboard-source-audit-2026-11-06.md). Giá trị này là điểm tham chiếu có điều kiện, không phải trở cực đại được bảo đảm toàn nhiệt độ/dòng của NeoCam.

Gọi `Vpack` là điện áp có tải đo giữa **P+ và P- của pack**, đã bao gồm ảnh hưởng nội bộ cell/PCM; không trừ thêm trở PCM lần nữa. Phân bổ trở ngoài pack từ connector/fuse/dây/PCB tới BAT XIAO là `Rexternal <= 0,100 ohm`; Q1 trên XIAO được cộng riêng. Với mô hình trở và khi buck còn ổn áp:

\[
I_{bat}=\frac{3,3I_{3V3,alloc}}{\eta V_{IN}}+I_{cam,alloc}+I_{aux},\qquad
V_{IN}=V_{pack}-I_{bat}(R_{external}+R_{Q1}).
\]

Đây là mô hình gộp bảo thủ, cho cả dòng phụ đi qua đường trở chung. Nếu nhánh phụ lấy trực tiếp từ pack trước Q1 thì cần tách dòng trên từng đoạn khi tính chi tiết. Chọn **các phân bổ thiết kế** `I_3V3_alloc = 0,600 A`, `I_cam_alloc = 0,150 A`, `I_aux = 0,002 A`, `eta = 0,85`, `Rexternal = 0,100 ohm` và điểm tham chiếu `RQ1 = 0,670 ohm`:

\[
a=0,150+0,002=0,152\ A,\quad k=\frac{3,3(0,600)}{0,85}=2,3294118\ W,\quad R=0,770\ ohm
\]

\[
I=a+\frac{k}{V_{pack}-RI}
\quad\Longrightarrow\quad
RI^2-(V_{pack}+Ra)I+V_{pack}a+k=0.
\]

Nghiệm dòng thấp là nghiệm được dùng trong mô hình, sau đó **phải kiểm tra lại VIN còn đủ để buck ổn áp**. Tại Vpack=4,2 V, nghiệm này cho:

\[
I=0,802264\ A,\quad V_{IN}=4,2-0,770(0,802264)=3,582257\ V.
\]

Riêng tổn hao Q1 theo điểm trở minh họa là `I^2 x 0,670 = 0,431 W`, cần đối chiếu khả năng tản nhiệt/package và điều kiện RDS(on); không biến phép ngoại suy này thành bảo đảm của linh kiện ở dòng0,802A.

Nếu đặt **VIN yêu cầu >=3,5 V** làm tiêu chí thiết kế có dự phòng (chưa phải ngưỡng duy nhất hãng bảo đảm), tính ngược:

\[
I_{at\ VIN=3,5}=0,152+2,3294118/3,5=0,817546\ A
\]

\[
V_{pack,required}=3,5+0,770(0,817546)=4,129511\ V.
\]

Ở bộ phân bổ tải nặng này, dải pack sử dụng được theo tiêu chí VIN bị thu hẹp mạnh ngay cả khi pack cấp được1A. Vì vậy không được kết luận pin đủ chỉ vì rating1A hoặc chỉ xét dung lượng1000mAh. Bảng cũ dùng Vbat trực tiếp và eta75% đã được **loại bỏ** vì bỏ qua sụt ápQ1. Không dùng nghiệm đại số ở pack thấp để khẳng định còn tạo đủ3,3V: khi VIN không đáp ứng, giả định buck ổn áp và công suất đầu ra cố định đã không còn hợp lệ.

Phép tính vẫn không chứng minh dòng NeoCam luôn dưới1A: dòng xung SD, inrush, tải MCU và camera maximum còn thiếu. Nếu cần vượt1A kéo dài thì đổi pack/đường dòng; không dựa vào xung10ms. Giảm tải, sửa đường Q1 hoặc thay kiến trúc nguồn là các hướng phải xem xét nếu muốn dùng nhiều dung lượng pin hơn ở tải lớn; không coi bộ pin lớn hơn là tự khắc giải quyết sụt áp.

## 4. 45 phút: tính ngược tiêu chí dùng được

Cho điện lượng đơn vị mAh, dòng mA, thời gian giờ:

\[
Q_{usable}\ge Q_{boot}+0,75I_{rec,avg}+Q_{close,worst}+Q_{reserve}
\]

Không thay điện lượng ghi nhãn cho `Qusable` trước ngưỡng hệ thống phải dừng. Không nhân tùy ý 75% hoặc 80% rồi gọi là đặc tính pin.

Để thấy độ nhạy, nếu **tự phân bổ** tổng chuẩn bị + kết thúc + dự phòng là 50 mAh, yêu cầu trở thành:

| Dòng quay trung bình giả định dùng để lập tiêu chí | Điện lượng quay 45 phút | Tổng Qusable cần có |
|---:|---:|---:|
| 300 mA | 225 mAh | 275 mAh |
| 500 mA | 375 mAh | 425 mAh |
| 800 mA | 600 mAh | 650 mAh |
| 1000 mA | 750 mAh | 800 mAh |

50 mAh chỉ là một khoản phân bổ có thể thay, chưa phải năng lượng kết thúc tệp đã chứng minh. Không có dòng tác vụ và đường cong xả tới ngưỡng vận hành nên **chưa xác nhận cả DNK1000mAh lẫn generic800mAh đạt 45 phút**. Với generic800mAh, trường hợp 1 A còn cần sử dụng trọn 800mAh trước ngưỡng dừng nếu giữ khoản 50mAh; không được kết luận luôn thiếu hay luôn đủ.

## 5. Phối hợp sạc và PCM

Tham chiếu mainboard V1.51: SGM40567-4.2, F nối GND, RIREF = 220 kilohm. [SGM40567, trang 3-4](https://www.sg-micro.com/rect/assets/e95e555a-a8a2-4761-b86a-6554dff2d8ed/SGM40567.pdf):

\[
I_{chg,nom}=24000/220=109,1\ mA
\]

Khoảng 109 mA danh định nhỏ hơn giới hạn sạc 500 mA của DNK. Chưa suy ra thời gian sạc chính xác; riêng phép chia danh định `1000/109,1 = 9,17 giờ` không bao gồm giai đoạn khác và tải hệ thống.

Bảng IC ở điều kiện VIN5V, F=GND, IBAT6mA, TJ25C cho điện áp sạc tối đa 4,234 V. **Đây không phải trần điện áp toàn thuật toán:** SGM40567 còn có bù điện trở khoảng2% (bảng1,2-2,7%) trong điều kiện thuật toán tương ứng. Không lấy4,234V làm upperbound toàn thời gian/nhiệt độ của điện áp cell. PCM DNK có ngưỡng quá sạc thấp nhất4,225V; so sánh riêng hai điểm bảng:

\[
V_{OC,min}-V_{chg,max}=4,225-4,234=-0,009V
\]

Hai điểm có vùng chồng9mV, nhưng **9mV không mô tả đầy đủ biên sạc** khi có bù điện trở. Đây là lý do **chưa ký duyệt tương thích sạc mọi điều kiện**, không phải bằng chứng chắc chắn ngắt nhầm. Cần điện áp tại cell xuyên suốt thuật toán, dung sai bộ sạc toàn điều kiện và độ trễ PCM; datasheet pack không đủ các thông tin đó. PCM cắt là bảo vệ lỗi, không nên dùng như cơ chế kết thúc sạc thường xuyên.

Nhiệt độ bên trong IC sạc khác nhiệt độ cell. Pack tham chiếu chỉ có hai dây, không chứng minh có NTC; không ghi rằng NeoCam đã tự ngắt sạc theo nhiệt độ cell. Với bản v0 giữ nguyên XIAO, sạc cần ở môi trường đã xác định; tính năng sản phẩm tự bảo vệ nhiệt cần thiết kế thêm hoặc dùng pack/charger có đường TS thích hợp.

## 6. Cầu chì, trở kháng và connector

[Littelfuse 467, Rev2023](https://www.littelfuse.com/assetdocs/fuse-467-datasheet?assetguid=4a59f034-1cca-460e-a5ba-e1e66247c76d): chọn tham chiếu **046701.5NR**, 1,5 A, 32 V, 0603; trở nguội danh định 0,0385 ohm, I2t nóng chảy danh định 0,0766 A2s. Dòng liên tục cần giảm tải tiêu chuẩn 25% và giảm thêm theo nhiệt độ.

Ở 25 C, `0,75 x 1,5 = 1,125 A`; tại ví dụ 70 C của hãng, thêm hệ số 0,8 cho `0,9 A`. Vì vậy không bảo đảm tải 1 A liên tục ở mọi nhiệt độ chỉ từ chữ 1,5 A. Tại 0,8 A và trở nguội danh định:

\[
\Delta V_F=0,8\times0,0385=30,8\ mV;\quad P_F=0,8^2\times0,0385=24,64\ mW
\]

Đặc tính mở ở 200% dòng định mức có thể tới 5 s; đây không phải bảo vệ tức thời dòng1,5 A hoặc cell1 A. Phối hợp năng lượng lỗi/dây/PCM vẫn cần kiểm tra. Ưu tiên cầu chì một lần trở thấp hơn PPTC có thể gây sụt áp đáng kể; không chọn PPTC chỉ vì Ihold ghi đủ lớn mà bỏ qua nhiệt độ/Rmax.

Nếu phân bổ **tổng sụt áp đường ngoài pack <=100 mV tại1 A**, thì `R_external <=0,100 ohm`. Trừ trở cầu chì danh định còn `61,5 milliohm` cho hai tiếp điểm, dây, công tắc và PCB; đây là tiêu chí bố trí, chưa phải trở đo. Cần dùng điện trở nóng/max thực tế khi duyệt, không trừ các giá trị danh định rồi gọi là bảo đảm. Trở trong pack phải tính riêng; giới hạn impedance hãng chưa đủ để suy ra mọi sụt áp DC/động.

[JST PH chính hãng](https://www.jst.com/wp-content/uploads/2025/06/ePH.pdf) quy định dòng2 A với AWG24. Bộ tham chiếu: **B2B-PH-K-S + PHR-2 + SPH-002T-P0.5S**. Khóa cơ giúp cắm đúng hướng nhưng không phát hiện dây pin đã được crimp ngược. Kiểm tra cực tính theo chân1/2 NeoCam trước lắp; không lấy màu dây làm bằng chứng duy nhất.

## 7. Không được nối tắt PCM

Kết nối cả đường sạc và tải qua **P+/P- của pack**. Mọi GND trên companion/XIAO/ADC/USB cùng tham chiếu P-, không nối vào B- của cell nằm phía trong PCM. Với pack dùng công tắc bảo vệ ở nhánh âm, nối dây phụ trực tiếp B- có thể tạo đường vòng vô hiệu hóa công tắc bảo vệ. Không lấy tín hiệu đo pin bằng cách mở pack hoặc nối vào cell trần. Nút tắt nguồn cứng không thay thao tác dừng/đóng tệp; không dùng nó khi đang ghi.

## 8. Trạng thái lựa chọn và tồn đọng

**Đã có lựa chọn tham chiếu:** pack DNK102050 cho đường1 A, cầu chì046701.5NR, hệconnectorJSTPH/AWG24, công thức phối hợp nguồn và tiêu chí điện lượng45phút. **Chưa đóng vòng:** nguồn mua/giá, model102040 thay thế, charge tolerance/PCM timing, nhiệtcell, khả năng pin dùng tới ngưỡng cần thiết, xung và lỗi đường dòng, kích thước vỏ hoàn chỉnh. Không coi pack52mm đã lắp vừa vỏGO3.
