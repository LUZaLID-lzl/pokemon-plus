#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate reference-grounded whole-body poses; installed skill owns retries."""
from pathlib import Path
import argparse,json,subprocess,sys
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--start',type=int,required=True);p.add_argument('--end',type=int,required=True);p.add_argument('--version',type=int,default=1);a=p.parse_args()
assert 117<=a.start<=a.end<=151 and a.version>0
plans=[(f,json.loads(f.read_text()))for f in (R/'assets/pokemon').glob('plan-*.json')]
selected=[(f,pl)for f,pl in plans if pl['range'][0]<=a.start<=a.end<=pl['range'][1]];assert len(selected)==1
planfile,plan=selected[0];scope=planfile.stem.removeprefix('plan-')
state=R/f'assets/animation-generation-{scope}.json'
previous=json.loads(state.read_text()) if state.exists() else {}
generated=previous.get('generated',[]);exported=previous.get('exported',[])
motions={int(k):v for k,v in json.loads((R/'assets/animation-direction-117-151.json').read_text()).items()}
def save(status,failed=None):
 latest=json.loads(state.read_text()) if state.exists() else previous
 latest.update(scope=scope,generator='mobiiot-imagegen',generated=sorted(set(latest.get('generated',[])+generated)),exported=sorted(set(latest.get('exported',[])+exported)),completed=False,status=status,failedNationalId=failed,runtimeIntegrated=False)
 state.write_text(json.dumps(latest,ensure_ascii=False,indent=2)+'\n')
qa=json.loads((R/f'docs/gen1-{scope}/visual-review.json').read_text())
assert set(range(a.start,a.end+1))<=set(qa['identityReviewed']),'Review the actual native identities before motion generation'
for item in plan['items']:
 dex=item['nationalId']
 if not a.start<=dex<=a.end:continue
 folder=R/f'assets/prompts/animation-{dex:03}';folder.mkdir(exist_ok=True)
 prompt=folder/f'master-v{a.version}.txt';log=folder/f'generation-v{a.version}.log';source=R/f'assets/source/animation-{dex:03}-master-v{a.version}.png'
 text=f'''Create a transparent pixel-art game-companion animation atlas of {dex:03} {item['english']}, matching the FIRST attached approved identity reference exactly. Preserve colors, face, proportions and anatomy: {item['anatomy']}.
The SECOND image is ONLY the layout reference: exactly EIGHT columns and SIX rows, 48 isolated COMPLETE whole-body poses. Use1536x1152 canvas, each cell192x192. Center a complete character at most85px wide and85px tall inside each cell, including every appendage. Leave at least70px transparent gutters, no touching neighboring characters. Fill SIX equal-height bands from top to bottom, no empty banner. Same left-facing camera, scale, body volume and baseline throughout. Native48px look, subject within40px, at most15 opaque colors plus transparency. Crisp square pixels and thin continuous deep-navy outline, no antialiasing, gradient, text, grid, shadow, ground or effects.
Row1: EIGHT idle breathing/blinking poses, seamless loop.
Row2: EIGHT articulated species-appropriate in-place travel phases, seamless loop.
Row3: EIGHT gentle happy poses returning to neutral.
Row4: EIGHT gentle eating/concentration phases, no extra food or props.
Row5: EIGHT LOW resting sleep poses. For eyed species ALL eyes fully CLOSED in ALL EIGHT cells, dark eyelid lines without pupils. Starfish have NO eyes: lower relaxed arms and a muted dark-red central jewel, no face or white jewel shine; subtle breathing or natural resting pulse, seamless loop.
Row6: EIGHT AWAKE compact game-action phases: prepare, brace, lean, gesture, peak, recoil, recover, neutral. Complete body and appendages inside each cell.
Species direction: {motions[dex]}
All SIX rows must each contain EIGHT distinct complete body drawings with actual joint, expression or species-body motion. Do not copy-shift, stretch, cut or assemble body parts. Maintain smooth phase spacing and gentle loop boundaries. The FIRST reference controls identity; never copy the SECOND reference's character. No row may be omitted or merged.'''
 if a.version>=2:text+='\nLayout revision: keep ALL complete poses within40% cell width and45% height with wider gutters; preserve all visible heads/limbs/objects. Do not omit species-appropriate sleep or bottom action band.'
 if dex==130 and a.version>=2:text+='\nAnatomy readability revision: draw both cream whiskers SHORT and THICK, curled directly against and visibly joined to the cream muzzle; no long hanging hairline loops. Every whisker, fin and tail joint must remain at least TWO native pixels wide after reduction to48px. Keep the full serpentine body and all48 poses; do not add legs.'
 if dex==130 and a.version>=3:text+='\nSleep-row correction: ALL EIGHT characters in ROW FIVE must keep their heads lowered, mouths FULLY CLOSED and BOTH eyes represented only by solid dark-navy horizontal curved eyelid lines. NO red iris, NO white sclera or highlight, and NO red open-mouth area anywhere in that sleep row. Especially the third and sixth sleep poses must remain asleep. Subtle coiled-body/fin breathing only; waking poses belong exclusively to other rows.'
 if dex==135 and a.version>=3:text+='\nSleep correction from rejected visual review: in ROW FIVE ALL EIGHT Jolteon must lie low with the head resting on the front paws, mouth closed, BOTH eyes drawn ONLY as thin horizontal dark eyelid lines. NO open circular eyes, pupils, eye highlights or white sclera in ANY of these eight sleeping drawings, especially columns FOUR, SIX and EIGHT. Keep ears relaxed and body breathing subtle; head lifting or waking belongs ONLY in the other rows. Preserve the complete quadruped and all six rows, eight unique whole-body poses per row.'
 if not source.exists():
  if log.exists():raise SystemExit('Prior task log exists; preserve it and use a meaningful new revision, never outer retries.')
  prompt.write_text(text);print(f'Generating{dex:03} {item["english"]}',flush=True)
  with log.open('w') as out:
   result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-{scope}/{dex:03}@8x.png'),'--image',str(R/'assets/animation-templates/001/atlas@4x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=out,stderr=subprocess.STDOUT)
  if result.returncode:save('image service failed; entire batch stopped at retry limit',dex);sys.exit(result.returncode)
 if dex not in generated:generated.append(dex)
 save('generated; measuring/exporting')
 result=subprocess.run([sys.executable,str(R/'tools/export_pet_animation.py'),str(dex),'--identity',f'assets/pokemon/gen1-{scope}/{dex:03}.png','--name',item['name'],'--version',str(a.version),'--alpha-threshold','230'])
 if result.returncode:save('source rejected by measured export; visual review required',dex);sys.exit(result.returncode)
 if dex not in exported:exported.append(dex)
 save('exported; awaiting visual QA/runtime integration')
print('Requested templates exported; manual visual review required before publishing.',flush=True)
