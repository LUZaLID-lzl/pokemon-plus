#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate eight complete Slowbro sleep bodies, never paint over eye pixels."""
from pathlib import Path
import subprocess,sys,json,argparse
R=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('dex',type=int,choices=[80,82]);p.add_argument('--version',type=int,default=2);args=p.parse_args();dex=args.dex;version=args.version;folder=R/f'assets/prompts/animation-{dex:03}'
source=R/f'assets/source/animation-{dex:03}-sleep-v{version}.png';prompt=folder/f'master-v{version}.txt';log=folder/f'generation-v{version}.log'
text='''Create eight complete pixel-art sleeping poses of the friendly pink creature in the attached reference, with the gray spiral shell still attached to its tail. Keep the pink body, cream muzzle/belly, two arms, two legs, ears and attached shell.
SINGLE horizontal row of EIGHT separate whole bodies on real transparent alpha. Left-facing camera, fixed body/head volume, crisp native48px pixel style, continuous thin deep-navy outline, at most15 opaque colors. Every complete silhouette stays within60% of cell width with generous transparent gutters.
All eight poses are low lying or curled resting bodies with tiny breathing/head angle changes. ALL eyes are fully CLOSED in every pose. Pink body's eye is only a thin navy curved line. The attached gray SHELL has TWO EYES: both are FULLY COVERED by gray eyelids matching shell color; only two narrow dark horizontal lines remain, with NO white eyeball or black pupil dots. Keep other shell spiral/ridges visible but never draw an open eye. All limbs tucked, shell resting next to body and tail connected. No upright/awake recovery frame.
Draw eight distinct whole-body breathing phases, never translated duplicates, stretched bodies or separate body parts. No text, ground, shadows, particles, pillows, props or floating shell. The previous sleep row retained open shell eyes and is rejected; these eight complete closed-eye bodies will replace only that row.'''
if dex==82:
 text='''Draw ONE ROW of EIGHT complete sleeping triangular metallic clusters matching the attached approved reference. Every cluster contains exactly THREE clearly visible round metallic spheres touching each other in the same triangle: one upper sphere and TWO lower spheres. Each sphere remains separately readable with its own full round outline, attached screw and red-blue tipped side magnets. No sphere may disappear behind another, become flattened or turn into a magnet. Keep body volume identical to awake reference.
All THREE eyes are completely closed in EVERY cell: cover the white eyeballs entirely with gray eyelids; each sphere has ONE narrow dark horizontal closed line, NO WHITE EYE AREA and NO PUPILS. All three closed-eye faces point toward viewer-left and stay visible. Sleeping is low gentle hovering with relaxed magnets; no crouching or squashing the three spheres.
Exactly eight distinct tiny tilt/magnet resting phases, gentle seamless loop, all three spheres connected in each complete body. Real transparent alpha, no ground/text/shadow/effects/grid. Spacious gutters; complete cluster at most60% of cell width. Crisp native48px pixel style, thin continuous navy outline, at most15 opaque colors. Fixed camera and body volume, no translated duplicates or body-part assembly. Previous sleep row hid the two lower spheres and is rejected. These eight entire clusters replace only that sleep row.'''
if dex==80 and version>=3:
 text+='''
Specific repair: the gray shell may use ONLY medium gray, dark slate and navy in ALL EIGHT bodies. Absolutely NO white or cream pixels, white dots, white ovals, pupil dots or highlights anywhere on the SHELL. Pink body may still have cream muzzle/belly. Both shell eyes must be ONLY narrow navy closed lines on a solid medium-gray face. Remove every white spot on the shell, keeping dark spiral grooves.
Spacious layout: one row on3072x768 transparent canvas, eight384px-wide cells. Each whole sleeping body INCLUDING shell is at most160px wide and150px tall, centered in its cell; leave at least200px empty transparent gutter between silhouettes. Each silhouette fully isolated. Keep all eight bodies low lying and eyes closed; no seated upright transition.'''
if not source.exists():
 if log.exists():raise SystemExit('Prior task log exists; preserve six-attempt limit')
 prompt.write_text(text)
 with log.open('w') as out:r=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-077-086/{dex:03}@8x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=out,stderr=subprocess.STDOUT)
 if r.returncode:
  p=R/'assets/animation-generation-077-086.json';j=json.loads(p.read_text());j.update(status=f'{dex:03} sleep revision service failed; entire batch stopped',failedNationalId=dex);p.write_text(json.dumps(j,ensure_ascii=False,indent=2)+'\n');sys.exit(r.returncode)
print(source)
