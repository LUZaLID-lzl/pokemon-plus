#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Reference-grounded 057-066 masters; installed skill owns all six attempts."""
from pathlib import Path
import argparse,json,subprocess,sys
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--start',type=int,default=57);p.add_argument('--end',type=int,default=66);p.add_argument('--version',type=int,default=1);args=p.parse_args()
assert 57<=args.start<=args.end<=66 and args.version>0
state=R/'assets/animation-generation-057-066.json'
previous=json.loads(state.read_text()) if state.exists() else {}
generated=previous.get('generated',[]);exported=previous.get('exported',[])
items=json.loads((R/'assets/pokemon/plan-057-066.json').read_text())['items']
identities={57: 'round shaggy CREAM angry monkey, PINK pig-like snout, pointed pink-lined ears, two long cream arms with BROWN hands, two short legs with brown feet, dark wrist and ankle cuffs, NO TAIL', 58: 'ORANGE QUADRUPED puppy, black tiger-like stripes on back and legs, fluffy CREAM forehead tuft, cream muzzle and chest ruff, small rounded dark nose, four short paws and attached fluffy cream curled tail', 59: 'large ORANGE QUADRUPED canine, bold black stripes, flowing CREAM mane around neck and cheeks, cream forehead tuft and long fluffy cream tail, four orange paws with cream leg tufts, dark nose, no horns', 60: 'small round BLUE tadpole, WHITE circular belly with ONE clearly visible dark clockwise spiral, tiny pink oval mouth, two small blue feet, long attached thin blue tail ending in broad pale translucent-blue fin, NO ARMS', 61: 'round BLUE biped frog, WHITE belly with ONE clear dark clockwise spiral, two large rounded eye bumps with white eyes, two blue arms with white mitten hands, two small blue feet, NO TAIL', 62: 'stocky muscular BLUE biped frog, WHITE belly with ONE clear dark clockwise spiral, two large rounded eye bumps with determined white eyes, two muscular attached blue arms with white mitten fists, two sturdy blue feet, NO TAIL', 63: 'small YELLOW psychic fox-like biped with permanently CLOSED narrow eyes, pointed ears with brown interiors, BROWN shoulder and torso armor-like markings, two short arms with three fingers, bent knees with three-toed feet, long thick attached yellow tail with brown band, NO SPOON', 64: 'YELLOW psychic fox-like biped, pointed ears, long pointed muzzle, two short attached yellow moustache whiskers, RED forehead star, BROWN torso and shoulder armor markings, one attached thick yellow tail with brown stripe, EXACTLY ONE small silver spoon gripped by one hand', 65: 'YELLOW psychic fox-like biped, long pointed ears, long muzzle and two long attached yellow moustache whiskers, BROWN torso and shoulder armor markings, two bent legs, EXACTLY TWO small silver spoons, one gripped in each hand, NO TAIL and NO RED STAR', 66: 'small pale BLUE-GRAY muscular humanoid biped, exactly THREE blunt dark head ridges, large reddish eyes, short broad reptilian muzzle, two attached muscular arms with small hands and two short legs with feet, small attached thick tail, no clothing or belt'}
motions={d:'Eight genuine articulated whole-body poses per action; keep fixed torso and head volume, all distinguishing parts attached. Alternate real bent-knee foot phases and arm counterbalance for walk. Happy gentle hop, eat mouth movement without props, sleep low resting body with ALL EYES FULLY CLOSED for all eight poses, attack compact hand thrust/recoil/recovery.' for d in range(57,67)}
for d in (58,59):motions[d]='Quadruped canine: eight alternating paw/knee articulation phases, straight compact back, fixed head/body volume. Mane and tail follow softly, stripes and fur patches stay consistent. Happy small hop, eat head dip and chewing, sleep low curled with eyes CLOSED in all eight poses, attack forepaw brace/small lunge/recoil/recover. Four connected legs, no stretching.'
motions[60]='Limbless-armed tadpole: TWO feet only and NO ARMS. Eight actual alternating little foot poses with soft attached tail/fin counterbalance, fixed round body volume and exactly ONE belly spiral. Happy little hop, eat tiny mouth open/close, sleep low tilted resting with both eyes CLOSED throughout, attack small body lean/recoil/recover, no water effects.'
for d in (61,62):motions[d]+=' Maintain exactly ONE belly spiral, white mitten hands, eye bumps, no tail.'
motions[63]+=' Eyes are ALWAYS fully CLOSED, even idle and walking. No spoon or props. Attached thick tail never disappears. Sleep is lower curled posture distinct from seated idle.'
motions[64]+=' Exactly ONE spoon continuously GRIPPED by one hand, no detached spoon. One attached tail and red forehead star. Sleep both eyes completely CLOSED, hands and spoon tucked inward.'
motions[65]+=' Exactly TWO spoons, ONE continuously GRIPPED by EACH hand, no floating spoon. NO TAIL. Sleep both eyes completely CLOSED with spoons tucked inward. Moustache attached in every pose.'
motions[57]+=' Primeape has NO TAIL. Cuffs on wrists/ankles remain attached.'
motions[66]+=' Three head ridges and short tail stay attached. No clothing or belt.'
common=(R/'assets/prompts/animation-027/master-v1.txt').read_text()
def save(status,failed=None):
    state.write_text(json.dumps(dict(previous,scope='057-066',generator='mobiiot-imagegen',generated=generated,exported=exported,completed=False,status=status,failedNationalId=failed,runtimeIntegrated=False),ensure_ascii=False,indent=2)+'\n')
for item in items:
    dex=item['nationalId']
    if not args.start<=dex<=args.end:continue
    folder=R/f'assets/prompts/animation-{dex:03}';folder.mkdir(exist_ok=True)
    prompt=folder/f'master-v{args.version}.txt';log=folder/f'generation-v{args.version}.log';source=R/f'assets/source/animation-{dex:03}-master-v{args.version}.png'
    text=common.replace('027 Sandshrew',f'{dex:03} {item["english"]}')
    a=text.index('Preserve ');b=text.index('. Match reference',a);text=text[:a]+'Preserve '+identities[dex]+text[b:]
    a=text.index('\n',text.index('REAL transparent alpha background.'));b=text.index('\nROW1',a);text=text[:a]+'\n'+motions[dex]+text[b:]
    if False:text=text.replace('Happy small hop','Happy head turn').replace('push, lift, small apex, descend, contact','gently tilt, turn, cheerful peak, return, rest')
    if not source.exists():
        if log.exists():raise SystemExit('Prior log exists: preserve it and choose a new revision, never add an outer service retry.')
        prompt.write_text(text);print(f'Generating {dex:03} {item["english"]}',flush=True)
        with log.open('w') as output:
            result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-057-066/{dex:03}@8x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=output,stderr=subprocess.STDOUT)
        if result.returncode:save('image service failed; entire batch stopped at retry limit',dex);sys.exit(result.returncode)
    if dex not in generated:generated.append(dex)
    save('generated; measuring/exporting')
    result=subprocess.run([sys.executable,str(R/'tools/export_pet_animation.py'),str(dex),'--identity',f'assets/pokemon/gen1-057-066/{dex:03}.png','--name',item['name'],'--version',str(args.version),'--alpha-threshold','230'])
    if result.returncode:save('source rejected by measured export; review required',dex);sys.exit(result.returncode)
    if dex not in exported:exported.append(dex)
    save('exported; awaiting visual QA/runtime integration')
print('Requested templates exported; visual QA required before runtime publishing.',flush=True)
