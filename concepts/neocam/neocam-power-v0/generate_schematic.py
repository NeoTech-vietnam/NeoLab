"""Rebuild the NeoCam companion-power reference schematic (KiCad 10).

This generates a schematic, not a PCB or fabrication package. Standard symbols
come from the installed KiCad library; the two module symbols are local.
"""
from pathlib import Path
import json
import math
import re
import uuid

HERE = Path(__file__).resolve().parent
LIB = Path(r"C:\Program Files\KiCad\10.0\share\kicad\symbols")
ROOT_UUID = "f9993b18-d3df-48ee-ae00-0b9fd455017d"
MUX_SHEET_UUID = "c37f59b0-4f78-44a4-ab3c-4e5860925377"
MUX_ROOT_UUID = "36f4f91f-34dc-4f9a-99c9-d9a87dbdeeb2"
BUCK_SHEET_UUID = "739b44c2-5905-49d8-a1e6-52254ed06939"
BUCK_ROOT_UUID = "6f006989-55be-40c4-8ee4-67106923277b"
ACTIVE_PATH = '/'+ROOT_UUID
SHARED_NETS={'GND','VBAT_XIAO','VBUS_REWORK','VIN_SYS','3V3_XIAO','VDD_SD'}

def uid():
    return str(uuid.uuid4())

def q(value):
    return json.dumps(str(value), ensure_ascii=False)

def number(value):
    return f"{value:.4f}".rstrip("0").rstrip(".") if value else "0"

def snap(value):
    return round(round(value / 1.27) * 1.27, 4)

PIN_BASE = (0, 0)

def extract_symbol(file, name):
    data = (LIB / (file + ".kicad_sym")).read_text(encoding="utf-8")
    start = data.index('(symbol "' + name + '"')
    depth = 0
    string = False
    escape = False
    for i in range(start, len(data)):
        ch = data[i]
        if string:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == '"':
                string = False
        elif ch == '"':
            string = True
        elif ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return data[start:i+1]
    raise ValueError(name)

def prop(name, value, x, y, hide=False, size=1.1, justify="", angle=0):
    return f'(property {q(name)} {q(value)} (at {number(x)} {number(y)} {angle}) ' + \
        ('(hide yes) ' if hide else '') + \
        f'(effects (font (size {size} {size})) {justify}))'

def rect(x1, y1, x2, y2):
    return f'(rectangle (start {x1} {y1}) (end {x2} {y2}) (stroke (width 0.254) (type default)) (fill (type background)))'

def pin(name, num, x, y, angle, kind="passive", length=5):
    x = snap(PIN_BASE[0] + x) - snap(PIN_BASE[0])
    y = snap(PIN_BASE[1]) - snap(PIN_BASE[1] - y)
    length = 5.08
    return f'(pin {kind} line (at {x} {y} {angle}) (length {length}) (name {q(name)} (effects (font (size 1 1)))) (number {q(num)} (effects (font (size 1 1)))))'

def libtext(text, x, y, size=1.1):
    return f'(text {q(text)} (at {x} {y} 0) (effects (font (size {size} {size}))))'

def existing_module_symbol(library, name, new_name):
    """Represent a retained module component without claiming a PCB footprint."""
    data=extract_symbol(library,name)
    marker='(property "ki_fp_filters"'
    if marker in data:
        start=data.index(marker);depth=0;inside=False;escape=False
        for end in range(start,len(data)):
            ch=data[end]
            if inside:
                if escape:escape=False
                elif ch=='\\':escape=True
                elif ch=='"':inside=False
            elif ch=='"':inside=True
            elif ch=='(':depth+=1
            elif ch==')':
                depth-=1
                if depth==0:
                    data=data[:start]+data[end+1:]
                    break
    data=data.replace('(symbol '+q(name), '(symbol '+q(new_name),1)
    return data.replace('(symbol "'+name+'_','(symbol "'+new_name+'_')

def custom(name, body, pins, description, fp=""):
    return f'''(symbol {q(name)}
      (pin_names (offset 1)) (exclude_from_sim yes) (in_bom yes) (on_board yes)
      {prop('Reference','U',0,40)} {prop('Value',name,0,37)}
      {prop('Footprint',fp,0,0,True)} {prop('Datasheet','',0,0,True)}
      {prop('Description',description,0,0,True)}
      (symbol {q(name+'_0_1')} {body})
      (symbol {q(name+'_1_1')} {''.join(pins)})
      (embedded_fonts no))'''

