# Học Huỳnh Văn Kiểm để thiết kế mặt sạc không dây DOCK-F01

Ngày lập: **31-10-2026**. Tài liệu này chuyển kiến thức điện tử công suất thành các bài toán cho **một mặt sạc dùng chung**, cấp điện luân phiên cho đồng hồ Neo hoặc điện thoại tương thích. Người dùng đã chọn mục tiêu **tối đa 25 W sạc không dây**, còn điểm đo, chuẩn/profile, điều kiện đạt mức này, kích thước và linh kiện phải được xác định và kiểm chứng. [Đặc điểm sản phẩm](../product-features.md).

**Ranh giới của các bài số:** chưa có số đo điện thoại, đồng hồ hay mạch Neo. Những giá trị 25 W tại DC đầu nhận, 80%, 12 V, 150 kHz, 10 µH, 3,0 V và 10 Wh bên dưới là **giả thiết học tập**. Đặc biệt, 150 kHz và mạch RLC minh họa không xác lập tần số hay sơ đồ Qi2 25W. Không dùng chúng để đặt hàng BOM hoặc khẳng định đã đạt chuẩn.

## 1. Bài toán thật trông như thế nào?

```text
Adapter ──┐
          ├── Quản lý nguồn/pin ── DC cho bộ phát ── Cầu khóa ── L/C + cuộn TX
Pin dock ─┘                                                    )))
                                                          từ trường biến thiên
                                                               (((
              Điện thoại tương thích  HOẶC  chân đồng hồ: cuộn RX
                                  ↓
                        Mạch nhận/chỉnh lưu
                                  ↓
                     Điện DC cho thiết bị nhận
```

Dock phục vụ **một thiết bị nhận tại một thời điểm**. Pin trong dock là nguồn dự phòng; pin riêng trong đồng hồ chưa được chốt. Để tương thích điện thoại còn cần giao tiếp phát–nhận, phát hiện vật lạ, điều khiển và kiểm thử. Đây là sơ đồ chức năng; số khóa và topology thực phải theo phương án được chọn.

Điểm đo phải được nêu trước mỗi phép tính:

- `P_TX,in`: DC tại đầu vào **khối phát** không dây.
- `P_RX,out`: DC tại đầu ra **mạch nhận** trong bài thử, trước các khối tải sau nó.
- `P_adapter`: DC tại đầu vào dock; có thể phải cấp thêm cho tải phụ và nạp pin dock.

Điện thoại thương mại không nhất thiết cho phép đo trực tiếp `P_RX,out`; thông số sạc công bố và công suất thật vào pin điện thoại không tự đồng nghĩa với điểm đo này. Mức 25 W được dùng tại `P_RX,out` bên dưới chỉ để tạo một bài toán rõ ràng.

## 2. Đọc phần nào trong sách, để giải bài nào?

Ổ D: đã được kiểm tra lại ngày 31-10-2026: sáu PDF còn đọc được và SHA-256 khớp [bản kiểm kê](huynh-van-kiem/source-inventory.json). Số trang dưới đây là **trang PDF tính từ 1**.

| Đọc trong Huỳnh Văn Kiểm | Kiến thức cần rút ra | Bài toán cho DOCK-F01 |
|---|---|---|
| [Chương 1](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/Chuong1_Modau.pdf), tr. 2–4 | Dạng sóng, trung bình, RMS, đỉnh, công suất và sóng hài | Bài 1–3: xác định điểm đo; tính điện áp/dòng trên mạch cộng hưởng. |
| [Chương 2](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/chuong2_Linh_kien_CS.pdf), tr. 7–11, 14–16 | Chuyển mạch, gate driver, tổn hao, nhiệt, bảo vệ | Bài 4–5: dòng khóa, tổn hao cuộn, nhiệt và điều kiện đọc datasheet. |
| [Chương 5](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/Chuong5_Bo_bien_doi_ap_mot_chieu.pdf), tr. 1–4, 7, 20–22 | Bộ băm, ripple, tăng áp, PWM, nguồn DC | Bài 1 và 6: nguồn adapter/pin, giới hạn dòng và chuyển đổi điện áp. |
| [Chương 6](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/03_resources/07_Dien-Tu-Cong-Suat-Huynh-Van-Kiem/Chuong6_Nghich_luu_doc_lap_va_bien_tan.pdf), tr. 7–8, 11–12, 27 | Cầu/nửa cầu, thành phần cơ bản, sóng hài, nguồn cao tần cộng hưởng | Bài 2–3 và mô phỏng: chuyển DC thành điện áp xoay chiều để kích cuộn phát. |

