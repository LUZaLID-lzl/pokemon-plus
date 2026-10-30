#!/usr/bin/env python3
"""Stage the three reviewed companions; missing130 never becomes an evolution target."""
from pathlib import Path
import argparse, hashlib, json, shutil
R=Path(__file__).resolve().parents[1]
def read(p): return json.loads((R/p).read_text())
def write(p,x): (R/p).write_text(json.dumps(x,ensure_ascii=False,indent=2)+'\n')
qa=read('docs/gen1-127-136/visual-review.json')
p=argparse.ArgumentParser();p.add_argument('--end',type=int,default=129);args=p.parse_args()
assert 129<=args.end<=132,'Eevee requires a separate branch-aware integration'
end=args.end;families=65+max(0,end-130);ids=list(range(127,end+1))
assert set(ids)<=set(qa['identityReviewed'])&set(qa['animationReviewed'])
roster=read('assets/pet-roster.json');assert len(roster)==126
plan=read('assets/pokemon/plan-127-136.json')
for d in ids:
 m=read(f'assets/animation-templates/{d}/manifest.json')
 assert m['frames']==48 and len(m['actions'])==6
 assert hashlib.sha256((R/m['source']).read_bytes()).hexdigest()==m['sourceSha256']
 for a in m['actions'].values():
  assert len(a['frames'])==len(a['durationsMs'])==8
  assert len({(R/f'assets/animation-templates/{d}'/f).read_bytes() for f in a['frames']})==8
 for f in m['items']:assert hashlib.sha256((R/f'assets/animation-templates/{d}'/f['image']).read_bytes()).hexdigest()==f['sha256']
backup=R/f'build/pets{end}-before-runtime';assert not backup.exists()
files=['CMakeLists.txt','partitions.csv','core/game.h','core/game.c','core/moves.c','core/moves.h','core/pokedex_roster.h','main/game_ui.c','main/battle_view.c','main/image_assets.c','main/image_assets.h','preview/preview.c','assets/pet-roster.json','assets/evolution.json','assets/moves.json','assets/pet-animation.json','web/app.js','web/index.html','web/animation-player.js','tools/package_firmware.py','tools/test.sh','tests/test_forms_117_126.mjs','assets/animation-templates/index.json','web/manifest.json','sdcard/pokemon-plus/manifest.json']
for f in files:
 dst=backup/f;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(R/f,dst)
for p in plan['items']:
 if p['nationalId'] in ids:roster.append(dict(id=p['artId'],nationalId=p['nationalId'],name=p['name'],species=p['species'],stage=p['stage'],source=f"pokemon/gen1-127-136/{p['nationalId']:03}.png"))
assert [p['id']for p in roster]==list(range(end))
write('assets/pet-roster.json',roster)
e=read('assets/evolution.json')
if end>=130:e['chains'].append(dict(species=64,nationalIds=[129,130],levels=[20],artIds=[128,129],trigger='level',steps=[dict(trigger='level',level=20)]))
e.update(version=16,imageFallback=f'001-024 and026-{end} dedicated48 poses;025 retains original24 translation slots',pendingChains=([dict(species=64,nationalIds=[129,130],level=20,reason='130 identity and full animation pending; evolution unavailable until reviewed runtime integration')]if end==129 else []))
write('assets/evolution.json',e)
groups=[
 [('夹住','Vise Grip','一般','用双角夹住对手','白色夹击'),('地球上投','Seismic Toss','格斗','抓起对手后投掷','橙色冲击'),('劈开','Slash','一般','挥动前肢劈击','白色爪痕'),('十字剪','X-Scissor','虫','交叉挥动前肢攻击','绿色交叉斩')],
 [('撞击','Tackle','一般','用身体撞击对手','白色冲击'),('角撞','Horn Attack','一般','用头角撞击对手','白色角痕'),('意念头锤','Zen Headbutt','超能','集中意念用头攻击','紫色念力'),('猛撞','Take Down','一般','猛烈冲撞对手','白色冲击')],
 [('跃起','Splash','一般','轻轻跃起摆动身体','浅色跳跃'),('撞击','Tackle','一般','用身体撞击对手','白色冲击'),('抓狂','Flail','一般','摆动身体全力攻击','白色冲击'),('挣扎','Struggle','一般','奋力挣扎攻击','白色冲击')],
 [('水枪','Water Gun','水','喷射水流攻击','蓝色水流'),('冰砾','Ice Shard','冰','释放冰砾攻击','蓝白冰晶'),('水之波动','Water Pulse','水','用水的波动攻击','蓝色波纹'),('冰冻光束','Ice Beam','冰','发射冰冷光束','蓝白光束')],
 [('变身','Transform','一般','模仿对手的姿态','浅色光环'),('撞击','Tackle','一般','用身体撞击对手','白色冲击'),('高速星星','Swift','一般','释放星形能量','白色星点'),('挣扎','Struggle','一般','奋力挣扎攻击','白色冲击')]
]
new=[]
for slot in range(4):
 for index,name in enumerate(['凯罗斯','肯泰罗','鲤鱼王','拉普拉斯','百变怪'][:families-62]):
  title,english,kind,description,visual=groups[index][slot]
  new.append(dict(id=248+slot*(families-62)+index,slot=slot,species=name,name=title,english=english,type=kind,description=description,visual=visual,source='https://wiki.52poke.com/wiki/'+title+'（招式）'))