module_name = "XIAO_ESP32S3_Sense_Power_Interface"
module_pins = [
    ("BAT+", "B+", -40, 25, 0, "passive"),
    ("BAT-", "B-", -40, 10, 0, "passive"),
    ("GND", "GND", -40, -5, 0, "power_in"),
    ("VIN_SYS / oldQ1_S_PAD3", "Q1_S3", -40, -20, 0, "power_in"),
    ("VBUS / oldQ1_G_PAD2", "Q1_G2", -40, -35, 0, "power_out"),
    ("3V3_KIT / LOAD", "3V3", 40, 25, 180, "power_in"),
    ("VDD_SPI (INT)", "VDD_SPI_ACCESS", 40, 20, 180, "power_out"),
    ("D0 / GPIO1 / ALERT_N", "D0", 40, 10, 180, "input"),
    ("SD_VDD_LOAD", "SD_VDD_LOAD", 40, 0, 180, "power_in"),
    ("D1 / GPIO2 / REC_N", "D1", 40, -5, 180, "input"),
    ("D3 / GPIO4 / LED", "D3", 40, -15, 180, "output"),
    ("D4 / GPIO5 / SDA", "D4", 40, -25, 180, "bidirectional"),
    ("D5 / GPIO6 / SCL", "D5", 40, -35, 180, "output"),
]
PIN_BASE = (160,83)
module_def = custom(module_name,
    rect(-34.29,35.56,35.56,-40.64) +
    libtext('XIAO; Q1 + D2 REMOVED',0,28,1.35) +
    libtext('USB/charger; stock buck on sheet3',0,19,1.0) +
    libtext('ESP32-S3 + OV3660',0,3,1.4) +
    libtext('microphone + microSD',0,-3,1.15),
    [pin(*p) for p in module_pins],
    'Selected mux rework: stock Q1 and D2 removed; old QM1/R6 omitted. Charger/buck retained. Named old-Q1 pads provide BAT/VIN/VBUS access; verify actual board before rework.')

switch_name = "MAX17048G_Plus"
switch_pins = [
    ("VDD", "3", -30, 15, 0, "power_in"),
    ("CELL", "2", -30, 5, 0, "input"),
    ("CTG", "1", -30, -5, 0, "passive"),
    ("QSTRT", "6", -30, -15, 0, "input"),
    ("SCL", "7", 30, 15, 180, "input"),
    ("SDA", "8", 30, 5, 180, "open_collector"),
    ("ALRT", "5", 30, -5, 180, "open_collector"),
    ("GND", "4", 0, -30, 90, "power_in"),
    ("EP", "9", 10, -30, 90, "passive"),
]
PIN_BASE = (145,175)
switch_def = custom(switch_name,
    rect(-24.13,25.4,25.4,-24.13) + libtext('1S VOLTAGE',0,15) + libtext('AND FUEL GAUGE',0,9) +
    libtext('I2C 0x36',0,-7),
    [pin(*p) for p in switch_pins],
    'ADI MAX17048G+, TDFN-8 exposed pad. Package land pattern remains to verify.',
    '')

PIN_BASE=(365,165)
pogo_def=custom('Rear_Charging_Reserved',rect(-2.54,1.27,2.54,-3.81),[
    pin('RESERVED_1','1',-5.08,0,0),pin('RESERVED_2','2',-5.08,-2.54,0)
], 'Unimplemented electrical interface, no footprint released.')
existing_r=existing_module_symbol('Device','R','Existing_R_MB9')
existing_c=existing_module_symbol('Device','C','Existing_C_Stock')
existing_l=existing_module_symbol('Device','L','Existing_L_Stock')
existing_fb=existing_module_symbol('Device','FerriteBead','Existing_FB_Stock')
PIN_BASE=(220,115)
mux_name='TPS2121RUXR'
mux_pins=[('IN1','7',-45,30,0,'power_in'),('IN2','2',-45,15,0,'power_in'),('PR1','6',-45,0,0,'input'),('CP2','3',-45,-15,0,'input'),('OUT','1',45,30,180,'power_out'),('OUT','8',45,15,180,'passive'),('ST','9',45,0,180,'open_collector'),('ILIM','10',45,-15,180,'passive'),('SS','11',45,-30,180,'passive'),('OV1','5',-20,-50,90,'input'),('OV2','4',-10,-50,90,'input'),('GND','12',0,-50,90,'power_in')]
mux_def=custom(mux_name,rect(-40.64,40.64,40.64,-45.72)+libtext('USB / BATTERY MUX',0,31,1.5)+libtext('XCOMP automatic selection',0,19,1.2)+libtext('Current limit + soft start',0,7,1.1)+libtext('No diode bypass',0,-9,1.15),[pin(*p) for p in mux_pins],'TI TPS2121RUXR VQFN-HR12. Pin1 and8 are both OUT; pin8 passive type is only the ERC representation of the same output rail. Land pattern must be verified.')
PIN_BASE=(150,105)
buck_name='SGM6029C_Stock'
buck_pins=[('VIN','B1',-35,20,0,'power_in'),('EN','C2',-35,5,0,'input'),('VSEL_MODE','C1',-35,-10,0,'passive'),('SW','B2',35,20,180,'power_out'),('VOS','A2',35,5,180,'input'),('GND','A1',0,-35,90,'power_in')]
buck_def=custom(buck_name,rect(-30.48,30.48,30.48,-30.48)+libtext('RETAINED BUCK',0,20,1.5)+libtext('SGM6029CYG/TR',0,9,1.2)+libtext('VOS senses OUT',0,-4,1.1)+libtext('Auto PSM',0,-17,1.1),[pin(*p) for p in buck_pins],'Existing stock XIAO mainboard V1.51 buck. Ball map A1 GND,A2 VOS,B1 VIN,B2 SW,C1 VSEL_MODE,C2 EN. No duplicate regulator to fit.')
custom_defs = [module_def,switch_def,pogo_def,existing_r,existing_c,existing_l,existing_fb,mux_def,buck_def]
(HERE / 'NeoCam.kicad_sym').write_text('(kicad_symbol_lib (version 20241209) (generator "neocam_reference")\n'+'\n'.join(custom_defs)+'\n)\n',encoding='utf-8')
(HERE / 'sym-lib-table').write_text('(sym_lib_table\n (version 7)\n (lib (name "NeoCam")(type "KiCad")(uri "${KIPRJMOD}/NeoCam.kicad_sym")(options "")(descr "NeoCam reference module interfaces"))\n)\n',encoding='utf-8')

