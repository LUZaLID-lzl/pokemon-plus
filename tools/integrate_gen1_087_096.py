#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Publish 087–096 only after complete visual and source/frame provenance QA."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
plan=json.loads((R/'assets/pokemon/plan-087-096.json').read_text())
qa=json.loads((R/'docs/gen1-087-096/visual-review.json').read_text())
assert set(range(87,97))<=set(qa['identityReviewed']) & set(qa['animationReviewed']),'Visual review incomplete'
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
    dex=item['nationalId'];entry=dict(id=item['artId'],nationalId=dex,name=item['name'],species=item['species'],stage=item['stage'],source=f'pokemon/gen1-087-096/{dex:03}.png')
    old=next((p for p in roster if p['nationalId']==dex),None)
    if old:assert old==entry
    else:roster.append(entry)
assert [p['id'] for p in roster]==list(range(96))
e=json.loads((R/'assets/evolution.json').read_text())
for target in plan['evolution']:
    steps=target['steps']
    chain=dict(species=target['species'],nationalIds=target['nationalIds'],levels=[s.get('level',0) for s in steps],artIds=[next(p['id'] for p in roster if p['nationalId']==d) for d in target['nationalIds']],trigger=steps[0]['trigger'] if len({s['trigger'] for s in steps})==1 else 'mixed',steps=steps)
    e['chains']=[c for c in e['chains'] if c['species']!=target['species']]+[chain]
e.update(version=12,imageFallback='001–024、026–096专用48姿态；025保留原24位移槽位',pendingChains=plan['pendingEvolution'])
assert len(e['chains'])==39 and sum(len(c['steps']) for c in e['chains'])==54
m=json.loads((R/'assets/moves.json').read_text())
new=[('臭泥', '拍击', 'Pound', '一般', '用身体拍打对手', '短促撞击'), ('大舌贝', '撞击', 'Tackle', '一般', '用贝壳撞击对手', '短促撞击'), ('鬼斯', '舌舔', 'Lick', '幽灵', '用舌头攻击对手', '紫色幽光'), ('大岩蛇', '撞击', 'Tackle', '一般', '用身体撞击对手', '短促撞击'), ('催眠貘', '拍击', 'Pound', '一般', '用手拍打对手', '短促撞击'), ('臭泥', '污泥攻击', 'Sludge', '毒', '投掷污泥攻击', '紫色毒团'), ('大舌贝', '冰砾', 'Ice Shard', '冰', '发射冰块攻击', '浅蓝冰晶'), ('鬼斯', '祸不单行', 'Hex', '幽灵', '释放幽光攻击', '紫色幽光环'), ('大岩蛇', '落石', 'Rock Throw', '岩石', '投掷岩石攻击', '褐色碎岩'), ('催眠貘', '念力', 'Confusion', '超能力', '使用念力攻击', '紫粉光环'), ('臭泥', '污泥炸弹', 'Sludge Bomb', '毒', '发射污泥团攻击', '紫色毒团'), ('大舌贝', '极光束', 'Aurora Beam', '冰', '发射极光攻击', '浅蓝冰光'), ('鬼斯', '暗影球', 'Shadow Ball', '幽灵', '发射暗影球攻击', '紫色暗影团'), ('大岩蛇', '岩崩', 'Rock Slide', '岩石', '用岩石压击对手', '褐色碎岩'), ('催眠貘', '精神利刃', 'Psycho Cut', '超能力', '释放念力利刃攻击', '紫粉光刃'), ('臭泥', '毒击', 'Poison Jab', '毒', '用毒性身体刺击', '紫色冲击'), ('大舌贝', '冰冻光束', 'Ice Beam', '冰', '发射冰冷光束攻击', '浅蓝冰束'), ('鬼斯', '暗影爪', 'Shadow Claw', '幽灵', '用暗影利爪攻击', '紫色爪痕'), ('大岩蛇', '地震', 'Earthquake', '地面', '震动地面攻击', '沙色地面冲击'), ('催眠貘', '精神强念', 'Psychic', '超能力', '释放强大念力攻击', '紫粉光环')]
m['moves']=[v for v in m['moves'] if v['id']<148]
for i,(species,name,english,kind,description,visual) in enumerate(new):
    m['moves'].append(dict(id=148+i,slot=i//5,species=species,name=name,english=english,type=kind,description=description,visual=visual,source='https://wiki.52poke.com/wiki/'+name+'（招式）'))
# No publishing until all source, visual, data and frame checks above pass.
write('assets/pet-roster.json',roster);write('assets/evolution.json',e);write('assets/moves.json',m)
plan['status']='all visual reviews passed; runtime validation pending';write('assets/pokemon/plan-087-096.json',plan)
print('Prepared 96 forms, 42 families, 54 transitions and 168 moves; runtime build/QA required.')
