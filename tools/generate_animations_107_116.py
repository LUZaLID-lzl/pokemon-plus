#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate reference-grounded whole-body poses; installed skill owns retries."""
from pathlib import Path
import argparse,json,subprocess,sys
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--start',type=int,default=107);p.add_argument('--end',type=int,default=116);p.add_argument('--version',type=int,default=1);a=p.parse_args()
assert 107<=a.start<=a.end<=116 and a.version>0
state=R/'assets/animation-generation-107-116.json'
previous=json.loads(state.read_text()) if state.exists() else {}
generated=previous.get('generated',[]);exported=previous.get('exported',[])
motions={107: 'Short alternating bent-knee steps; purple tunic and both red gloves follow natural arm counterbalance. Happy gentle knee bend and glove lift. Eat small mouth changes, no props. Sleep LOW seated with both eyes fully CLOSED and both gloves tucked against torso. Last row compact glove preparation, small near-arm extension inside silhouette, recoil and recovery; no glove detachment or exaggerated reach.', 108: 'Real alternating short steps, thick tail follows with gentle bending. ONE tongue always connects to mouth, rests compactly against chest and never enters gutter. Happy small head tilt; eat subtle connected tongue/mouth changes, no food. Sleep LOW seated, both eyes CLOSED, tongue tucked close to body. Last row short tongue/shoulder gesture then recover, entire attached tongue inside cell.', 109: 'Floating sphere has NO limbs. Travel gentle tilt and face changes, attached small bumps follow the round body; never stretch whole sphere. Happy brows/mouth change; eat small mouth changes. Sleep LOW hovering sphere with BOTH eyes fully CLOSED throughout. Last row awake slight inclination, facial concentration and recovery. No smoke, detached gas clouds, sparks or props.', 110: 'TWO spherical heads stay connected by a thick short neck in ALL48 poses; large head near/lower and small head upper/rear. Both faces remain visible. Travel small relative head/neck bends, not disconnected copies. Happy both expressions change; eat small mouth motion. Sleep LOW tucked floating posture, ALL FOUR eyes on both heads fully CLOSED in ALL8 frames. Last row small connected neck turn and recovery. No smoke, clouds or loose parts.', 111: 'FOUR attached legs on a quadruped at all times, with far legs naturally partially occluded; alternate foreleg/hindleg walking phases. Attached horn/ears/back plates/tail remain recognizable. Happy small foreleg bend, no standing on hind legs. Eat lowered muzzle, no props. Sleep LOW lying with folded FOUR legs, BOTH eyes CLOSED. Last row compact foreleg brace and head nod then recovery, no charge across gutters.', 112: 'TWO legs, TWO attached clawed arms, thick tail follows torso. Genuine alternating bent-knee steps and arm counterbalance. Happy small knee bend and head tilt; eat small mouth motion. Sleep LOW seated, arms and tail folded close, BOTH eyes fully CLOSED. Last row compact shoulder/arm gesture then recover; horn stays attached and no projectile.', 113: 'Round pink body remains constant volume, TWO small feet alternate steps and TWO arms gently counterbalance. ONE cream egg remains securely touching front pouch in every pose. Happy small knee bend and arm lift. Eat small mouth changes. Sleep LOW seated with BOTH eyes CLOSED, egg secure in pouch. Last row small arm/pouch cradling gesture and recovery. Never add a second egg or drop the egg.', 114: 'TWO red feet alternate beneath compact blue vine body. Actual small changes in connected vine curls and feet, never loose vine fragments; TWO white eyes stay visible inside ONE black face opening. Happy eyes and feet tilt. Eat use eye concentration and gentle vine movement, no mouth. Sleep LOW tucked vines with BOTH eyes CLOSED as dark eyelid lines in black face opening. Last row small connected vine/foot gesture and recovery.', 115: 'Mother TWO legs walk with short alternating steps and arms balance, attached tail bends. EXACTLY ONE purple baby stays securely inside belly pouch throughout, never falls or floats. Happy BOTH mother and baby small facial changes. Eat small muzzle changes without props. Sleep LOW seated mother with tail beside body, ALL FOUR eyes of mother and baby fully CLOSED in EVERY sleep frame. Last row compact protective arm gesture near pouch and recovery; never separate baby.', 116: 'NO arms or legs. Travel genuine connected dorsal-fin flutter and curled-tail bend, blue body gently tilts in place, no whole-body stretch. Happy small snout/head tilt. Eat small tubular-snout movement, no food. Sleep LOW floating/tucked curled tail, BOTH reddish eyes fully CLOSED as dark lines throughout. Last row compact snout inclination, attached fin flutter, recoil and recovery. No water drops, bubbles or detached marks.'}
def save(status,failed=None):
 latest=json.loads(state.read_text()) if state.exists() else previous
 latest.update(scope='107-116',generator='mobiiot-imagegen',generated=sorted(set(latest.get('generated',[])+generated)),exported=sorted(set(latest.get('exported',[])+exported)),completed=False,status=status,failedNationalId=failed,runtimeIntegrated=False)
 state.write_text(json.dumps(latest,ensure_ascii=False,indent=2)+'\n')
plan=json.loads((R/'assets/pokemon/plan-107-116.json').read_text())
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
Row5: EIGHT LOW resting sleep poses. ALL visible eyes on every head fully CLOSED in ALL EIGHT cells, dark eyelid lines without eyeballs/pupils; subtle breathing, seamless loop.
Row6: EIGHT AWAKE compact game-action phases: prepare, brace, lean, gesture, peak, recoil, recover, neutral. Complete body and appendages inside each cell.
Species direction: {motions[dex]}
All SIX rows must each contain EIGHT distinct complete body drawings with actual joint, expression or species-body motion. Do not copy-shift, stretch, cut or assemble body parts. Maintain smooth phase spacing and gentle loop boundaries. The FIRST reference controls identity; never copy the SECOND reference's character. No row may be omitted or merged.'''
 if a.version>=2:text+='\nLayout revision: keep ALL complete poses within40% cell width and45% height with wider gutters; preserve all visible heads/limbs/objects. Do not omit closed-eye sleep or bottom action band.'
 if not source.exists():
  if log.exists():raise SystemExit('Prior task log exists; preserve it and use a meaningful new revision, never outer retries.')
  prompt.write_text(text);print(f'Generating{dex:03} {item["english"]}',flush=True)
  with log.open('w') as out:
   result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-107-116/{dex:03}@8x.png'),'--image',str(R/'assets/animation-templates/001/atlas@4x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=out,stderr=subprocess.STDOUT)
  if result.returncode:save('image service failed; entire batch stopped at retry limit',dex);sys.exit(result.returncode)
 if dex not in generated:generated.append(dex)
 save('generated; measuring/exporting')
 result=subprocess.run([sys.executable,str(R/'tools/export_pet_animation.py'),str(dex),'--identity',f'assets/pokemon/gen1-107-116/{dex:03}.png','--name',item['name'],'--version',str(a.version),'--alpha-threshold','230'])
 if result.returncode:save('source rejected by measured export; visual review required',dex);sys.exit(result.returncode)
 if dex not in exported:exported.append(dex)
 save('exported; awaiting visual QA/runtime integration')
print('Requested templates exported; manual visual review required before publishing.',flush=True)