standard_names = [('Device','Battery_Cell'),('Device','R'),('Device','C'),('Device','C_Polarized'),('Device','Fuse'),('Device','LED'),('Connector','TestPoint'),('Switch','SW_Push'),('Connector_Generic','Conn_01x02'),('power','PWR_FLAG')]
symbols = {}
for library, name in standard_names:
    definition=extract_symbol(library,name)
    symbols[library+':'+name] = definition.replace('(symbol '+q(name), '(symbol '+q(library+':'+name),1)
symbols['NeoCam:'+module_name]=module_def.replace('(symbol '+q(module_name), '(symbol '+q('NeoCam:'+module_name),1)
symbols['NeoCam:'+switch_name]=switch_def.replace('(symbol '+q(switch_name), '(symbol '+q('NeoCam:'+switch_name),1)
symbols['NeoCam:Rear_Charging_Reserved']=pogo_def.replace('(symbol "Rear_Charging_Reserved"','(symbol "NeoCam:Rear_Charging_Reserved"',1)
symbols['NeoCam:Existing_R_MB9']=existing_r.replace('(symbol "Existing_R_MB9"','(symbol "NeoCam:Existing_R_MB9"',1)
symbols['NeoCam:'+mux_name]=mux_def.replace('(symbol '+q(mux_name),'(symbol '+q('NeoCam:'+mux_name),1)
for name,definition in [('Existing_C_Stock',existing_c),('Existing_L_Stock',existing_l),('Existing_FB_Stock',existing_fb),(buck_name,buck_def)]:
    symbols['NeoCam:'+name]=definition.replace('(symbol '+q(name),'(symbol '+q('NeoCam:'+name),1)

objects=[]
components=[]
connections=[]

def text(value,x,y,size=1.15):
    objects.append(f'(text {q(value)} (at {x} {y} 0) (effects (font (size {size} {size})) (justify left top)) (uuid {q(uid())}))')

def wire(a,b):
    objects.append(f'(wire (pts (xy {number(a[0])} {number(a[1])}) (xy {number(b[0])} {number(b[1])})) (stroke (width 0) (type default)) (uuid {q(uid())}))')

def junction(x,y):
    objects.append(f'(junction (at {number(x)} {number(y)}) (diameter 0) (color 0 0 0 0) (uuid {q(uid())}))')

def label(value,x,y):
    if value in SHARED_NETS:
        objects.append(f'(global_label {q(value)} (shape bidirectional) (at {number(x)} {number(y)} 0) (effects (font (size 1.1 1.1)) (justify left)) (uuid {q(uid())}))')
    else:
        objects.append(f'(label {q(value)} (at {number(x)} {number(y)} 0) (effects (font (size 1.1 1.1)) (justify left bottom)) (uuid {q(uid())}))')

def nc(x,y):
    objects.append(f'(no_connect (at {number(x)} {number(y)}) (uuid {q(uid())}))')

def sym(lib_id,ref,value,x,y,angle=0,fp='',datasheet='',dnp=False,props=None,field_side='right',existing=False):
    dx,dy=(4,-2) if field_side=='right' else (0,-7)
    if lib_id=='NeoCam:'+module_name:dy=-44
    if lib_id=='NeoCam:'+switch_name:dy=-35
    if lib_id=='NeoCam:'+mux_name:dy=-57
    if lib_id=='NeoCam:'+buck_name:dy=-44
    if ref=='QM1':dx,dy=5,6
    justify='(justify left)' if field_side=='right' else ''
    hidden_ref = ref.startswith('#')
    entry=f'''(symbol (lib_id {q(lib_id)}) (at {x} {y} {angle}) (unit 1)
    (exclude_from_sim yes) (in_bom {'no' if hidden_ref or existing else 'yes'}) (on_board {'no' if hidden_ref or existing else 'yes'}) (dnp {'yes' if dnp else 'no'})
    (uuid {q(uid())})
    {prop('Reference',ref,x+dx,y+dy,hidden_ref,justify=justify,angle=angle%180)}
    {prop('Value',value,x+dx,y+dy+2.5,hidden_ref,justify=justify,angle=angle%180)}
    {prop('Footprint',fp,x,y,True)} {prop('Datasheet',datasheet,x,y,True)}
    {''.join(prop(k,v,x,y,True) for k,v in (props or {}).items())}
    (instances (project "neocam-power-v0" (path {q(ACTIVE_PATH)} (reference {q(ref)}) (unit 1)))))'''
    objects.append(entry)
    components.append(dict(reference=ref,value=value,lib_id=lib_id,x=x,y=y,angle=angle,dnp=dnp,existing_on_xiao=existing,sheet_path=ACTIVE_PATH,manufacturer_part=(props or {}).get('ManufacturerPart',''),footprint=fp,datasheet=datasheet))

