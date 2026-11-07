# NeoCam: thẩm định số học thay Q1 bằng MOSFET trở thấp

Ngày: **2026-11-06**. Phân tích thiết kế, không đo hoặc sửa board thật. Các phép tính chỉ là mô hình trở DC và buck còn ổn áp; không thay mô phỏng chuyển nguồn/diode khởi động/inrush.

**Ghi chú quyết định sau audit:** hồ sơ tổng v0.4 chọn **DMP2035U-7 với Rhot nghiệm thu <=0,100ohm**, không dùng67,5milliohm làm giới hạn nóng được chấp nhận. Những bảng67,5milliohm dưới đây được giữ nguyên là khảo sát độ nhạy lịch sử. Quyết định v0.4 còn thêm R6=10kilohm từ gate/VBUS xuốngGND, giữ R9stock100kilohm; xem mục7 cuối tài liệu.

## 1. Linh kiện được đối chiếu

Nguồn mới: [Diodes DMP2035U, DS31830 Rev11-2, June2025](https://www.diodes.com/datasheet/download/DMP2035U.pdf), trang1-3 và6; [trang sản phẩm hãng](https://www.diodes.com/part/view/DMP2035U).

| Đại lượng | DMP2035U theo tài liệu hiện hành |
|---|---|
| Loại/đặt hàng | P-MOS, **DMP2035U-7**, SOT23 |
| VDS / VGS absolute maximum | -20 V / +/-10 V; không dùng rating +/-8 V của revision cũ |
| RDS(on) max ở -2,5 V | **45 milliohm**, ID=-4 A,25 C, shortpulse để giảm tự nóng |
| RDS(on) max ở -4,5 V / -1,8 V | 35 /62 milliohm, theo điều kiện bảng |
| RthetaJA | 153,5 C/W trên FR4/2oz với pad tối thiểu;100 C/W với vùng đồng25x25mm |
| Dòng diode thân liên tục | 1,2 A ở điều kiện Note7; không tự áp cho mọi adapter nhỏ |
| Chân SOT23 | Gate1, Source2, Drain3; kiểm tra bản vẽ/CAD khi gán footprint |

Nguồn cũ: [LRC LP0404N3T5G RevG](https://www.lrc.cn/Upload/PDF/Product/MOS/LP0404N3T5G.pdf): Rmax0,67ohm tại VGS=-2,5V/ID=-660mA/25C. RthetaJA131C/W phụ thuộc fixture1,5x1,5inchFR4, vùngđồng1sqinch/2oz; không dùng làm khả năng tản nhiệt đã biết của XIAO. Package của linh kiện cũ cần lấy đúng bản vẽ hãng; các danh mục nhận diện **SOT-883-3**, không coi là SOT23 hoặc mặc định SC89.

**Không thay trực tiếp vào footprint cũ.** DMP2035U cần adapter/rework ba net, hoặc một PCB chọn nguồn mới. Khi dùng topology giống mainboard: **Drain -> VBAT, Source -> VIN, Gate -> VBUS** và giữ pulldown gate phù hợp; diode thân có chiều cấp ban đầu VBAT sang VIN. Không đặt MOSFET mới nối tiếp bên ngoài mà vẫn giữ Q1 cũ trong đường tải rồi tính như đã loại bỏ trở0,67ohm.

## 2. Tiêu chí được lựa chọn và giới hạn nóng

Đặt mục tiêu đường Q1 ở **1 A**: sụt áp<=100mV và tổn hao<=100mW. Từ Ohm và Joule:

\[
R_{Q1,hot}\le\min(0,100/1;0,100/1^2)=0,100\ ohm.
\]

DMP2035U có45milliohm max tại điểm25C. Cho phép **hệ số nóng1,5 do dự án chọn** để khảo sát:

\[
R_{Q1,new,design}=0,045(1,5)=0,0675\ ohm.
\]

Đây không phải max nóng hãng bảo đảm. Đồ thị normalizedR theo nhiệt độ là typical, không đủ chứng minh max toàn lô. Khi nghiệm thu phải xác nhận thực tế Rhot<=100milliohm, |VGS| đủ và điều kiện nhiệt; không viết rằng hệ số1,5 đã được nhà sản xuất bảo đảm. Lấy45milliohm ở VGS=-2,5V làm điểm thiết kế bảo thủ so với nguồn3,5V có căn cứ về gate drive nhưng vẫn phải xét transientVGS khi cắmUSB.

## 3. So sánh ở cùng dòng điện

Giữ dòng I bằng nhau để chỉ thấy hiệu quả đổi trở; chưa ghép feedback sụt áp-buck:

| I qua Q1 | Cũ0,670ohm: sụt/nhiệt | Mới25C0,045ohm: sụt/nhiệt | Mới nóng khảo sát0,0675ohm: sụt/nhiệt |
|---:|---:|---:|---:|
| 0,5 A | 335mV /167,5mW | 22,5mV /11,25mW | 33,75mV /16,875mW |
| 0,8 A | 536mV /428,8mW | 36mV /28,8mW | 54mV /43,2mW |
| 1,0 A | 670mV /670mW | 45mV /45mW | 67,5mV /67,5mW |

Tại cùng dòng, mô hình mới nóng giảmsụt/nhiệt89,93% so với điểm trở cũ. Không gọi đó là tiết kiệm89,93% năng lượng toàn NeoCam. Điểm trở cũ được ngoại suy ngoài dòng kiểmtra/25C trong một số hàng, nên không gọi bảng là worstcase được bảo đảm.

## 4. Mô hình ghép và tiêu chí VIN

Các phân bổ của hồ sơ tổng: Icam=0,150A, Iaux=0,002A, eta>=0,85, Rexternal<=0,100ohm, VINyêu cầu>=3,5V. I3V3 là **tổng rail**, không phải dòng thêm ngoài MCU. Đây là tiêu chí thiết kế, không phải dòng tác vụ đã chứng minh.

Vpack đo dưới tải tại P+/P- của protectedpack; không trừ nội trởcell/PCM lần hai. Với R=Rexternal+RQ1, I0=0,152A và k=3,3I3V3/eta:

\[
I=I_0+k/V_{IN},\quad V_{IN}=V_{pack}-RI
\]

\[
V_{IN}^2-(V_{pack}-RI_0)V_{IN}+Rk=0.
\]

Lấy nghiệm VINcao rồi kiểm tra VIN>=3,5V. Dưới điều kiện buck ổn áp không còn đúng, nghiệm đại số không dự báo được dòng/đầu ra thật.

Tính ngược ở VIN3,5V:

| I3V3 phân bổ | Ipack ởVIN3,5V | Vpack cần:cũ0,670ohm | Vpack cần:mới25C0,045ohm | Vpack cần:mới nóng0,0675ohm |
|---:|---:|---:|---:|---:|
| 100mA | 0,262924A | 3,702452V | 3,538124V | 3,544040V |
| 200mA | 0,373849A | 3,787864V | 3,554208V | 3,562620V |
| 300mA | 0,484773A | 3,873275V | 3,570292V | 3,581199V |
| 600mA | 0,817546A | 4,129511V | 3,618544V | 3,636939V |

Vì I ở điểm VINcốđịnh giống nhau, đây là so sánh đường dẫn hợp lệ trong mô hình. Tại600mArail, nhiệtQ1 ở cùng VIN3,5V giảm từ447,816mW xuống45,116mW với trường hợp mới nóng khảo sát.

### Ghép ở cùng Vpack: I cũng thay đổi

Các ô `FAIL` chỉ nghĩa không đạt VIN3,5V đã chọn, không khẳng định reset thực tế đúng tại đó. Nếu nghiệm VIN<3,3V, không dùng như trạng thái buck vẫn tạo3,3V; số chỉ minh họa việc giả định đầu ra cố định đã mất cơ sở.

| I3V3 | Vpack | Cũ: I/VIN/PQ1 theo mô hình | Mới nóng: I/VIN/PQ1 theo mô hình |
|---:|---:|---|---|
| 100mA | 3,6V | 0,266358A /3,394904V /47,534mW;FAIL | 0,261170A /3,556254V /4,604mW;PASS |
| 100mA | 3,7V | 0,263004A /3,497487V /46,345mW;FAIL | 0,258169A /3,656757V /4,499mW;PASS |
| 100mA | 4,0V | 0,254050A /3,804382V /43,243mW;PASS | 0,250086A /3,958111V /4,222mW;PASS |
| 100mA | 4,2V | 0,248856A /4,008381V /41,493mW;PASS | 0,245350A /4,158904V /4,063mW;PASS |
| 200mA | 3,6V | 0,387160A /3,301887V /100,428mW;FAIL | 0,371480A /3,537777V /9,315mW;PASS |
| 200mA | 3,7V | 0,379871A /3,407499V /96,682mW;FAIL | 0,365387A /3,638798V /9,012mW;PASS |
| 200mA | 4,0V | 0,360597A /3,722340V /87,120mW;PASS | 0,348997A /3,941543V /8,221mW;PASS |
| 200mA | 4,2V | 0,349532A /3,930860V /81,856mW;PASS | 0,339411A /4,143149V /7,776mW;PASS |
| 300mA | 3,6V | 0,515634A /3,202962V /178,139mW;FAIL | 0,482967A /3,519103V /15,745mW;PASS |
| 300mA | 3,7V | 0,503642A /3,312196V /169,949mW;FAIL | 0,473683A /3,620658V /15,145mW;PASS |
| 300mA | 4,0V | 0,472297A /3,636331V /149,453mW;PASS | 0,448753A /3,924834V /13,593mW;PASS |
| 300mA | 4,2V | 0,454519A /3,850020V /138,414mW;PASS | 0,434198A /4,127272V /12,726mW;PASS |
| 600mA | 3,6V | 0,968011A /2,854631V /627,821mW;FAIL | 0,824884A /3,461832V /45,929mW;FAIL |
| 600mA | 3,7V | 0,933332A /2,981334V /583,643mW;FAIL | 0,805394A /3,565097V /43,784mW;PASS |
| 600mA | 4,0V | 0,847954A /3,347075V /481,748mW;FAIL | 0,753322A /3,873819V /38,306mW;PASS |
| 600mA | 4,2V | 0,802264A /3,582257V /431,230mW;PASS | 0,723091A /4,078882V /35,293mW;PASS |

Do dòng thay đổi giữa hai nghiệm, không lấy phần giảm nhiệt của bảngghép để nói chỉ do tỷ sốtrở. Ví dụ tại4,2V/600mArail, packcurrent giảm0,079173A theo mô hình, nhưng không thể nhân số này cho0,75h rồi tuyên bố thời lượng thật tăng: hiệu suất/tải/điện áp pin đều đổi theo thời gian.

## 5. Nhiệt độ25/40/60C: kiểm tra có điều kiện

Quan hệ xác lập `Tj = Ta_local + P_Q1 RthetaJA`. Nhiệt độ Ta_local tạiPCB có thể cao hơn nhiệt độ phòng vì camera/LDO/buck/vỏ. Không dùng sốRthetaJA của fixture hãng làm số đo adapter.

Giữ **cùng I=1A**, cho Rold0,67ohm không tăng theo nhiệt chỉ để thấy độ nhạy, Rnewhot0,0675ohm do dự án chọn, và từng fixture công bố riêng:

| Ta_local | Cũ P0,670W/131C/W: Tj minh họa | Mới P0,0675W/153,5C/W: Tj minh họa |
|---:|---:|---:|
| 25C | 112,77C | 35,36C |
| 40C | 127,77C | 50,36C |
| 60C | 147,77C | 70,36C |

Hai cột dùng fixture khác nhau; cột cũ còn cốđịnhtrở25C khi nóng nên **không phải worstcase**, cột mới cũng dùng hệsốnóng chọn. Bảng không chứng minh XIAO đạt/hỏng tại các nhiệt độ này. Với mục tiêu Tj<=100C ở Ta_local60C và I1A/Rnewhot67,5milliohm, ngân sách nhiệt cho adapter là `(100-60)/0,0675 = 592,6C/W`; phải chứng minh PCB/adapter đáp ứng, không mặc định từ package. Dòng diode thân khởiđộng và quá độ chưa được tính bởi tổn hao dẫn I2R này.

## 6. Quyết định và chi phí

**Đề xuất thay đường Q1 bằng DMP2035U-7 cho bản thiết kế có sửa mainboard**, vì mô hình giảm rõ sụt áp/nhiệt và mở dải Vpack ở cùng tải; vẫn giữ bảnstock làm đối chiếu. Đây không phải thaypinlàxong hoặc chứng minh45phút. Chọn linh kiện mới không được bỏ qua đườngD2USB, gateVBUS, nguồn đo pin và sạcstock.

Chưa lấy báo giá ViệtNam trong audit này; không ghi giá tiền không có nguồn. MOSFET chỉ là một phần chi phí: còn adapter/PCB, rework tháoQ1cũ, dâyđấu và kiểmtra. SOT23 lớn hơn package cũ, nên cải thiện nguồn phải được đối chiếu với giới hạn kíchthướcGO3 và vùngantenna. Không mua hoặc thực hiệnrework trong audit.

Những câu **không được kết luận**: Rhot67,5milliohm được hãng bảo đảm; cảhệ tiếtkiệm90%; chip1,4A/4,9A đồng nghĩa cảboardcấpđược dòngđó; thaySOT23 trực tiếp footprintcũ; chắctrên3,6V luônđạtVIN3,5V ở600mArail; thời lượng45phút đãđạt; nhiệtroom60C lànhiệtcellcho phép sạc. Đầu vào còn cần xácnhận: revision thật, địnhmức/nhiệtboard, transientUSB/Q1/bodydiode, đườngngưỡngpin và tải thực.

## 7. Kiểm tra số học quyết định cuối v0.4

Đây là tiêu chí được hồ sơ tổng chọn sau khảo sát, không phải một datasheet mới cho MOSFET. Với Vpackmin3,7V, VINmin3,5V, Rexternal<=0,100ohm và giới hạn thiết kế Ipack<=1A:

\[
R_{Q1,hot}\le\frac{3,7-3,5}{1}-0,100=0,100\ ohm,\quad
P_{Q1}\le1^2(0,100)=0,100\ W.
\]

Tại Vpack3,7V và I3V3phân bổ600mA, eta0,85/Icam150mA/Iaux2mA như trên, RQ1=0,100ohm và Rexternal=0,100ohm cho nghiệm ghép:

\[
V_{IN}=3,537917344\ V,\quad I_{pack}=0,810413281\ A.
\]

Sụt riêng Q1=81,041mV, sụt đường ngoài=81,041mV, nhiệtQ1=65,677mW. Tiêu chí VIN3,5V được đạt **trong mô hình và các điều kiện đã chọn**. Ngược lại, ở VIN3,5V thì Ipack=0,817546218A và Vpackcần=3,663509244V. Không gọi3,6635V là ngưỡng dừngfirmware đã chốt.

Với Ta_local<=60C và Tj mục tiêu<=85C, tại tổn hao tối đa phân bổ100mW:

\[
\theta_{JA,effective}\le(85-60)/0,100=250\ C/W.
\]

250C/W là ngân sách nhiệt của adapter/PCB cần xác nhận. Nếu dùng đúng điểm ghép3,7V ở trên và theta250C/W thì Tj mô hình tại Ta25/40/60C lần lượt41,419/56,419/76,419C. Không thay nhiệt độ PCB/vỏ thật bằng nhiệt độ phòng. Fixture153,5C/W của hãng chỉ là một đối chiếu, không phải số đo adapter.

R6=10kilohm song song R9stock100kilohm tạo9,090909kilohm danh định. Nếu chỉ biết R6+1% thì trở pulldown hữu hiệu không lớn hơn10,1kilohm; dòng rò gate10microampere tạo lệchgate<=0,101V theo mô hìnhDC. R6 lấy thêm0,5mA từUSB5V, tổn hao2,5mW; tại5,25V và R6-1%, tổn hao2,784mW. Đây là dòng USB khi có nguồn, không phải dòng pin liên tục khi gate ởGND.

Mô hình tụVBUS2microfarad khôngtải cho RCstock0,200s và RCmới0,018182s. Đây không phải độtrễchuyểnUSBthật: còn nguồnVBUS, diode, tải, điệndung hiệu dụng, gatecharge/Miller và trạng thái buck. Không suy ra chuyểnUSB/pin khôngbrownout từhai hằngsốRC.
