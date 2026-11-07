"""Independently check critical connections in the KiCad exported netlist."""
from pathlib import Path
import re
import json

here=Path(__file__).resolve().parent
tokens=re.findall(r'"(?:\\.|[^"\\])*"|[()]|[^\s()]+',(here/'exports/neocam-power-v0.net').read_text(encoding='utf-8'))
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
 'VBAT_XIAO':{('F1','2'),('U1','B+'),('U2','2'),('U2','3'),('C1','1')},
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
required_gnd={('BT1','2'),('J1','2'),('U1','B-'),('U1','GND'),('U2','1'),('U2','4'),('U2','6'),('U2','9'),('C1','2'),('C2','2'),('SW1','2'),('D1','1')}
assert required_gnd<=gnd,required_gnd-gnd
pack_net=next(p for p in nets.values() if ('BT1','1') in p)
assert {('BT1','1'),('J1','1'),('F1','1')}<=pack_net
assert ('U1','B+') not in pack_net, 'F1 bypassed'
led_net=next(p for p in nets.values() if ('D1','2') in p)
assert led_net=={('D1','2'),('R5','2')},led_net
for pin in ('1','2'):
    pogo_net=next(p for p in nets.values() if ('J2',pin) in p)
    assert pogo_net=={('J2',pin)}, 'Reserved pogo connected'
report='PASS: 7 exact named-net checks; protected return; fuse not bypassed; LED polarity; both reserved pogo pins isolated.\n'
(here/'exports/connectivity-check.txt').write_text(report,encoding='utf-8')
print(report,end='')