def tpoint(name,x,y):
    label(name,x,y)

RFP='Resistor_SMD:R_0603_1608Metric'
CFP='Capacitor_SMD:C_0603_1608Metric'
module_datasheet='https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/'
tmux_datasheet='https://www.ti.com/lit/ds/symlink/tmux1511.pdf'

text('NeoCam | Companion power, battery sensing and local controls',20,16,2.4)
text('REFERENCE DESIGN v0.6 | 2026-11-06 | power-mux / stock buck, sheet 1 of 3; not released for manufacture',20,23,1.3)

text('1. Protected battery and stock XIAO',20,36,1.6)
sym('Device:Battery_Cell','BT1','DNK102050 1S 1000mAh + PCM',35,67,fp='',datasheet='https://www.dnkpower.com/wp-content/uploads/2023/11/DNK102050-3.7V-1000mAh-Lipo-Battery-Specification.pdf',props={'Selection':'Technical reference only; charge/PCM coordination and procurement unresolved'})
sym('Connector_Generic:Conn_01x02','J1','JST PH2 battery',65,58,fp='Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical',field_side='top')
wire((35,61.92),(35,58));wire((35,58),(59.92,58))
wire((35,69.54),(35,76));wire((35,76),(55,76));wire((55,76),(55,60.54));wire((55,60.54),(59.92,60.54))
wire((45,58),(45,45));wire((45,45),(76.19,45));junction(45,58)
sym('Device:Fuse','F1','046701.5NR | 1.5A',80,45,90,fp='Fuse:Fuse_0603_1608Metric',datasheet='https://www.littelfuse.com/products/fuses/surface-mount-fuses/surface-mount-fuses/467.aspx',field_side='top')
wire((83.81,45),(105,45));label('VBAT_XIAO',105,45)
sym('power:PWR_FLAG','#FLG03','PWR_FLAG',105,45)
label('GND',55,76)
sym('power:PWR_FLAG','#FLG01','PWR_FLAG',45,58)
sym('power:PWR_FLAG','#FLG02','PWR_FLAG',55,76)
sym('NeoCam:'+module_name,'U1','XIAO Sense; remove Q1 + D2',160,83,field_side='top',datasheet=module_datasheet)
wire((120,58),(110,58));label('VBAT_XIAO',110,58)
wire((120,73),(110,73));label('GND',110,73)
wire((120,88),(110,88));label('GND',110,88)
wire((120,103),(90,103));label('VIN_SYS',90,103)
wire((120,118),(90,118));label('VBUS_REWORK',90,118)
wire((200,58),(215,58));label('3V3_XIAO',215,58)
wire((200,63),(215,63));label('VDD_SPI',215,63)
wire((200,73),(215,73));label('BAT_ALERT_N',215,73)
wire((200,83),(215,83));label('VDD_SD',215,83)
wire((200,88),(215,88));label('REC_N',215,88)
wire((200,98),(215,98));label('STATUS_LED',215,98)
wire((200,108),(215,108));label('GAUGE_SDA',215,108)
wire((200,118),(215,118));label('GAUGE_SCL',215,118)
text('SELECTED v0.6: keep stock buck and stock output capacitors.',250,45,1.25)
text('Remove v0.4 QM1 and R6; no diode bypass around mux.',250,51)
text('USB/BAT -> TPS2121 -> VIN_SYS on sheet 2.',250,57)
text('Stock USB-C/charger/buck retained; buck shown on sheet3.',250,63)
text('Stock VBUS -> charger -> BAT connection retained.',250,69)
text('VDD_SPI is an INTERNAL tap; do not supply it externally.',250,75)
text('Access old Q1 pads only after board revision/pin verification.',250,81)
text('Normal system current <=0.9A; hold-up budget uses 1A.',250,87)
text('VBAT_XIAO at mux IN2 >=3.8V under load.',250,93)
text('No-reset behavior is an acceptance target, not a bench result.',250,99)
text('TPs are planned access; probe local ESP RTC/CPU/analog pads after their filters.',250,105)

# The retained stock VBUS pulldown is not a new PCB component.
sym('NeoCam:Existing_R_MB9','R_MB9','100k (existing)',95,80,existing=True,field_side='right')
wire((95,76.19),(95,68));label('VBUS_REWORK',95,68)
wire((95,83.81),(95,88));label('GND',95,88)
objects.append(f'''(sheet (at 25 85) (size 50.8 25.4) (stroke (width 0.1524) (type default)) (fill (color 0 0 0 0)) (uuid {q(MUX_SHEET_UUID)})
 (property "Sheetname" "USB_Battery_Power_Mux" (at 25 83.73 0) (effects (font (size 1.27 1.27)) (justify left bottom)))
 (property "Sheetfile" "neocam-power-mux-v0.6.kicad_sch" (at 25 111.67 0) (effects (font (size 1 1)) (justify left top)))
 (instances (project "neocam-power-v0" (path {q('/'+ROOT_UUID)} (page "2")))))''')
