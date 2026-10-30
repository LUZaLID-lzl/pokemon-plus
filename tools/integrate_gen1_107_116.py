#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Publish 107–116 only after complete visual and source/frame provenance QA."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
plan=json.loads((R/'assets/pokemon/plan-107-116.json').read_text())
qa=json.loads((R/'docs/gen1-107-116/visual-review.json').read_text())
assert set(range(107,117))<=set(qa['identityReviewed']) & set(qa['animationReviewed']),'Visual review incomplete'
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
    dex=item['nationalId'];entry=dict(id=item['artId'],nationalId=dex,name=item['name'],species=item['species'],stage=item['stage'],source=f'pokemon/gen1-107-116/{dex:03}.png')
    old=next((p for p in roster if p['nationalId']==dex),None)
    if old:assert old==entry
    else:roster.append(entry)
assert [p['id'] for p in roster]==list(range(116))
e=json.loads((R/'assets/evolution.json').read_text())
for target in plan['evolution']:
    steps=target['steps']
    chain=dict(species=target['species'],nationalIds=target['nationalIds'],levels=[s.get('level',0) for s in steps],artIds=[next(p['id'] for p in roster if p['nationalId']==d) for d in target['nationalIds']],trigger=steps[0]['trigger'] if len({s['trigger'] for s in steps})==1 else 'mixed',steps=steps)
    e['chains']=[c for c in e['chains'] if c['species']!=target['species']]+[chain]
e.update(version=14,imageFallback='001–024、026–116专用48姿态；025保留原24位移槽位',pendingChains=plan['pendingEvolution'])
assert len(e['chains'])==46 and sum(len(c['steps']) for c in e['chains'])==61
m=json.loads((R/'assets/moves.json').read_text())
new=[('艾比郎', '音速拳', 'Mach Punch', '格斗', '快速挥拳攻击', '橙色冲击'), ('大舌头', '舌舔', 'Lick', '幽灵', '用舌头舔击对手', '紫色幽光'), ('瓦斯弹', '浊雾', 'Smog', '毒', '喷出浊雾攻击', '紫色雾点'), ('独角犀牛', '角撞', 'Horn Attack', '一般', '用角撞击对手', '白色撞击'), ('吉利蛋', '拍击', 'Pound', '一般', '用手拍打对手', '短促拍击'), ('蔓藤怪', '吸取', 'Absorb', '草', '吸取对手的能量', '绿色光点'), ('袋兽', '拍击', 'Pound', '一般', '用手拍打对手', '短促拍击'), ('墨海马', '水枪', 'Water Gun', '水', '喷射水流攻击', '蓝色水流'), ('艾比郎', '雷电拳', 'Thunder Punch', '电', '用带电的拳头攻击', '金色电流'), ('大舌头', '踩踏', 'Stomp', '一般', '用脚踩踏对手', '白色冲击'), ('瓦斯弹', '撞击', 'Tackle', '一般', '用身体撞击对手', '短促撞击'), ('独角犀牛', '岩石爆击', 'Rock Blast', '岩石', '投射岩石攻击', '岩色碎块'), ('吉利蛋', '连环巴掌', 'Double Slap', '一般', '连续拍打对手', '白色连击'), ('蔓藤怪', '藤鞭', 'Vine Whip', '草', '用藤蔓抽打对手', '绿色藤蔓'), ('袋兽', '二连击', 'Double Hit', '一般', '连续撞击两次', '白色连击'), ('墨海马', '泡沫光线', 'Bubble Beam', '水', '发射泡沫攻击', '浅蓝泡沫'), ('艾比郎', '冰冻拳', 'Ice Punch', '冰', '用冰冷的拳头攻击', '蓝白冰晶'), ('大舌头', '摔打', 'Slam', '一般', '用身体或尾巴摔打', '白色重击'), ('瓦斯弹', '污泥攻击', 'Sludge', '毒', '投射污泥攻击', '紫色飞点'), ('独角犀牛', '重踏', 'Bulldoze', '地面', '踏地震动攻击', '沙色震波'), ('吉利蛋', '炸蛋', 'Egg Bomb', '一般', '投出蛋状物攻击', '白色冲击'), ('蔓藤怪', '超级吸取', 'Mega Drain', '草', '吸取更多能量', '绿色光点'), ('袋兽', '百万吨重拳', 'Mega Punch', '一般', '使出强力拳击', '白色重击'), ('墨海马', '盐水', 'Brine', '水', '用盐水冲击对手', '蓝色水浪'), ('艾比郎', '火焰拳', 'Fire Punch', '火', '用带火的拳头攻击', '橙红火星'), ('大舌头', '强力鞭打', 'Power Whip', '草', '用长舌强力挥击', '绿色挥击'), ('瓦斯弹', '污泥炸弹', 'Sludge Bomb', '毒', '发射污泥团攻击', '紫色爆点'), ('独角犀牛', '地震', 'Earthquake', '地面', '震动地面攻击', '沙色震波'), ('吉利蛋', '舍身冲撞', 'Double-Edge', '一般', '全力撞击对手', '白色强光'), ('蔓藤怪', '强力鞭打', 'Power Whip', '草', '用藤蔓强力挥击', '绿色挥击'), ('袋兽', '迷昏拳', 'Dizzy Punch', '一般', '挥拳使对手晕眩', '白色旋转冲击'), ('墨海马', '水炮', 'Hydro Pump', '水', '喷射猛烈水流', '蓝色水柱')]
m['moves']=[v for v in m['moves'] if v['id']<188]
for i,(species,name,english,kind,description,visual) in enumerate(new):
    m['moves'].append(dict(id=188+i,slot=i//8,species=species,name=name,english=english,type=kind,description=description,visual=visual,source='https://wiki.52poke.com/wiki/'+name+'（招式）'))
# No publishing until all source, visual, data and frame checks above pass.
write('assets/pet-roster.json',roster);write('assets/evolution.json',e);write('assets/moves.json',m)
plan['status']='all visual reviews passed; runtime validation pending';write('assets/pokemon/plan-107-116.json',plan)
print('Prepared116 forms,55 families,61 transitions and220 moves; runtime build/QA required.')
