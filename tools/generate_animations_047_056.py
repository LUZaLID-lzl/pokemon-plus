#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Reference-grounded 047-056 masters; installed skill owns all six attempts."""
from pathlib import Path
import argparse,json,subprocess,sys
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--start',type=int,default=47);p.add_argument('--end',type=int,default=56);p.add_argument('--version',type=int,default=1);args=p.parse_args()
assert 47<=args.start<=args.end<=56 and args.version>0
state=R/'assets/animation-generation-047-056.json'
previous=json.loads(state.read_text()) if state.exists() else {}
generated=previous.get('generated',[]);exported=previous.get('exported',[])
items=json.loads((R/'assets/pokemon/plan-047-056.json').read_text())['items']
identities={
47:'squat orange six-legged crab insect, two front pincers and four short rear legs, BLANK WHITE eyes with NO black pupils, ONE broad LARGE RED mushroom cap with cream spots attached by a pale stalk over back; never two small mushrooms',
48:'round fluffy PURPLE furball, two large RED compound eyes, two pale straight attached antennae, two tiny pale hands and pale feet, small mouth with two tiny fangs, no wings',
49:'pale LAVENDER moth with FOUR broad light-purple connected wing lobes, small head with three short forehead points, large RED compound eyes, short thin attached legs and compact segmented body',
50:'ONE smooth BROWN rounded upright mole body, tiny black eyes, large oval PINK nose, connected compact jagged BROWN earth base; NO visible limbs, hair, teeth or tail',
51:'EXACTLY THREE smooth BROWN rounded mole heads joined in ONE shared connected BROWN earth base, middle/back head tallest, two front heads shorter, each tiny black eyes and oval PINK nose; no limbs, hair or tail',
52:'upright CREAM cat with GOLD oval forehead coin, pointed dark-lined ears, large oval eyes, short connected whiskers, tiny hands, brownish feet and long thin attached tail with curled BROWN tip',
53:'sleek CREAM QUADRUPED cat, RED round forehead jewel, narrow red eyes, short attached whiskers, four compact paws, rounded dark-lined ears and long thin cream tail curled at end; no mane or gold coin',
54:'round YELLOW duck with broad PALE CREAM bill, small white eyes and black pupils, exactly THREE short dark head hairs, two short yellow hands/wings, pale webbed feet and small attached yellow tail; NO jewel',
55:'BLUE biped duck, pale long bill, RED forehead jewel, three swept blue head spikes, white eyes with dark pupils, two blue arms with pale webbed hands, pale webbed feet and long attached blue tail',
56:'round shaggy CREAM monkey, PINK pig-like nose, small pointed pink-lined ears, expressive eyes, two cream arms with BROWN hands, two short legs with brown feet and long thin attached cream tail; no metal cuffs or red bands',
}
motions={
47:'Six-legged insect: compact alternating jointed-leg crawling cycle, front pincers distinct from four rear legs. One large mushroom cap stays attached with fixed cap size, gently follows body. Happy gentle leg lift, eat pincer/mouth movement, sleep low with both eyes CLOSED, attack small pincer thrust and recovery. Never add pupils or detach mushroom.',
48:'Round furball biped: alternate foot and arm articulation with attached antennae softly following. Fur/body volume fixed. Happy small hop, eat attached hand/mouth phases, sleep lowered compact resting body with both compound-eye lids fully CLOSED. Attack small arm thrust and recoil. No floating fuzz.',
49:'Moth: movement is in-place FLIGHT using eight real connected-wing articulation phases, not walking feet. Four wing lobes remain attached. Happy wing lift, eat mouth/leg phases, sleep compact folded-wing rest with eyes CLOSED, attack short wing/body thrust/recoil/recovery. No detached scales or effects.',
50:'Limbless mole with connected earth base: movement is gentle head/body lean left/right within its own attached earth silhouette; do not add walking feet or arms. Eight real changing head/nose/mouth poses, not a translated identical image. Happy cheerful head turn, eat subtle mouth/nose movement, sleep all eyes CLOSED with gentle resting head tilt, attack head leans forward then recoils and returns. Fixed body scale and earth silhouette volume; no floating dirt or motion particles.',
51:'Three limbless moles in ONE connected shared earth base. ALL eight frames have EXACTLY THREE complete heads, each with its own nose and eyes. Movement uses gently alternating individual head leans, never walking feet. Happy small staggered head turns, eat subtle nose/mouth phases, sleep ALL THREE pairs of eyes CLOSED for ALL eight poses with small staggered resting head tilts, attack three heads lean in sequence/recoil/recover. Heads remain attached to base, no merging, extra heads, arms, legs, floating dirt or translating identical sprite.',
52:'Small upright cat: eight alternating foot phases, hands counterbalance, stable torso/head volume, forehead coin and whiskers stay attached. Tail softly follows while brown curled tip stays intact. Happy small hop, eat short hand/mouth movement, sleep low curled CLOSED-eye rest, attack short attached paw jab/recoil/recover.',
53:'Compact quadruped cat: eight alternating paw phases with real knee articulation; preserve straight back, fixed torso/head volume, forehead jewel and whiskers attached. Tail softly follows, never disconnected. Happy small hop, eat head dip and chewing, sleep low curled CLOSED-eye rest, attack forepaw brace and small lunge/recoil/recover.',
54:'Round biped duck: eight alternating webbed-foot waddling phases, short wings counterbalance, bill and three hairs stay attached. Happy small hop, eat small bill open/close phases with head dips, sleep low resting eyes CLOSED, attack small wing jab/lean/recoil/recover. No objects in hands, no extra hair.',
55:'Biped duck: eight alternating webbed-foot phases with real ankle/knee motion and attached arms/tail counterbalance. Red jewel and head spikes fixed. Happy small hop, eat bill movement and head dip, sleep low eyes CLOSED rest, attack compact webbed-hand jab/recoil/recover, no elongated body.',
56:'Compact monkey: eight alternating foot and attached-arm phases, stable shaggy torso/head, tail stays attached and softly follows. Happy small hop, eat small hand/mouth phases without food, sleep low curled eyes CLOSED, attack short fist jab, brace/recoil/recover. No detached fur, metal cuffs or new props.'
}
common=(R/'assets/prompts/animation-027/master-v1.txt').read_text()
def save(status,failed=None):
    state.write_text(json.dumps(dict(previous,scope='047-056',generator='mobiiot-imagegen',generated=generated,exported=exported,completed=False,status=status,failedNationalId=failed,runtimeIntegrated=False),ensure_ascii=False,indent=2)+'\n')
