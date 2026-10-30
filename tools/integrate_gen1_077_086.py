#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Publish 077–086 only after complete visual and source/frame provenance QA."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
plan=json.loads((R/'assets/pokemon/plan-077-086.json').read_text())
qa=json.loads((R/'docs/gen1-077-086/visual-review.json').read_text())
assert set(range(77,87))<=set(qa['identityReviewed']) & set(qa['animationReviewed']),'Visual review incomplete'
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
    dex=item['nationalId'];entry=dict(id=item['artId'],nationalId=dex,name=item['name'],species=item['species'],stage=item['stage'],source=f'pokemon/gen1-077-086/{dex:03}.png')
    old=next((p for p in roster if p['nationalId']==dex),None)
    if old:assert old==entry
    else:roster.append(entry)
assert [p['id'] for p in roster]==list(range(86))
e=json.loads((R/'assets/evolution.json').read_text())
for target in plan['evolution']:
    steps=target['steps']
    chain=dict(species=target['species'],nationalIds=target['nationalIds'],levels=[s.get('level',0) for s in steps],artIds=[next(p['id'] for p in roster if p['nationalId']==d) for d in target['nationalIds']],trigger=steps[0]['trigger'] if len({s['trigger'] for s in steps})==1 else 'mixed',steps=steps)
    e['chains']=[c for c in e['chains'] if c['species']!=target['species']]+[chain]
e.update(version=11,imageFallback='001–024、026–086专用48姿态；025保留原24位移槽位',pendingChains=plan['pendingEvolution'])
assert len(e['chains'])==35 and sum(len(c['steps']) for c in e['chains'])==49
m=json.loads((R/'assets/moves.json').read_text())
new=[('小火马', '火花', 'Ember', '火', '发射小火焰攻击', '火团与火星'), ('呆呆兽', '撞击', 'Tackle', '一般', '用身体撞击对手', '短促撞击'), ('小磁怪', '电击', 'Thunder Shock', '电', '释放电流攻击', '锯齿电流'), ('大葱鸭', '啄', 'Peck', '飞行', '用尖喙啄击', '白色冲刺气流'), ('嘟嘟', '啄', 'Peck', '飞行', '用尖喙啄击', '白色冲刺气流'), ('小海狮', '头锤', 'Headbutt', '一般', '用头部撞击对手', '短促撞击'), ('小火马', '踩踏', 'Stomp', '一般', '抬起前蹄踩踏', '短促撞击'), ('呆呆兽', '水枪', 'Water Gun', '水', '喷射水流攻击', '蓝色水流'), ('小磁怪', '电光', 'Spark', '电', '释放电光冲击', '电流与闪光'), ('大葱鸭', '连斩', 'Fury Cutter', '虫', '用大葱连续挥击', '绿色斩击'), ('嘟嘟', '乱击', 'Fury Attack', '一般', '用尖喙连续攻击', '短促连击'), ('小海狮', '冰砾', 'Ice Shard', '冰', '发射冰块攻击', '浅蓝冰晶'), ('小火马', '火焰轮', 'Flame Wheel', '火', '以火焰包裹身体冲击', '火焰冲刺'), ('呆呆兽', '念力', 'Confusion', '超能力', '使用念力攻击', '紫粉光环'), ('小磁怪', '加农光炮', 'Flash Cannon', '钢', '聚集光芒发射', '银白光束'), ('大葱鸭', '劈开', 'Slash', '一般', '挥动大葱劈击', '白色斩痕'), ('嘟嘟', '三重攻击', 'Tri Attack', '一般', '由三个方向同时攻击', '三道冲击光'), ('小海狮', '极光束', 'Aurora Beam', '冰', '发射极光攻击', '浅蓝冰光'), ('小火马', '喷射火焰', 'Flamethrower', '火', '喷射猛烈火焰攻击', '火焰喷射'), ('呆呆兽', '精神强念', 'Psychic', '超能力', '释放强大念力攻击', '紫粉光环'), ('小磁怪', '十万伏特', 'Thunderbolt', '电', '释放强大电流攻击', '电流与强光'), ('大葱鸭', '空气斩', 'Air Slash', '飞行', '发射空气利刃攻击', '白色风刃'), ('嘟嘟', '啄钻', 'Drill Peck', '飞行', '旋转尖喙攻击', '白色旋转气流'), ('小海狮', '冰冻光束', 'Ice Beam', '冰', '发射冰冷光束攻击', '浅蓝冰束')]
m['moves']=[v for v in m['moves'] if v['id']<124]
for i,(species,name,english,kind,description,visual) in enumerate(new):
    m['moves'].append(dict(id=124+i,slot=i//6,species=species,name=name,english=english,type=kind,description=description,visual=visual,source='https://wiki.52poke.com/wiki/'+name+'（招式）'))
# No publishing until all source, visual, data and frame checks above pass.
write('assets/pet-roster.json',roster);write('assets/evolution.json',e);write('assets/moves.json',m)
plan['status']='all visual reviews passed; runtime validation pending';write('assets/pokemon/plan-077-086.json',plan)
print('Prepared 86 forms, 37 families, 49 transitions and 148 moves; runtime build/QA required.')
