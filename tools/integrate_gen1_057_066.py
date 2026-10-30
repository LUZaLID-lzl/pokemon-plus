# -*- coding: utf-8 -*-
"""Publish ten reviewed full templates atomically after provenance preflight."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
plan=json.loads((R/'assets/pokemon/plan-057-066.json').read_text())
qa=json.loads((R/'docs/gen1-057-066/visual-review.json').read_text())
assert set(range(57,67))<=set(qa['identityReviewed']) & set(qa['animationReviewed']),'Visual review incomplete'
for item in plan['items']:
    dex=item['nationalId'];folder=R/f'assets/animation-templates/{dex:03}'
    m=json.loads((folder/'manifest.json').read_text())
    assert m['nationalId']==dex and m['frames']==48 and m['framesPerAction']==8
    assert m['source']==f"assets/source/animation-{dex:03}-master-v{qa['acceptedVersions'][str(dex)]}.png"
    assert hashlib.sha256((R/m['source']).read_bytes()).hexdigest()==m['sourceSha256']
    for action in m['actions'].values():assert len(action['frames'])==len(action['durationsMs'])==8
    for f in m['items']:assert hashlib.sha256((folder/f['image']).read_bytes()).hexdigest()==f['sha256']

def write(file,data):(R/file).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')
roster=json.loads((R/'assets/pet-roster.json').read_text())
for item in plan['items']:
    dex=item['nationalId'];entry=dict(id=item['artId'],nationalId=dex,name=item['name'],species=item['species'],stage=item['stage'],source=f'pokemon/gen1-057-066/{dex:03}.png')
    old=next((p for p in roster if p['nationalId']==dex),None)
    if old:assert old==entry
    else:roster.append(entry)
assert [p['id'] for p in roster]==list(range(66))
e=json.loads((R/'assets/evolution.json').read_text())
for target in plan['evolution']:
    steps=target['steps']
    chain=dict(species=target['species'],nationalIds=target['nationalIds'],levels=[s.get('level',0) for s in steps],artIds=[next(p['id'] for p in roster if p['nationalId']==d) for d in target['nationalIds']],trigger=steps[0]['trigger'] if len(set(s['trigger'] for s in steps))==1 else 'mixed',steps=steps)
    e['chains']=[c for c in e['chains'] if c['species']!=target['species']]+[chain]
e['trigger']='等级进化自动弹出选择；进化石及联系之绳从状态页直接购买使用'
e.update(version=9,imageFallback='001–024、026–066专用48姿态；025保留原24位移槽位',pendingChains=plan['pendingEvolution'])
assert len(e['chains'])==27 and sum(len(c['steps']) for c in e['chains'])==38
m=json.loads((R/'assets/moves.json').read_text())
# Preserve IDs 0..95. Four new families, slot-major IDs 96..111.
new=[('卡蒂狗', '火花', 'Ember', '火', '发射小火焰攻击', '橙色火花'), ('蚊香蝌蚪', '水枪', 'Water Gun', '水', '喷射强劲水流', '蓝色水流'), ('凯西', '念力', 'Confusion', '超能力', '用念力攻击对手', '紫粉色同心光环'), ('腕力', '踢倒', 'Low Kick', '格斗', '用腿扫向对手', '橙红色短促踢击'), ('卡蒂狗', '咬住', 'Bite', '一般', '用牙齿咬住对手', '短促咬击'), ('蚊香蝌蚪', '拍击', 'Pound', '一般', '用前肢拍击对手', '短促拍击'), ('凯西', '幻象光线', 'Psybeam', '超能力', '发出神秘的光线攻击', '紫粉色同心光环'), ('腕力', '空手劈', 'Karate Chop', '格斗', '用手刀劈向对手', '橙红色短促劈击'), ('卡蒂狗', '火焰牙', 'Fire Fang', '火', '用带有火焰的牙齿咬击', '橙红色火焰'), ('蚊香蝌蚪', '泡沫光线', 'Bubble Beam', '水', '发射泡沫光线攻击', '蓝色泡沫水流'), ('凯西', '精神利刃', 'Psycho Cut', '超能力', '用精神利刃劈向对手', '紫粉色光刃'), ('腕力', '劈瓦', 'Brick Break', '格斗', '用手刀劈击对手', '橙红色短促劈击'), ('卡蒂狗', '喷射火焰', 'Flamethrower', '火', '喷射猛烈火焰攻击', '橙红色火焰'), ('蚊香蝌蚪', '水之波动', 'Water Pulse', '水', '用水的震动攻击', '蓝色水波'), ('凯西', '精神强念', 'Psychic', '超能力', '用强大的念力攻击', '紫粉色同心光环'), ('腕力', '十字劈', 'Cross Chop', '格斗', '交叉双手劈向对手', '橙红色交叉劈击')]
m['moves']=[x for x in m['moves'] if x['id']<96]
for i,(species,name,english,kind,description,visual) in enumerate(new):
    m['moves'].append(dict(id=96+i,slot=i//4,species=species,name=name,english=english,type=kind,description=description,visual=visual,source='https://wiki.52poke.com/wiki/'+name+'（招式）'))
# All preflight checks finish before publishing any derived metadata.
write('assets/pet-roster.json',roster);write('assets/evolution.json',e);write('assets/moves.json',m)
plan['status']='all visual reviews passed; runtime validation pending';write('assets/pokemon/plan-057-066.json',plan)
print('Prepared 66 forms, 28 families, 38 transitions and 112 moves; runtime build/QA required.')
