#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Reference-grounded 067-076 masters; installed skill owns all six attempts."""
from pathlib import Path
import argparse,json,subprocess,sys
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--start',type=int,default=67);p.add_argument('--end',type=int,default=76);p.add_argument('--version',type=int,default=1);args=p.parse_args()
assert 67<=args.start<=args.end<=76 and args.version>0
state=R/'assets/animation-generation-067-076.json'
previous=json.loads(state.read_text()) if state.exists() else {}
generated=previous.get('generated',[]);exported=previous.get('exported',[])
items=json.loads((R/'assets/pokemon/plan-067-076.json').read_text())['items']
identities={p['nationalId']:p['anatomy'] for p in items}
motions={d:'Eight genuine articulated whole-body poses per action. Fixed head and torso volume. Walk alternating bent knees and short foot steps, arms counterbalance; happy small hop; eat attached hand/mouth movement without props; sleep low resting body with ALL eyes fully CLOSED in ALL eight frames; attack compact attached arm jab/recoil/recovery. Keep all distinguishing features attached and visible.' for d in range(67,77)}
motions[67]+=' Exactly TWO arms, TWO legs; three head ridges and gold belt buckle remain attached, no tail.'
motions[68]+=' EXACTLY FOUR attached muscular arms in TWO shoulder pairs in EVERY pose, all four fists visible and distinct, exactly TWO legs. Sleep four fists tucked around low body, do not merge or remove arms. No tail.'
motions[69]='Small rooted plant: eight alternating bent-root-foot walking phases; two leaf arms counterbalance, yellow bell head and pink lip move gently while brown stem stays attached. Happy small root lift, eat small pink mouth chewing phases, sleep low bent stem with both eyes CLOSED and leaf arms lowered, attack compact connected leaf jab/head lean then recovery. No human hands or animal legs, no floating leaves, fixed head and stem sizes.'
motions[70]='Limbless pitcher-bell plant: NO FEET. Travel is gentle in-place floating with alternating leaf-arm angles and attached hooked vine counterbalance, eight real leaf/mouth poses rather than translated identical image. Happy slight leaf lift, eat pink lip open/close phases, sleep low tilted hanging body with BOTH EYES fully CLOSED for all eight frames, attack connected head lean and leaves thrust then recoil/recovery. Keep two leaves and brown vine attached; no body elongation.'
motions[71]='Pitcher plant: NO FEET. Travel uses gentle in-place floating, articulated attached side leaves, mouth lid and attached vine counterbalance. Eight changing real whole-body poses, no translated identical sprite. Happy leaf/lid lift, eat subtle lip chewing with attached lid movement, sleep low tilted resting pitcher, ALL EYES CLOSED throughout and lid relaxed, attack short mouth/leaf lean then recoil/recovery. Vine end leaf and lid never detach, no effects or stretching.'
for d in (72,73):motions[d]='Floating jellyfish: NO ARMS, FEET or walking legs. Eight real alternating bends of fully attached tentacles to swim in place, fixed dome volume and TWO red gems. Happy gentle tentacle curl, eat subtle underside mouth movement without props, sleep low floating rest with eyes FULLY CLOSED for all eight poses, tiny attached tentacle breathing phases; attack tentacles thrust softly forward, recoil and recover. No water particles, disconnected tentacles or body stretching.'
motions[72]+=' Exactly TWO tentacles in every pose, connected to underside.'
motions[73]+=' Dense connected tentacle bunch with EIGHT distinct visible pale tips in every pose, no disappearing tentacles.'
motions[74]='Rock creature has exactly TWO attached arms and NO LEGS or feet. In-place hovering movement uses eight genuine arm bends/rock head tilts with fixed round body size, never duplicate translated poses. Happy small arm lift, eat mouth chewing with arm bend, sleep low floating head tilt with both eyes FULLY CLOSED, attack one rock-fist jab followed by recoil/recovery. Never add ground, shadow, legs, particles or detached stones.'
motions[75]+=' Exactly FOUR attached rock arms and TWO short legs in every pose; all four fists clearly distinct. Sleep all four arms tucked around low rock body; no disappearing arms, detached rocks or tail.'
motions[76]+=' Stable BROWN polygonal shell, gray reptilian head and short attached limbs, pale claws. Short two-foot steps, shell never deforms or rotates independently; low curled sleep with eyes CLOSED. No tail or detached rocks.'
common=(R/'assets/prompts/animation-027/master-v1.txt').read_text()
def save(status,failed=None):
    state.write_text(json.dumps(dict(previous,scope='067-076',generator='mobiiot-imagegen',generated=generated,exported=exported,completed=False,status=status,failedNationalId=failed,runtimeIntegrated=False),ensure_ascii=False,indent=2)+'\n')
