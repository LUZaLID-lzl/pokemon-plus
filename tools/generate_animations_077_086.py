#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Reference-grounded whole-body poses. Installed skill owns six attempts; no outer retries."""
from pathlib import Path
import argparse,json,subprocess,sys
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--start',type=int,default=77);p.add_argument('--end',type=int,default=86);p.add_argument('--version',type=int,default=1);a=p.parse_args()
assert 77<=a.start<=a.end<=86 and a.version>0
state=R/'assets/animation-generation-077-086.json';previous=json.loads(state.read_text()) if state.exists() else {}
generated=previous.get('generated',[]);exported=previous.get('exported',[])
items=json.loads((R/'assets/pokemon/plan-077-086.json').read_text())['items']
motions={
77:'Four-legged horse: eight genuine short alternating front and hind leg steps, stable torso, attached flame mane and tail flow softly in every row. Happy small hoof lift. Eat lower muzzle and chew without props. Sleep lying with all four legs tucked, both eyes CLOSED throughout. Action compact front-hoof brace and head lean followed by recovery. No horn.',
78:'Four-legged horned horse: eight genuine short alternating front and hind leg steps, stable torso and ONE forehead horn, attached flame mane and tail flow softly. Happy small front-hoof lift. Eat lower muzzle and chew. Sleep lying with four legs tucked, both eyes CLOSED throughout. Action short foreleg brace and head lean followed by recovery. No wings, no detached flame particles.',
79:'Quadruped: FOUR short legs stay attached, thick cream-tipped tail attached. Travel uses eight short alternating leg bends. Happy small head/foreleg lift. Eat subtle muzzle chewing. Sleep low lying body with ALL eyes CLOSED throughout, tail relaxed. Action compact head lean/foreleg brace, recoil, recovery. No shell or human hands.',
80:'Upright biped with TWO arms, TWO legs, and ONE gray shell visibly attached to tail in EVERY frame. Travel short leg steps, small arm counterbalance. Happy small arm lift. Eat subtle muzzle chewing. Sleep seated-to-low rest with ALL eyes including shell eyes CLOSED throughout, shell attached beside body. Action compact arm push and recover; shell never floats or detaches.',
81:'Floating metallic creature has no legs or mouth. Travel eight distinct in-place body tilts, attached side magnets turn gently with torso and top screw changes angle. Happy small eye squint/magnet lift. Eat indicated by eye and magnet pulses without a mouth or props. Sleep low floating rest with SINGLE central eyelid fully CLOSED in ALL eight poses. Action compact magnet lean/brace/recoil/recovery. Keep red/blue ends and all screws attached; no sparks.',
82:'Floating compact cluster: exactly THREE connected metallic eye spheres at ALL times, each with screw and side magnets. No legs or mouth. Travel eight true whole-cluster tilts with subtle relative magnet bends, no shifted duplicate frames. Happy slight magnet lift/eye squint. Eat subtle attached eye/magnet pulses. Sleep low floating rest, ALL THREE eyes fully CLOSED throughout. Action compact collective lean and attached magnet gestures, recoil and recover. Keep cluster connected, no disappearing units, no sparks.',
83:'Duck biped: TWO webbed feet alternate genuine short steps, TWO wings stay attached. ONE green-and-white leek remains gripped by near wing and touches body in EVERY pose, including sleeping. Happy small wing/head lift. Eat gentle beak chewing with leek held against body, no extra props. Sleep low sitting pose with eyes CLOSED and leek resting against attached wing. Action compact held-leek tilt/head lean and recovery; do not swing it into adjacent cells.',
84:'Flightless bird has EXACTLY TWO whole heads and TWO attached necks in every pose. TWO long legs make short alternating running steps, body volume stable. Happy small knee bend/head bob. Eat BOTH beaks chewing, no props. Sleep seated low with both necks tucked gently and ALL eyes on BOTH heads CLOSED throughout. Action compact double-head lean, recoil, recovery. No wings, missing head, detached beak or feather.',
85:'Flightless bird has EXACTLY THREE whole heads on THREE attached necks in every pose, each with two dark crest feathers. TWO long legs make short alternating running steps, stable body. Happy small knee bend/three-head bob. Eat THREE beaks chewing, no props. Sleep seated low with all three necks tucked and ALL eyes on ALL THREE heads CLOSED throughout. Action compact three-head lean, recoil, recovery. Keep red tail feathers attached, no wings or missing heads.',
86:'Seal has TWO front flippers and ONE forked rear tail flipper, no walking legs. Travel eight genuine flipper paddling and low belly-scooting phases, stable round body. Happy small head/front flipper lift. Eat subtle mouth chewing without fish or props. Sleep low lying curl, both eyes fully CLOSED throughout, flippers resting. Action compact front-flipper brace/head lean, recoil and recovery. Keep single horn/two tusks and rear flipper connected.'}
def save(status,failed=None):
 state.write_text(json.dumps(dict(previous,scope='077-086',generator='mobiiot-imagegen',generated=generated,exported=exported,completed=False,status=status,failedNationalId=failed,runtimeIntegrated=False),ensure_ascii=False,indent=2)+'\n')
