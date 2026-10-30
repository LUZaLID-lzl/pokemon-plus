#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate eight complete Slowbro sleep bodies, never paint over eye pixels."""
from pathlib import Path
import subprocess,sys,json
R=Path(__file__).resolve().parents[1];folder=R/'assets/prompts/animation-080'
source=R/'assets/source/animation-080-sleep-v2.png';prompt=folder/'master-v2.txt';log=folder/'generation-v2.log'
text='''Create eight complete pixel-art sleeping poses of the friendly pink creature in the attached reference, with the gray spiral shell still attached to its tail. Keep the pink body, cream muzzle/belly, two arms, two legs, ears and attached shell.
SINGLE horizontal row of EIGHT separate whole bodies on real transparent alpha. Left-facing camera, fixed body/head volume, crisp native48px pixel style, continuous thin deep-navy outline, at most15 opaque colors. Every complete silhouette stays within60% of cell width with generous transparent gutters.
All eight poses are low lying or curled resting bodies with tiny breathing/head angle changes. ALL eyes are fully CLOSED in every pose. Pink body's eye is only a thin navy curved line. The attached gray SHELL has TWO EYES: both are FULLY COVERED by gray eyelids matching shell color; only two narrow dark horizontal lines remain, with NO white eyeball or black pupil dots. Keep other shell spiral/ridges visible but never draw an open eye. All limbs tucked, shell resting next to body and tail connected. No upright/awake recovery frame.
Draw eight distinct whole-body breathing phases, never translated duplicates, stretched bodies or separate body parts. No text, ground, shadows, particles, pillows, props or floating shell. The previous sleep row retained open shell eyes and is rejected; these eight complete closed-eye bodies will replace only that row.'''
if not source.exists():
 if log.exists():raise SystemExit('Prior task log exists; preserve six-attempt limit')
 prompt.write_text(text)
 with log.open('w') as out:r=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/'assets/pokemon/gen1-077-086/080@8x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=out,stderr=subprocess.STDOUT)
 if r.returncode:
  p=R/'assets/animation-generation-077-086.json';j=json.loads(p.read_text());j.update(status='080 sleep revision service failed; entire batch stopped',failedNationalId=80);p.write_text(json.dumps(j,ensure_ascii=False,indent=2)+'\n');sys.exit(r.returncode)
print(source)
