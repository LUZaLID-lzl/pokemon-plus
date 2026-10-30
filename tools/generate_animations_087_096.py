#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Reference-grounded whole-body poses. Installed skill owns six attempts; no outer retries."""
from pathlib import Path
import argparse,json,subprocess,sys
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--start',type=int,default=87);p.add_argument('--end',type=int,default=96);p.add_argument('--version',type=int,default=1);a=p.parse_args()
assert 87<=a.start<=a.end<=96 and a.version>0
state=R/'assets/animation-generation-087-096.json';previous=json.loads(state.read_text()) if state.exists() else {}
generated=previous.get('generated',[]);exported=previous.get('exported',[])
items=json.loads((R/'assets/pokemon/plan-087-096.json').read_text())['items']
motions={
87:'Seal: TWO front flippers and ONE forked rear tail, no legs. Eight genuine front-flipper paddling/belly-scooting phases, head and rear flipper counterbalance. Happy small head/fin lift. Eat subtle mouth chewing, no props. Sleep low lying curved body, ALL eyes CLOSED in all eight poses, flippers relaxed. Compact front-flipper brace/head lean, recoil and recovery. Single horn/tusks retained.',
88:'Slime has no legs. Eight genuine connected sludge-fold and attached arm articulations for low gliding, keep total body volume and low puddle base stable. Happy small connected arm lift. Eat mouth gently opens/closes, no props. Sleep broad LOW settled puddle body, ALL eyes CLOSED throughout as dark eyelid lines with no white eyeballs. Compact attached arm lean/recoil/recovery. No liquid drops, detached pieces or stretched duplicate bodies.',
89:'Broad sludge body with TWO long attached arms and one continuous low puddle base, no legs. Eight distinct arm bends and body-fold undulations for gliding, total body volume stable. Happy small arm raise. Eat gentle mouth/cheek changes. Sleep LOWER settled body with folded arms, ALL eyes CLOSED as dark lid lines with no pale eyeballs in all eight cells. Compact arm brace/lean/recoil/recovery, no detached drops or added feet.',
90:'Hinged bivalve: no arms or legs. Eight genuine short rocking/shell hinge phases for low scooting. Keep BOTH shell halves visibly connected by hinge. Pink tongue stays connected to mouth and lies against lower shell in awake rows. Happy small shell opening. Eat gentle tongue curl/mouth movement. Sleep low tilted shell with two halves nearly CLOSED, inner face hidden naturally and no visible white eyeballs in any sleep pose, tongue tucked inside. Action compact shell brace/hinge and recovery, no detached tongue or shell.',
91:'Spiked bivalve: no arms or legs, all thick spikes attached. Travel eight genuine compact shell tilt/hinge phases, never copy-shift a closed shell. Happy gentle hinge opening and face change. Eat subtle inner mouth/shell movements. Sleep LOW tilted closed resting shell, face hidden naturally, no white eyeballs or pupils visible in any of eight poses; spikes stay attached. Compact shell brace/lean/close/recover, do not extend spikes into gutters.',
92:'Floating sphere: NO arms or legs. One compact opaque connected violet gas fringe touching head. Travel eight genuine face tilts with changing connected fringe folds, volume stays round; no detached wisps. Happy cheerful eye/mouth/fringe changes. Eat subtle mouth movement without props. Sleep LOW floating compact sphere with ALL eyes CLOSED in all eight cells as violet eyelids/dark lines and no pale eyeballs or pupils. Compact face lean/fringe curl/recoil/recovery. Do not erase head or draw shadow.',
93:'Floating ghost has TWO three-clawed hands held against cheeks/lower body so they overlap naturally into one connected silhouette, no legs or invented arm tubes. Eight genuine claw/face tilts, hand flexes stay touching face/body. Happy compact grin/claw lift. Eat subtle mouth movement. Sleep LOW resting hover with relaxed claws held against body, ALL eyes CLOSED in all eight cells with purple eyelids and dark lines, no white eyeballs. Compact claw brace/head lean/recoil/recovery. Keep complete spikes/ears in every cell.',
94:'Rounded upright biped: TWO connected short arms and TWO short legs. Eight genuine alternating short leg steps, arm counterbalance, spikes/tail stay attached. Happy small knee bend/hop returning to neutral. Eat gentle mouth/cheek changes. Sleep LOW sitting curled body with both eyes fully CLOSED in all eight cells, purple lids/dark lines and no red eyeballs or pupils. Compact arm brace/head lean/recoil/recovery, no particles.',
95:'Rock snake with no limbs, compact COILED chain of connected boulders, one angular horned head. Eight genuine alternating rock-joint bends/coiling phases, tail and head articulations maintain body volume and continuous rock chain. Happy head lift/small coil changes. Eat subtle jaw hinge. Sleep LOW coiled body, head rests on coil, ALL eyes CLOSED in every sleep cell with gray lids/dark lines, no white eyeballs. Compact head/neck brace/lean/recoil/recovery. Never straighten into adjacent cells or detach boulders/horn.',
96:'Tapir-like biped: TWO short yellow arms and TWO short brown legs, one short yellow downward trunk and rounded ears. Eight genuine short alternating leg steps with small arm counterbalance and trunk bend. Happy small hop/arm lift returning to neutral. Eat gentle trunk/mouth/cheek changes, no props. Sleep LOW seated curl with ALL eyes CLOSED throughout, trunk relaxed against chest. Compact forearm brace/head lean/recoil/recovery. Yellow upper body and brown lower half remain consistent.'}

