#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Publish batch metadata only after every new template passes source preflight."""
from pathlib import Path
import hashlib
import json

ROOT=Path(__file__).resolve().parents[1]
plan=json.loads((ROOT/'assets/pokemon/plan-028-036.json').read_text())
missing=[]
for item in plan['items']:
    dex=item['nationalId'];folder=ROOT/f'assets/animation-templates/{dex:03}'
    path=folder/'manifest.json'
    if not path.exists():missing.append(f'{dex:03} manifest');continue
    m=json.loads(path.read_text())
    assert m['nationalId']==dex and m['frames']==48 and m['framesPerAction']==8
    assert hashlib.sha256((ROOT/m['source']).read_bytes()).hexdigest()==m['sourceSha256']
    for frame in m['items']:
        p=folder/frame['image']
        if not p.exists():missing.append(str(p))
        else:assert hashlib.sha256(p.read_bytes()).hexdigest()==frame['sha256']
if missing:raise SystemExit('Batch remains unpublished; missing: '+', '.join(missing))

def write(name,data):
    (ROOT/name).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')

roster=json.loads((ROOT/'assets/pet-roster.json').read_text())
for item in plan['items']:
    dex=item['nationalId']
    entry={'id':item['artId'],'nationalId':dex,'name':item['name'],'species':item['species'],'stage':item['stage'],'source':f'pokemon/gen1-028-036/{dex:03}.png'}
    previous=next((p for p in roster if p['nationalId']==dex),None)
    if previous:assert previous==entry
    else:roster.append(entry)
assert [p['id'] for p in roster]==list(range(36))
write('assets/pet-roster.json',roster)

e=json.loads((ROOT/'assets/evolution.json').read_text())
e['version']=6;e['imageFallback']='001–024、026–036专用48姿态；025保留原24位移槽位'
e['trigger']='达到等级弹出选择；忙碌/挑战特效结束后延后；雷之石/月之石从状态页发起'
e['pendingChains']=[]
for target in plan['evolution']:
    species=target['species']
    chain={'species':species,'nationalIds':target['nationalIds'],'levels':[x.get('level',0) for x in target['steps']],
           'artIds':[next(p['id'] for p in roster if p['nationalId']==dex) for dex in target['nationalIds']],
           'trigger':'mixed' if len(target['steps'])>1 and any('item' in x for x in target['steps']) else 'moon-stone' if 'item' in target['steps'][0] else 'level',
           'steps':[dict(x,trigger='moon-stone' if 'item' in x else 'level') for x in target['steps']]}
    e['chains']=[c for c in e['chains'] if c['species']!=species]+[chain]
for chain in e['chains']:
    if 'steps' not in chain:
        chain['steps']=[{'trigger':'thunder-stone','item':'雷之石','price':200} if chain.get('trigger')=='thunder-stone' else {'trigger':'level','level':level} for level in chain['levels']]
assert len(e['chains'])==14 and sum(len(c['steps']) for c in e['chains'])==22
write('assets/evolution.json',e)

m=json.loads((ROOT/'assets/moves.json').read_text())
new=[
('尼多兰','毒针','Poison Sting','毒','射出毒针攻击','紫色毒针'),
('尼多朗','啄','Peck','飞行','用尖锐的喙啄击','短促前冲与风刃'),
('皮皮','拍击','Pound','一般','用前肢拍击对手','短促拍击'),
('尼多兰','咬住','Bite','一般','用牙齿咬住对手','咬击轨迹'),
('尼多朗','角撞','Horn Attack','一般','用尖角撞击对手','尖角前冲'),
('皮皮','魅惑之声','Disarming Voice','妖精','用妖精之声攻击','粉色星芒与声波'),
('尼多兰','毒液冲击','Venoshock','毒','将毒液泼向对手','紫色毒液'),
('尼多朗','毒针','Poison Sting','毒','射出毒针攻击','紫色毒针'),
('皮皮','巨声','Hyper Voice','一般','用响亮的声音攻击','扩散声波'),
('尼多兰','毒击','Poison Jab','毒','用带毒的肢体刺击','紫色刺击'),
('尼多朗','毒击','Poison Jab','毒','用带毒的肢体刺击','紫色刺击'),
('皮皮','月亮之力','Moonblast','妖精','借助月亮的力量攻击','粉色月光星芒'),
]
m['moves']=[x for x in m['moves'] if x['id']<44]
for i,(species,name,english,kind,description,visual) in enumerate(new):
    m['moves'].append({'id':44+i,'slot':i//3,'species':species,'name':name,'english':english,'type':kind,'description':description,'visual':visual,'source':'https://wiki.52poke.com/wiki/'+name+'（招式）'})
write('assets/moves.json',m)
plan['status']='templates reviewed; metadata prepared, pending runtime build/validation'
write('assets/pokemon/plan-028-036.json',plan)
print('Prepared 36 forms, 14 families, 22 evolution transitions and 56 moves; run prepare_assets and validation before release.')
