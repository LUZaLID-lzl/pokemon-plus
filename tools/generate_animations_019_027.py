#!/usr/bin/env python3
"""Reference-grounded 019–027 masters. Image CLI owns bounded retries."""
from pathlib import Path
import json,subprocess,sys
R=Path(__file__).resolve().parents[1]
items=json.loads((R/'assets/pokemon/gen1-019-027/manifest.json').read_text())['items']
identities={19:'small PURPLE quadruped mouse, cream muzzle and belly, round pink-lined ears, white incisors, long thin curled purple tail attached to rump',20:'stocky TAN brown rodent, cream belly and muzzle, large white front incisors, round ears, attached long thin tail; preserve bulky reference proportions',21:'small brown bird with tan face and chest, short sharp beak, reddish wings, pink feet, ragged dark brown head feathers',22:'large brown bird with a LONG straight pointed beak, long slender neck, red head crest, cream throat and chest, broad brown wings and attached tail',23:'PURPLE snake with yellow neck band, yellow belly and yellow attached tail rattle, coiled body, NO ARMS NO LEGS',24:'PURPLE cobra with broad attached hood, exact reference red/yellow/black hood markings, cream lower belly, attached coiled tail, NO ARMS NO LEGS',25:'YELLOW mouse, long black-tipped ears, two red cheeks, brown back stripes, attached angular lightning-bolt tail, small arms and feet; reference Pikachu identity, not Raichu',26:'ORANGE mouse, cream belly, long dark-tipped curled ears, yellow cheeks, long THIN dark tail attached to rump ending in a yellow lightning-bolt shape; reference Raichu, not yellow Pikachu',27:'small yellow-tan armored pangolin, precise segmented brick-like shell plates on head/back, cream muzzle and belly, small ears, short attached tail, short claws and four limbs; preserve shell, no back spikes'}
progress=R/'assets/animation-generation-019-027.json'
if progress.exists() and json.loads(progress.read_text()).get('completed'):
 print('Accepted templates complete; preserving approved sources.');sys.exit(0)
for item in items:
 d=item['id'];folder=R/f'assets/prompts/animation-{d:03}';folder.mkdir(parents=True,exist_ok=True)
 if d in (23,24):motion='Snake anatomy: smooth modest S-wave slithering with connected coils, head held steady. Happy gently lift head and sway; eat head dips and mouth opens/closes modestly; sleep eyes CLOSED throughout, body coiled low and head resting; attack anticipation then small head/neck strike and recover. Never invent limbs, detach head, or stretch overall body.'
 elif d in (21,22):motion='Bird anatomy: eight alternating stepping phases with feet articulation, small attached wing balance and tail sway. Happy small hop with wing lift; eat beak dips/nibbles then recovers; sleep eyes CLOSED throughout, head tucked low and wings folded; attack brace then modest beak thrust/wing extension/recover. Keep long beak/neck of Fearow stable, never turn into a different bird.'
 elif d in (25,26):motion='Mouse anatomy: eight biped alternating foot walk phases, arms counterbalance, attached lightning tail moves subtly. Happy small hop; eat head lowers and nibble/chew/swallow; sleep eyes CLOSED throughout, resting low with ears and tail relaxed; attack brace then small arm/body lunge and recover. Ears/tail stay attached, no electric effects.'
 else:motion='Rodent/pangolin anatomy: eight quadruped alternating-foot walk phases with true joint articulation; tiny body bob. Happy small lift/bounce; eat head lowers and nibble/chew/swallow; sleep eyes CLOSED throughout, low tucked resting pose; attack anticipation, brace, modest head/front-foot lunge, recoil and recover. No biped redesign or body stretching.'
 prompt=f'''Create production MASTER SPRITESHEET national dex {d:03} {item['english']}. The attached image is EXACT approved identity and pixel art. SAME creature in all cells. Preserve {identities[d]}. Match reference proportions, thin deep-navy outline, palette and facial features. Never evolve, redesign or mix species.
EXACTLY EIGHT COLUMNS x SIX ROWS = 48 ISOLATED complete sprites. Every row has exactly EIGHT distinct sequential poses, especially last row. Strict regular 8x6 grid; no omitted, merged or duplicated cells. Each sprite confined to its own cell, maximum silhouette 65% cell width and 70% cell height including beak, wings, tail and ears, with generous completely transparent gutters. Same logical native48x48 canvas, maximum40px silhouette. Fixed global scale and camera facing LEFT, feet/resting baseline consistent. Crisp low-resolution pixel art, <=16 opaque colors, square pixels, continuous 1px navy outer outline. No smoothing, blur, shadows, props, text, labels, grids, particles, checkerboards, colored fringe or floor. REAL transparent alpha background.
{motion}
ROW1 IDLE: eight subtle breathing/blinking phases, stable volume, seamless loop.
ROW2 TRAVEL: eight anatomy-specific movement phases IN PLACE, seamless loop. No sliding unchanged sprite.
ROW3 HAPPY: anticipation, push, lift, small apex, descend, contact, settle, neutral.
ROW4 EAT: lower head, open mouth, nibble, chew, swallow, raise, settle, neutral. Seamless loop, no food props.
ROW5 SLEEP: ALL EIGHT frames eyes FULLY CLOSED, low resting posture and tiny slow breathing phases. Never standing with open eyes. Seamless loop.
ROW6 ATTACK: anticipation, brace, lean/extend, small lunge, peak, recoil, recover, neutral. EXACTLY EIGHT COMPLETE COMPACT creatures in eight separated cells. No elongated whole-body pose, projectile or effects.
Priorities: exact reference identity, correct count48, stable scale/anatomy, gently spaced natural action phases, connected appendages, clean native pixels. Happy/attack frame8 recovers neutral. No whole-body scaling/stretching or cut-body deformation.
'''
 pf=folder/'master-v1.txt';out=R/f'assets/source/animation-{d:03}-master-v1.png'
 if not out.exists():
  pf.write_text(prompt);print(f'Generating {d:03} {item["name"]}',flush=True)
  with (folder/'generation-v1.log').open('w') as log:
   p=subprocess.run([sys.executable,str(Path.home()/'.codex/skills/mobiiot-imagegen/scripts/imagegen.py'),'edit','--image',str(R/f'assets/pokemon/gen1-019-027/{d:03}@8x.png'),'--prompt-file',str(pf),'--output',str(out)],stdout=log,stderr=subprocess.STDOUT)
  if p.returncode:sys.exit(p.returncode)
 progress.write_text(json.dumps({'scope':'019-027','generated':[x['id'] for x in items if (R/f"assets/source/animation-{x['id']:03}-master-v1.png").exists()],'completed':False,'status':'pending QA/export/integration'},indent=2)+'\n')
print('Nine masters generated; pending QA/export/integration',flush=True)
