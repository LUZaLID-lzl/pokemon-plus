"""Publish the ten-form batch only after all ten complete templates exist."""
from pathlib import Path
import hashlib, json
R=Path(__file__).resolve().parents[1]
plan=json.loads((R/'assets/pokemon/plan-037-046.json').read_text())
missing=[]
for item in plan['items']:
    dex=item['nationalId'];folder=R/f'assets/animation-templates/{dex:03}'
    if not (folder/'manifest.json').exists():missing.append(f'{dex:03}');continue
    m=json.loads((folder/'manifest.json').read_text())
    assert m['nationalId']==dex and m['frames']==48 and m['framesPerAction']==8
    assert hashlib.sha256((R/m['source']).read_bytes()).hexdigest()==m['sourceSha256']
    for frame in m['items']:
        assert hashlib.sha256((folder/frame['image']).read_bytes()).hexdigest()==frame['sha256']
if missing:raise SystemExit('Batch remains unpublished; incomplete templates: '+', '.join(missing))

def write(file,data):(R/file).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')

roster=json.loads((R/'assets/pet-roster.json').read_text())
for item in plan['items']:
    dex=item['nationalId'];entry=dict(id=item['artId'],nationalId=dex,name=item['name'],species=item['species'],stage=item['stage'],source=f'pokemon/gen1-037-046/{dex:03}.png')
    old=next((p for p in roster if p['nationalId']==dex),None)
    if old:assert old==entry
    else:roster.append(entry)
assert [p['id'] for p in roster]==list(range(46))
e=json.loads((R/'assets/evolution.json').read_text())
for target in plan['evolution']:
    steps=[dict(s,trigger={'火之石':'fire-stone','叶之石':'leaf-stone','月之石':'moon-stone'}.get(s.get('item'),'level')) for s in target['steps']]
    chain=dict(species=target['species'],nationalIds=target['nationalIds'],levels=[s.get('level',0) for s in steps],artIds=[next(p['id'] for p in roster if p['nationalId']==d) for d in target['nationalIds']],trigger='mixed' if len(steps)>1 else steps[0]['trigger'],steps=steps)
    e['chains']=[c for c in e['chains'] if c['species']!=target['species']]+[chain]
e.update(version=7,imageFallback='001–024、026–046专用48姿态；025保留原24位移槽位',pendingChains=plan['pendingEvolution'],trigger='等级进化自动弹出选择；雷/火/月/叶之石从状态页直接购买使用')
assert len(e['chains'])==18 and sum(len(c['steps']) for c in e['chains'])==27
m=json.loads((R/'assets/moves.json').read_text())
# Slot-major additions; old IDs 0..55 never change.
new=[
('六尾','火花','Ember','火','发射小火苗攻击','橙色火花'),
('胖丁','拍击','Pound','一般','用前肢拍击对手','短促拍击'),
('超音蝠','吸血','Leech Life','虫','用尖牙吸取对手体力','虫系短促冲击'),
('走路草','吸取','Absorb','草','吸取对手体力','绿色能量轨迹'),
('派拉斯','抓','Scratch','一般','用爪子抓向对手','短促爪击'),
('六尾','电光一闪','Quick Attack','一般','快速冲向对手','快速前冲'),
('胖丁','魅惑之声','Disarming Voice','妖精','用妖精之声攻击','粉色星芒与声波'),
('超音蝠','咬住','Bite','一般','用牙齿咬住对手','咬击轨迹'),
('走路草','溶解液','Acid','毒','用酸液攻击对手','紫色液滴'),
('派拉斯','吸取','Absorb','草','吸取对手体力','绿色能量轨迹'),
('六尾','火焰旋涡','Fire Spin','火','用旋转火焰攻击','旋转火星'),
('胖丁','巨声','Hyper Voice','一般','用响亮的声音攻击','扩散声波'),
('超音蝠','空气利刃','Air Cutter','飞行','发出锐利气流攻击','短促风刃'),
('走路草','超级吸取','Mega Drain','草','用更强能量吸取体力','绿色能量轨迹'),
('派拉斯','连斩','Fury Cutter','虫','用锋利前肢斩击','虫系斩击'),
('六尾','喷射火焰','Flamethrower','火','喷射强烈火焰攻击','橙色火焰'),
('胖丁','月亮之力','Moonblast','妖精','借助月亮的力量攻击','粉色月光星芒'),
('超音蝠','毒液冲击','Venoshock','毒','将毒液泼向对手','紫色毒液'),
('走路草','花瓣舞','Petal Dance','草','用花瓣般的能量攻击','绿色叶片轨迹'),
('派拉斯','十字剪','X-Scissor','虫','交叉前肢斩向对手','虫系交叉斩击'),
]
m['moves']=[x for x in m['moves'] if x['id']<56]
for i,(species,name,english,kind,description,visual) in enumerate(new):
    m['moves'].append(dict(id=56+i,slot=i//5,species=species,name=name,english=english,type=kind,description=description,visual=visual,source='https://wiki.52poke.com/wiki/'+name+'（招式）'))
write('assets/pet-roster.json',roster);write('assets/evolution.json',e);write('assets/moves.json',m)
plan['status']='all templates present; metadata prepared, runtime validation pending';write('assets/pokemon/plan-037-046.json',plan)
print('Prepared 46 forms, 19 families, 27 transitions and 76 moves; runtime build/QA required.')