Các mục trên là điểm vào gần DOCK-F01; lộ trình [đọc đủ sáu chương](huynh-van-kiem/README.md) vẫn được giữ. **Chương 3** bổ sung cách phân tích khoảng dẫn và RMS của sóng AC; **Chương 4** bổ sung chỉnh lưu và mô hình tải nạp, đặc biệt mục IV.1 tr. 1–6. Đây là kiến thức nền để đọc đường dòng và mạch nhận, không phải chỉ dẫn sao chép mạch điều áp lưới hoặc SCR vào dock.

Hai chú ý khi đọc nguồn: Chương 1 tr. 3 in `T = ω/(2π)`; quan hệ đúng là **`T = 2π/ω = 1/f`**. Chương 5 tr. 3 dùng `ΔI = (Imax − Imin)/2`; với ký hiệu đỉnh–đỉnh thường gặp, `ΔIpp = Imax − Imin = 2ΔI_sách`.

[Bài cầu diode 220 V nạp ắc quy đã học](D:/workspace/NeoLearning/07_mechatronics_engineering_master_program/02_Industrial-Electronics-For-Mechatronics-System/02_exercises/01_solution_note.md) giúp luyện cùng phương pháp **dạng sóng → trung bình/RMS/đỉnh → công suất và nhiệt**. Bài đó có mô hình cầu diode, điện lưới và tải khác; không đưa nguyên sơ đồ hay giá trị 220 V, 40 A sang dock.

> Các bản đồ đọc cũ được lập trước concept mặt sạc: mã feature trong đó có thể khác mã hiện tại, và nhận xét cũ “dock chưa cần tạo AC” không còn đúng cho khối phát không dây. Bộ phát cần điện áp/dòng biến thiên cao tần; dock vẫn nhận nguồn DC ở ranh giới đầu vào.

## 3. Bài 1 — Từ công suất nhận đến nguồn cấp

**Đề.** Giả sử tại đầu ra DC của mạch nhận có `P_RX,out = 25 W`, và hiệu suất từ DC vào TX đến DC ra RX là `η_TX–RX = 0,80`. Giả sử dock còn có tải phụ ngoài TX `1 W` và nạp pin dock với `5 W` **tại pin**, mạch nạp hiệu suất `0,90`. Tính công suất cần tại bus DC đang cấp các khối này.

Theo định nghĩa hiệu suất `η = P_ra/P_vào`:

```text
P_TX,in = 25 / 0,80 = 31,25 W
P_vào_mạch_nạp = 5 / 0,90 ≈ 5,56 W
P_bus = 31,25 + 5,56 + 1 ≈ 37,81 W
```

`37,81 W` là tại **bus giả định**, chưa bao gồm hao hụt phía trước bus hoặc dự phòng cho quá độ. Nạp pin dock đồng thời với một thiết bị bên ngoài vẫn phù hợp giới hạn một thiết bị nhận. Đây không phải công bố adapter cần mua; cần chọn hợp đồng nguồn, hiệu suất và điều kiện thực sau.

**Tự làm:** nếu không nạp pin dock thì `P_bus` bằng bao nhiêu? Đáp số: `32,25 W` theo cùng giả thiết. Tại sao không thể lấy luôn adapter 25 W? Vì chỉ riêng khối phát trong ví dụ đã cần 31,25 W ở đầu vào của nó.

## 4. Bài 2 — Cầu khóa và giá trị RMS

