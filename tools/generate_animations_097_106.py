#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate reference-grounded whole-body poses; installed skill owns retries."""
from pathlib import Path
import argparse,json,subprocess,sys
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--start',type=int,default=97);p.add_argument('--end',type=int,default=106);p.add_argument('--version',type=int,default=1);a=p.parse_args()
assert 97<=a.start<=a.end<=106 and a.version>0
state=R/'assets/animation-generation-097-106.json'
previous=json.loads(state.read_text()) if state.exists() else {}
generated=previous.get('generated',[]);exported=previous.get('exported',[])
motions={
97:'Short alternating leg steps with gentle arm counterbalance. Keep white neck ruff, short pointed nose and ONE small pendulum connected by a short cord to gripping hand and touching chest. The hand/coin may shift together gently but no loose pieces. Happy small knee bend. Eat small nose/mouth changes without food. Sleep low seated, BOTH eyes fully CLOSED, pendulum rests against body. Final row compact arm gesture and recovery, no effects.',
98:'Crab: FOUR walking legs in two visible pairs, TWO attached pincers and TWO eyestalks. Travel genuine alternate bent leg phases with a small shell rock, not mammal steps. Claws open/close subtly while staying attached. Sleep crouched low with claws folded beside shell, BOTH eyestalk eyes CLOSED throughout. Final row compact pincer brace/forward pinch/recovery, no detached parts.',
99:'Crab: FOUR walking legs, TWO eyestalks and TWO claws; ONE near claw is much larger than the other. Keep giant claw folded beside shell within its cell. Genuine short scuttling leg phases and gentle asymmetric claw gestures. Sleep crouched with claws relaxed, BOTH eyestalk eyes fully CLOSED. Final row compact near-claw opening/pinch/recovery, never reach into gutters.',
100:'A RED-TOP WHITE-BOTTOM sphere with exactly TWO eyes and NO mouth or limbs. Travel eight genuine slight rocking/rotation poses with eye expression changes, keep horizontal red-white boundary nearly level and body circular. Happy cheerful eyes; eat use small eye/face concentration only, never invent mouth. Sleep low resting sphere, BOTH eyes CLOSED as dark lines in red half. Final row compact lean/rock/recover, no sparks, smoke or explosion.',
101:'A WHITE-TOP RED-BOTTOM sphere with eyes/brows and a smiling mouth, NO limbs. Travel genuine subtle rocking/rotation with changing brows and mouth, volume stays round and red-white boundary nearly level. Happy broad small grin phases; eat small mouth chewing without props. Sleep low resting sphere, BOTH eyes CLOSED in white half and mouth relaxed. Final row compact rock/brace/recovery, no sparks or explosions.',
102:'EXACTLY SIX complete pink egg heads naturally touching in a compact overlapping cluster, never turn them into a single creature or lose eggs. Travel alternate slight tilts of the individual eggs while all six stay visibly countable. Happy six facial changes. Eat small mouth changes without food. Sleep low cluster, ALL visible eyes on every faced egg CLOSED throughout; rear cracked crown remains open shell with yellow interior, no detached fragments. Final row compact collective lean/recovery; no arms or legs.',
103:'EXACTLY THREE attached yellow coconut heads with THREE faces, connected broad green leaf crown, tan short trunk and TWO legs, NO arms. Travel genuine alternating leg steps, coconut/leaf counterbalance; all heads visible and countable. Happy small knee bend/leaf lift. Eat small coconut mouth changes. Sleep low seated trunk, ALL SIX eyes on three heads fully CLOSED, leaves relaxed and connected. Final row compact trunk lean/leaf motion/recovery.',
104:'TWO short arms and TWO short legs, ONE ivory skull helmet with two horns, ONE short bone club firmly touching gripping hand and side. Keep bone/helmet/tail connected through natural overlap. Genuine alternating steps with arm counterbalance. Happy small knee bend. Eat subtle muzzle motion beneath helmet. Sleep low seated curl, BOTH eye sockets contain CLOSED dark eyelid lines, no pale eyeballs. Final row a compact bone-club gesture beside body then recovery, never detach or extend club into gutter.',
105:'TWO arms and TWO legs, ivory skull helmet with two horns, ONE ivory bone club gripped beside body. Genuine short alternating steps and arm counterbalance. Happy slight knee bend. Eat subtle muzzle motion. Sleep low seated with bone resting against side, BOTH visible eyes CLOSED within helmet sockets, no eyeballs/pupils. Final row compact whole-arm club gesture then recovery. Bone club stays in the hand and touches body by overlap, no floating props.',
106:'TWO slim attached arms with three blunt fingers and TWO segmented spring-like legs with three-toed feet. Oval tan body has TWO narrow white eyes and NO mouth or nose. Genuine alternate bent-knee short steps, arms counterbalance; segmented legs flex at joints, never stretch whole body. Happy small knee bend/hand lift; eat use subtle eye/body concentration without inventing a mouth. Sleep LOW seated with segmented legs folded, BOTH eyes fully CLOSED. Final row compact bent-knee leg lift, small forward foot gesture, recoil and neutral; whole figure stays inside cell.'}
def save(status,failed=None):
 latest=json.loads(state.read_text()) if state.exists() else previous
 latest.update(scope='097-106',generator='mobiiot-imagegen',generated=sorted(set(latest.get('generated',[])+generated)),exported=sorted(set(latest.get('exported',[])+exported)),completed=False,status=status,failedNationalId=failed,runtimeIntegrated=False)
 state.write_text(json.dumps(latest,ensure_ascii=False,indent=2)+'\n')
