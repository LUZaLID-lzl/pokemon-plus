"""Publish ten reviewed full templates atomically after provenance preflight."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
plan=json.loads((R/'assets/pokemon/plan-047-056.json').read_text())
qa=json.loads((R/'docs/gen1-047-056/visual-review.json').read_text())
assert set(range(47,57))<=set(qa['identityReviewed']) & set(qa['animationReviewed']),'Visual review incomplete'
for item in plan['items']:
    dex=item['nationalId'];folder=R/f'assets/animation-templates/{dex:03}'
    m=json.loads((folder/'manifest.json').read_text())
    assert m['nationalId']==dex and m['frames']==48 and m['framesPerAction']==8
    assert hashlib.sha256((R/m['source']).read_bytes()).hexdigest()==m['sourceSha256']
    for action in m['actions'].values():assert len(action['frames'])==len(action['durationsMs'])==8
    for f in m['items']:assert hashlib.sha256((folder/f['image']).read_bytes()).hexdigest()==f['sha256']

def write(file,data):(R/file).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')
roster=json.loads((R/'assets/pet-roster.json').read_text())
for item in plan['items']:
    dex=item['nationalId'];entry=dict(id=item['artId'],nationalId=dex,name=item['name'],species=item['species'],stage=item['stage'],source=f'pokemon/gen1-047-056/{dex:03}.png')
    old=next((p for p in roster if p['nationalId']==dex),None)
    if old:assert old==entry
    else:roster.append(entry)
assert [p['id'] for p in roster]==list(range(56))
e=json.loads((R/'assets/evolution.json').read_text())
for target in plan['evolution']:
    steps=[dict(s,trigger='level') for s in target['steps']]
    chain=dict(species=target['species'],nationalIds=target['nationalIds'],levels=[s['level'] for s in steps],artIds=[next(p['id'] for p in roster if p['nationalId']==d) for d in target['nationalIds']],trigger='level',steps=steps)
    e['chains']=[c for c in e['chains'] if c['species']!=target['species']]+[chain]
e.update(version=8,imageFallback='001–024、026–056专用48姿态；025保留原24位移槽位',pendingChains=plan['pendingEvolution'])
assert len(e['chains'])==23 and sum(len(c['steps']) for c in e['chains'])==32
m=json.loads((R/'assets/moves.json').read_text())
# Preserve IDs 0..75. Five new families, slot-major IDs 76..95.
new=[
('毛球','撞击','Tackle','一般','用身体撞击对手','短促撞击'),
('地鼠','抓','Scratch','一般','用尖锐的爪子抓击','短促爪击'),
('喵喵','抓','Scratch','一般','用尖锐的爪子抓击','短促爪击'),
('可达鸭','水枪','Water Gun','水','喷射强劲水流','蓝色水流'),
('猴怪','抓','Scratch','一般','用尖锐的爪子抓击','短促爪击'),
('毛球','吸血','Leech Life','虫','用尖牙攻击对手','虫系短促冲击'),
('地鼠','掷泥','Mud-Slap','地面','向对手投掷泥土','棕色泥土轨迹'),
('喵喵','咬住','Bite','一般','用牙齿咬住对手','短促咬击'),
('可达鸭','抓','Scratch','一般','用尖锐的爪子抓击','短促爪击'),
('猴怪','踢倒','Low Kick','格斗','用腿扫向对手','橙红色短促踢击'),
('毛球','幻象光线','Psybeam','超能力','发出神秘的光线攻击','紫粉色同心光环'),
('地鼠','挖洞','Dig','地面','钻地后突袭对手','棕色地面冲击'),
('喵喵','乱抓','Fury Swipes','一般','用爪子连续抓击','短促连续爪击'),
('可达鸭','水之波动','Water Pulse','水','用水的震动攻击','蓝色水波'),
('猴怪','空手劈','Karate Chop','格斗','用手刀劈向对手','橙红色短促劈击'),
('毛球','虫鸣','Bug Buzz','虫','用振动发出声波攻击','虫系扩散声波'),
('地鼠','地震','Earthquake','地面','震动地面攻击','棕色震动冲击'),
('喵喵','劈开','Slash','一般','用利爪劈向对手','短促利爪斩击'),
('可达鸭','念力','Confusion','超能力','用念力攻击对手','紫粉色同心光环'),
('猴怪','十字劈','Cross Chop','格斗','交叉双手劈向对手','橙红色交叉劈击'),
]
m['moves']=[x for x in m['moves'] if x['id']<76]
for i,(species,name,english,kind,description,visual) in enumerate(new):
    m['moves'].append(dict(id=76+i,slot=i//5,species=species,name=name,english=english,type=kind,description=description,visual=visual,source='https://wiki.52poke.com/wiki/'+name+'（招式）'))
# All preflight checks finish before publishing any derived metadata.
write('assets/pet-roster.json',roster);write('assets/evolution.json',e);write('assets/moves.json',m)
plan['status']='all visual reviews passed; runtime validation pending';write('assets/pokemon/plan-047-056.json',plan)
print('Prepared 56 forms, 24 families, 32 transitions and 96 moves; runtime build/QA required.')
