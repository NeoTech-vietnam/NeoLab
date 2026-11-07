# NeoCam: tụ giữ nguồn và chuyển USB/pin khi đang quay

Ngày: **2026-11-06**. Audit từ tài liệu hãng và phương trình quá độ; chưa đo board, không xác nhận đã chuyển nguồn không reset. Phạm vi là chuyển giữa **USB khỏe và pin khỏe đang gắn**, không phải duy trì quay khi cả hai nguồn mất hoặc pack bị tháo.

**Quyết định hiện hành v0.5:** TPS2121 và **3 x KEMET T520X337M010ATE025** tại OUT/VIN. Bộ điều kiện cuối và phép tính nằm ở **mục 10**: \(C_{eff}\ge750\,\mu F\), ESR≤15 mΩ, điện áp thật IN2≥3,8 V, gap≤50 µs, điện lượng ngược≤20 µC và peak tổng≤3 A. Khi đồng thời đáp ứng, mô hình cho VIN tối thiểu **3,541667 V**. Các phương án 2 tụ ở mục 8–9 là lịch sử.

**Nguồn tụ hiện hành:** [KEMET/YAGEO T52X/T530, 2026-10-26](https://yageogroup.com/content/datasheet/asset/file/KEM_T2076_T52X-530), đã đọc trực tiếp. Endurance cho phép dung lượng giảm và ESR tăng; vì vậy **750 µF / 15 mΩ là điều kiện cần nghiệm thu, không phải bảo đảm toàn tuổi thọ tự suy từ 3 x 330 µF**.

## 1. Dữ liệu power mux và giới hạn thời gian

[TPS2116 RevA, trang6-7,11-12](https://www.ti.com/lit/ds/symlink/tps2116.pdf): chuyển break-before-make; tSW **8us typical tại5V**,6,2us typical tại3,3V, điều kiện RL10ohm/CL10uF/25C. **Không có tSW maximum**. RON max59milliohm ởVIN3,3V/IOUT200mA/-40..105C; không tự gọi đó là max ở1A. Softstart1,7ms tại5V cũng typical/CL10uF. Nếu đầu vào sụt nhanh hơn1V/10us, nguồn còn lại cần>=2,5V để tránh khả năng reset mux.

[TPS2121 RevF, trang7 và15-17](https://www.ti.com/lit/ds/symlink/tps2121.pdf): tFSW5us là **typical**, trong chế độ fastswitchover CP2>=VREF, không phải upperbound đã bảo đảm cho NeoCam. Thời gian/điện áp đầu ra còn phụ thuộc giới hạn dòng và điều kiện reverseblocking. IC có programmable softstart/currentlimit; cần cấu hình đúng thay vì suy ra mọi chế độ đều5us.

Khi chuyển từUSB cao xuốngpin thấp, mux có thể chờ output tụ xả tới điện áp gần đầu vào pin cộng ngưỡng reverseblocking mới dẫn nhánh pin. Vì vậy không dùng tùy tiện toàn khoảng5V->3,5V làm headroom cho deadtime. Phải kiểm tra cả điện áp sau pin tiếp quản, ESR/ESL và gap thực quanh thời điểm đó. Tụ lớn kéo dài thời gian output về gần pin nhưng không làm mất nguyên tắc reverseblocking.

**Không được viết:** chọn một mux có tSWtyp5/8us cộng một tụ là đã chứng minh không reset. Có thể chọn một khoảng gián đoạn **tgap_design** và suy ra yêu cầu Ceff; sau đó chứng minh mux/mạch nằm trong khoảng đó bằng dữ liệu phù hợp/mô hình hãng và kiểm chứng.

## 2. Công thức và các điểm đặt tụ

Tụ bulk đặt tại **OUT mux/VIN XIAO**, không mắc trực tiếp một tụ hàng trăm microfarad vàoUSB connector rồi bỏ qua inrush. Input mux vẫn cần decoupling theo hãng. Tụ100nF/MLCC nhỏ gầnbuck và đường hồi ngắn xử lý thành phần tần số cao; không dùng bulk thay mọi decoupling.

Trong khoảng nguồn không cấp tải, với Ihold được chặn trên:

\[
\Delta V_{hold}\le I_{hold}t_{gap}/C_{eff},\quad
\Delta V_{total}\simeq I_{hold}t_{gap}/C_{eff}+I_{step}ESR+L_{loop}\frac{dI}{dt}.
\]

Với tải côngsuấtgầnkhôngđổi, dòng tăng khi áp giảm. Muốn dùng côngthức dòngkhôngđổi bảo thủ, Ihold phải>=Pmax/Vmin cộng các dòng còn lại; không thay dòngUSB trước chuyển sang làm giới hạn. Côngthức năng lượng lý tưởng tương ứng:

\[
\tfrac12 C_{eff}(V_{start}^2-V_{end}^2)\ge P_{hold,max}t_{gap}.
\]

Hai cách chỉ dùng khi đầu vào/tải/hiệusuất được định nghĩa nhất quán, không nhân hiệu suất hai lần.

Điều kiện DC sau khi pin tiếp quản:

\[
V_{post}\ge V_{pack,loaded}-I_{path}(R_{external}+R_{mux}).
\]

Điều kiện bảo thủ để vẫn giữ VIN>=3,5V qua vùng handoff:

\[
V_{pack,min}\ge3,5+I_{path}(R_{external}+R_{mux})
+I_{step}ESR+\Delta V_{ESL,budget}+I_{hold}t_{gap}/C_{eff}.
\]

Đây là tiêu chí bảo thủ giả định vùng chuyển bắt đầu gần điện áp backup, không phải chính xác toàn dạng sóngUSB->pin. Vpack là điện áp tạiP+/P- có tải của pack; không trừ thêm nội trởcell/PCM lần nữa.

## 3. Các tụ có mã thật

| Mã tham chiếu | Dữ liệu tại điều kiện datasheet | Kích thước / giới hạn |
|---|---|---|
| **KEMET T520X337M010ATE025** | 330uF,+/-20%,10V,ESR25milliohm tại25C/100kHz | X/7343-43:7,3x4,3x4,3mm; polymer tantalum; nhỏ/phẳng hơn tụcan, không có báo giáVN trong audit |
| **Panasonic6SVP470M** | 470uF,+/-20%,6,3V,ESR15milliohm tại100kHz; leakage592uAmax | đườngkính8mm,cao12mm; khá lớn so với NeoCam |
| **NichiconRNU0J102MDN1** | 1000uF,6,3V,ESR7milliohm tại20C/100kHz; dòngrò1260uA theo bảng | đườngkính8mm,dài11,5mm; xác nhận hậu tốđónggói khi đặt hàng |

Nguồn: [KEMET/YAGEO T52X/T530 bản 2026-10-26](https://yageogroup.com/content/datasheet/asset/file/KEM_T2076_T52X-530), [Panasonic 6SVP470M](https://industrial.panasonic.com/ww/products/pt/os-con/models/6SVP470M), [catalog Nichicon RNU](https://www.nichicon.co.jp/english/series_items/catalog_pdf/e-rnu.pdf), [trang Nichicon đúng mã](https://www.nichicon.com/en-us/part/rnu0j102mdn1/1739/). Mã KEMET và các điều kiện endurance được kiểm tra trực tiếp trong tài liệu hãng, trình bày ở mục 10. Tình trạng vòng đời, mã đóng gói và hàng giao vẫn cần xác nhận trước BOM sản xuất.

Tụpolymer không có cùng cơ chế mấtC theoDCbias nhưMLCCClassII, nhưng dung sai, nhiệt độ, tuổi thọ và ESR vẫn phải xét. Không tự lấy0,8Cnom là Ceff bảo đảm toànđờivàmọinhiệtđộ; đó chỉ là corner dung sai banđầu-20% dùng trong bảng tính sau.

USB có thể5,25V vàovershoot:tụ6,3V chưa mặc nhiên sai, nhưng chỉ còn1,05V margin trước rating. Không áp quy tắc20%derating của mọi tantalum lên mọi polymer: KO-CAP≤10V thường khuyếnnghị10%derating, >10V20% theo hướngdẫn dòng sản phẩm. 6,3x0,9=5,67V vẫn lớn hơn5,25V;10V là lựa chọn có margin rõ hơn. Chọn10V không tự tăngCeff của mọiMLCC; phải đọc đườngcong đúng mã.

## 4. Ví dụ định lượng có điều kiện

Chọn các **ngân sách dự án**, không phải thông số hãng đã chứng minh: Ihold=Istep=Ipath=1A, Rexternal<=0,100ohm, Rmux<=0,100ohm cần nghiệmthu ở tải/nhiệt, deltaVESL<=20mV. Với Vpackmin=3,9V, điệnáp DC sauđườngnguồn>=3,7V; còn200mV cho quáđộ để giữ3,5V.

| Phương án | Ccorner-20% | ESR dùng trong ví dụ | Vdip tổng nếu gap50us | Vdip tổng nếu gap100us | Kết quả với headroom200mV |
|---|---:|---:|---:|---:|---|
| 1x330uF KEMET | 264uF | 25mOhm | 234,39mV | 423,79mV | Không đạt cảhai gap |
| 2x330uF KEMET song song | 528uF | 12,5mOhm | 127,20mV | 221,89mV | Đạt50us; không đạt100us |
| 1x470uF Panasonic | 376uF | 15mOhm | 167,98mV | 300,96mV | Đạt50us; không đạt100us |
| 1x1000uF Nichicon | 800uF | 7mOhm | 89,50mV | 152,00mV | Đạt50/100us theo các corner đã chọn |

Các ESR trên có điều kiện tần số/nhiệt cụ thể, chưa là maxmọinhiệt độ. ESRsong song12,5mOhm giả định phân phối layout tương đương; ESR/ESL thựcđườngmạch phải xác nhận. ESRstep vàCdischarge có thể cộng bảo thủ nhưng không thay SPICEmodelphụthuộctầnsố.

Với2x330uF và gap50us, Vmin>=3,7-0,12720=**3,57280V**. MinimumVpack từ bấtđẳngthức trên là3,82720V; chọn3,9V tạo margin72,80mV cho cácngân sách đãnêu. Nếu muốn gap100us, cùng tụ/môhình cầnVpack>=**3,92189V**, dođó chọn4,0V mới tạo margin78,11mV. Không dùng3,7V làmmincủa sảnphẩm ởI1A/Rtotal0,2ohm rồi cộng thêmdroop: điệnápDC đã chỉ3,5V, không còn khoảngtrốngchoquáđộ.

Nếu muốn giữ một tụ330uF tạiVpack3,9V vàcácngân sáchtrên, gapcho phép chỉ:

\[
t_{gap}\le264uF(0,200-0,025-0,020)/1A=40,92us.
\]

Các gap40,92/50/100us này là **mục tiêu cần chứng minh**, không phải tSWmax củaTPS2116/TPS2121. Không chốt danhnghĩa tụ trước khi chọn bộtiêu chíMUX/headroom/bandwidth/tolerance.

## 5. MLCC và DCbias

[Murata giải thích DCbias của tụ điệnmôi hằngsốcao](https://www.murata.com/support/faqs/capacitor/ceramiccapacitor/char/0005) và [TDK modelDCbias](https://www.tdk-electronics.tdk.com/en/373812/tech-library/articles/tools-services/dynamic-dc-bias-model-for-accurate-circuit-simulation/1035570): C thayđổi theo điệnáp DC, phải dùng môhình/đồthị đúngpart. Ví dụGRM31CR61A476ME15L là47uF/10V/X5R/1206 nhưng chỉ thôngsố47uF không cho Ceff tại5,25V.

Không có một điểmCeff5,25V được bảođảm của mãMLCC cụthể trongaudit này; dođó **không cộng dunglượngMLCC vào bảođảm hold-up**. Dùngbulkpolymer cho cornerngân sách, MLCC cho tầnsốcao; khi có dữliệuSimSurfing/TVCLchínhhãng mới thay thế bằng mộtbankMLCC nhỏ hơn. Không tự viết47uF còn50% hoặc80% khi chưa tragiátrị.

## 6. Inrush và slewrate

\[
I_{cap}=C\frac{dV}{dt};\quad Q=C\Delta V;\quad E=\tfrac12CV^2.
\]

Với bank660uF,5V và slewramp tươngđương softstartTPS2116typ1,7ms: Icap≈**1,94A**, chưa cộngload và dòngsạc. LấyC+20%=792uF,5,25V vàcùng1,7ms cho2,45A; nhưng vìtSSchỉtyp/CL10uF, đây là cảnhbáo độlớn, không phải maxinrush đãtínhđúngcủamux.

Nếu **chọn** khốngchế ramp10ms, Cmax792uF tới5,25V cho Icap≈415,8mA; còn load/charger phải cộng và kiểm tra currentlimit/slew thực. Battery-onlycoldstart tới4,2V cho332,6mA riêngtụ, không được cộng với tảiquay0,8A rồi gọi pack1A đủ. Có thể cần giữ tải ởpreparation thấp cho tới nguồnổnđịnh; đó là yêu cầu cần thiếtkế/chứngminh, không phải firmwaređãlàm.

TPS2121 cho lập trìnhsoftstart/currentlimit; currentlimit có thể kéo dài thờigiannângáp khi chuyểnpin->USB. TPS2116 không cho tự chọn10mschỉ bằngghi chú. Không đặtbulk660/1000uF trực tiếpVBUS trướcswitch rồi nóiUSBkhông cóinrush; cần kiểmtra quyđịnhUSB, cable/sourcecapability và phầninput. Cũng cầnso sánh nănglượngxung/fuse/PCM trên đườngpin ởcoldstart.

## 7. Phạm vi hoàn thành

Đã có côngthức, mãtụthật, cảnhbáomax/typ, ngân sáchheadroom vàcác phươngán50/100us. **Chưa chốt mộtbanktụ được nghiệmthu cho NeoCam**, vì cần cùng lúc đóngmodeMUX,timingmax/target, RONở1A/nhiệt, inputUSBfall/rise, Ceff/ESRtheonhiệt, inrush vàđápứngbuck.

Một tụ800uFhiệudụng ở3,7V xuống3,5V chỉ dựtrữ khoảng0,576mJ; tải3W dùng hếtkhoảng0,192ms lýtưởng. Tụgiữhandoff không phải pinbackupđểlưu tệp. Khi khôngcópin, rútUSB cóthể reset; nếu sảnphẩm cầnlưu antoàn trườnghợpcảhainguồnmất phải thiết kếbài toánnănglượngkếtthúc tệpriêng.

## 8. Quyết định v0.5 và kiểm tra độc lập

Hồ sơ tổng chọn **TPS2121**, không dùng diodefallback song song, thay hai đườngpass củaQM1/D2stock trong biếnthể adapter. Bankoutput chọn **2xT520X337M010ATE025**, Cnom660uF; SS1uF; RILM80kilohm1%. Giữ chargerstock,nguồnUSB5V có khả năng đượcxácnhận3A; không mặc định cổngPCUSB cấp3A.

Các điều kiện nghiệmthu của mô hình mới:

- I_SYS/Ihold/Istep<=1A; Rexternal<=0,100ohm; Rmuxhot<=0,100ohm tại cấu hình thật.
- Vpack cótải>=3,9V cho kịchbản xét; Ceffbank>=500uF, ESReffective<=30milliohm, ngân sáchdroopdoESL/layout<=20mV.
- tgap_total<=50us; đây là khoảng thiếu dòng tổng cần nghiệmthu tại điểmhandoff, không thay bằng tFSWtyp5us của hãng.
- SRoutput<=200V/s khi khởiđộng; tảiPREP<=0,5A; không cho vào chếđộrecord nặng trướcnguồn ổnđịnh.

Khi đồngthời đápứng các điều kiện này:

\[
V_{IN,min}\ge3,9-1(0,100+0,100)-\frac{1(50us)}{500uF}-1(0,030)-0,020
=\boxed{3,55\ V}.
\]

Giới hạn3,55V cao hơn tiêu chíVIN3,5V50mV; tiêu chíVIN3,5V vẫn chưa là bảođảm đápứngbuck/rail3V3mọixung. Khôngkhẳngđịnh hệ khôngreset nếu mộtđầuvào nghiệmthu chưađược xácnhận.

Bank660uF +/-20% cho Cminbanđầu528uF và Cmax792uF. Khoản dựphòng giữa528và500chỉ28uF, khoảng5,3% của528uF. Không tự coiCeff>=500uFđãđạt trên nhiệtđộ/lãohóa/DCbias chỉ từdungsaibanđầu. Ngược lại không dùnglại một hệsốMLCC tùyý cho tụpolymer; phải đốichiếu đặc tính đúngpacktụ/điềukiện.

[TPS2121 bảng9-1](https://www.ti.com/lit/ds/symlink/tps2121.pdf#page=13) cho estimatedslewrate88V/s tạiCSS1uF/VIN5V. Giới hạn200V/s là tiêu chí nghiệmthu riêng. TừCmax792uF:

\[
I_{cap,max,model}=792uF(200V/s)=158,4mA;
\quad I_{coldstart}\le500+158,4=658,4mA.
\]

658,4mA chỉ gồm dòngra củaMUX cấp tụvàPREP trong môhình; dòngnuôiMUX/nhánhngoài ởpack phải cộng nhưng còn dư341,6mA trongcontract1A. Điều kiện này giúp giải xungkhởiđộngbìnhthường, không phải giới hạn dòng lỗi. SR200V/s cần áp dụng lên độdốc tức thời cóýnghĩa, không chỉ giátrịtrungbìnhđườngramp.

### 8.1. Điểm đo MAX17048 và điều kiện3,8V

MAXđoVBAT_XIAO sauđườngfuse/dây. Khi I<=1A/Rexternal<=0,1ohm, Vpack>=3,9V **suyra** VBAT_XIAO>=3,8V. Chiềungượckhôngđúng: đoVBAT3,8V không chứngminhVpack3,9V vìRexternal cóthểnhỏhơn0,1ohm.

Cóthể dùng một tiêu chí **trực tiếp tại điểmđo**, độc lập với suyngượctrên:

\[
V_{BAT\_XIAO,true}\ge3,8V\Rightarrow
V_{IN,steady}\ge3,8-1(0,100)=3,7V\Rightarrow V_{IN,min}\ge3,55V.
\]

Điều kiện3,8V trong bấtđẳngthức là **điệnápthật**, không phải mãADC thô; phảicộng sai sốVCELL, độtrễ250ms, gợn/sụtáp động, hysteresis vànănglượnglưutệp khi xácđịnh ngưỡngfirmware. Không chốt tựđộng3,8V làmngưỡngdừngquay. KhiUSBnuôi, BATnodereadingkhôngtựchứngminh packđanggắn.

### 8.2. ILIM và phối hợp bảo vệ còn mở

[TPS2121 bảng7.5](https://www.ti.com/lit/ds/symlink/tps2121.pdf#page=8) tại RILM80kilohm ghi **1/1,5/2A min/typ/max**, không phải currentlimitđúng1A. Dungsađiệntrở1% vàđiều kiệncurrentlimit cầnđược xét; không gọi2A là maxchắcchắn của mạch đãlắp khi chưa tính cả chúng. Giớihạn1A của audit là **contract vận hànhbìnhthường**, không phải lỗiđãđược phầncứngkhốngchếở1A.

Điểmphốihợp phảiđóng: protectedpackDNK chỉđịnhmức1A liên tục, PCMquádòng3-5A, cònMUXcóthểgiớihạn1-2A. Một lỗi gây1-2A kéo dài cóthểvượt địnhmứccell nhưng chưachạmPCM, vàfuse1,5Akhôngphảibảovệ1A tức thời. Cần đánhgiácurrentlimittheothờigian, nhiệtcell/fuse/MUX hoặc đổi packđịnhmứcphùhợp/thêmgiớihạnlỗichuyênbiệt. Không nóiTPS2121đãđóngvòngbảovệcell chỉ vìcoldstart658mA<1A.

NguồnUSB5V3A được chọn làmcontract cấpnguồn códựphòng, không phải phépnegotiationUSBđãđượctriểnkhai. DònggiớihạnMUXởoutput gộpcảloadvàtụ; khiước lượngngânsáchinput không cộngIcaplầnnữa bênngoàiILIM. Charger109mAnhánhVBUSvẫnphảicộngriêng. Tồnđọngdònglỗi/nguồnUSBnày không thayđổi phép tínhhold-upđúngtheocácđiềukiệnđãghi.

Vpack3,9Vhoặcđiểmđo3,8Vcóthểloạimộtphầndunglượngpin, vìvậykhôngáp bảochứngnàycho toàn45phút khi pinxuốngthấp. Cần dùngđườngcongxả vàdòngtácthật đểxácđịnh khoảngvận hành đạtchuyểnnguồnkhôngreset.

## 9. Phép tính hold-up v0.5 sau bổ sung reverse-charge

**Mục này thay kết quả3,55V ở mục8 khi dùng bộngânsách cuối đã thêm reversecharge.** Mục8 giữlại để truyvết. Giới hạn dòngvậnhànhbìnhthường hạ còn **I_SYS<=0,9A** để cókhoảngdựphòngvới currentlimit; môhìnhhold-up vẫn dùng **Iload=1A** bảo thủ. Yêu cầu minILIMthực lớn hơn0,9A cần đượcnghiệmthu, không tự suy chắcchắn từdòngtable80kilohm khi điệntrởlắp códungsa1%.

Các điều kiện bổ sung: điệnlượngchảyngược/lostcharge trongordinaryhandoff **Qreverse<=10microcoulomb**, dòngđỉnhtổngtạibanktụ **Icap,peak<=1,2A**, tgap_total<=50us, Ceff>=500uF, ESR<=30milliohm, droopESL/ringing<=20mV. Qreverse gồm phầnthoátnănglượng vàođườngnguồncũ/inputcapacitor có liênquan, chưa phải một maximum hãngcôngbố. DòngIcap,peak phải baogồm dòngtải **cộng** dòngngượcđồngthời, không phải chỉ dòngngược.

\[
Q_{forward}=1A(50us)=50uC;
\quad Q_{total}=50+10=60uC.
\]

\[
\Delta V_C\le60uC/500uF=0,120V;
\quad\Delta V_{ESR}\le1,2A(0,030ohm)=0,036V.
\]

\[
\Delta V_{total}\le0,120+0,036+0,020=\boxed{0,176V}.
\]

Với **IN2/VBAT_XIAOthật>=3,8V**, giới hạnđườngMUX1A/Rmux<=0,100ohm cho điểmbắtđầu bảo thủ3,7V:

\[
V_{IN,min}\ge3,8-1(0,100)-0,176=\boxed{3,524V}.
\]

Marginso vớiVIN3,5Vchỉ24mV. Mọi saikhác chưa nằmtrongngânsách tải, điệnápđiểmđo, ringing/dynamic, nhiệt/điệntrở, thờigian vàđiệnlượngngược phải được xửlý trongcác điều kiệnnghiệmthu, không trừtùyý rồi vẫncoi3,524Vđượcgiữ. ĐiệnápMAXđọc cầncộng sai số/độtrễ đểsuyra IN2thật; không đặtngưỡng mãđọcthẳng3,8V màbỏquasai số.

Qreverse10uC và Icappeak1,2A là **hai điều kiện phải đồngthời đạt**. Tụ cóthể cấp đỉnhrấtnhanh trước khiRCBngắt; thresholdRCB/delaytyp củaMUXkhông tựbảođảm hai điềukiệntrên. Ví dụtải1Ađồngthời reverse1A sẽ làmIcap2A, vượtngânsáchESR1,2A dù tổngđiệnlượngngược ngắn còn<=10uC. Vìvậyphải xét cảdạngdòng vàdiệntích dòngngược, không chỉ tSW hoặc tổngQ.

Phạmvi là tháoUSBbìnhthường, open-cable/inputcapsharing đã xácđịnh, pin khỏe đanggắn; **không bao hàm chập cứngVBUS**, tháopin, cảhainguồnmất hoặc lỗi đườngmạch. Không bỏ phối hợp bảovệcell1A với ILIM1-2A ởmục8.2. Không tuyênbố khôngresetđãđạt khi chỉ tínhđúng0,176V màchưađóng nhữngđiềukiệnnày.

## 10. Quyết định hiện hành: 3 x 330 µF và điều kiện nghiệm thu

**Bản v0.5 chọn 3 tụ KEMET T520X337M010ATE025 song song.** Phần này thay các số của phương án 2 tụ ở mục 8–9; các mục đó được giữ để truy vết.

Nguồn chính đã đọc trực tiếp: [KEMET/YAGEO T52X/T530, ngày 2026-10-26, 50 trang](https://yageogroup.com/content/datasheet/asset/file/KEM_T2076_T52X-530). Bảng trang 25 ghi T520X337M010A(3)E025; trang 3 quy định (3)=T cho chân phủ thiếc. Đây là đúng mã **T520X337M010ATE025**.

| Thông số một tụ | Dữ liệu hãng / điều kiện |
|---|---|
| Dung lượng, dung sai, điện áp | 330 µF, ±20%, 10 V |
| ESR ban đầu | Tối đa 25 mΩ tại 25°C, 100 kHz |
| Dòng rò | Tối đa 330 µA tại 25°C, sau 5 phút ở điều kiện bảng |
| Ripple | 3.100 mArms tại 45°C, 100 kHz; không đồng nhất với mọi dạng xung |
| Nhiệt độ, endurance | Tối đa 105°C; 2.000 giờ theo điều kiện thử |
| Case X | 7343-43; dài 7,3±0,3 mm, rộng 4,3±0,3 mm, cao 4,0±0,3 mm |

### 10.1. Ngân sách cuối và phép tính

- Dòng vận hành bình thường \(I_{SYS}\le0,9\,A\); phép tính giữ nguồn dùng tải 1 A để có dự phòng.
- Điện áp thật tại IN2/VBAT_XIAO ít nhất 3,8 V; \(R_{mux,hot}\le0,100\,\Omega\).
- Cả bank có \(C_{eff}\ge750\,\mu F\) và ESR hiệu dụng, gồm đường nối, không quá 15 mΩ.
- Điện lượng mất thêm do chảy ngược \(Q_{reverse}\le20\,\mu C\); dòng đỉnh **tổng qua bank tụ** không quá 3 A.
- Khoảng thiếu dòng tổng \(t_{gap}\le50\,\mu s\); droop do ESL/ringing không quá 20 mV.

Đây là các điều kiện phải đồng thời nghiệm thu, không phải thông số maximum mới của mux hoặc của tụ.

\[
C_{nom}=990\,\mu F,\quad C_{initial,min}=792\,\mu F,\quad C_{max}=1188\,\mu F.
\]

\[
Q_{total}=1\,A(50\,\mu s)+20\,\mu C=70\,\mu C;
\quad\Delta V_C=70/750=0,093333\,V.
\]

\[
\Delta V_{ESR}=3\,A(0,015\,\Omega)=0,045\,V;
\quad\Delta V_{total}=0,093333+0,045+0,020=0,158333\,V.
\]

\[
V_{IN,min}\ge3,8-1(0,100)-0,158333=\boxed{3,541667\,V}.
\]

Margin còn 41,667 mV so với tiêu chí VIN 3,5 V. Dòng đỉnh phải bao gồm tải và dòng ngược đồng thời. Không lấy threshold RCB 2 A và thời gian typical rồi suy ra chắc chắn điện lượng ngược≤20 µC và peak tổng≤3 A.

### 10.2. Endurance không tự bảo đảm 750 µF / 15 mΩ

Trang 5 của tài liệu hãng cho phép sau phép thử endurance: dung lượng thay đổi −20%/+10% **so với giá trị ban đầu**; ESR nằm trong 2 lần giới hạn ban đầu, theo điều kiện và ngoại lệ của bảng.

Ghép riêng corner dung sai ban đầu −20% với mức giảm endurance −20%:

\[
C_{bank,endurance,corner}=3(330)(0,8)(0,8)=633,6\,\mu F.
\]

Giới hạn ESR mỗi nhánh sau thử có thể là \(2(25)=50\,m\Omega\), tương đương **16,667 mΩ** cho ba nhánh lý tưởng. Hai corner này không thỏa bộ điều kiện **750 µF / 15 mΩ** đã chọn.

Do đó, không được viết rằng 3 tụ tự bảo đảm bộ điều kiện này trong toàn tuổi thọ chỉ từ datasheet. Giữ 750 µF / 15 mΩ là **yêu cầu nghiệm thu ứng dụng**; phải đóng dải nhiệt độ, thời gian sử dụng, điện áp, đặc tính tần số và tiêu chí cuối đời, hoặc tăng dự phòng/đổi linh kiện khi cần. Khoảng từ 792 xuống 750 µF chỉ là 42 µF, khoảng 5,30%.

Để thấy độ nhạy, nếu chỉ thay C=633,6 µF và ESR=16,667 mΩ vào cùng mô hình, giữ các ngân sách khác:

\[
V_{IN,min,corner}\simeq3,7-\frac{70\,\mu C}{633,6\,\mu F}
-3(0,016667)-0,020\simeq3,51952\,V.
\]

Kết quả này chưa cộng mọi thay đổi nhiệt độ/layout và không chứng minh tuổi thọ hệ thống. Nó cũng không làm bộ điều kiện 750 µF / 15 mΩ tự động được thỏa mãn. Không nhân tiếp tùy ý mọi sai số nếu các bảng chưa cho biết chúng có thể đồng thời xảy ra.

ESR song song lý tưởng ban đầu là 25/3=8,333 mΩ. ESR hiệu dụng thực còn phụ thuộc phân bố dòng, nhiệt độ, tần số và đường PCB. Không coi giá trị lý tưởng là số đo bank thật.

### 10.3. Khởi động và dòng rò

Với slew rate đầu ra cần nghiệm thu≤200 V/s:

\[
I_{cap,start}\le1188\,\mu F(200\,V/s)=237,6\,mA;
\quad I_{PREP}+I_{cap,start}\le500+237,6=737,6\,mA.
\]

Phải cộng dòng nuôi mux và các nhánh phụ khi quy về pack. Giữ PREP≤0,5 A và chưa vào chế độ quay nặng trước khi nguồn ổn định. CSS=1 µF cho điểm estimated slew rate 88 V/s tại 5 V của hãng; nó không tự chứng minh giới hạn tức thời 200 V/s.

Ba tụ có tổng dòng rò theo điểm bảng là \(3(330)=990\,\mu A\). Cộng mux active 400 µA và MAX17048 40 µA cho khoảng 1,43 mA tại những điều kiện tương ứng, trước điện trở điều khiển và dòng rò khác. Ngân sách phụ 2 mA vẫn cần đối chiếu toàn điều kiện; không coi tụ là phụ tải 0 A hoặc đổi dòng rò theo tỷ lệ điện áp tùy ý.

### 10.4. Những giới hạn vẫn còn

- Điện áp thật 3,8 V tại điểm đo không phải ngưỡng firmware đã chốt. Cần sai số, sampling, hysteresis và năng lượng hoàn tất tệp.
- Vpack≥3,9 V là điều kiện đủ theo ngân sách đường ngoài; số đo VBAT_XIAO=3,8 V không tự chứng minh Vpack=3,9 V.
- Phối hợp cell 1 A với ILIM 1–2 A vẫn chưa đóng. Giới hạn vận hành bình thường 0,9 A không thay bảo vệ dòng lỗi.
- Phạm vi là tháo USB bình thường với pin khỏe đang gắn; không gồm hard-short VBUS, tháo pack hoặc mất cả hai nguồn.
- Chưa hứa đạt điều kiện chuyển nguồn trong toàn bộ 45 phút khi pin giảm áp.
- Ba tụ tăng diện tích, chiều cao, chi phí và năng lượng khởi động; chưa có CAD vỏ chứng minh lắp vừa.
