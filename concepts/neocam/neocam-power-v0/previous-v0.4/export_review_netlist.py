"""Export retained-module component nodes for connectivity review only.

KiCad's board netlist omits parts flagged on_board=no. The main schematic keeps
that flag for retained stock D2/R9, so a future PCB import does not add duplicates.
This review export changes ONLY those two placement flags in a temporary copy.
Electrical pins, wires, labels and BOM flags remain unchanged.
"""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import tempfile

here=Path(__file__).resolve().parent
source=here/'neocam-power-v0.kicad_sch'
data=source.read_text(encoding='utf-8')
replacements=[]
for match in re.finditer(r'\(symbol\s+\(lib_id\s',data):
    start=match.start();depth=0;inside=False;escape=False
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
            if depth==0:break
    block=data[start:end+1]
    for ref in ('D_MB2','R_MB9'):
        if f'(property "Reference" "{ref}"' in block:
            assert '(on_board no)' in block
            assert '(in_bom no)' in block
            replacements.append((start,end+1,block.replace('(on_board no)','(on_board yes)',1),ref))
assert len(replacements)==2, replacements
review=data
for start,end,block,ref in sorted(replacements,reverse=True):
    review=review[:start]+block+review[end:]
normalized_original=data
normalized_review=review
for start,end,changed_block,ref in replacements:
    original_block=data[start:end]
    normalized_original=normalized_original.replace(original_block,original_block.replace('(on_board no)','(on_board MODULE_PRESENTATION_ONLY)',1),1)
    normalized_review=normalized_review.replace(changed_block,changed_block.replace('(on_board yes)','(on_board MODULE_PRESENTATION_ONLY)',1),1)
assert normalized_original==normalized_review, 'Non-placement content changed'
out=here/'exports/neocam-power-v0-review-full.net'
with tempfile.TemporaryDirectory(prefix='neocam-review-netlist-') as temp:
    temp_source=Path(temp)/source.name
    temp_source.write_text(review,encoding='utf-8')
    subprocess.run([r'C:\Program Files\KiCad\10.0\bin\kicad-cli.exe','sch','export','netlist','-o',str(out),str(temp_source)],check=True)
manifest={
    'purpose':'Connectivity review; not a board fabrication/import netlist',
    'main_source':str(source),
    'main_source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
    'only_changes':{ref:{'on_board':['no','yes']} for *_,ref in replacements},
    'unchanged':'All electrical pins, wires, labels, BOM flags and component values',
    'normalization':'Only the two documented stock-part on_board booleans replaced by an identical sentinel before hashing',
    'source_normalized_sha256':hashlib.sha256(normalized_original.encode('utf-8')).hexdigest(),
    'review_source_normalized_sha256':hashlib.sha256(normalized_review.encode('utf-8')).hexdigest(),
    'normalized_sources_identical':normalized_original==normalized_review,
    'output':out.name,
    'output_sha256':hashlib.sha256(out.read_bytes()).hexdigest(),
}
(here/'exports/review-netlist-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
print('Review-only full netlist exported; original schematic placement/BOM flags preserved.')