write(f'assets/moves-plan-127-{end}.json',dict(status='reviewed companions; runtime validation required',firstFamily=62,lastFamily=families-1,previousMoveCount=248,moves=new))
moves=read('assets/moves.json');assert len(moves['moves'])==248;moves['moves']+=new;write('assets/moves.json',moves)
def edit(p,a,b):
 f=R/p;s=f.read_text();assert a in s,(p,a);f.write_text(s.replace(a,b))
edit('core/game.h','PP_SPECIES 62',f'PP_SPECIES {families}')
edit('core/game.c','"鸭嘴火兽"};','"鸭嘴火兽", '+', '.join('"'+x+'"'for x in ['凯罗斯','肯泰罗','鲤鱼王','拉普拉斯','百变怪'][:families-62])+'};')
if end>=130:
 edit('core/game.h','PP_EVOLUTIONS 64','PP_EVOLUTIONS 65')
 edit('core/game.c','{56,0,0,120,"宝石海星"}};','{56,0,0,120,"宝石海星"}, {64,20,0,129,"暴鲤龙"}};')
types={'一般':'PP_NORMAL','格斗':'PP_FIGHTING','虫':'PP_BUG','超能':'PP_PSYCHIC','水':'PP_WATER','冰':'PP_ICE'}
edit('core/moves.c','\n};\nconst pp_move *pp_move_at',''.join('\n    {"'+v['name']+'", "'+v['type']+'", "'+v['description']+'", '+types[v['type']]+'},'for v in new)+'\n};\nconst pp_move *pp_move_at')
edit('core/moves.c',': 220+slot*7+species-55]',f': species<62 ? 220+slot*7+species-55 : 248+slot*{families-62}+species-62]')
for p in ['main/game_ui.c','preview/preview.c']:edit(p,'0xf1d16c,0xeab77f','0xf1d16c,0xeab77f,'+','.join(['0xc8ae8d','0xceac7f','0xef9f69','0x8ecde8','0xcba9dc'][:families-62]))
if end==129:edit('main/game_ui.c','label(body, "最终形态", 80,','label(body, p->species==64 ? "Lv.20进化 · 130待接入" : "最终形态", 80,')
for p in ['CMakeLists.txt','main/game_ui.c','web/index.html','web/app.js']:
 f=R/p;f.write_text(f.read_text().replace('0.19.0','0.20.0').replace('0190-pets126-ready1',f'0200-pets{end}-ready1'))
for a,b in [('<small>126</small>',f'<small>{end}</small>'),('<small>248</small>',f'<small>{families*4}</small>'),('125套动作模板',f'{end-1}套动作模板'),('001-126-animation-templates.zip',f'001-{end}-animation-templates.zip'),('62 个伙伴家庭，126 个已接入形态。',f'{families} 个伙伴家庭，{end} 个已接入形态。'),('<strong>126<small>',f'<strong>{end}<small>'),('001–126 · 共6024播放帧',f'001–{end} · 共{6024+(end-126)*48}播放帧')]:edit('web/index.html',a,b)
for p in ['web/app.js','web/animation-player.js']:edit(p,'[116,117,118,119].includes(dex)','[116,117,118,119,'+','.join(str(d)for d in range(129,min(end,131)+1))+'].includes(dex)')
edit('tools/package_firmware.py','122–126本世代独立。','122–128本世代独立；'+('129在20级进化130暴鲤龙。'if end>=130 else'129鲤鱼王已接入，20级进化130暂待后续完整资源，不开放缺图进化。'))
edit('tests/test_forms_117_126.mjs','assert.equal(pendingChains.length,0);','assert(!pendingChains.some(c=>c.nationalIds.some(d=>d<=126)));')
edit('tools/test.sh','node "$root/tests/test_evolution_demo.mjs"','cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I "$root/core" "$root/core/game.c" "$root/core/moves.c" "$root/tests/test_forms_127_129.c" -lm -o "$root/tests/build/test_forms_127_129"\n"$root/tests/build/test_forms_127_129"\nnode "$root/tests/test_forms_127_129.mjs"\nnode "$root/tests/test_evolution_demo.mjs"')
print(f'Staged{end} forms/{families} families/{families*4} moves; validation/build required,not flashed')
