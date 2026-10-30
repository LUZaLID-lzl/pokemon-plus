#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Resume reference-grounded MobiIoT generation without overwriting approved art."""
from pathlib import Path
import json,subprocess,sys
R=Path(__file__).resolve().parents[1]
identities={
2:('Ivysaur','妙蛙草','turquoise quadruped with red eyes, four sturdy legs, green back leaves and a large CLOSED pink flower bud. Keep bud closed, not a full flower'),
3:('Venusaur','妙蛙花','broad heavy turquoise quadruped, red eyes, squat four legs, large broad back leaves and a fully OPEN pink flower with yellow center on a visible thick stem. Preserve the open flower, never a bulb'),
4:('Charmander','小火龙','small orange biped lizard with cream belly, round head, blue eyes, two short arms, two legs, and attached long tail with a connected small flame at its tip. Keep tail flame attached and compact'),
5:('Charmeleon','火恐龙','red biped lizard, pointed horn-like head crest, cream belly, two clawed arms, two legs, long attached flaming tail. No wings, not orange Charmander'),
6:('Charizard','喷火龙','orange biped dragon, cream belly, two head horns, two arms, two legs, two attached teal-membrane wings and attached flaming tail. Keep wings complete within cell, do not add extra limbs'),
7:('Squirtle','杰尼龟','small blue biped turtle, large rounded head, brown shell with cream rim and cream belly, two short arms, two legs, compact curled tail. No cannons or ear wings'),
8:('Wartortle','卡咪龟','blue biped turtle with brown shell, cream belly and rim, TWO large feathery pale-blue ears and bushy curled pale-blue tail, two arms and two legs. Preserve ear and tail identity, no cannons'),
9:('Blastoise','水箭龟','heavy blue biped turtle, brown shell, cream belly and shell rim, two short ears, TWO short gray water cannons emerging from shell shoulders, two arms and two strong legs. Cannons stay attached and contained, no water effects')}
base=(R/'assets/templates/pet-animation-48/sample-001.txt').read_text()
layout=base[base.index('EXACTLY 8 COLUMNS'):base.index('ROW 1 IDLE:')]
for dex,(english,chinese,identity) in identities.items():
    folder=R/f'assets/prompts/animation-{dex:03}';folder.mkdir(parents=True,exist_ok=True)
    prompt=f'Create a production animation MASTER SPRITESHEET for national dex {dex:03} {english}. The attached image is the EXACT approved identity and pixel style. Draw the SAME {english} in every cell. Preserve: {identity}. Match reference head/body/appendage proportions and colors. Never evolve or mix species.\n\n'+layout
    quad=dex in (2,3)
    prompt+='''ROW 1 IDLE: eight sequential subtle breathing and blinking phases; feet anchored, blink in frames5-6, stable body volume. Seamless loop.
ROW 2 WALK: eight sequential true '''+('quadruped diagonal-leg walk' if quad else 'biped alternating left/right foot walk')+''' phases IN PLACE: contact, down, passing, up, opposite contact, down, passing, up. Real joint and foot articulation, tiny body bob <=1 native pixel. Tail/flower/ears/wings sway gently without identity change. Feet alternate visibly, no sliding unchanged sprite, no whole-body deformation. Seamless loop, fixed orientation.
ROW 3 HAPPY: anticipation, push-off, lift, small apex, descend, contact, settle, neutral; natural small hop <=3 native pixels; never scale the body. Frame1 and8 return to idle.
ROW 4 EAT: head lowers slightly, mouth opens, nibble, chew, swallow, raise head, settle, neutral. Anatomically connected head, anchored feet, no food props. Seamless loop.
ROW 5 SLEEP: all eight frames show eyes CLOSED throughout, natural low resting pose, limbs tucked, small slow breathing, attached appendages resting safely; no standing poses or open eyes, no sleep symbols. Seamless loop.
ROW 6 ATTACK: lean back, brace, lean forward, extend arms/front feet, small lunge <=2 native pixels, recoil, recover, neutral. Fixed front-left camera, no projectile/fire/water/vine/effects outside body. Frame1 and8 match idle.
PRIORITIES: reference identity, 8x6 exact count/order, eight distinct articulated sequential poses for each action, crisp low-resolution pixels, stable body volume, smooth phase spacing and continuous navy outline. No text, shadows, floor, grids, particles, blurring, checkerboards, duplicated unchanged frames or stretched bodies. Transparent real alpha background.
'''
    promptfile=folder/'master-v1.txt';promptfile.write_text(prompt)
    output=R/f'assets/source/animation-{dex:03}-master-v1.png'
    if output.exists():continue
    print(f'Generating {dex:03} {chinese}',flush=True)
    with (folder/'generation-v1.log').open('w') as log:
        result=subprocess.run([sys.executable,str(Path.home()/'.codex/skills/mobiiot-imagegen/scripts/imagegen.py'),'edit','--image',str(R/f'assets/pokemon/gen1-001-009/{dex:03}@8x.png'),'--prompt-file',str(promptfile),'--output',str(output)],stdout=log,stderr=subprocess.STDOUT)
    if result.returncode:
        print(f'STOP: {dex:03} generation failed, see {folder}/generation-v1.log',flush=True);sys.exit(result.returncode)
    progress={'scope':'002-009 reference-grounded animation masters','generated':[i for i in identities if (R/f'assets/source/animation-{i:03}-master-v1.png').exists()],'status':'generating','completed':False}
    (R/'assets/animation-generation-002-009.json').write_text(json.dumps(progress,indent=2)+'\n')
print('All 8 masters generated; pending visual QA/export/integration',flush=True)
