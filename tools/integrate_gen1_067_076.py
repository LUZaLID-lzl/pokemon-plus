#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Publish 067–076 only after complete visual and source/frame provenance QA."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
plan=json.loads((R/'assets/pokemon/plan-067-076.json').read_text())
qa=json.loads((R/'docs/gen1-067-076/visual-review.json').read_text())
assert set(range(67,77))<=set(qa['identityReviewed']) & set(qa['animationReviewed']),'Visual review incomplete'
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
    dex=item['nationalId'];entry=dict(id=item['artId'],nationalId=dex,name=item['name'],species=item['species'],stage=item['stage'],source=f'pokemon/gen1-067-076/{dex:03}.png')
    old=next((p for p in roster if p['nationalId']==dex),None)
    if old:assert old==entry
    else:roster.append(entry)
assert [p['id'] for p in roster]==list(range(76))
e=json.loads((R/'assets/evolution.json').read_text())
for target in plan['evolution']:
    steps=target['steps']
    chain=dict(species=target['species'],nationalIds=target['nationalIds'],levels=[s.get('level',0) for s in steps],artIds=[next(p['id'] for p in roster if p['nationalId']==d) for d in target['nationalIds']],trigger=steps[0]['trigger'] if len({s['trigger'] for s in steps})==1 else 'mixed',steps=steps)
    e['chains']=[c for c in e['chains'] if c['species']!=target['species']]+[chain]
e.update(version=10,imageFallback='001–024、026–076专用48姿态；025保留原24位移槽位',pendingChains=plan['pendingEvolution'])
assert len(e['chains'])==31 and sum(len(c['steps']) for c in e['chains'])==45
m=json.loads((R/'assets/moves.json').read_text())
new=[
('喇叭芽','藤鞭','Vine Whip','草','挥动细长藤蔓抽打','绿色藤蔓'),
('玛瑙水母','毒针','Poison Sting','毒','射出毒针攻击','紫色针状光点'),
('小拳石','撞击','Tackle','一般','用身体撞击对手','短促撞击'),
('喇叭芽','溶解液','Acid','毒','用酸液攻击对手','紫色酸液光团'),
('玛瑙水母','溶解液','Acid','毒','用酸液攻击对手','紫色酸液光团'),
('小拳石','落石','Rock Throw','岩石','投掷岩石攻击','棕灰色岩石'),
('喇叭芽','飞叶快刀','Razor Leaf','草','飞射锋利的叶片','绿色飞叶'),
('玛瑙水母','泡沫光线','Bubble Beam','水','发射泡沫光线攻击','蓝色泡沫水流'),
('小拳石','岩崩','Rock Slide','岩石','用岩石攻击对手','棕灰色岩石'),
('喇叭芽','种子炸弹','Seed Bomb','草','发射坚硬的种子攻击','绿色种子光团'),
('玛瑙水母','水炮','Hydro Pump','水','喷射猛烈水流攻击','蓝色水流'),
('小拳石','地震','Earthquake','地面','震动地面攻击','土黄色地面震纹')]
m['moves']=[v for v in m['moves'] if v['id']<112]
for i,(species,name,english,kind,description,visual) in enumerate(new):
    m['moves'].append(dict(id=112+i,slot=i//3,species=species,name=name,english=english,type=kind,description=description,visual=visual,source='https://wiki.52poke.com/wiki/'+name+'（招式）'))
# No publishing until all source, visual, data and frame checks above pass.
write('assets/pet-roster.json',roster);write('assets/evolution.json',e);write('assets/moves.json',m)
plan['status']='all visual reviews passed; runtime validation pending';write('assets/pokemon/plan-067-076.json',plan)
print('Prepared 76 forms, 31 families, 45 transitions and 124 moves; runtime build/QA required.')
