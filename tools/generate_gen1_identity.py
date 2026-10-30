#!/usr/bin/env python3
"""One reference-grounded identity task; installed skill owns all six attempts."""
from pathlib import Path
import argparse,json,subprocess,sys
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('dex',type=int);p.add_argument('--version',type=int,default=1);a=p.parse_args();assert 118<=a.dex<=151 and a.version>0
plans=[(f,json.loads(f.read_text()))for f in (R/'assets/pokemon').glob('plan-*.json')]
found=[(f,plan,x)for f,plan in plans for x in plan.get('items',[])if x['nationalId']==a.dex];assert len(found)==1
planfile,plan,item=found[0];scope=planfile.stem.removeprefix('plan-')
source=R/f'assets/source/identity-{a.dex:03}-v{a.version}.png';prompt=R/f'assets/prompts/identity-{a.dex:03}-v{a.version}.txt';log=R/f'build/identity-{a.dex:03}-generation-v{a.version}.log'
reference=R/'assets/pokemon/gen1-001-009/001@8x.png';role='The approved sprite is a STYLE reference only. Keep its compact rounded proportions, crisp uniform pixel grid and continuous deep-navy outline, but draw only the requested new character in its own correct colors.'
if item['stage']:
 family=sorted((x for _,pl in plans for x in pl.get('items',[])if x['species']==item['species']and x['stage']<item['stage']),key=lambda x:x['stage'])
 if item['species']==67:family=[x for x in family if x['nationalId']==133]
 assert family,'Missing base identity mapping'
 prev=family[-1];priorplan=next(f for f,pl in plans if any(x['nationalId']==prev['nationalId']for x in pl['items']));reference=R/f"assets/pokemon/gen1-{priorplan.stem.removeprefix('plan-')}/{prev['nationalId']:03}@8x.png";assert reference.is_file(),'Draw and review pre-evolution before its evolved identity'
 role='The attached sprite is the actual pre-evolution and style reference. Evolve it into the requested new character, retaining recognizable family traits and the established game style.'
text=f'''Create ONE complete {item['english']} (national Pokedex{a.dex:03}) pixel sprite for this existing handheld companion game. {role}
Character anatomy and colors: {item['anatomy']}.
Calm friendly neutral pose, face slightly to the left. Complete body centered on a genuinely transparent background with generous empty margins; all appendages attached and inside the image. Native48x48 pixel grid, silhouette at most40x40, at most15 opaque colors plus transparency. Continuous thin dark-navy outer outline, two or three flat color shades per material, clean connected pixel clusters. Crisp nearest-neighbor integer enlargement for review. No antialiasing, gradients, text, checkerboard, ground, shadow or effects. Do not invent human limbs. ONE single neutral identity drawing, not an animation atlas.'''
if not source.exists():
 if log.exists():raise SystemExit('Prior task log exists; do not stack retries or overwrite the record')
 assert not prompt.exists(),'Keep any prior prompt; use an explicit new task version'
 prompt.write_text(text)
 with log.open('w')as out:
  result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(reference),'--prompt-file',str(prompt),'--output',str(source)],stdout=out,stderr=subprocess.STDOUT)
 if result.returncode:
  print('Identity task exhausted skill retry limit; STOP THE WHOLE BATCH',flush=True);sys.exit(result.returncode)
subprocess.run([sys.executable,str(R/'tools/export_gen1_identity.py'),str(a.dex),'--version',str(a.version)],check=True)
print('Exported one identity; raw/native visual review required before animation generation',flush=True)