**Đề.** Xem cầu H lý tưởng có điện áp bus DC `VDC = 12 V`. Giả sử điện áp **giữa hai đầu cầu** là hình vuông đối xứng `+12/−12 V`, 50% chu kỳ. Lấy `f = 150 kHz`. Tính điện áp trung bình, RMS toàn sóng và RMS của thành phần cơ bản.

```text
v_cầu(t):  +12 V  ┌──────┐        ┌──────┐
                  │      │        │      │
           −12 V  ┘      └────────┘      └──→ t
                  <------ T = 6,667 µs ----->
```

- `Vavg = 0 V` vì hai nửa chu kỳ có diện tích bằng nhau và trái dấu.
- `Vrms = √[(12² + (−12)²)/2] = 12 V` cho **toàn bộ sóng vuông**.
- Từ hệ số Fourier `b1 = (1/π)∫₀²π v(θ)sinθ dθ = 4VDC/π`, biên độ thành phần cơ bản là `4VDC/π`. Vì nó hình sin: `V1,rms = 4VDC/(π√2) ≈ 10,804 V`.

Ba giá trị này trả lời ba câu hỏi khác nhau. `±12 V` là áp vi sai **sau cầu H lý tưởng**, không kết luận dock có hai nguồn `+12 V` và `−12 V`. Mạch nguồn TX thật có thể dùng topology và tần số khác.

## 5. Bài 3 — Cộng hưởng L–C và điện áp trên tụ

**Đề.** Chỉ để học, dùng mô hình `L = 10 µH` nối tiếp `C` ở `f₀ = 150 kHz`. Hãy tìm `C` để `X_L = X_C` và giả sử công suất cơ bản trong mạch tương đương là `31,25 W`, điện áp cơ bản từ bài 2.

Với `ω = 2πf`, ta có `X_L = ωL` và `X_C = 1/(ωC)`. Cho hai độ lớn bằng nhau:

```text
ωL = 1/(ωC)  →  C = 1/(ω²L) ≈ 112,58 nF
X_L = X_C ≈ 9,425 Ω tại 150 kHz
I₁,rms ≈ P₁/V₁,rms = 31,25/10,804 ≈ 2,893 A
R_tương_đương ≈ V₁,rms/I₁,rms ≈ 3,735 Ω
V_C,1,rms ≈ I₁,rms·X_C ≈ 27,26 V
V_C,1,đỉnh ≈ √2·27,26 ≈ 38,55 V
I₁,đỉnh ≈ √2·2,893 ≈ 4,09 A
```

Các giá trị này là **xấp xỉ theo thành phần cơ bản**, tại cộng hưởng và tải tương đương thuần trở. `R_tương_đương` gộp tổn hao và tải quy đổi trong mô hình; không phải điện trở vật lý trong điện thoại. Tụ có thể thấy áp cao hơn bus 12 V; phải xét cả điện áp đỉnh, điện áp AC, dòng RMS, tổn hao, dung sai và nhiệt theo datasheet và sơ đồ thực.

### Đối chiếu với mô phỏng QSPICE đã chạy

[Netlist RLC học tập](f01-rlc-educational.cir) dùng nguồn xung lý tưởng vi sai ±12 V, `R = 3,735104 Ω`, `L = 10 µH`, `C = 112,579 nF`; phân tích `.tran 0 400u 0 5n`, đo từ `200 µs` đến `400 µs` tương đương khoảng 30 chu kỳ. Mô phỏng thực tế bằng `QSPICE64.exe` hoàn tất và QPOST đọc được [kết quả đo](f01-rlc-educational.measurements.txt); [dạng sóng QRAW](f01-rlc-educational.qraw) đã được tạo.

| Đại lượng | Tính tay, chỉ sóng cơ bản | QSPICE, toàn dạng sóng |
|---|---:|---:|
| `I(Lcoil)` RMS | 2,893 A | 2,89636 A |
| `I(Lcoil)` đỉnh dương | 4,091 A | 4,08472 A |
| `V(n2)` — áp tụ RMS | 27,26 V | 27,2633 V |
| `V(n2)` — áp tụ đỉnh dương | 38,55 V | 38,7965 V |

