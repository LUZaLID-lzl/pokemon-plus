#!/usr/bin/env python3
"""Generate genuinely closed-eye sleep bodies, preserving five reviewed full rows."""
from pathlib import Path
import argparse,hashlib,json,subprocess,sys
from statistics import median
from PIL import Image
from pet_identity import groups
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('dex',type=int,choices=[48,53,56]);args=p.parse_args();dex=args.dex
folder=R/f'assets/prompts/animation-{dex:03}'
source=R/f'assets/source/animation-{dex:03}-sleep-v2.png'
master=R/f'assets/source/animation-{dex:03}-master-v2.png'
prompt=folder/'master-v2.txt';log=folder/'generation-v2.log'
identity=('Venonat: ONE round fluffy PURPLE furball, two attached pale antennae, tiny pale hands/feet, two tiny fangs. BOTH compound eyes FULLY covered by PURPLE eyelids matching body; ONLY TWO thin dark horizontal CLOSED curves remain, NO RED at either eye, no circles, no pupils.' if dex==48 else 'Persian: sleek CREAM quadruped cat, ONE RED forehead jewel, dark ear interiors, attached whiskers and curled cream tail. BOTH eyes FULLY covered by CREAM eyelids; ONLY TWO thin navy CLOSED curves remain, NO RED EYES, no pupils. Keep the red forehead jewel distinct from the closed eyes.')
if dex==56:identity='Mankey: compact shaggy CREAM monkey, PINK pig nose, two pointed pink-lined ears, attached cream arms with brown hands, brown feet and one long attached curled cream tail. Low tucked resting pose, both eyes fully closed as two dark curves, no pupils or open eyes.'
text=f'''Draw exactly EIGHT complete, separated, sleeping {identity}
Use the attached approved identity as the reference for colors, anatomy and proportions. A SINGLE HORIZONTAL ROW, EIGHT columns, eight full bodies total, all sleeping with eyes CLOSED. No extra row, text, border, grid, props, effects or particles. Real transparent alpha background.
Native48px pixel design, square hard pixels, continuous thin deep navy outline. Every body fits within 65% cell width, large transparent gaps. Camera three-quarter LEFT, full anatomy stays attached. Preserve fixed body/head volume and size; no stretching or copied identical poses.
Eight gentle distinct sleeping breathing phases: 1 low resting posture, head slightly down; 2 tiny chest rise; 3 slight head tilt left; 4 slow chest expansion; 5 relaxed chest; 6 slight head tilt right; 7 tiny chest fall; 8 settle close to first rest. Eyes remain completely CLOSED in ALL EIGHT frames. Resting paws tucked and attached, tail stays attached. Do not draw awake eyes even in the last frame.
This is a revised sleep row. The previous source sleep row was absent or retained open eyes and was rejected. The resulting eight whole bodies will replace only the sleep action, while the five other original generated rows are retained pixel-exactly.'''
if not source.exists():
    if log.exists():raise SystemExit('Existing attempt log: do not add outer service retries')
    prompt.write_text(text)
    with log.open('w') as f:
        result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-047-056/{dex:03}@8x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=f,stderr=subprocess.STDOUT)
    if result.returncode:
        state=R/'assets/animation-generation-047-056.json';s=json.loads(state.read_text());s.update(status='sleep revision service failed; entire image batch stopped at retry limit',failedNationalId=dex);state.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n');sys.exit(result.returncode)
# Assembly is a separate command, after visual review of generated sleep row.
print(source)
