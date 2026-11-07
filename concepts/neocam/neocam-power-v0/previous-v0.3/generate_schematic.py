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
    ("D0 / GPIO1 / ALERT_N", "D0", 40, 10, 180, "input"),
    ("D1 / GPIO2 / REC_N", "D1", 40, -5, 180, "input"),
    ("D3 / GPIO4 / LED", "D3", 40, -15, 180, "output"),
    ("D4 / GPIO5 / SDA", "D4", 40, -25, 180, "bidirectional"),
    ("D5 / GPIO6 / SCL", "D5", 40, -35, 180, "output"),
]
PIN_BASE = (160,83)
module_def = custom(module_name,
    rect(-34.29,35.56,35.56,-40.64) +
    libtext('STOCK XIAO + SENSE',0,28,1.5) +
    libtext('USB-C, charger and buck onboard',0,19,1.05) +
    libtext('ESP32-S3 + OV3660',0,3,1.4) +
    libtext('microphone + microSD',0,-3,1.15),
    [pin(*p) for p in module_pins],
    'Module interface, not a replacement for the manufacturer schematic. Pin IDs are board pad names; no footprint released.')

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
custom_defs = [module_def, switch_def,pogo_def]
(HERE / 'NeoCam.kicad_sym').write_text('(kicad_symbol_lib (version 20241209) (generator "neocam_reference")\n'+'\n'.join(custom_defs)+'\n)\n',encoding='utf-8')
(HERE / 'sym-lib-table').write_text('(sym_lib_table\n (version 7)\n (lib (name "NeoCam")(type "KiCad")(uri "${KIPRJMOD}/NeoCam.kicad_sym")(options "")(descr "NeoCam reference module interfaces"))\n)\n',encoding='utf-8')

standard_names = [('Device','Battery_Cell'),('Device','R'),('Device','C'),('Device','Fuse'),('Device','LED'),('Switch','SW_Push'),('Connector_Generic','Conn_01x02'),('power','PWR_FLAG')]
symbols = {}
for library, name in standard_names:
    definition=extract_symbol(library,name)
    symbols[library+':'+name] = definition.replace('(symbol '+q(name), '(symbol '+q(library+':'+name),1)
symbols['NeoCam:'+module_name]=module_def.replace('(symbol '+q(module_name), '(symbol '+q('NeoCam:'+module_name),1)
symbols['NeoCam:'+switch_name]=switch_def.replace('(symbol '+q(switch_name), '(symbol '+q('NeoCam:'+switch_name),1)
symbols['NeoCam:Rear_Charging_Reserved']=pogo_def.replace('(symbol "Rear_Charging_Reserved"','(symbol "NeoCam:Rear_Charging_Reserved"',1)

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
    if lib_id=='NeoCam:'+module_name:dy=-44
    if lib_id=='NeoCam:'+switch_name:dy=-35
    justify='(justify left)' if field_side=='right' else ''
    hidden_ref = ref.startswith('#')
    entry=f'''(symbol (lib_id {q(lib_id)}) (at {x} {y} {angle}) (unit 1)
    (exclude_from_sim yes) (in_bom {'no' if hidden_ref else 'yes'}) (on_board {'no' if hidden_ref else 'yes'}) (dnp {'yes' if dnp else 'no'})
    (uuid {q(uid())})
    {prop('Reference',ref,x+dx,y+dy,hidden_ref,justify=justify,angle=angle%180)}
    {prop('Value',value,x+dx,y+dy+2.5,hidden_ref,justify=justify,angle=angle%180)}
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
text('REFERENCE DESIGN v0.3 | 2026-11-06 | schematic only; not released for manufacture',20,23,1.3)

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
sym('NeoCam:'+module_name,'U1','XIAO ESP32-S3 Sense (stock)',160,83,field_side='top',datasheet=module_datasheet)
wire((120,58),(110,58));label('VBAT_XIAO',110,58)
wire((120,73),(110,73));label('GND',110,73)
wire((120,88),(110,88));label('GND',110,88)
wire((110,73),(105,73));wire((105,73),(105,88));wire((105,88),(110,88))
junction(110,73);junction(110,88)
wire((200,58),(215,58));label('3V3_XIAO',215,58)
wire((200,73),(215,73));label('BAT_ALERT_N',215,73)
wire((200,88),(215,88));label('REC_N',215,88)
wire((200,98),(215,98));label('STATUS_LED',215,98)
wire((200,108),(215,108));label('GAUGE_SDA',215,108)
wire((200,118),(215,118));label('GAUGE_SCL',215,118)
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

text('2. Battery voltage and charge estimate over I2C',20,132,1.6)
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
text('Not electrically implemented in v0.3.',325,180)
text('No connection to BAT or USB VBUS.',325,186)
text('Needs 5V protection, source ORing,',325,192)
text('reverse-current and polarity design.',325,198)

text('Operating target: >=45min standalone image + audio; Wi-Fi/BLE OFF. 720p/30FPS remains a functional target.',20,270,1.15)
text('Battery/PCM reference is conditional. Runtime, brownout, thermal, charging and safe file finalization are not measured.',20,276,1.15)

header=f'''(kicad_sch (version 20260306) (generator "eeschema") (generator_version "10.0")
  (uuid {q(ROOT_UUID)}) (paper "A3")
  (title_block (title "NeoCam companion power and controls") (date "2026-11-06") (rev "0.3-reference") (company "NeoLab") (comment 1 "Not released for manufacture"))
  (lib_symbols {''.join(symbols.values())})
'''
def snap_object(match):
    return '('+match.group(1)+' '+number(snap(float(match.group(2))))+' '+number(snap(float(match.group(3))))
objects=[re.sub(r'\((at|xy)\s+(-?[0-9]+(?:\.[0-9]+)?)\s+(-?[0-9]+(?:\.[0-9]+)?)(?=[\s)])',snap_object,obj) for obj in objects]
(HERE/'neocam-power-v0.kicad_sch').write_text(header+'\n'.join(objects)+f'\n(sheet_instances (path "/" (page "1"))) (embedded_fonts no)\n)\n',encoding='utf-8')
(HERE/'components-reference.json').write_text(json.dumps(components,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(f'Generated {len(components)} symbols and {len(objects)} schematic objects')
