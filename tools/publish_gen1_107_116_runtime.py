#!/usr/bin/env python3
"""Publish only the fully reviewed107–116 batch, preserving previous IDs and saved data."""
from pathlib import Path
import json, shutil, subprocess,sys
R=Path(__file__).resolve().parents[1]
qa=json.loads((R/'docs/gen1-107-116/visual-review.json').read_text())
assert set(range(107,117))<=set(qa['identityReviewed'])&set(qa['animationReviewed'])
assert len(json.loads((R/'assets/pet-roster.json').read_text()))==106
backup=R/'build/pets116-before-runtime';assert not backup.exists()
files=['CMakeLists.txt','partitions.csv','core/game.h','core/game.c','core/moves.c','core/moves.h','core/pokedex_roster.h','main/game_ui.c','main/battle_view.c','main/image_assets.c','main/image_assets.h','preview/preview.c','assets/pet-roster.json','assets/evolution.json','assets/moves.json','assets/pet-animation.json','web/app.js','web/index.html','web/animation-player.js','tools/prepare_assets.py','tools/validate_assets.py','tools/prepare_builtin_assets.py','tools/validate_builtin_firmware.py','tools/package_firmware.py','tools/test.sh','tests/test_forms_097_106.c','assets/animation-templates/index.json','web/manifest.json','sdcard/pokemon-plus/manifest.json']
for file in files:
 dst=backup/file;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(R/file,dst)
subprocess.run([sys.executable,str(R/'tools/integrate_gen1_107_116.py')],check=True)
def edit(file,a,b):
 p=R/file;s=p.read_text();assert a in s,(file,a);p.write_text(s.replace(a,b))
edit('core/game.h','PP_SPECIES 47','PP_SPECIES 55')
edit('core/game.h','PP_MAX_LEVEL 40','PP_MAX_LEVEL 100')
edit('core/game.h','PP_EVOLUTIONS 59','PP_EVOLUTIONS 61')
edit('core/game.c','"卡拉卡拉", "飞腿郎"};','"卡拉卡拉", "飞腿郎", "艾比郎", "大舌头", "瓦斯弹", "独角犀牛", "吉利蛋", "蔓藤怪", "袋兽", "墨海马"};')
edit('core/game.c','{45,28,0,104,"嘎啦嘎啦"}};','{45,28,0,104,"嘎啦嘎啦"}, {49,35,0,109,"双弹瓦斯"}, {50,42,0,111,"钻角犀兽"}};')
types={'一般':'PP_NORMAL','水':'PP_WATER','电':'PP_ELECTRIC','草':'PP_GRASS','地面':'PP_GROUND','格斗':'PP_FIGHTING','冰':'PP_ICE','火':'PP_FIRE','毒':'PP_POISON','岩石':'PP_ROCK','幽灵':'PP_GHOST'}
new=json.loads((R/'assets/moves.json').read_text())['moves'][188:];assert len(new)==32
edit('core/moves.c','\n};\nconst pp_move *pp_move_at',''.join('\n    {"'+v['name']+'", "'+v['type']+'", "'+v['description']+'", '+types[v['type']]+'},'for v in new)+'\n};\nconst pp_move *pp_move_at')
edit('core/moves.c',': 168+slot*5+species-42]',': species<47 ? 168+slot*5+species-42 : 188+slot*8+species-47]')
for file in ['main/game_ui.c','preview/preview.c']:
 edit(file,'0xf4ad7a,0xef9d9d,0xe9b4cb,0xd8bb98,0xd4b591','0xf4ad7a,0xef9d9d,0xe9b4cb,0xd8bb98,0xd4b591,0xe9ad8c,0xefb1cb,0xb8a1e8,0xaeb5c5,0xf2bbd0,0x79b9ee,0xd5b28b,0x8bd4ed')
edit('main/game_ui.c','label(body, "最终形态", 80,','label(body, p->species==54 ? "Lv.32进化 · 117待接入" : "最终形态", 80,')
for file in ['CMakeLists.txt','main/game_ui.c','web/index.html','web/app.js']:
 p=R/file;p.write_text(p.read_text().replace('0.17.0','0.18.0').replace('0170-pets106-ready3','0180-pets116-ready1'))
for a,b in [('<small>106</small>','<small>116</small>'),('<small>188</small>','<small>220</small>'),('105套动作模板','115套动作模板'),('001-106-animation-templates.zip','001-116-animation-templates.zip'),('47 个伙伴家庭，106 个已接入形态。','55 个伙伴家庭，116 个已接入形态。'),('<strong>106<small>','<strong>116<small>'),('001–106 · 共5064播放帧','001–116 · 共5544播放帧')]:edit('web/index.html',a,b)
for file in ['web/app.js','web/animation-player.js']:
 edit(file,"dex===102?'倾摇':'移动'","dex===102?'倾摇':[109,110].includes(dex)?'悬浮':dex===114?'藤足挪动':dex===116?'游动':'移动'")
edit('main/image_assets.c','IMAGE_LIMIT 12200','IMAGE_LIMIT 13200')
edit('main/image_assets.c','(26u * 1024u * 1024u)','(28u * 1024u * 1024u)')
edit('tools/package_firmware.py','106为独立形态。','106/107/108/113/114/115为本世代独立形态；109在35级进化110、111在42级进化112；116的32级进化等待117完整资源后开放。')
edit('tests/test_forms_097_106.c','PP_SPECIES==47 && PP_DEX_COUNT==106 && PP_EVOLUTIONS==59 && PP_MOVE_COUNT==188','PP_SPECIES>=47 && PP_DEX_COUNT>=106 && PP_EVOLUTIONS>=59 && PP_MOVE_COUNT>=188')
edit('tools/test.sh','node "$root/tests/test_evolution_demo.mjs"','cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I "$root/core" "$root/core/game.c" "$root/core/moves.c" "$root/tests/test_forms_107_116.c" -lm -o "$root/tests/build/test_forms_107_116"\n"$root/tests/build/test_forms_107_116"\nnode "$root/tests/test_forms_107_116.mjs"\nnode "$root/tests/test_evolution_demo.mjs"')
print('Stagedv0.18.0:116 forms/55 families/61 transitions/220 moves; full QA/build required.')