for ref,value,x,net in [('TP1','BAT',255,'VBAT_XIAO'),('TP2','VIN',295,'VIN_SYS'),('TP3','VBUS',335,'VBUS_REWORK'),('TP4','GND',375,'GND')]:
    sym('Connector:TestPoint',ref,value,x,115,fp='TestPoint:TestPoint_Pad_D1.5mm',field_side='right')
    wire((x,115),(x,124));label(net,x,124)
sym('Connector:TestPoint','TP6','3V3_KIT_TAP',230,48,fp='TestPoint:TestPoint_Pad_D1.5mm',field_side='right')
wire((230,48),(230,58));label('3V3_XIAO',230,58)
sym('Connector:TestPoint','TP7','VDD_SPI_INTERNAL',230,68,fp='TestPoint:TestPoint_Pad_D1.5mm',field_side='right')
wire((230,68),(230,78));label('VDD_SPI',230,78)

text('2. Battery voltage and charge estimate over I2C',20,132,1.6)
objects.append(f'''(sheet (at 25 145) (size 43.18 20.32) (stroke (width 0.1524) (type default)) (fill (color 0 0 0 0)) (uuid {q(BUCK_SHEET_UUID)})
 (property "Sheetname" "3V3_Buck_and_SD" (at 25 143.73 0) (effects (font (size 1.27 1.27)) (justify left bottom)))
 (property "Sheetfile" "neocam-power-3v3-v0.6.kicad_sch" (at 25 166.59 0) (effects (font (size 1 1)) (justify left top)))
 (instances (project "neocam-power-v0" (path {q('/'+ROOT_UUID)} (page "3")))))''')
sym('NeoCam:'+switch_name,'U2','MAX17048G+',145,175,datasheet='https://www.analog.com/media/en/technical-documentation/data-sheets/MAX17048-MAX17049.pdf',field_side='top')
wire((115,160),(105,160));label('VBAT_XIAO',80,160)
wire((105,160),(105,170));wire((105,170),(115,170));junction(105,160)
sym('Device:C','C1','100nF 10V X7R',80,175,fp=CFP)
wire((80,171.19),(80,160));wire((80,160),(105,160))
wire((80,178.81),(80,205));label('GND',80,205)
wire((115,180),(105,180));label('GND',105,180)
wire((115,190),(105,190));label('GND',105,190)
wire((145,205),(145,215));wire((155,205),(155,215));wire((145,215),(155,215));label('GND',145,215)
for ref,x,end_y,net in [('R1',195,160,'GAUGE_SCL'),('R2',215,170,'GAUGE_SDA'),('R3',235,180,'BAT_ALERT_N')]:
    sym('Device:R',ref,'10k 1%' if ref=='R3' else '2.2k 1%',x,140,fp=RFP)
    wire((x,136.19),(x,130));label('3V3_XIAO',x,130)
    wire((x,143.81),(x,end_y));wire((175,end_y),(x,end_y));wire((x,end_y),(x+8,end_y));junction(x,end_y);label(net,x+8,end_y)
text('VDD and CELL sense protected battery after F1.',35,230)
text('No battery-derived analog voltage reaches ESP GPIO.',35,236)
text('SCL/SDA pull-ups use host 3V3, not the battery.',35,242)
text('Bus: 100kHz; total capacitance target <=150pF.',35,248)
text('Fuel-gauge SOC requires matching/validating cell model.',35,254)
text('Do not set safe-stop thresholds from SOC alone.',35,260)

text('3. Record button and status LED',270,132,1.6)
sym('Device:R','R4','10k 1%',290,150,fp=RFP)
wire((290,146.19),(290,140));label('3V3_XIAO',290,140)
wire((290,153.81),(290,165));wire((290,165),(290,185));label('REC_N',290,165);junction(290,165)
sym('Switch:SW_Push','SW1','B3U-1000P RECORD / STOP',300,185,fp='Button_Switch_SMD:SW_SPST_B3U-1000P',field_side='top',datasheet='https://components.omron.com/us-en/products/switches/B3U')
wire((290,185),(294.92,185));wire((305.08,185),(320,185));wire((320,185),(320,210));label('GND',320,210)
sym('Device:C','C2','100nF 10V X7R',270,185,fp=CFP)
wire((270,181.19),(270,165));wire((270,165),(290,165));wire((270,188.81),(270,210));label('GND',270,210)
text('Active LOW on D1/GPIO2; firmware debounces.',265,217)
sym('Device:R','R5','2.2k 1%',290,240,90,fp=RFP,field_side='top')
wire((286.19,240),(270,240));label('STATUS_LED',270,240)
sym('Device:LED','D1','RED 0603',315,240,180,fp='LED_SMD:LED_0603_1608Metric',field_side='top')
wire((293.81,240),(311.19,240));wire((318.81,240),(335,240));label('GND',335,240)
text('D3/GPIO4; GPIO21 reserved for microSD CS.',265,250)

