"""Reference-grounded masters; skill owns six attempts, with no outer retry."""
from pathlib import Path
import argparse, json, subprocess, sys
R=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--start',type=int,default=37);parser.add_argument('--end',type=int,default=46);parser.add_argument('--version',type=int,default=1);args=parser.parse_args()
assert 37<=args.start<=args.end<=46 and args.version>0
state=R/'assets/animation-generation-037-046.json'
previous=json.loads(state.read_text()) if state.exists() else {}
generated=previous.get('generated',[]);exported=previous.get('exported',[])
items=json.loads((R/'assets/pokemon/gen1-037-046/manifest.json').read_text())['items']
identities={
37:'small orange-red quadruped fox, cream muzzle/chest, large pointed ears, curled forelock, SIX distinct curled orange fluffy tails all attached behind the compact body',
38:'cream pale-gold quadruped fox, pointed ears, small cream tuft, red eyes, NINE long fluffy cream tails all attached and fanning behind the compact body',
39:'round PINK balloon, large teal eyes, short pointed dark-lined ears, single curled forelock, tiny arms/feet, no tail and no wings',
40:'tall round PINK rabbit-like body, very long dark-lined upright ears, curled forelock, teal eyes, broad WHITE belly, tiny arms/feet, no tail',
41:'small BLUE EYELESS bat, purple inner wing membranes, pointed ears, open oval mouth with two tiny fangs and two attached thin lower legs. NO eyes may appear in any frame',
42:'large BLUE bat, enormous open dark mouth dominating the compact body, four short pale fangs, small narrow eyes, purple inner wing membranes and two attached thin legs',
43:'small round indigo-blue radish body, two little feet, tiny red eyes and small mouth, FIVE broad GREEN leaves attached atop the head, no arms or pot',
44:'round BLUE plant, short arms/feet, sleepy eyes, tiny drool attached to mouth, large reddish-brown lobed flower bud with four curled downward orange-brown sepals attached atop head',
45:'small dark BLUE plant with short arms/feet and red eyes, enormous rounded FIVE-petal RED flower with cream spots and orange center attached atop head, no green leaves',
46:'small orange SIX-legged crab insect, large white eyes with black pupils, two short front pincers and four short rear legs, exactly TWO red mushrooms with cream spots attached on the back',
}
motions={
'fox':'Quadruped fox: eight alternating foot walk phases, real knee/ankle articulation, stable head/body volume. All tails remain attached and retain their exact count, softly follow body motion without becoming separate creatures. Happy small hop, eat head dips and gentle chew, sleep low curled posture with all eyes CLOSED, attack brace/small forepaw lunge/recoil/recover.',
'round':'Short biped: eight alternating foot walk phases, tiny arms counterbalance, stable body volume. Ears and forelock remain attached and keep their length. Happy small hop; eat gentle mouth/arm phases; sleep low resting posture with all eyes CLOSED; attack attached arm jab and small lean, recoil, recover. No whole-body squash, stretch or scaling.',
'bat':'Bat anatomy: movement is IN-PLACE flight with eight genuine connected-wing articulation phases, never leg-walking. Wings fold and extend within cell, head/mouth/body scale stable. Happy small wing lift; eat gentle mouth/fang movement; sleep compact hanging/resting pose with wings tucked around body. No props, no new feet or arms. Attack small connected-wing/body thrust, recoil/recover. Wing membranes, ear tips and legs always attached.',
'plant':'Plant anatomy: eight tiny alternating-foot steps, stable body and attached head foliage/flower, no full-body scaling. Leaves/petals softly follow motion and retain their count. No arms may be added to Oddish. Happy small lift, eat mouth moves and body gently settles, sleep low resting posture all eyes CLOSED (Gloom retains sleepy eyes), attack brace then small head/body lean, recoil and recover. No loose leaves, spores, projectiles or detached flower.',
'crab':'Six-legged insect anatomy: eight compact alternating-foot crawling phases; front pincers remain distinct from four rear legs, all attached. TWO mushrooms always remain attached, neither duplicated nor merged. Happy tiny lift; eat small pincer/mouth phases without props; sleep low resting posture with BOTH eyes CLOSED; attack brace/short attached pincer thrust, recoil/recover. No giant single mushroom, no added limbs or whole-body stretch.'
}
common=(R/'assets/prompts/animation-027/master-v1.txt').read_text()
def save(status,failed=None):
    state.write_text(json.dumps(dict(previous,scope='037-046',generator='mobiiot-imagegen',generated=generated,exported=exported,completed=False,status=status,failedNationalId=failed,runtimeIntegrated=False),ensure_ascii=False,indent=2)+'\n')
for item in items:
    dex=item['id']
    if not args.start<=dex<=args.end:continue
    folder=R/f'assets/prompts/animation-{dex:03}';folder.mkdir(exist_ok=True)
    prompt=folder/f'master-v{args.version}.txt';log=folder/f'generation-v{args.version}.log'
    source=R/f'assets/source/animation-{dex:03}-master-v{args.version}.png'
    text=common.replace('027 Sandshrew',f'{dex:03} {item["english"]}')
    a=text.index('Preserve ');b=text.index('. Match reference',a)
    text=text[:a]+'Preserve '+identities[dex]+text[b:]
    a=text.index('\n',text.index('REAL transparent alpha background.'));b=text.index('\nROW1',a)
    kind='fox' if dex<=38 else 'round' if dex<=40 else 'bat' if dex<=42 else 'plant' if dex<=45 else 'crab'
    text=text[:a]+'\n'+motions[kind]+text[b:]
    if dex==41:
        text=text.replace('breathing/blinking','breathing/ear movement').replace('eyes FULLY CLOSED','eyeless face unchanged; no eyes added')
    if not source.exists():
        if log.exists():raise SystemExit('Prior request log exists; preserve it and use a new version after authorized continuation: '+str(log))
        prompt.write_text(text);print(f'Generating {dex:03} {item["english"]}',flush=True)
        with log.open('w') as output:
            result=subprocess.run([sys.executable,str(R/'tools/mobiiot_image_diagnostics.py'),'edit','--image',str(R/f'assets/pokemon/gen1-037-046/{dex:03}@8x.png'),'--prompt-file',str(prompt),'--output',str(source)],stdout=output,stderr=subprocess.STDOUT)
        if result.returncode:save('image service failed; batch stopped at retry limit',dex);sys.exit(result.returncode)
    if dex not in generated:generated.append(dex)
    save('generated; measuring/exporting')
    result=subprocess.run([sys.executable,str(R/'tools/export_pet_animation.py'),str(dex),'--identity',f'assets/pokemon/gen1-037-046/{dex:03}.png','--name',item['name'],'--version',str(args.version),'--alpha-threshold','230'])
    if result.returncode:save('source rejected by measured export; review required',dex);sys.exit(result.returncode)
    if dex not in exported:exported.append(dex)
    save('exported; awaiting visual QA/runtime integration')
print('Ten templates exported; separate visual QA required before publishing.',flush=True)
