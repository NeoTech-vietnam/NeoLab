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

def uid():
    return str(uuid.uuid4())

def q(value):
    return json.dumps(str(value), ensure_ascii=False)

def number(value):
    return f"{value:.4f}".rstrip("0").rstrip(".") if value else "0"

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

def prop(name, value, x, y, hide=False, size=1.1, justify=""):
    return f'(property {q(name)} {q(value)} (at {number(x)} {number(y)} 0) ' + \
        ('(hide yes) ' if hide else '') + \
        f'(effects (font (size {size} {size})) {justify}))'

def rect(x1, y1, x2, y2):
    return f'(rectangle (start {x1} {y1}) (end {x2} {y2}) (stroke (width 0.254) (type default)) (fill (type background)))'

def pin(name, num, x, y, angle, kind="passive", length=5):
    return f'(pin {kind} line (at {x} {y} {angle}) (length {length}) (name {q(name)} (effects (font (size 1 1)))) (number {q(num)} (effects (font (size 1 1)))))'

def libtext(text, x, y, size=1.1):
    return f'(text {q(text)} (at {x} {y} 0) (effects (font (size {size} {size}))))'

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
    ("3V3", "3V3", 40, 25, 180, "power_out"),
    ("D0 / GPIO1 / ADC1_CH0", "D0", 40, 10, 180, "input"),
    ("D1 / GPIO2 / REC_N", "D1", 40, -5, 180, "input"),
    ("D3 / GPIO4 / LED", "D3", 40, -20, 180, "output"),
]
module_def = custom(module_name,
    rect(-35,35,35,-35) +
    libtext('STOCK XIAO + SENSE',0,28,1.5) +
    libtext('USB-C, charger and buck onboard',0,19,1.05) +
    libtext('ESP32-S3 + OV3660',0,3,1.4) +
    libtext('microphone + microSD',0,-3,1.15) +
    libtext('USB charging only in this revision',0,-28,1.0),
    [pin(*p) for p in module_pins],
    'Module interface, not a replacement for the manufacturer schematic. Pin IDs are board pad names; no footprint released.')

switch_name = "TMUX1511PWR"
switch_pins = [
    ("VDD", "14", 0, 30, 270, "power_in"),
    ("S1", "2", -25, 15, 0, "passive"),
    ("D1", "3", 25, 15, 180, "passive"),
    ("SEL1", "1", -25, 0, 0, "input"),
    ("SEL2", "4", -15, -40, 90, "input"),
    ("S2", "5", -10, -40, 90, "passive"),
    ("D2", "6", -5, -40, 90, "passive"),
    ("GND", "7", 0, -40, 90, "power_in"),
    ("D3", "8", 5, -40, 90, "passive"),
    ("S3", "9", 10, -40, 90, "passive"),
    ("SEL3", "10", 15, -40, 90, "input"),
    ("D4", "11", 25, -15, 180, "passive"),
    ("S4", "12", 25, -20, 180, "passive"),
    ("SEL4", "13", 25, -25, 180, "input"),
]
switch_def = custom(switch_name,
    rect(-20,25,20,-35) + libtext('CH1',0,15) + libtext('S1 <-> D1',0,9) +
    libtext('Powered-off',0,-8) + libtext('isolation',0,-14),
    [pin(*p) for p in switch_pins],
    'TI TMUX1511PWR, TSSOP-14. Signal powered-off protection up to 3.6V at VDD=0.',
    'Package_SO:TSSOP-14_4.4x5mm_P0.65mm')

custom_defs = [module_def, switch_def]
(HERE / 'NeoCam.kicad_sym').write_text('(kicad_symbol_lib (version 20241209) (generator "neocam_reference")\n'+'\n'.join(custom_defs)+'\n)\n',encoding='utf-8')
(HERE / 'sym-lib-table').write_text('(sym_lib_table\n (version 7)\n (lib (name "NeoCam")(type "KiCad")(uri "${KIPRJMOD}/NeoCam.kicad_sym")(options "")(descr "NeoCam reference module interfaces"))\n)\n',encoding='utf-8')

standard_names = [('Device','Battery_Cell'),('Device','R'),('Device','C'),('Device','Fuse'),('Device','LED'),('Switch','SW_Push'),('Connector_Generic','Conn_01x02'),('power','PWR_FLAG')]
symbols = {}
for library, name in standard_names:
    definition=extract_symbol(library,name)
    symbols[library+':'+name] = definition.replace('(symbol '+q(name), '(symbol '+q(library+':'+name),1)
symbols['NeoCam:'+module_name]=module_def.replace('(symbol '+q(module_name), '(symbol '+q('NeoCam:'+module_name),1)
symbols['NeoCam:'+switch_name]=switch_def.replace('(symbol '+q(switch_name), '(symbol '+q('NeoCam:'+switch_name),1)

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
    objects.append(f'(label {q(value)} (at {number(x)} {number(y)} 0) (effects (font (size 1.1 1.1)) (justify left bottom)) (uuid {q(uid())}))')

def nc(x,y):
    objects.append(f'(no_connect (at {number(x)} {number(y)}) (uuid {q(uid())}))')