Khác biệt là hợp lý vì nguồn mô phỏng là sóng vuông có sườn hữu hạn và còn các hài; tính tay chỉ lấy sóng cơ bản. Từ dòng RMS mô phỏng, công suất trung bình trên `Rtotal` ước tính `I²R ≈ 31,33 W`; đây là **công suất trên điện trở mô hình**, không phải số đo điện thoại hay pin.

Để tự chạy lại trong PowerShell trên máy hiện tại:

```powershell
Set-Location 'E:\workspace\NeoLab\concepts\modular-desk-clock\dc-power-dock\learning'
& 'C:\Program Files\QSPICE\QSPICE64.exe' '.\f01-rlc-educational.cir'
& 'C:\Program Files\QSPICE\QPOST.exe' '.\f01-rlc-educational.cir' -r '.\f01-rlc-educational.qraw' -o '.\f01-rlc-educational.measurements.txt'
```

Mở QRAW bằng QSPICE và xem `V(src)`, `I(Lcoil)`, `V(n2)`. Mô hình này **không có** bốn MOSFET thật, cuộn RX, chỉnh lưu, điều khiển Qi, FOD, pin hay điện thoại; nó không chứng minh hiệu suất 80%, tương thích hoặc khả năng sạc 25 W.

## 6. Bài 4 — Từ dòng cuộn đến MOSFET và nhiệt

Giữ các giả thiết của bài 3. Giả sử điện trở AC của cuộn phát `R_AC = 0,20 Ω` tại điều kiện xét. Với dòng cơ bản `I₁,rms ≈ 2,893 A`:

```text
P_cuộn ≈ I₁,rms²·R_AC ≈ 1,67 W
```

Nếu giả sử hệ số nhiệt hữu hiệu từ cuộn tới môi trường `Rθ = 18 °C/W` và môi trường `25 °C`, mô hình một điện trở nhiệt cho `ΔT ≈ 1,67·18 = 30,1 °C`, nhiệt độ nút cuộn xấp xỉ `55,1 °C`. `Rθ` ở đây chỉ là giả thiết; nhiệt độ thật còn phụ thuộc vỏ, tản nhiệt, cuộn RX và các nguồn nhiệt khác.

Với cầu H lý tưởng, dòng hình sin cùng pha điện áp và mỗi MOSFET dẫn đúng nửa chu kỳ, bỏ qua deadtime và diode thân: `I_FET,rms ≈ I₁,rms/√2 ≈ 2,045 A`; đỉnh khoảng `4,09 A`. Nếu **giả sử** `RDS(on) = 20 mΩ` ở đúng `VGS` và nhiệt độ vận hành, tổn hao dẫn mỗi khóa khoảng `I_FET,rms²·RDS(on) ≈ 0,084 W`. Chưa tính tổn hao chuyển mạch, driver, deadtime, phục hồi diode và hài.

**Ý nghĩa chọn linh kiện:** dòng cuộn AC, dòng RMS từng MOSFET và dòng pin DC là ba đại lượng khác nhau. Không dùng một thông số `RDS(on)` nếu datasheet công bố nó ở `VGS` hoặc nhiệt độ khác điều kiện thực.

## 7. Bài 5 — Nguồn pin và thời gian mang đi

**Trường hợp giả định 1S, chưa phải pin được chọn:** nếu điện áp pin dưới tải là `3,0 V`, khối từ pin đến đầu vào TX hiệu suất `90%`, và TX cần `31,25 W`, thì dòng trung bình pin là:

```text
I_pin = 31,25 / (3,0·0,90) ≈ 11,57 A
```

Giá trị này chỉ áp dụng tại điều kiện đã giả định và chưa tính tải phụ; nó cho thấy cần xét dòng xả, bảo vệ, sụt áp, nhiệt và năng lượng sử dụng được của pack. Không suy ra kiến trúc 1S có phù hợp trước khi có pack và tải thực.