plan=json.loads((R/'assets/pokemon/plan-097-106.json').read_text())
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
 if dex==100 and a.version>=2:
  text+='\nSphere motion revision: depict ONE intact red-upper/white-lower round ball in every cell. Express the six activities through gentle eyelid changes, gaze, subtle tilt and recovery. Keep the center seam nearly horizontal and circular volume constant. No mouth, feet, arms, loose parts or surrounding marks. Row4 is quiet concentration with eye changes; row5 has two fully closed eyelid curves in every frame; row6 is an awake forward inclination and gentle return. Keep all48 complete balls evenly separated.'
 if dex==104 and a.version>=2:
  text+='\nCalm companion motion revision: preserve the approved ivory two-horn helmet, brown body and single short ivory accessory in the near hand. Keep this accessory lowered beside the hip throughout all48 cells, touching the hand and side by natural overlap. Use small alternating knee and foot phases, quiet shoulder counterbalance and gentle muzzle expressions, with no wide arm swings. Row3 is a shy knee bend and small head tilt. Row5 is eight low seated resting poses, with closed eyelid lines visibly inside both helmet openings and the accessory resting alongside the body. Row6 is an attentive game-ready sequence: straighten, look ahead, small forward weight shift, pause, gentle shoulder turn, settle, recover, neutral. No striking or thrown accessory. Keep the face readable and both feet attached; actual small joint changes in each cell, no global scaling or translated copies.'
 if not source.exists():
  if log.exists():raise SystemExit('Prior task log exists; preserve it and use a meaningful new revision, never outer retries.')
  prompt.write_text(text);print(f'Generating{dex:03} {item["english"]}',flush=True)
  with log.open('w') as out:
   result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-097-106/{dex:03}@8x.png'),'--image',str(R/'assets/animation-templates/001/atlas@4x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=out,stderr=subprocess.STDOUT)
  if result.returncode:save('image service failed; entire batch stopped at retry limit',dex);sys.exit(result.returncode)
 if dex not in generated:generated.append(dex)
 save('generated; measuring/exporting')
 result=subprocess.run([sys.executable,str(R/'tools/export_pet_animation.py'),str(dex),'--identity',f'assets/pokemon/gen1-097-106/{dex:03}.png','--name',item['name'],'--version',str(a.version),'--alpha-threshold','230'])
 if result.returncode:save('source rejected by measured export; visual review required',dex);sys.exit(result.returncode)
 if dex not in exported:exported.append(dex)
 save('exported; awaiting visual QA/runtime integration')
print('Requested templates exported; manual visual review required before publishing.',flush=True)