for item in items:
 dex=item['nationalId']
 if not a.start<=dex<=a.end:continue
 folder=R/f'assets/prompts/animation-{dex:03}';folder.mkdir(exist_ok=True)
 prompt=folder/f'master-v{a.version}.txt';log=folder/f'generation-v{a.version}.log';source=R/f'assets/source/animation-{dex:03}-master-v{a.version}.png'
 text=f'''Draw a transparent pixel-art animation sheet of {dex:03} {item['english']}, exactly the same friendly game creature as the attached approved reference. Preserve exact face, colors, proportions and connected anatomy: {item['anatomy']}.
Layout: 8 columns by 6 rows, exactly48 separate COMPLETE whole-body poses. Spacious1536x1152 canvas. Every complete creature including all appendages occupies at most55% of cell width and65% of cell height. Generous fully transparent gutters in BOTH axes, no touching bodies or cropping, no detached components. Consistent left-facing camera, body volume, scale and baseline. Native48px pixel style with subject within40px, at most15 opaque colors plus transparency, crisp square pixels, thin continuous deep-navy contour. REAL transparent alpha, no text/ground/props/particles/checkerboard/shadows.
Rows top to bottom:
1. Idle: eight subtle breathing and blinking poses, seamless loop.
2. Travel: eight genuine articulated in-place phases, seamless loop.
3. Happy: eight small cheerful phases returning to neutral.
4. Eat: eight gentle chewing or species-appropriate phases without extra food props.
5. Sleep: eight LOW resting poses with ALL eyes CLOSED throughout, subtle breathing, seamless loop.
6. Action: eight compact game-action phases: prepare, brace, lean, extend, peak, recoil, recover, neutral. No effects. Complete body and every appendage in each cell.
Species motion: {motions[dex]}
Every row must contain eight DISTINCT full-body poses with genuine joint/eye/mouth changes, not copies shifted or stretched. Fixed body volume, no cut-body assembly. Gentle phase spacing, no abrupt flips. Happy and action end at neutral. The approved reference is authoritative for identity.'''
 if a.version>=2:
  text+='''\nLayout repair: each complete creature must occupy at most40% of its cell width and45% of cell height. Center each in its cell and leave at least70px of fully transparent horizontal gutter and60px vertical gutter. Keep every flame/tail inside these bounds. Exactly48 isolated alpha silhouettes, no touching fringe. Use compact gestures close to body. For Rapidash, ONE clearly visible ivory forehead horn must remain attached in ALL48 cells, distinct from ears and orange mane. Sleeping all legs tucked and ALL eyes closed in every cell.'''
 if dex==78 and a.version>=3:
  text='''Create a pixel-art animation atlas for the horned cream horse in the attached approved reference. Preserve the one ivory forehead horn, two ears, four gray-hoofed legs and connected orange-red mane and tail. All shapes are friendly and compact.
Use a transparent 1536x1152 canvas, with exactly eight columns and six rows. Each complete horse is centered inside a192x192 cell, with its whole silhouette no wider than85px and no taller than90px, including mane, horn and tail. Keep generous fully transparent space between every pair of horses. Exactly48 isolated whole-body silhouettes. No detached flame particles, shadows, text or grid.
Maintain the left-facing view, cream torso volume, native48px pixel style, crisp square blocks, continuous thin deep-navy outline and at most15 opaque colors plus transparency.
Row1: eight subtly different idle breathing/blinking phases.
Row2: eight short alternating four-leg walking phases, drawn in place.
Row3: eight small cheerful forehoof lifts that return to neutral.
Row4: eight muzzle lowering and gentle chewing phases; four legs remain beneath torso.
Row5: eight low lying resting phases; all four legs tucked, eye fully CLOSED in EVERY pose. Keep the single horn visible above the muzzle. Tiny breathing/mane changes only.
Row6: eight compact forehoof brace and head-nod phases: prepare, brace, lean, peak, recoil, recover, settle, neutral. Keep tail close to rear body.
Draw each complete pose separately with real leg/head/eyelid articulation, no translated or stretched duplicate frames. Keep the horn attached and visible in all48 images. Loop boundaries are gentle; no changes in body size or facing.'''
 if not source.exists():
  if log.exists():raise SystemExit('Preserve prior task log; use a new meaningful revision, no outer service retries.')
  prompt.write_text(text);print(f'Generating {dex:03} {item["english"]}',flush=True)
  with log.open('w') as out:result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-077-086/{dex:03}@8x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=out,stderr=subprocess.STDOUT)
  if result.returncode:save('image service failed; entire batch stopped at retry limit',dex);sys.exit(result.returncode)
 if dex not in generated:generated.append(dex)
 save('generated; measuring/exporting')
 result=subprocess.run([sys.executable,str(R/'tools/export_pet_animation.py'),str(dex),'--identity',f'assets/pokemon/gen1-077-086/{dex:03}.png','--name',item['name'],'--version',str(a.version),'--alpha-threshold','230'])
 if result.returncode:save('source rejected by measured export; visual review required',dex);sys.exit(result.returncode)
 if dex not in exported:exported.append(dex)
 save('exported; awaiting visual QA/runtime integration')
print('Requested templates exported; visual QA required before publishing.',flush=True)