Nếu giả sử pin có `10 Wh` **hữu dụng ở cả hai chế độ**, hiệu suất pin→TX `90%` và TX→RX `80%`, bỏ qua tải phụ:

| Công suất DC đầu ra RX giả định | Công suất pin | Thời gian lý tưởng |
|---|---:|---:|
| 25 W liên tục | `25/(0,90·0,80) ≈ 34,72 W` | `10/34,72 ≈ 0,288 giờ ≈ 17,3 phút` |
| 2 W liên tục cho đồng hồ cơ bản | `2/(0,90·0,80) ≈ 2,78 W` | `10/2,78 ≈ 3,6 giờ` |

Không có bằng chứng pack thật cung cấp cùng 10 Wh ở hai dòng xả, hoặc duy trì 25 W liên tục. Bài này giúp phân biệt **công suất tối đa của mặt sạc** với mục tiêu **đồng hồ di động vài giờ**.

## 8. Bài 6 — Đặt lệch cuộn dây ảnh hưởng thế nào?

Xét mô hình lý tưởng **hai mạch cộng hưởng nối tiếp**, có `L1 = L2 = 10 µH`, `f = 150 kHz`, điện trở tổng phía nhận `R2 = 4 Ω`; giả sử mạch điều khiển giữ `I1 = 3 A RMS` không đổi. Với hệ số ghép `k`, hệ số hỗ cảm là `M = k√(L1L2)`. Ở cộng hưởng, điện áp cảm ứng do TX có độ lớn `ωMI1`, nên trong mô hình này:

```text
I2 = (ωM/R2)·I1
k = 0,20 → M = 2 µH → I2 ≈ 1,414 A RMS
k = 0,10 → M = 1 µH → I2 ≈ 0,707 A RMS
```