text('4. Rear pogo charging - reserved only',325,141,1.4)
sym('NeoCam:Rear_Charging_Reserved','J2','POGO FUTURE / DNP',365,165,dnp=True,field_side='top')
nc(359.92,165);nc(359.92,167.54)
text('Not electrically implemented in v0.6.',325,180)
text('No connection to BAT or USB VBUS.',325,186)
text('Needs 5V protection, source ORing,',325,192)
text('reverse-current and polarity design.',325,198)

text('Operating target: >=45min standalone image + audio; Wi-Fi/BLE OFF. 720p/30FPS remains a functional target.',20,270,1.15)
text('MUX/timing/hold-up limits are qualification budgets; switching, runtime, thermal and file finalization remain unmeasured.',20,276,1.15)

main_objects=list(objects)
objects=[]
ACTIVE_PATH='/'+ROOT_UUID+'/'+MUX_SHEET_UUID
text('NeoCam | USB / battery power mux and hold-up network',20,16,2.4)
text('REFERENCE v0.6 | sheet 2 of 3 | TPS2121 XCOMP; no-reset is a qualification target',20,23,1.3)
sym('NeoCam:'+mux_name,'U3','TPS2121RUXR',220,115,datasheet='https://www.ti.com/lit/ds/symlink/tps2121.pdf',field_side='top')

# USB input and PR1 divider (1/3); stock charger remains on USB on sheet 1.
wire((60,85),(110,85));wire((110,85),(175,85));label('VBUS_REWORK',60,85)
sym('Device:C','C3','4.7uF 10V X7R',110,100,fp='Capacitor_SMD:C_0805_2012Metric')
wire((110,85),(110,96.19));junction(110,85);wire((110,103.81),(110,115));label('GND',110,115)
sym('Device:R','R7','20k 1%',65,115,fp=RFP)
sym('Device:R','R8','10k 1%',65,140,fp=RFP)
wire((65,85),(65,111.19));junction(65,85)
wire((65,118.81),(65,127.5));wire((65,127.5),(65,136.19));wire((65,127.5),(85,127.5));junction(65,127.5);label('PR1_SENSE',85,127.5)
wire((65,143.81),(65,155));label('GND',65,155)
wire((175,115),(155,115));label('PR1_SENSE',155,115)

# Protected battery input and CP2 divider (1/3).
wire((145,100),(150,100));wire((150,100),(175,100));label('VBAT_XIAO',145,100)
sym('Device:C','C4','4.7uF 10V X7R',150,125,fp='Capacitor_SMD:C_0805_2012Metric')
wire((150,100),(150,121.19));junction(150,100);wire((150,128.81),(150,140));label('GND',150,140)
sym('Device:R','R9','20k 1%',95,165,fp=RFP)
sym('Device:R','R10','10k 1%',95,190,fp=RFP)
wire((95,161.19),(95,150));label('VBAT_XIAO',95,150)
wire((95,168.81),(95,178));wire((95,178),(95,186.19));wire((95,178),(115,178));junction(95,178);label('CP2_SENSE',115,178)
wire((95,193.81),(95,205));label('GND',95,205)
wire((175,130),(155,130));label('CP2_SENSE',155,130)

# Both physical OUT pins connect to the same rail; no Schottky fallback.
wire((265,85),(280,85));wire((280,85),(310,85));wire((310,85),(330,85));wire((330,85),(350,85));wire((350,85),(370,85));wire((370,85),(390,85));label('VIN_SYS',390,85)
wire((265,100),(280,100));wire((280,100),(280,85));junction(280,85)
for ref,x,value,lib_id,footprint in [
    ('C5',310,'4.7uF 10V X7R','Device:C','Capacitor_SMD:C_0805_2012Metric'),
    ('C6',330,'100nF 10V','Device:C',CFP),
    ('C8',350,'330uF 10V','Device:C_Polarized','Capacitor_Tantalum_SMD:CP_EIA-7343-43_Kemet-X'),
    ('C9',370,'330uF 10V','Device:C_Polarized','Capacitor_Tantalum_SMD:CP_EIA-7343-43_Kemet-X'),
    ('C10',390,'330uF 10V','Device:C_Polarized','Capacitor_Tantalum_SMD:CP_EIA-7343-43_Kemet-X')]:
    sym(lib_id,ref,value,x,110,fp=footprint,props={'ManufacturerPart':'T520X337M010ATE025' if ref in ('C8','C9','C10') else 'Value/package selected; supplier code pending','Polarity':'+ to VIN_SYS; - to GND' if ref in ('C8','C9','C10') else 'Unpolarized'})
    wire((x,106.19),(x,85));junction(x,85)
    wire((x,113.81),(x,145));label('GND',x,145)

