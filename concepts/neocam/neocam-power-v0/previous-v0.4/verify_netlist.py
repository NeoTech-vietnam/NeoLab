"""Independently check critical connections in the KiCad exported netlist."""
from pathlib import Path
import re
import json

here=Path(__file__).resolve().parent
tokens=re.findall(r'"(?:\\.|[^"\\])*"|[()]|[^\s()]+',(here/'exports/neocam-power-v0-review-full.net').read_text(encoding='utf-8'))
i=0
def read():
    global i
    token=tokens[i];i+=1
    if token=='(':
        result=[]
        while tokens[i]!=')':result.append(read())
        i+=1
        return result
    return json.loads(token) if token.startswith('"') else token
tree=read()
def children(node,key):return [x for x in node if isinstance(x,list) and x and x[0]==key]
def val(node,key):return children(node,key)[0][1]
nets={}
for net in children(children(tree,'nets')[0],'net'):
    nets[val(net,'name').lstrip('/')]=set((val(n,'ref'),val(n,'pin')) for n in children(net,'node'))
expected={
 '3V3_XIAO':{('U1','3V3'),('R1','1'),('R2','1'),('R3','1'),('R4','1')},
 'VBAT_XIAO':{('F1','2'),('U1','B+'),('U2','2'),('U2','3'),('C1','1'),('QM1','3'),('TP1','1')},
 'VIN_REWORK':{('QM1','2'),('U1','Q1_S3'),('D_MB2','1'),('TP2','1')},
 'VBUS_REWORK':{('QM1','1'),('U1','Q1_G2'),('D_MB2','2'),('R_MB9','1'),('R6','1'),('TP3','1')},
 'GAUGE_SCL':{('U1','D5'),('U2','7'),('R1','2')},
 'GAUGE_SDA':{('U1','D4'),('U2','8'),('R2','2')},
 'BAT_ALERT_N':{('U1','D0'),('U2','5'),('R3','2')},
 'REC_N':{('U1','D1'),('R4','2'),('C2','1'),('SW1','1')},
 'STATUS_LED':{('U1','D3'),('R5','1')},
}
for name, pins in expected.items():
    actual={p for p in nets[name] if not p[0].startswith('#')}
    assert actual==pins,(name,actual,pins)
gnd=nets['GND']
required_gnd={('BT1','2'),('J1','2'),('U1','B-'),('U1','GND'),('U2','1'),('U2','4'),('U2','6'),('U2','9'),('C1','2'),('C2','2'),('SW1','2'),('D1','1'),('R_MB9','2'),('R6','2'),('TP4','1')}
assert required_gnd<=gnd,required_gnd-gnd
pack_net=next(p for p in nets.values() if ('BT1','1') in p)
assert {('BT1','1'),('J1','1'),('F1','1')}<=pack_net
assert ('U1','B+') not in pack_net, 'F1 bypassed'
led_net=next(p for p in nets.values() if ('D1','2') in p)
assert led_net=={('D1','2'),('R5','2')},led_net
for pin in ('1','2'):
    pogo_net=next(p for p in nets.values() if ('J2',pin) in p)
    assert pogo_net=={('J2',pin)}, 'Reserved pogo connected'
# Verify the actual native export too: its only omitted graph nodes here are
# the two retained stock components deliberately excluded from board placement.
tokens=re.findall(r'"(?:\\.|[^"\\])*"|[()]|[^\s()]+',(here/'exports/neocam-power-v0.net').read_text(encoding='utf-8'));i=0
native=read()
native_nets={val(n,'name').lstrip('/'):set((val(p,'ref'),val(p,'pin')) for p in children(n,'node')) for n in children(children(native,'nets')[0],'net')}
for name,pins in expected.items():
    native_expected={p for p in pins if p[0] not in ('D_MB2','R_MB9')}
    actual={p for p in native_nets[name] if not p[0].startswith('#')}
    assert actual==native_expected,('native',name,actual,native_expected)
assert (required_gnd-{('R_MB9','2')})<=native_nets['GND']

# Check the main schematic's retained-part metadata and real library pin map.
tokens=re.findall(r'"(?:\\.|[^"\\])*"|[()]|[^\s()]+',(here/'neocam-power-v0.kicad_sch').read_text(encoding='utf-8'));i=0
schematic=read()
instances={next(p[2] for p in children(s,'property') if p[1]=='Reference'):s for s in children(schematic,'symbol')}
definitions={s[1]:s for s in children(children(schematic,'lib_symbols')[0],'symbol')}
for ref in ('D_MB2','R_MB9'):
    assert val(instances[ref],'on_board')=='no'
    assert val(instances[ref],'in_bom')=='no'
def pinmap(lib_id):
    return {val(p,'number'):val(p,'name') for unit in children(definitions[lib_id],'symbol') for p in children(unit,'pin')}
assert pinmap(val(instances['QM1'],'lib_id'))=={'1':'G','2':'S','3':'D'}
assert pinmap(val(instances['D_MB2'],'lib_id'))=={'1':'K','2':'A'}
assert set(pinmap(val(instances['R_MB9'],'lib_id')))=={'1','2'}
module_pins=pinmap(val(instances['U1'],'lib_id'))
assert module_pins['Q1_S3']=='VIN_Q1_SOURCE_PAD'
assert module_pins['Q1_G2']=='VBUS_Q1_GATE_PAD'
manifest=json.loads((here/'exports/review-netlist-manifest.json').read_text())
assert manifest['normalized_sources_identical'] and manifest['source_normalized_sha256']==manifest['review_source_normalized_sha256']
report='PASS: native netlist + 9 exact full-review net checks; native schematic QM1 G1/S2/D3 and D2 K1/A2 pin maps; retained D2/R9 excluded from BOM/board; normalized source/review hashes identical after only two documented placement booleans; protected return, fuse, LED and reserved pogo checks.\n'
(here/'exports/connectivity-check.txt').write_text(report,encoding='utf-8')
print(report,end='')