def sym(lib_id,ref,value,x,y,angle=0,fp='',datasheet='',dnp=False,props=None,field_side='right'):
    dx,dy=(4,-2) if field_side=='right' else (0,-7)
    justify='(justify left)' if field_side=='right' else ''
    hidden_ref = ref.startswith('#')
    entry=f'''(symbol (lib_id {q(lib_id)}) (at {x} {y} {angle}) (unit 1)
    (exclude_from_sim yes) (in_bom {'no' if hidden_ref else 'yes'}) (on_board {'no' if hidden_ref else 'yes'}) (dnp {'yes' if dnp else 'no'})
    (uuid {q(uid())})
    {prop('Reference',ref,x+dx,y+dy,hidden_ref,justify=justify)}
    {prop('Value',value,x+dx,y+dy+2.5,hidden_ref,justify=justify)}
    {prop('Footprint',fp,x,y,True)} {prop('Datasheet',datasheet,x,y,True)}
    {''.join(prop(k,v,x,y,True) for k,v in (props or {}).items())}
    (instances (project "neocam-power-v0" (path {q('/'+ROOT_UUID)} (reference {q(ref)}) (unit 1)))))'''
    objects.append(entry)
    components.append(dict(reference=ref,value=value,lib_id=lib_id,x=x,y=y,angle=angle,dnp=dnp,footprint=fp,datasheet=datasheet))

def tpoint(name,x,y):
    label(name,x,y)

RFP='Resistor_SMD:R_0603_1608Metric'
CFP='Capacitor_SMD:C_0603_1608Metric'
module_datasheet='https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/'
tmux_datasheet='https://www.ti.com/lit/ds/symlink/tmux1511.pdf'

text('NeoCam | Companion power, battery sensing and local controls',20,16,2.4)
text('REFERENCE DESIGN v0.2 | 2026-11-06 | schematic only; not released for manufacture',20,23,1.3)

text('1. Protected battery and stock XIAO',20,36,1.6)
sym('Device:Battery_Cell','BT1','DNK102050 1S 1000mAh + PCM',35,67,fp='',datasheet='https://www.dnkpower.com/wp-content/uploads/2023/11/DNK102050-3.7V-1000mAh-Lipo-Battery-Specification.pdf',props={'Selection':'Technical reference only; charge/PCM coordination and procurement unresolved'})
sym('Connector_Generic:Conn_01x02','J1','JST PH2 battery',65,58,fp='Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical',field_side='top')
wire((35,61.92),(35,58));wire((35,58),(59.92,58))
wire((35,69.54),(35,76));wire((35,76),(55,76));wire((55,76),(55,60.54));wire((55,60.54),(59.92,60.54))
wire((45,58),(45,45));wire((45,45),(76.19,45));junction(45,58)
sym('Device:Fuse','F1','046701.5NR | 1.5A',80,45,90,fp='Fuse:Fuse_0603_1608Metric',datasheet='https://www.littelfuse.com/products/fuses/surface-mount-fuses/surface-mount-fuses/467.aspx',field_side='top')
wire((83.81,45),(105,45));label('VBAT_XIAO',105,45)
label('PACK_NEG',55,76)
sym('power:PWR_FLAG','#FLG01','PWR_FLAG',45,58)
sym('power:PWR_FLAG','#FLG02','PWR_FLAG',55,76)
sym('NeoCam:'+module_name,'U1','XIAO ESP32-S3 Sense (stock)',160,83,field_side='top',datasheet=module_datasheet)
wire((120,58),(110,58));label('VBAT_XIAO',110,58)
wire((120,73),(110,73));label('PACK_NEG',110,73)
wire((120,88),(110,88));label('GND',110,88)
wire((110,73),(105,73));wire((105,73),(105,88));wire((105,88),(110,88))
junction(110,73);junction(110,88)
wire((200,58),(215,58));label('3V3_XIAO',215,58)
wire((200,73),(215,73));label('VBAT_ADC',215,73)
wire((200,88),(215,88));label('REC_N',215,88)
wire((200,103),(215,103));label('STATUS_LED',215,103)
text('USB-C is on U1. Use regulated 5V USB source.',250,48)
text('Stock charger and 3.3V buck retained.',250,54)
text('Mainboard reference V1.51; expansion reference v1.2.',250,60)
text('Actual expansion v1.1 equivalence remains to verify.',250,66)
text('No parallel external charger on BAT pads.',250,76)
text('No hard power switch: stop -> flush/sync/close -> sleep.',250,82)
text('F1 supplements PCM; it is not a 1A current limiter.',250,88)
text('All returns use protected PACK_NEG; never cell B-.',250,94)
text('U1 pin IDs denote pad names, not package pin numbers.',250,100)
text('No U1 footprint or PCB placement is approved.',250,106)

