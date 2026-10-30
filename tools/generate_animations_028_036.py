#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate reference-grounded masters; export only complete measured 48-pose sheets.

The image service CLI owns its six-attempt retry limit. Never retry it here.
Exported templates remain unpublished until visual QA and runtime integration.
"""
from pathlib import Path
import argparse
import json
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
CLI = Path.home() / '.codex/skills/mobiiot-imagegen/scripts/imagegen.py'
STATE = ROOT / 'assets/animation-generation-028-036.json'
parser=argparse.ArgumentParser()
parser.add_argument('--start',type=int,default=28)
parser.add_argument('--end',type=int,default=36)
parser.add_argument('--version',type=int,default=1)
parser.add_argument('--diagnostics',action='store_true',help='Run the skill client with safe error classification; same six-attempt loop')
args=parser.parse_args()
assert 28<=args.start<=args.end<=36 and args.version>0
items = json.loads((ROOT / 'assets/pokemon/gen1-028-036/manifest.json').read_text())['items']
identities = {
    28: 'ochre pangolin, cream muzzle/belly, brown triangular back quills, long white foreclaws, small pointed ears and attached thick pointed tail; Sandslash, NOT brick-armored Sandshrew',
    29: 'small pale BLUE female poison rabbit, tiny horn, broad ears with dark teal interiors, dark blue spots, four short legs, small attached tail',
    30: 'medium BLUE quadruped Nidorina, pointed ears, short horn, dark blue spots/dorsal spines and attached stub tail; not bipedal Nidoqueen',
    31: 'large BLUE bipedal Nidoqueen, cream chest/belly armor, pointed ears, short horn, short strong arms, thick legs and attached thick blue tail',
    32: 'small PURPLE male poison rabbit, very large purple ears with teal interiors, prominent pointed horn, dark purple spots and short back spines, four short legs and attached stub tail',
    33: 'medium PURPLE quadruped Nidorino, long forehead horn, upright teal-lined ears, jagged purple dorsal spines, four sturdy legs and attached short tail',
    34: 'large PURPLE bipedal Nidoking, cream chest/belly, long forehead horn, pointed teal-lined ears, large dorsal spines, short strong arms with white claws, thick legs and attached long thick tail',
    35: 'small round PINK Clefairy, dark-tipped ears, one curled forelock, tiny eyes, short arms/feet, small attached pink back winglets and curled tail; no star or horn',
    36: 'larger pale PINK Clefable, long dark-tipped pointed ears, curled forelock, plump round body, short arms/feet, larger pointed pink fairy wings and attached curled tail; no star or horn',
}
common = (ROOT / 'assets/prompts/animation-027/master-v1.txt').read_text()
previous=json.loads(STATE.read_text()) if STATE.exists() else {}
generated = previous.get('generated',[])
exported = previous.get('exported',[])


def save(status, failed=None):
    STATE.write_text(json.dumps(dict(previous,**{
        'scope': '028-036', 'generator': 'mobiiot-imagegen',
        'generated': generated, 'exported': exported, 'completed': False,
        'status': status, 'failedNationalId': failed,
        'note': 'Measured export requires separate visual review before runtime publication.',
    }), indent=2) + '\n')


for item in items:
    dex = item['id']
    if not args.start<=dex<=args.end:continue
    folder = ROOT / f'assets/prompts/animation-{dex:03}'
    folder.mkdir(parents=True, exist_ok=True)
    prompt_file = folder / f'master-v{args.version}.txt'
    source = ROOT / f'assets/source/animation-{dex:03}-master-v{args.version}.png'
    log_file=folder / f'generation-v{args.version}.log'
    reference = f'assets/pokemon/gen1-028-036/{dex:03}.png'
    text = common.replace('027 Sandshrew', f'{dex:03} {item["english"]}')
    start = text.index('Preserve ')
    end = text.index('. Match reference', start)
    text = text[:start] + 'Preserve ' + identities[dex] + text[end:]
    start = text.index('\n', text.index('REAL transparent alpha background.'))
    end = text.index('\nROW1', start)
    motion = (
        'Quadruped anatomy: eight small alternating-foot walking phases, joints articulate while torso volume remains stable. Short tail, ears, horn and back spines stay attached. Happy small lift/bounce; eat head dips and mouth gently nibbles; sleep all eyes CLOSED, resting low with limbs tucked; attack brace, small front-foot/head thrust, recoil, recover.'
        if dex in (28, 29, 30, 32, 33) else
        'Biped anatomy: eight small alternating-foot walking phases, arms counterbalance, torso volume stable. Ears, horn, back spines/wings and tail remain attached. Happy small hop; eat head dips, gentle chew then recover; sleep all eyes CLOSED, low resting pose with limbs tucked; attack brace, small attached arm thrust/body lean, recoil, recover. No whole-body stretch or morphing.'
    )
    text = text[:start] + '\n' + motion + text[end:]
    if not source.exists():
        if log_file.exists():raise SystemExit(f'Refusing to overwrite prior request log: {log_file}; use a new version after user-authorized continuation.')
        prompt_file.write_text(text)
        print(f'Generating {dex:03} {item["english"]}', flush=True)
        with log_file.open('w') as log:
            result = subprocess.run([
                sys.executable, str(ROOT/'tools/mobiiot_image_diagnostics.py' if args.diagnostics else CLI), 'edit', '--image',
                str(ROOT / f'assets/pokemon/gen1-028-036/{dex:03}@8x.png'),
                '--prompt-file', str(prompt_file), '--output', str(source),
            ], stdout=log, stderr=subprocess.STDOUT)
        if result.returncode:
            save('image service failed; batch stopped at retry limit', dex)
            sys.exit(result.returncode)
    if dex not in generated:generated.append(dex)
    save('generated; measuring/exporting')
    result = subprocess.run([
        sys.executable, str(ROOT / 'tools/export_pet_animation.py'), str(dex),
        '--identity', reference, '--name', item['name'], '--alpha-threshold', '230',
        '--version',str(args.version),
    ])
    if result.returncode:
        save('source rejected by measured export; review required', dex)
        sys.exit(result.returncode)
    if dex not in exported:exported.append(dex)
    save('exported; awaiting visual QA/runtime integration')
print('Nine templates exported; separate visual QA and integration required.', flush=True)
