"""Check the native two-sheet mux design, full review export and presentation flags."""
from pathlib import Path
import hashlib
import json
import re

here=Path(__file__).resolve().parent

def parse(text):
    tokens=re.findall(r'"(?:\\.|[^"\\])*"|[()]|[^\s()]+',text)
    index=0
    def read():
        nonlocal index
        token=tokens[index];index+=1
        if token=='(':
            result=[]
            while tokens[index]!=')':result.append(read())
            index+=1
            return result
        return json.loads(token) if token.startswith('"') else token
    return read()
def children(node,key):return [x for x in node if isinstance(x,list) and x and x[0]==key]
def val(node,key):return children(node,key)[0][1]
def nets_from(tree):
    return {val(n,'name').split('/')[-1]:set((val(p,'ref'),val(p,'pin')) for p in children(n,'node')) for n in children(children(tree,'nets')[0],'net')}
def instance_map(tree):
    return {next(p[2] for p in children(s,'property') if p[1]=='Reference'):s for s in children(tree,'symbol')}
def prop(instance,name):return next(p[2] for p in children(instance,'property') if p[1]==name)

expected={
 '3V3_XIAO':{('U1','3V3'),('R1','1'),('R2','1'),('R3','1'),('R4','1'),('R11','1')},
 'VBAT_XIAO':{('F1','2'),('U1','B+'),('U2','2'),('U2','3'),('C1','1'),('TP1','1'),('U3','2'),('C4','1'),('R9','1')},
 'VIN_SYS':{('U1','Q1_S3'),('TP2','1'),('U3','1'),('U3','8'),('C5','1'),('C6','1'),('C8','1'),('C9','1'),('C10','1')},
 'VBUS_REWORK':{('U1','Q1_G2'),('R_MB9','1'),('TP3','1'),('U3','7'),('C3','1'),('R7','1')},
 'PR1_SENSE':{('U3','6'),('R7','2'),('R8','1')},
 'CP2_SENSE':{('U3','3'),('R9','2'),('R10','1')},
 'MUX_ST':{('U3','9'),('R11','2'),('TP5','1')},
 'GAUGE_SCL':{('U1','D5'),('U2','7'),('R1','2')},
 'GAUGE_SDA':{('U1','D4'),('U2','8'),('R2','2')},
 'BAT_ALERT_N':{('U1','D0'),('U2','5'),('R3','2')},
 'REC_N':{('U1','D1'),('R4','2'),('C2','1'),('SW1','1')},
 'STATUS_LED':{('U1','D3'),('R5','1')},
}
required_gnd={('BT1','2'),('J1','2'),('U1','B-'),('U1','GND'),('U2','1'),('U2','4'),('U2','6'),('U2','9'),('C1','2'),('C2','2'),('SW1','2'),('D1','1'),('R_MB9','2'),('TP4','1'),('U3','4'),('U3','5'),('U3','12'),('R8','2'),('R10','2'),('R12','2')}
required_gnd|={(c,'2') for c in ('C3','C4','C5','C6','C7','C8','C9','C10')}
for filename,is_native in [('neocam-power-v0.net',True),('neocam-power-v0-review-full.net',False)]:
    nets=nets_from(parse((here/'exports'/filename).read_text(encoding='utf-8')))
    for name,pins in expected.items():
        wanted=pins-{('R_MB9','1')} if is_native else pins
        actual={p for p in nets[name] if not p[0].startswith('#')}
        assert actual==wanted,(filename,name,actual,wanted)
    assert (required_gnd-({('R_MB9','2')} if is_native else set()))<=nets['GND']
    assert next(n for n in nets.values() if ('U3','10') in n)=={('U3','10'),('R12','1')}
    assert next(n for n in nets.values() if ('U3','11') in n)=={('U3','11'),('C7','1')}
    assert next(n for n in nets.values() if ('D1','2') in n)=={('D1','2'),('R5','2')}
    pack=next(n for n in nets.values() if ('BT1','1') in n)
    assert {('BT1','1'),('J1','1'),('F1','1')}<=pack and ('U1','B+') not in pack
    for p in ('1','2'):assert next(n for n in nets.values() if ('J2',p) in n)=={('J2',p)}
    assert not any(ref in ('QM1','R6','D_MB2') for n in nets.values() for ref,pin in n)

root=parse((here/'neocam-power-v0.kicad_sch').read_text(encoding='utf-8'))
child=parse((here/'neocam-power-mux-v0.5.kicad_sch').read_text(encoding='utf-8'))
instances=instance_map(root)|instance_map(child)
assert not any(ref in instances for ref in ('QM1','R6','D_MB2'))
assert val(instances['R_MB9'],'on_board')=='no' and val(instances['R_MB9'],'in_bom')=='no'
definitions={s[1]:s for t in (root,child) for s in children(children(t,'lib_symbols')[0],'symbol')}
def pinmap(lib_id):return {val(p,'number'):val(p,'name') for unit in children(definitions[lib_id],'symbol') for p in children(unit,'pin')}
assert pinmap(val(instances['U3'],'lib_id'))=={'7':'IN1','2':'IN2','6':'PR1','3':'CP2','1':'OUT','8':'OUT','9':'ST','10':'ILIM','11':'SS','5':'OV1','4':'OV2','12':'GND'}
assert prop(instances['R7'],'Value')=='20k 1%' and prop(instances['R8'],'Value')=='10k 1%'
assert prop(instances['R9'],'Value')=='20k 1%' and prop(instances['R10'],'Value')=='10k 1%'
assert prop(instances['R12'],'Value')=='80.0k 1%' and prop(instances['C7'],'Value')=='1uF 10V X7R'
for ref in ('C8','C9','C10'):
    assert prop(instances[ref],'ManufacturerPart')=='T520X337M010ATE025'
    assert prop(instances[ref],'Polarity')=='+ to VIN_SYS; - to GND'
manifest=json.loads((here/'exports/review-netlist-manifest.json').read_text())
assert manifest['normalized_sources_identical']
assert manifest['source_normalized_sha256']==manifest['review_source_normalized_sha256']
assert manifest['main_source_sha256']==hashlib.sha256((here/'neocam-power-v0.kicad_sch').read_bytes()).hexdigest()
assert manifest['unchanged_child_source_sha256']==hashlib.sha256((here/'neocam-power-mux-v0.5.kicad_sch').read_bytes()).hexdigest()
report='PASS: native and review two-sheet graphs; 12 exact named nets; TPS2121 all12pins, bothOUTpins, PR1/CP2 dividers, ILIM/SS, STtestpoint, 3polarizedbulkcapacitors; removedQM1/R6/D2 absent; retainedR9 flags; sourcehashes; protectedreturn/fuse/LED/pogo.\n'
(here/'exports/connectivity-check.txt').write_text(report,encoding='utf-8')
print(report,end='')