for item in items:
    dex=item['nationalId']
    if not args.start<=dex<=args.end:continue
    folder=R/f'assets/prompts/animation-{dex:03}';folder.mkdir(exist_ok=True)
    prompt=folder/f'master-v{args.version}.txt';log=folder/f'generation-v{args.version}.log';source=R/f'assets/source/animation-{dex:03}-master-v{args.version}.png'
    text=common.replace('027 Sandshrew',f'{dex:03} {item["english"]}')
    a=text.index('Preserve ');b=text.index('. Match reference',a);text=text[:a]+'Preserve '+identities[dex]+text[b:]
    a=text.index('\n',text.index('REAL transparent alpha background.'));b=text.index('\nROW1',a);text=text[:a]+'\n'+motions[dex]+text[b:]
    if dex in (50,51):text=text.replace('Happy small hop','Happy head turn').replace('push, lift, small apex, descend, contact','gently tilt, turn, cheerful peak, return, rest')
    if not source.exists():
        if log.exists():raise SystemExit('Prior log exists: preserve it and choose a new revision, never add an outer service retry.')
        prompt.write_text(text);print(f'Generating {dex:03} {item["english"]}',flush=True)
        with log.open('w') as output:
            result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-047-056/{dex:03}@8x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=output,stderr=subprocess.STDOUT)
        if result.returncode:save('image service failed; entire batch stopped at retry limit',dex);sys.exit(result.returncode)
    if dex not in generated:generated.append(dex)
    save('generated; measuring/exporting')
    result=subprocess.run([sys.executable,str(R/'tools/export_pet_animation.py'),str(dex),'--identity',f'assets/pokemon/gen1-047-056/{dex:03}.png','--name',item['name'],'--version',str(args.version),'--alpha-threshold','230'])
    if result.returncode:save('source rejected by measured export; review required',dex);sys.exit(result.returncode)
    if dex not in exported:exported.append(dex)
    save('exported; awaiting visual QA/runtime integration')
print('Requested templates exported; visual QA required before runtime publishing.',flush=True)