text('2. Battery sensing with powered-off isolation',20,123,1.6)
sym('Device:R','R1','100k 1%',75,145,fp=RFP)
sym('Device:R','R2','100k 1%',75,180,fp=RFP)
sym('Device:C','C1','100nF 10V X7R',100,180,fp=CFP)
wire((75,141.19),(75,135));label('VBAT_XIAO',75,135)
wire((75,148.81),(75,160));wire((75,160),(75,176.19))
wire((75,160),(100,160));wire((100,160),(120,160));wire((100,160),(100,176.19));junction(75,160);junction(100,160)
label('DIV_FILTERED',100,160)
wire((75,183.81),(75,205));wire((100,183.81),(100,205));wire((75,205),(100,205));label('GND',75,205)
sym('NeoCam:'+switch_name,'U2','TMUX1511PWR',145,175,fp='Package_SO:TSSOP-14_4.4x5mm_P0.65mm',datasheet=tmux_datasheet,field_side='top')
wire((145,145),(145,135));label('3V3_XIAO',145,135)
wire((120,175),(110,175));label('3V3_XIAO',110,175)
wire((170,160),(190,160));wire((190,160),(205,160));label('VBAT_ADC',205,160)
sym('Device:R','R3','100k 1%',190,180,fp=RFP)
wire((190,160),(190,176.19));junction(190,160)
wire((190,183.81),(190,210));label('GND',190,210)
sym('Device:C','C2','100nF 10V X7R',210,140,fp=CFP)
wire((210,136.19),(210,130));label('3V3_XIAO',210,130)
wire((210,143.81),(210,150));label('GND',210,150)
for xp in (130,135,140,145,150,155,160):
    wire((xp,215),(xp,220))
    if xp not in (130,160):junction(xp,220)
wire((130,220),(160,220));wire((160,220),(180,220));junction(160,220)
for yp in (190,195,200):
    wire((170,yp),(180,yp))
wire((180,190),(180,195));wire((180,195),(180,200));wire((180,200),(180,220));junction(180,195);junction(180,200)
label('GND',180,220)
text('U2 CH1 ON with 3V3; CH2-4 disabled and tied GND.',35,230)
text('C1 is before U2. No reservoir capacitor on ADC side.',35,236)
text('ON: R2 || R3 = 50k; VBAT_ADC ~= VBAT_XIAO / 3.',35,242)
text('OFF: U2 isolates DIV; R3 discharges the ADC node.',35,248)
text('Firmware: ADC1_CH0, calibration; no ADC pull-up.',35,254)
text('Transition sequencing and leakage require validation.',35,260)

text('3. Record button and status LED',245,123,1.6)
sym('Device:R','R4','10k 1%',265,145,fp=RFP)
wire((265,141.19),(265,135));label('3V3_XIAO',265,135)
wire((265,148.81),(265,160));wire((265,160),(265,180));label('REC_N',265,160);junction(265,160)
sym('Switch:SW_Push','SW1','RECORD / STOP',275,180,fp='',field_side='top')
wire((265,180),(269.92,180));wire((280.08,180),(295,180));wire((295,180),(295,205));label('GND',295,205)
sym('Device:C','C3','100nF 10V X7R',245,180,fp=CFP)
wire((245,176.19),(245,160));wire((245,160),(265,160));wire((245,183.81),(245,205));label('GND',245,205)
text('Active LOW on D1/GPIO2; firmware debounces.',245,211)
sym('Device:R','R5','2.2k 1%',265,230,90,fp=RFP,field_side='top')
wire((261.19,230),(245,230));label('STATUS_LED',245,230)
sym('Device:LED','D1','GREEN 0603',290,230,180,fp='LED_SMD:LED_0603_1608Metric',field_side='top')
wire((268.81,230),(286.19,230));wire((293.81,230),(310,230));label('GND',310,230)
text('D3/GPIO4. Keep GPIO21 free for microSD CS.',245,240)
text('Use distinct recording/saving/error patterns.',245,246)

text('4. Rear pogo charging - reserved only',325,141,1.4)
sym('Connector_Generic:Conn_01x02','J2','POGO FUTURE / DNP',365,165,dnp=True,field_side='top')
nc(359.92,165);nc(359.92,167.54)
text('Not electrically implemented in v0.2.',325,180)
text('No connection to BAT or USB VBUS.',325,186)
text('Needs 5V protection, source ORing,',325,192)
text('reverse-current and polarity design.',325,198)

text('Operating target: >=45min standalone image + audio; Wi-Fi/BLE OFF. 720p/30FPS remains a functional target.',20,278,1.15)
text('Battery/PCM reference is conditional. Runtime, brownout, thermal, charging and safe file finalization are not measured.',20,284,1.15)

header=f'''(kicad_sch (version 20260306) (generator "eeschema") (generator_version "10.0")
  (uuid {q(ROOT_UUID)}) (paper "A3")
  (title_block (title "NeoCam companion power and controls") (date "2026-11-06") (rev "0.2-reference") (company "NeoLab") (comment 1 "Not released for manufacture"))
  (lib_symbols {''.join(symbols.values())})
'''
(HERE/'neocam-power-v0.kicad_sch').write_text(header+'\n'.join(objects)+f'\n(sheet_instances (path "/" (page "1"))) (embedded_fonts no)\n)\n',encoding='utf-8')
(HERE/'components-reference.json').write_text(json.dumps(components,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(f'Generated {len(components)} symbols and {len(objects)} schematic objects')