Nếu `R2` gồm điện trở cuộn nhận `0,2 Ω` và tải AC thuần trở `3,8 Ω`, thì `P_tải = I2²·3,8` lần lượt khoảng `7,59 W` và `1,90 W`. Hai mức là kết quả của bài giả định **giữ I1 không đổi**, không phải 25 W Qi2 hay quan hệ chung cho bộ sạc thực tế: bộ điều khiển và tải có thể thay đổi. Khoảng cách, lệch tâm, kích thước và hướng cuộn làm thay đổi ghép từ. [WPC: coupling factor](https://www.wirelesspowerconsortium.com/knowledge-base/magnetic-induction/coupling-factor/)

## 9. Chuyển kết quả thành tiêu chí chọn linh kiện

| Khối | Cần lấy gì từ bài toán và datasheet? | Ví dụ nguồn có thể khảo sát |
|---|---|---|
| **TX trong dock** | Profile đúng cho mục tiêu 25 W; điều kiện đầu vào, giới hạn nhiệt, FOD, xác thực/firmware và khả năng tích hợp. | [MAXIC MT201, WPC Qi ID 24250](https://jpsapi.wirelesspowerconsortium.com/products/qi/24250): subsystem PTx, MPP25, 25 W, **không phải evaluation module**. [Hãng](https://www.maxictech.com/en/about/news/107.html) nêu đường kính 56,53 mm, dày 8,0 mm và nhiều cấu hình nguồn vào; chưa xác minh mua hàng/điều kiện 25 W từng nguồn. |
| **RX trong chân đồng hồ** | Đầu ra DC đủ tải liên tục, khởi động và chế độ thấp; kích thước cuộn và tương thích profile với TX. | [TI BQ51013C-Q1EVM](https://www.ti.com/tool/BQ51013C-Q1EVM): board **nhận** BPP 5 W, 5 V đến 1 A; cuộn 48 × 32 mm. Có thể khảo sát nếu tải đồng hồ phù hợp; chưa thử với MT201 và không phải TX 25 W. |
| **Cuộn và ferrite** | Hình học, cảm kháng, điện trở AC, dòng RMS, nhiệt và khoảng cách theo đúng TX/RX được hỗ trợ. | Nếu đánh giá subsystem hoàn chỉnh như MT201, giữ cuộn/ferrite và điều kiện tích hợp theo hãng; không tự thay cuộn từ mô hình bài 3. |
| **Tụ cộng hưởng** | Điện áp AC/đỉnh, dòng RMS và tần số, ESR, nhiệt và dung sai; điện áp trên tụ có thể vượt bus DC. | Bài 3 chỉ là điều kiện lọc ban đầu; chọn mạng tụ theo sơ đồ tham chiếu và thử nghiệm của TX thực. |
| **MOSFET/driver** | `VDS`, dòng, `RDS(on)` theo `VGS`/nhiệt, `Qg`, tổn hao, deadtime và PCB. | Nếu tự thiết kế mạch rời, đọc ví dụ và điều kiện [ST STL20N6F7](https://www.st.com/resource/en/datasheet/stl20n6f7.pdf) trong [nghiên cứu linh kiện](qi25-component-research.md); chưa chọn nó cho Neo. |
| **Nguồn, pack, bảo vệ** | Điện áp và dòng tại mỗi biên, hồ sơ nguồn/cáp, dòng pin thấp áp, FOD, nhiệt và chuyển nguồn. | Đánh giá theo tải đo được, pack và sơ đồ khối thực; chưa chọn pack hay IC sạc chỉ dựa vào 25 W. |

Board TI 5 W chỉ phù hợp phía **đồng hồ** nếu tải và profile được chứng minh phù hợp; điện thoại nhận điện qua bộ nhận riêng của điện thoại. Hồ sơ WPC của MT201 liệt kê MPP25; khả năng làm việc với board nhận BPP của TI cần được nhà cung cấp xác nhận và thử thực tế. [Hồ sơ nghiên cứu nguồn linh kiện](qi25-component-research.md) phân biệt thông số hãng, chứng nhận subsystem và những điều chưa được xác nhận. Một subsystem đã chứng nhận không tự chứng nhận toàn bộ dock Neo; tích hợp vỏ/cuộn phải tuân theo điều kiện của WPC và nhà cung cấp. [WPC](https://www.wirelesspowerconsortium.com/knowledge-base/testing-and-certification/qi-components-and-subsystems/)

## 10. Bài làm tiếp theo để biến mô hình thành thiết kế

1. Vẽ lại đường năng lượng và đánh dấu **TX input DC, RX output DC, adapter, pin**; điền dấu hỏi thay cho mọi giá trị chưa đo.
2. Chọn một cụm đồng hồ cơ bản để đo điện áp, dòng trung bình và dòng đỉnh ở các chế độ khởi động, hiển thị, sáng/tối; thêm module từng cái. Nếu chưa có cụm thật, dùng tải giả để học và ghi rõ đó không phải số đo đồng hồ.
3. Chọn mẫu iPhone/Samsung đại diện và định nghĩa phép thử ở các mức công suất, vị trí, ốp, adapter và pin dock. Không đồng nhất thông số tối đa của máy với kết quả Neo.
4. Xin/đọc đầy đủ tài liệu TX, RX, profile, coil, FOD, firmware, source contract, tích hợp và khả năng mua; xác nhận cặp TX/RX thử thật trước khi lập BOM.
5. Chạy các bài mô phỏng từ đơn giản đến mô hình ghép cuộn; ghi topology, giả thiết, trace và sai lệch. Mô phỏng RLC trên là bước học đầu, không là kiểm thử Qi.
6. Sau khi có phần cứng, đo đồng thời công suất vào TX và ra RX trên fixture phù hợp, hiệu suất, nhiệt, sai lệch vị trí, tải thấp/đỉnh, tháo lắp và chuyển nguồn; ghi thiết bị, điều kiện và kết quả.

Tài liệu là cơ sở học và lập thiết kế. Ngoài mô phỏng RLC lý tưởng và phép tính ghi rõ ở trên, chưa có phép đo phần cứng hay kết quả chứng nhận cho dock Neo.