# OV1/OV2 grounded: normal regulated 5V source only; not an input-OVP design.
for xp in (200,210,220):wire((xp,165),(xp,175))
wire((200,175),(210,175));wire((210,175),(220,175));junction(210,175);label('GND',200,175)
sym('Device:R','R12','80.0k 1%',275,155,fp=RFP)
wire((265,130),(275,130));wire((275,130),(275,151.19));wire((275,158.81),(275,175));label('GND',275,175)
sym('Device:C','C7','1uF 10V X7R',255,190,fp=CFP)
wire((265,145),(265,175));wire((265,175),(255,175));wire((255,175),(255,186.19));wire((255,193.81),(255,205));label('GND',255,205)
wire((265,115),(280,115));label('MUX_ST',280,115)
sym('Device:R','R11','10k 1%',310,180,fp=RFP)
wire((310,176.19),(310,165));label('3V3_XIAO',310,165)
wire((310,183.81),(310,195));label('MUX_ST',310,195)
sym('Connector:TestPoint','TP5','MUX_STATUS',345,185,fp='TestPoint:TestPoint_Pad_D1.5mm',field_side='right')
wire((345,185),(345,195));label('MUX_ST',345,195)

text('USB -> IN1; protected BAT -> IN2; both OUT pins -> VIN_SYS.',20,222,1.2)
text('C8/C9/C10: 3x KEMET T520X337M010ATE025, polarized polymer tantalum, 25mOhm each.',20,230,1.2)
text('Bulk PLUS = VIN_SYS; MINUS = GND. Land pattern and effective capacitance require verification.',20,238,1.2)
text('ACCEPTANCE (not TI maxima): tgap<=50us, Qreverse<=20uC, Ceff_total>=750uF, ESR<=15mOhm.',20,250,1.15)
text('1A hold-up load; Icap_peak<=3A; ESL/routing20mV; normal I_SYS<=0.9A.',20,258,1.15)
text('IN2 >=3.8V loaded; modeled VIN minimum3.542V under these budgets; SS ramp <=200V/s is a target.',20,266,1.15)
text('TI 5us is TYPICAL. USB/pin no-reset, startup/inrush and reverse charge remain unqualified.',20,274,1.15)
child_objects=list(objects)
objects=[]
ACTIVE_PATH='/'+ROOT_UUID+'/'+BUCK_SHEET_UUID
text('NeoCam | retained stock 3V3 buck, Sense SD branch and rail checks',20,16,2.4)
text('REFERENCE V1.51 / v0.6 | sheet 3 of 3 | actual board revision unconfirmed; NO additional 3V3 bulk',20,23,1.3)
sym('NeoCam:'+buck_name,'U_MB3','SGM6029CYG/TR (existing)',150,105,existing=True,datasheet='https://www.sg-micro.com/rect/assets/3323f5dc-718f-4b81-b820-398ddb702989/SGM6029.pdf',field_side='top')

# Existing mainboard VIN filtering and EN=VIN.
wire((60,85),(65,85));wire((65,85),(85,85));wire((85,85),(100,85));wire((100,85),(115,85));label('VIN_SYS',60,85)
for ref,x,value in [('C_MB3',65,'2.2uF (existing)'),('C_MB4',85,'4.7uF (existing)')]:
    sym('NeoCam:Existing_C_Stock',ref,value,x,110,existing=True,props={'Source':'Seeed mainboard V1.51, retained existing component'})
    wire((x,106.19),(x,85));junction(x,85);wire((x,113.81),(x,130));label('GND',x,130)
wire((115,100),(100,100));wire((100,100),(100,85));junction(100,85)
sym('NeoCam:Existing_R_MB9','R_MB13','249k 1% (existing)',100,135,existing=True)
wire((115,115),(100,115));wire((100,115),(100,131.19));wire((100,138.81),(100,150));label('GND',100,150)
wire((150,140),(150,150));label('GND',150,150)

# Existing SW -> L4 -> OUT. VOS senses OUT, never the switching node.
sym('NeoCam:Existing_L_Stock','L_MB4','470nH (existing)',220,85,90,existing=True,field_side='top')
wire((185,85),(216.19,85))
wire((223.81,85),(240,85));wire((240,85),(270,85));wire((270,85),(295,85));wire((295,85),(330,85));label('3V3_XIAO',330,85)
wire((185,100),(240,100));wire((240,100),(240,85));junction(240,85)
for ref,x in [('C_MB5',270),('C_MB6',295)]:
    sym('NeoCam:Existing_C_Stock',ref,'4.7uF (existing)',x,110,existing=True,props={'Source':'Seeed mainboard V1.51, retained existing output capacitor'})
    wire((x,106.19),(x,85));junction(x,85);wire((x,113.81),(x,130));label('GND',x,130)
sym('power:PWR_FLAG','#FLG05','PWR_FLAG',330,85)
text('Existing IC/L/C/R retained on XIAO; no duplicate buck is fitted.',20,170,1.2)
text('EN=VIN; VSEL_MODE=249k to GND selects the reference 3.3V / auto-PSM setting.',20,178,1.15)