def save(status,failed=None):
 latest=json.loads(state.read_text()) if state.exists() else previous
 latest.update(scope='087-096',generator='mobiiot-imagegen',generated=sorted(set(latest.get('generated',[])+generated)),exported=sorted(set(latest.get('exported',[])+exported)),completed=False,status=status,failedNationalId=failed,runtimeIntegrated=False)
 state.write_text(json.dumps(latest,ensure_ascii=False,indent=2)+'\n')
for item in items:
 dex=item['nationalId']
 if not a.start<=dex<=a.end:continue
 folder=R/f'assets/prompts/animation-{dex:03}';folder.mkdir(exist_ok=True)
 prompt=folder/f'master-v{a.version}.txt';log=folder/f'generation-v{a.version}.log';source=R/f'assets/source/animation-{dex:03}-master-v{a.version}.png'
 text=f'''Draw a transparent pixel-art animation sheet of {dex:03} {item['english']}, exactly the same friendly game creature as the attached approved reference. Preserve exact face, colors, proportions and connected anatomy: {item['anatomy']}.
Layout: 8 columns by 6 rows, exactly48 separate COMPLETE whole-body poses. Spacious1536x1152 canvas. Every complete creature including all appendages occupies at most45% of cell width and50% of cell height. Generous fully transparent gutters in BOTH axes, no touching bodies or cropping, no detached components. Consistent left-facing camera, body volume, scale and baseline. Native48px pixel style with subject within40px, at most15 opaque colors plus transparency, crisp square pixels, thin continuous deep-navy contour. REAL transparent alpha, no text/ground/props/particles/checkerboard/shadows.
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
  text+='\nLayout repair: compact every complete pose within40% of cell width and45% cell height; leave at least70px transparent horizontal and60px vertical gutters. Keep all appendages attached, all sleep eyes closed, exactly48 isolated complete silhouettes.'

 if dex==87 and a.version>=2:
  text='Create a pixel-art animation atlas of the white seal in the attached approved reference: one short forehead horn, two tiny tusks, two front flippers and a forked rear tail. White smooth rounded body; no walking legs.\nTransparent1536x1152 canvas. EXACTLY8columns and6rows,48 whole seals, one seal centered in EACH192x192 cell. Each complete silhouette including tail is at most85px wide and85px tall. Keep at least80px transparent gutters. Consistent left-facing camera and body volume. Native48px look,15 opaque colors, crisp square pixels, connected thin navy outline. No text/shadow/grid/props/effects.\nROW1: EIGHT idle breathing/blinking phases.\nROW2: EIGHT short front-flipper paddling/belly-sliding phases, drawn in place; flippers bend differently, tail counterbalances.\nROW3: EIGHT cheerful small head/fin lifts ending at neutral.\nROW4: EIGHT muzzle lowering/chewing phases ending neutral, no food props.\nROW5: EIGHT LOW lying resting seals. BOTH EYES FULLY CLOSED in EVERY one of these eight poses. Dark closed eyelid lines, NO open eyeballs. Low head rests, fins relaxed; subtle breathing changes.\nROW6: EIGHT compact front-flipper brace and head nod phases: prepare, brace, lean, peak, recoil, recover, settle, neutral. ALL EIGHT distinct complete seals.\nSix clearly separated horizontal rows, eight complete poses per row. Keep whole tail/flippers/horn visible and connected in all48 cells. Draw real joint/mouth/eye poses; no translated or stretched duplicates. Gentle loop boundaries. Do not omit the sixth row or mix awake frames into sleep row.'

 if dex!=87 or a.version>=3:
  text+='\nSTRICT LAYOUT REFERENCE: the SECOND attached image is a finished8x6 action atlas. Copy ONLY its exact count and placement of six rows/eight columns, never its character. The FIRST attached image is the sole identity reference. Fill the entire canvas height with SIX bands. Do not leave a large empty top or bottom banner. Put idle in top1/6, travel in second1/6, happy third, eat fourth, sleep fifth, action sixth/bottom1/6. Exactly8 complete bodies in each of the SIX bands. Keep the same compact body size across all48 cells. Row5 MUST have closed eyes throughout; row6 is AWAKE species-appropriate brace/lean/recovery. Do not merge row5/row6 or omit eat.'

 if dex==93 and a.version>=2:
  text+='\nRevision2 pose direction: a gentle small floating companion, calm expressions and compact movements. Keep both three-digit hands beside the lower cheeks, resting against the sides through natural overlap; no added forearms. Across each row vary the finger curl, ear tilt and facial expression gradually while preserving its rounded head volume. For sleep use two thin horizontal purple eyelids throughout all eight cells and a small relaxed mouth; the head stays visible and comfortably low. For the final action row show a modest two-hand forward gesture, then a gradual return to the relaxed hovering pose. This is the same reference character, with the corrected six-row layout and species-specific hand gestures rather than seal/flipper instructions.'

 if dex==95 and a.version>=2:
  text+='\nRevision2 compact coil choreography: preserve the FIRST reference\'s small rounded rock coil, horned head facing left and tapered rock tail. Keep the head just left of the coil, with the tail curved inward below it. Vary the angles between adjacent complete rocks gradually; never add feet, separate stones, or extend the body into a straight line. Idle jaw/eyes/horn tilt, travel gentle coil-uncoil joint phases, happy modest head lift, eat small jaw hinge, sleep head lowered against the coil with both eyelids fully closed in every cell, final action compact head nod/neck bend and smooth return. Maintain the same number and relative sizes of rocks through all48 drawings. Prioritize a low wide coil fitting the generous cell margins over a tall upright neck.'

 if not source.exists():
  if log.exists():raise SystemExit('Preserve prior task log; use a new meaningful revision, no outer service retries.')
  prompt.write_text(text);print(f'Generating {dex:03} {item["english"]}',flush=True)
  with log.open('w') as out:result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-087-096/{dex:03}@8x.png'),*(['--image',str(R/'assets/animation-templates/001/atlas@4x.png')] if dex!=87 or a.version>=3 else []),'--prompt-file',str(prompt),'--output',str(source)],stdout=out,stderr=subprocess.STDOUT)
  if result.returncode:save('image service failed; entire batch stopped at retry limit',dex);sys.exit(result.returncode)
 if dex not in generated:generated.append(dex)
 save('generated; measuring/exporting')
 result=subprocess.run([sys.executable,str(R/'tools/export_pet_animation.py'),str(dex),'--identity',f'assets/pokemon/gen1-087-096/{dex:03}.png','--name',item['name'],'--version',str(a.version),'--alpha-threshold','230'])
 if result.returncode:save('source rejected by measured export; visual review required',dex);sys.exit(result.returncode)
 if dex not in exported:exported.append(dex)
 save('exported; awaiting visual QA/runtime integration')
print('Requested templates exported; visual QA required before publishing.',flush=True)
