#!/usr/bin/env python3
"""Stage reviewed117-126 only; building and release QA remain mandatory."""
from pathlib import Path
import json,shutil,subprocess,sys
R=Path(__file__).resolve().parents[1]
qa=json.loads((R/'docs/gen1-117-126/visual-review.json').read_text());assert set(range(117,127))<=set(qa['identityReviewed'])&set(qa['animationReviewed'])
assert len(json.loads((R/'assets/pet-roster.json').read_text()))==116
backup=R/'build/pets126-before-runtime';assert not backup.exists()
files=['CMakeLists.txt','partitions.csv','core/game.h','core/game.c','core/moves.c','core/moves.h','core/pokedex_roster.h','main/game_ui.c','main/battle_view.c','main/image_assets.c','main/image_assets.h','preview/preview.c','assets/pet-roster.json','assets/evolution.json','assets/moves.json','assets/pet-animation.json','web/app.js','web/index.html','web/animation-player.js','tools/prepare_assets.py','tools/validate_assets.py','tools/prepare_builtin_assets.py','tools/validate_builtin_firmware.py','tools/package_firmware.py','tools/test.sh','tests/test_forms_107_116.c','tests/test_forms_107_116.mjs','assets/animation-templates/index.json','web/manifest.json','sdcard/pokemon-plus/manifest.json']
for f in files:
 dst=backup/f;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(R/f,dst)
subprocess.run([sys.executable,str(R/'tools/integrate_gen1_117_126.py')],check=True)
def edit(f,a,b):
 p=R/f;s=p.read_text();assert a in s,(f,a);p.write_text(s.replace(a,b))
edit('core/game.h','PP_SPECIES 55','PP_SPECIES 62');edit('core/game.h','PP_EVOLUTIONS 61','PP_EVOLUTIONS 64')
edit('core/game.c','"墨海马"};','"墨海马", "角金鱼", "海星星", "魔墙人偶", "飞天螳螂", "迷唇姐", "电击兽", "鸭嘴火兽"};')
edit('core/game.c','{50,42,0,111,"钻角犀兽"}};','{50,42,0,111,"钻角犀兽"}, {54,32,0,116,"海刺龙"}, {55,33,0,118,"金鱼王"}, {56,0,0,120,"宝石海星"}};')
edit('core/game.c','(t->species==25 || t->species==38)','(t->species==25 || t->species==38 || t->species==56)')
edit('core/game.c','(species==25 || species==38)','(species==25 || species==38 || species==56)')
types={'一般':'PP_NORMAL','水':'PP_WATER','电':'PP_ELECTRIC','草':'PP_GRASS','地面':'PP_GROUND','格斗':'PP_FIGHTING','冰':'PP_ICE','火':'PP_FIRE','毒':'PP_POISON','岩石':'PP_ROCK','幽灵':'PP_GHOST','超能':'PP_PSYCHIC','飞行':'PP_FLYING','虫':'PP_BUG'}
new=json.loads((R/'assets/moves-plan-117-126.json').read_text())['moves']
edit('core/moves.c','\n};\nconst pp_move *pp_move_at',''.join('\n    {"'+v['name']+'", "'+v['type']+'", "'+v['description']+'", '+types[v['type']]+'},'for v in new)+'\n};\nconst pp_move *pp_move_at')
edit('core/moves.c',': 188+slot*8+species-47]',': species<55 ? 188+slot*8+species-47 : 220+slot*7+species-55]')
for f in ['main/game_ui.c','preview/preview.c']:edit(f,'0xd5b28b,0x8bd4ed','0xd5b28b,0x8bd4ed,0xf0b889,0xd7b07b,0xe8bed9,0xa0cf87,0xd6acde,0xf1d16c,0xeab77f')
edit('main/game_ui.c','p->species==54 ? "Lv.32进化 · 117待接入" : "最终形态"','"最终形态"')
for f in ['CMakeLists.txt','main/game_ui.c','web/index.html','web/app.js']:
 p=R/f;p.write_text(p.read_text().replace('0.18.0','0.19.0').replace('0180-pets116-ready1','0190-pets126-ready1'))
for a,b in [('<small>116</small>','<small>126</small>'),('<small>220</small>','<small>248</small>'),('115套动作模板','125套动作模板'),('001-116-animation-templates.zip','001-126-animation-templates.zip'),('55 个伙伴家庭，116 个已接入形态。','62 个伙伴家庭，126 个已接入形态。'),('<strong>116<small>','<strong>126<small>'),('001–116 · 共5544播放帧','001–126 · 共6024播放帧')]:edit('web/index.html',a,b)
for f in ['web/app.js','web/animation-player.js']:edit(f,"dex===116?'游动':'移动'","[116,117,118,119].includes(dex)?'游动':[120,121].includes(dex)?'星臂游动':'移动'")
edit('web/app.js',"(x.nationalId===41?'包含合翼休息姿态。':'包含闭眼睡姿。')","(x.nationalId===41?'包含合翼休息姿态。':[120,121].includes(x.nationalId)?'包含低伏暗心静息姿态。':'包含闭眼睡姿。')")
edit('main/image_assets.c','IMAGE_LIMIT 13200','IMAGE_LIMIT 14300');edit('main/image_assets.c','(28u * 1024u * 1024u)','(30u * 1024u * 1024u)')
edit('tools/package_firmware.py','116的32级进化等待117完整资源后开放。','116在32级进化117、118在33级进化119、120使用水之石200金币进化121；122–126本世代独立。')
edit('tests/test_forms_107_116.c','PP_SPECIES==55 && PP_DEX_COUNT==116 && PP_EVOLUTIONS==61 && PP_MOVE_COUNT==220','PP_SPECIES>=55 && PP_DEX_COUNT>=116 && PP_EVOLUTIONS>=61 && PP_MOVE_COUNT>=220')
edit('tests/test_forms_107_116.c','finals[]={47,48,51,52,53,54}','finals[]={47,48,51,52,53}')
edit('tests/test_forms_107_116.c','i<6;i++','i<sizeof(finals)/sizeof(finals[0]);i++')
edit('tests/test_forms_107_116.mjs','[106,107,109,111,112,113,114,115]','[106,107,109,111,112,113,114]')
edit('tests/test_forms_107_116.mjs',"assert.equal(pendingChains.length,1);assert.deepEqual(pendingChains[0].nationalIds,[116,117]);assert(!roster.some(p=>p.nationalId===117));","if(!roster.some(p=>p.nationalId===117)){assert.equal(pendingChains.length,1);assert.deepEqual(pendingChains[0].nationalIds,[116,117]);}else assert.equal(demo.offer(demo.add(115).id).next.nationalId,117);")
edit('tools/test.sh','node "$root/tests/test_evolution_demo.mjs"','cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I "$root/core" "$root/core/game.c" "$root/core/moves.c" "$root/tests/test_forms_117_126.c" -lm -o "$root/tests/build/test_forms_117_126"\n"$root/tests/build/test_forms_117_126"\nnode "$root/tests/test_forms_117_126.mjs"\nnode "$root/tests/test_evolution_demo.mjs"')
print('Stagedv0.19.0/126 forms/62 families/64 evolutions/248 moves;build/QA required,not flashed')
