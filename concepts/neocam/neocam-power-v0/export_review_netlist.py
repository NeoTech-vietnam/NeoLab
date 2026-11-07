"""Export a review graph containing retained stock parts, without placing duplicates.

Only explicit retained-component on_board booleans change in temporary copies.
Per-page normalized hashes verify all electrical content remains identical.
"""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import tempfile

here=Path(__file__).resolve().parent
sources=[here/'neocam-power-v0.kicad_sch',here/'neocam-power-mux-v0.6.kicad_sch',here/'neocam-power-3v3-v0.6.kicad_sch']
stock_refs={'R_MB9','U_MB3','L_MB4','R_MB13','C_MB3','C_MB4','C_MB5','C_MB6','FB_SENSE1','C_SENSE8','C_SENSE9'}
found=set()
reviews={}
records={}
changes={}
for source in sources:
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
        for ref in stock_refs:
            if f'(property "Reference" "{ref}"' in block:
                assert ref not in found and '(on_board no)' in block and '(in_bom no)' in block
                found.add(ref)
                replacements.append((start,end+1,block.replace('(on_board no)','(on_board yes)',1),ref))
    review=data
    for start,end,block,ref in sorted(replacements,reverse=True):review=review[:start]+block+review[end:]
    normalized_source=data;normalized_review=review
    for start,end,changed,ref in replacements:
        original=data[start:end]
        normalized_source=normalized_source.replace(original,original.replace('(on_board no)','(on_board MODULE_PRESENTATION_ONLY)',1),1)
        normalized_review=normalized_review.replace(changed,changed.replace('(on_board yes)','(on_board MODULE_PRESENTATION_ONLY)',1),1)
    assert normalized_source==normalized_review
    reviews[source.name]=review
    records[source.name]={
        'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
        'source_normalized_sha256':hashlib.sha256(normalized_source.encode('utf8')).hexdigest(),
        'review_source_normalized_sha256':hashlib.sha256(normalized_review.encode('utf8')).hexdigest(),
        'normalized_identical':True,
    }
    changes[source.name]={ref:{'on_board':['no','yes']} for *_,ref in replacements}
assert found==stock_refs,(found,stock_refs)
out=here/'exports/neocam-power-v0-review-full.net'
with tempfile.TemporaryDirectory(prefix='neocam-review-netlist-') as temp:
    for name,content in reviews.items():(Path(temp)/name).write_text(content,encoding='utf8')
    subprocess.run([r'C:\Program Files\KiCad\10.0\bin\kicad-cli.exe','sch','export','netlist','-o',str(out),str(Path(temp)/sources[0].name)],check=True)
manifest={
 'purpose':'Three-sheet connectivity review, not a fabrication/import netlist',
 'only_changes':changes,
 'source_files':records,
 'stock_refs_temporarily_included':sorted(found),
 'normalization':'Only the 11 explicit stock-part on_board booleans normalized to the same sentinel; every wire,pin,label,value,BOM flag unchanged',
 'normalized_sources_identical':all(x['normalized_identical'] for x in records.values()),
 'output':out.name,
 'output_sha256':hashlib.sha256(out.read_bytes()).hexdigest(),
}
(here/'exports/review-netlist-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf8')
print('Review-only three-page graph exported; 11 retained-part placement flags normalized and checked.')