for item in items:
    dex=item['nationalId']
    if not args.start<=dex<=args.end:continue
    folder=R/f'assets/prompts/animation-{dex:03}';folder.mkdir(exist_ok=True)
    prompt=folder/f'master-v{args.version}.txt';log=folder/f'generation-v{args.version}.log';source=R/f'assets/source/animation-{dex:03}-master-v{args.version}.png'
    text=common.replace('027 Sandshrew',f'{dex:03} {item["english"]}')
    a=text.index('Preserve ');b=text.index('. Match reference',a);text=text[:a]+'Preserve '+identities[dex]+text[b:]
    a=text.index('\n',text.index('REAL transparent alpha background.'));b=text.index('\nROW1',a);text=text[:a]+'\n'+motions[dex]+text[b:]
    if args.version>=2:
        anatomy=identities[dex].replace('muscular humanoid biped','sturdy cartoon biped').replace('muscular arms','strong arms').replace('muscular ARMS','strong ARMS').replace('black briefs','dark shorts')
        text=f'''Draw a transparent pixel-art animation sheet of {dex:03} {item["english"]}, the same friendly game creature as the attached approved reference. Preserve its exact face, colors, proportions and connected anatomy: {anatomy}.
Layout: 8 columns by 6 rows, exactly 48 separate COMPLETE whole-body poses. All cells have generous transparent margins; no touching or cropped bodies. Same scale, left-facing camera and baseline. Native 48px style with subject within40px, at most15 opaque colors plus transparency, crisp square pixels and continuous thin deep-navy outline. Real transparent background, no text, ground, props, particles, checkerboard or shadows.
Rows top to bottom:
1. Idle: eight breathing and blinking phases, seamless loop.
2. Travel: eight genuine articulated in-place phases, seamless loop.
3. Happy: eight small cheerful movement phases returning to neutral.
4. Eat: eight head/mouth chewing phases without food props, seamless loop.
5. Sleep: eight low resting poses with ALL eyes CLOSED throughout, subtle breathing, seamless loop.
6. Action: eight compact game-action poses: prepare, brace, lean, extend, peak, recoil, recover, neutral. Keep complete body and every appendage attached in each cell.
Motion and anatomy requirements: {motions[dex]}
Keep consistent body volume and exact reference identity throughout. Every row must have eight DISTINCT complete poses, not copies shifted or stretched. No cut-body assembly. Gentle phase spacing for smooth playback. Last frame of happy/action recovers neutral.'''
    if dex==76 and args.version>=3:
        text+='''
Layout repair: use a spacious 1536x1152 transparent canvas with eight columns and six rows. Make each complete creature including every claw at most45% of cell width and55% of cell height. Center every body in its own cell, leaving at least50px fully transparent horizontal gutters and40px vertical gutters. No neighboring silhouettes may touch, including row6 poses4 and5. All 48 bodies need their own fully isolated alpha component. Row6 uses compact bent-arm gestures close to the shell, never a stretched neck or extended straight arm. Keep the shell rigid and both short arms/legs attached. Sleep fully closed eyes and low curled rest throughout all eight cells.'''
    if not source.exists():
        if log.exists():raise SystemExit('Prior log exists: preserve it and choose a new revision, never add an outer service retry.')
        prompt.write_text(text);print(f'Generating {dex:03} {item["english"]}',flush=True)
        with log.open('w') as output:
            result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-067-076/{dex:03}@8x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=output,stderr=subprocess.STDOUT)
        if result.returncode:save('image service failed; entire batch stopped at retry limit',dex);sys.exit(result.returncode)
    if dex not in generated:generated.append(dex)
    save('generated; measuring/exporting')
    result=subprocess.run([sys.executable,str(R/'tools/export_pet_animation.py'),str(dex),'--identity',f'assets/pokemon/gen1-067-076/{dex:03}.png','--name',item['name'],'--version',str(args.version),'--alpha-threshold','230'])
    if result.returncode:save('source rejected by measured export; review required',dex);sys.exit(result.returncode)
    if dex not in exported:exported.append(dex)
    save('exported; awaiting visual QA/runtime integration')
print('Requested templates exported; visual QA required before runtime publishing.',flush=True)
