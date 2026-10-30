#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Publish 097–106 only after complete visual and source/frame provenance QA."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
plan=json.loads((R/'assets/pokemon/plan-097-106.json').read_text())
qa=json.loads((R/'docs/gen1-097-106/visual-review.json').read_text())
assert set(range(97,107))<=set(qa['identityReviewed']) & set(qa['animationReviewed']),'Visual review incomplete'
for item in plan['items']:
    dex=item['nationalId'];folder=R/f'assets/animation-templates/{dex:03}'
    m=json.loads((folder/'manifest.json').read_text())
    assert m['nationalId']==dex and m['frames']==48 and m['framesPerAction']==8
    assert m['source']==f"assets/source/animation-{dex:03}-master-v{qa['acceptedVersions'][str(dex)]}.png"
    assert hashlib.sha256((R/m['source']).read_bytes()).hexdigest()==m['sourceSha256']
    for action in m['actions'].values():
        assert len(action['frames'])==len(action['durationsMs'])==8
        assert len({hashlib.sha256((folder/p).read_bytes()).hexdigest() for p in action['frames']})==8
    for f in m['items']:assert hashlib.sha256((folder/f['image']).read_bytes()).hexdigest()==f['sha256']

def write(file,data):(R/file).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')
roster=json.loads((R/'assets/pet-roster.json').read_text())
for item in plan['items']:
    dex=item['nationalId'];entry=dict(id=item['artId'],nationalId=dex,name=item['name'],species=item['species'],stage=item['stage'],source=f'pokemon/gen1-097-106/{dex:03}.png')
    old=next((p for p in roster if p['nationalId']==dex),None)
    if old:assert old==entry
    else:roster.append(entry)
assert [p['id'] for p in roster]==list(range(106))
e=json.loads((R/'assets/evolution.json').read_text())
for target in plan['evolution']:
    steps=target['steps']
    chain=dict(species=target['species'],nationalIds=target['nationalIds'],levels=[s.get('level',0) for s in steps],artIds=[next(p['id'] for p in roster if p['nationalId']==d) for d in target['nationalIds']],trigger=steps[0]['trigger'] if len({s['trigger'] for s in steps})==1 else 'mixed',steps=steps)
    e['chains']=[c for c in e['chains'] if c['species']!=target['species']]+[chain]
e.update(version=13,imageFallback='001–024、026–106专用48姿态；025保留原24位移槽位',pendingChains=plan['pendingEvolution'])
assert len(e['chains'])==44 and sum(len(c['steps']) for c in e['chains'])==59
m=json.loads((R/'assets/moves.json').read_text())
new=[('大钳蟹', '夹住', 'Vice Grip', '一般', '用双钳夹住对手', '短促夹击'), ('霹雳电球', '撞击', 'Tackle', '一般', '用身体撞击对手', '短促撞击'), ('蛋蛋', '撞击', 'Tackle', '一般', '用身体撞击对手', '短促撞击'), ('卡拉卡拉', '拍击', 'Pound', '一般', '用手拍打对手', '短促撞击'), ('飞腿郎', '踢倒', 'Low Kick', '格斗', '压低身体踢击', '橙色冲击'), ('大钳蟹', '泡沫光线', 'Bubble Beam', '水', '发射泡沫攻击', '浅蓝泡沫'), ('霹雳电球', '电击', 'Thunder Shock', '电', '释放电流攻击', '金色电弧'), ('蛋蛋', '种子机关枪', 'Bullet Seed', '草', '发射种子攻击', '绿色种子'), ('卡拉卡拉', '骨棒', 'Bone Club', '地面', '用骨棒挥击', '沙色冲击'), ('飞腿郎', '二连踢', 'Double Kick', '格斗', '快速踢击两次', '橙色连击'), ('大钳蟹', '蟹钳锤', 'Crabhammer', '水', '用巨钳挥击', '蓝色水浪'), ('霹雳电球', '电光', 'Spark', '电', '用电光冲击', '金色闪光'), ('蛋蛋', '念力', 'Confusion', '超能力', '使用念力攻击', '紫粉光环'), ('卡拉卡拉', '骨头回力镖', 'Bonemerang', '地面', '投出回旋骨棒攻击', '沙色回旋'), ('飞腿郎', '飞踢', 'Jump Kick', '格斗', '跃起踢击对手', '橙色飞踢'), ('大钳蟹', '十字剪', 'X-Scissor', '虫', '双钳交叉挥击', '绿色交叉'), ('霹雳电球', '十万伏特', 'Thunderbolt', '电', '释放强力电流', '金色雷电'), ('蛋蛋', '精神强念', 'Psychic', '超能力', '释放强大念力', '紫粉光环'), ('卡拉卡拉', '骨棒乱打', 'Bone Rush', '地面', '挥舞骨棒攻击', '沙色连击'), ('飞腿郎', '百万吨重踢', 'Mega Kick', '一般', '使出强力踢击', '白色重击')]
m['moves']=[v for v in m['moves'] if v['id']<168]
for i,(species,name,english,kind,description,visual) in enumerate(new):
    m['moves'].append(dict(id=168+i,slot=i//5,species=species,name=name,english=english,type=kind,description=description,visual=visual,source='https://wiki.52poke.com/wiki/'+name+'（招式）'))
# No publishing until all source, visual, data and frame checks above pass.
write('assets/pet-roster.json',roster);write('assets/evolution.json',e);write('assets/moves.json',m)
plan['status']='all visual reviews passed; runtime validation pending';write('assets/pokemon/plan-097-106.json',plan)
print('Prepared 106 forms, 47 families, 59 transitions and 188 moves; runtime build/QA required.')
