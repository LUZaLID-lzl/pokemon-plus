#!/usr/bin/env python3
"""Generate nine identity-grounded animation masters; CLI owns bounded retries."""
from pathlib import Path
import json, subprocess, sys
R=Path(__file__).resolve().parents[1]
items=json.loads((R/'assets/pokemon/gen1-010-018/manifest.json').read_text())['items']
base=(R/'assets/templates/pet-animation-48/sample-001.txt').read_text()
layout=base[base.index('EXACTLY 8 COLUMNS'):base.index('ROW 1 IDLE:')]
identities={10:'green segmented caterpillar, red Y-shaped antenna, yellow belly and yellow ring markings, tiny paired feet',11:'rigid angular green cocoon with narrow eyes; NO legs, arms, wings or mouth added',12:'purple butterfly body, red compound eyes, two antennae, attached large pale wings with dark veins, small feet',13:'tan segmented caterpillar, pink rounded nose and feet, single white head horn and white tail stinger',14:'yellow angular cocoon, dark eyes and small existing chest ridges; NO legs, wings or mouth added',15:'yellow-and-black striped bee, red eyes, two attached translucent wings, two long white forearm drills, pointed abdominal stinger; preserve all attached features',16:'small round brown bird, cream face and chest, short brown head crest, pink feet, compact folded wings and tail',17:'brown bird, cream chest, swept red head crest, pink feet, long striped tail and attached wings',18:'large brown bird, cream chest, long flowing red-and-yellow crest, broad attached wings, pink feet and fan tail'}
progress_file=R/'assets/animation-generation-010-018.json'
if progress_file.exists() and json.loads(progress_file.read_text()).get('completed'):
 print('Accepted 010–018 templates already complete; preserving approved masters.');sys.exit(0)
for item in items:
 d=item['id']; folder=R/f'assets/prompts/animation-{d:03}';folder.mkdir(parents=True,exist_ok=True)
 if d in (11,14):
  motion='Rigid cocoon anatomy: subtle angular whole-shell rocking and tiny in-place scoots, shell plates retain shape. No walking limbs. Happy is a small rock/bounce, eat is a gentle shell pulse without invented mouth, sleep eyes fully closed and shell resting tilted, attack is brace/forward body tackle/recover. Never stretch the shell.'
 elif d in (10,13):
  motion='Caterpillar anatomy: travel uses a smooth segmented crawling wave and tiny paired feet, head remains recognizable. Happy is head lift and small bounce, eat head dips and mouth nibbles, sleep body curled low with eyes CLOSED, attack head rears then compact forward tackle and recovery. No biped arms or legs.'
 elif d in (12,15):
  motion='Flying insect anatomy: travel is eight smoothly spaced wingbeat phases in place, attached wings raise/spread/downstroke/fold and repeat, body floats subtly. Idle gentle wing movement. Happy buoyant lift, eat head dips, sleep eyes CLOSED with wings folded and body resting, attack brace then compact forward thrust and recover. No disconnected wings or weapon effects.'
 else:
  motion='Bird anatomy: travel is eight alternating stepping phases with small attached wing balance and tail sway; real feet articulation, no sliding. Happy tiny hop with wing lift, eat beak lowers and nibbles, sleep eyes CLOSED, head tucked and folded wings, attack anticipation then small beak lunge/wing extension/recover. Preserve attached crest, tail, wings and feet.'
 prompt=f'Production MASTER SPRITESHEET: national dex {d:03} {item["english"]}. Attached is EXACT approved identity and pixel art. Same creature in all 48 cells. Preserve {identities[d]}. Match reference colors, proportions, outline and face. Never evolve or mix identities.\n'+layout+'\n'+motion+'\nROWS EXACTLY: 1 IDLE subtle breathing/blinking; 2 TRAVEL anatomy-specific movement; 3 HAPPY anticipation/lift/apex/descend/contact/settle; 4 EAT lower/nibble/chew/swallow/recover; 5 SLEEP closed eyes in EVERY frame, eight subtle slow breathing phases; 6 ATTACK anticipation/brace/extend/lunge/recoil/recover. Each row exactly EIGHT visibly distinct sequential poses with gentle phase spacing. Loops idle/travel/eat/sleep seamless; happy/attack return to neutral on frame8. Shared camera facing LEFT in every cell. Same scale throughout, no whole-body squash/stretch. Keep silhouette connected, including wings/antennae/stingers. Fully transparent alpha gutters. No text, grids, floor, shadows, props, particles, projectiles, blur or checkerboard.'
 pf=folder/'master-v1.txt';pf.write_text(prompt)
 out=R/f'assets/source/animation-{d:03}-master-v1.png'
 if not out.exists():
  print(f'Generating {d:03} {item["name"]}',flush=True)
  with (folder/'generation-v1.log').open('w') as log:
   p=subprocess.run([sys.executable,str(Path.home()/'.codex/skills/mobiiot-imagegen/scripts/imagegen.py'),'edit','--image',str(R/f'assets/pokemon/gen1-010-018/{d:03}@8x.png'),'--prompt-file',str(pf),'--output',str(out)],stdout=log,stderr=subprocess.STDOUT)
  if p.returncode: sys.exit(p.returncode)
 (R/'assets/animation-generation-010-018.json').write_text(json.dumps({'scope':'010-018','generated':[x['id'] for x in items if (R/f"assets/source/animation-{x['id']:03}-master-v1.png").exists()],'completed':False,'status':'generated masters; pending QA/export/integration'},ensure_ascii=False,indent=2)+'\n')
print('All nine masters generated; pending QA/export/integration',flush=True)