# Existing Sense SD bead and capacitors. Downstream SD supply is a distinct net.
text('Sense v1.2 SD branch: measure AFTER the ferrite bead',20,191,1.4)
wire((160,200),(216.19,200));label('3V3_XIAO',160,200)
sym('NeoCam:Existing_FB_Stock','FB_SENSE1','120R-1.3A (existing)',220,200,90,existing=True,field_side='top')
wire((223.81,200),(270,200));wire((270,200),(295,200));wire((295,200),(320,200));wire((320,200),(335,200));label('VDD_SD',335,200)
for ref,x,value in [('C_SENSE8',270,'100nF (existing)'),('C_SENSE9',295,'1uF (existing)')]:
    sym('NeoCam:Existing_C_Stock',ref,value,x,225,existing=True,props={'Source':'Seeed expansion v1.2 C8/C9; not inferred from capacity'})
    wire((x,221.19),(x,200));junction(x,200);wire((x,228.81),(x,245));label('GND',x,245)
sym('power:PWR_FLAG','#FLG06','PWR_FLAG',320,200);junction(320,200)
sym('Connector:TestPoint','TP8','SD_VDD_AFTER_FB',335,190,fp='TestPoint:TestPoint_Pad_D1.5mm',field_side='right')
wire((335,190),(335,200))
text('120R is a ferrite impedance marking; do not model it as 120 Ohm DC resistance.',20,255,1.15)
text('TOTAL 3V3<=600mA. Vendor Ceff5-10uF; selected qualification8-10uF counts distributed caps, not nominal9.4uF.',20,263,1.15)
text('Qualification: ESP/SD3.1-3.5V; 3.3V-memory VDD_SPI2.7-3.6V; verify actual board/memory configuration.',20,271,1.15)
text('R(high-side+L+trace)<=0.5Ohm is a requirement: at VIN3.541667V and0.6A, DC available>=3.241667V.',20,279,1.15)
buck_objects=list(objects)
objects=main_objects

header=f'''(kicad_sch (version 20260306) (generator "eeschema") (generator_version "10.0")
  (uuid {q(ROOT_UUID)}) (paper "A3")
  (title_block (title "NeoCam companion power and controls") (date "2026-11-06") (rev "0.6-3V3") (company "NeoLab") (comment 1 "Stock buck retained; not released for manufacture"))
  (lib_symbols {''.join(symbols.values())})
'''
def snap_object(match):
    return '('+match.group(1)+' '+number(snap(float(match.group(2))))+' '+number(snap(float(match.group(3))))
def orient_globals(items):
    segments=[]
    for item in items:
        if item.startswith('(wire '):
            points=re.findall(r'\(xy\s+(-?[0-9.]+)\s+(-?[0-9.]+)\)',item)
            segments.append(tuple((float(x),float(y)) for x,y in points))
    result=[]
    for item in items:
        if item.startswith('(global_label '):
            match=re.search(r'\(at\s+(-?[0-9.]+)\s+(-?[0-9.]+)\s+0\)',item)
            x,y=map(float,match.groups())
            right_neighbor=any((a==(x,y) and b[0]>x and b[1]==y) or (b==(x,y) and a[0]>x and a[1]==y) for a,b in segments)
            if right_neighbor:
                item=item[:match.start()]+f'(at {number(x)} {number(y)} 180)'+item[match.end():]
                item=item.replace('(justify left)','(justify right)',1)
        result.append(item)
    return result
objects=[re.sub(r'\((at|xy)\s+(-?[0-9]+(?:\.[0-9]+)?)\s+(-?[0-9]+(?:\.[0-9]+)?)(?=[\s)])',snap_object,obj) for obj in objects]
objects=orient_globals(objects)
(HERE/'neocam-power-v0.kicad_sch').write_text(header+'\n'.join(objects)+f'\n(sheet_instances (path "/" (page "1"))) (embedded_fonts no)\n)\n',encoding='utf-8')
child_objects=[re.sub(r'\((at|xy)\s+(-?[0-9]+(?:\.[0-9]+)?)\s+(-?[0-9]+(?:\.[0-9]+)?)(?=[\s)])',snap_object,obj) for obj in child_objects]
child_objects=orient_globals(child_objects)
child_header=header.replace(q(ROOT_UUID),q(MUX_ROOT_UUID),1).replace('NeoCam companion power and controls','NeoCam USB / battery power mux')
(HERE/'neocam-power-mux-v0.6.kicad_sch').write_text(child_header+'\n'.join(child_objects)+'\n(embedded_fonts no)\n)\n',encoding='utf-8')
buck_objects=[re.sub(r'\((at|xy)\s+(-?[0-9]+(?:\.[0-9]+)?)\s+(-?[0-9]+(?:\.[0-9]+)?)(?=[\s)])',snap_object,obj) for obj in buck_objects]
buck_objects=orient_globals(buck_objects)
buck_header=header.replace(q(ROOT_UUID),q(BUCK_ROOT_UUID),1).replace('NeoCam companion power and controls','NeoCam retained 3V3 buck and SD branch')
(HERE/'neocam-power-3v3-v0.6.kicad_sch').write_text(buck_header+'\n'.join(buck_objects)+'\n(embedded_fonts no)\n)\n',encoding='utf-8')
(HERE/'components-reference.json').write_text(json.dumps(components,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(f'Generated {len(components)} symbols and {len(objects)} schematic objects')

