"""Audit stable move IDs, firmware parity and every published skill animation."""
from pathlib import Path
import hashlib, json, re
from PIL import Image
import numpy as np
from move_catalog import load_catalog, skill_manifest, TYPES

root=Path(__file__).resolve().parents[1]
catalog=load_catalog();moves=catalog['moves']
old=hashlib.sha256(json.dumps([{k:v for k,v in m.items() if k!="power"} for m in moves[:272]],ensure_ascii=False,sort_keys=True).encode()).hexdigest()
assert old=='38cb9a654e78784f7881c3ac6e8fb42226915d7a55dc2eff33b6ee61d47e1cc7','Legacy move data changed'
source=(root/'core/moves.c').read_text()
rows=re.findall(r'\{"([^"]+)", "([^"]+)", "([^"]+)", PP_([A-Z]+), (\d+)\}',source)
assert len(rows)==len(moves)
for m,row in zip(moves,rows):
    assert (*[m[k] for k in ['name','type','description']],TYPES[m['type']],str(m['power']))==row
form_rows=re.findall(r'\{(\d+), (\d+), (\d+), (\d+)\}',source.split('form_power[151][4] = {')[1])
assert [[int(x) for x in row] for row in form_rows]==[f['powers'] for f in catalog['formStats']]
roster=json.loads((root/'assets/pet-roster.json').read_text())
skills=skill_manifest(roster)
for p in ['web/manifest.json','sdcard/pokemon-plus/manifest.json']:
    assert json.loads((root/p).read_text())['skills']==skills
for move in skills:
    icon=(root/'sdcard/pokemon-plus'/move['icon']).resolve()
    assert icon.is_file(),icon
    with Image.open(root/f'docs/screenshots/skill-{move["id"]}.gif') as clip:
        assert clip.size==(360,360) and clip.n_frames>1
        duration=0;hashes=set()
        for f in range(clip.n_frames):
            clip.seek(f);duration+=clip.info.get('duration',0)
            hashes.add(hashlib.sha256(clip.convert('RGB').tobytes()).hexdigest())
        assert duration==6000 and len(hashes)>16,(move['id'],duration,len(hashes))
    if not move['runtimeIntegrated']:
        # Isolate only the FX layer for this audit of the same actor-render path:
        # water/fire frames otherwise legitimately cover some of the pet pixels.
        # This catches missing/substituted actors despite animated narration/FX.
        screenshot=np.asarray(Image.open(root/f'docs/screenshots/skill-{move["id"]}-actor.png').convert('RGB')).astype(int)
        best=0.0
        indices=(2*np.arange(56)+1)*48//112
        for f in range(16):
            action='idle' if f<8 else 'attack'
            path=root/f'assets/animation-templates/{move["nationalIds"][0]:03}/right/{action}-{f%8:02}.png'
            source=np.asarray(Image.open(path).convert('RGBA'))
            sampled=source[indices[:,None],indices[None,:]]
            opaque=sampled[:,:,3]>0
            # RGB565 flush loses at most7/3/7 from each original channel.
            rgb=sampled[:,:,:3].astype(int)
            expected=(rgb>>[3,2,3])<<[3,2,3]
            for shift in range(-3,9):
                actual=screenshot[160:216,52+shift:108+shift]
                match=np.all(np.abs(actual-expected)<=8,axis=2)[opaque].mean()
                best=max(best,float(match))
        assert best>=.99,(move['id'],'actual pet does not match approved attack art',best)
report=dict(status='PASS',families=len(catalog['families']),forms=151,configurations=len(moves),
            uniqueNames=len({m['name'] for m in moves}),runtimeFamilies=len({p['species'] for p in roster}),
            runtimeSkills=sum(m['runtimeIntegrated'] for m in skills),
            previewOnlySkills=sum(not m['runtimeIntegrated'] for m in skills),
            legacyMoveSha256=old,gifsValidated=len(skills),sourceMatchedPreviewActors=sum(not m["runtimeIntegrated"] for m in skills),newGifs=list(range(268,316)))
(root/'build/skills-gen1-audit.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
print(json.dumps(report,ensure_ascii=False))
