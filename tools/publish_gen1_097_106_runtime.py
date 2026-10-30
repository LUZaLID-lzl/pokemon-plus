#!/usr/bin/env python3
"""Stage the complete reviewed106 release, retaining old save and move IDs."""
from pathlib import Path
import json, shutil, subprocess, sys
R=Path(__file__).resolve().parents[1]
qa=json.loads((R/'docs/gen1-097-106/visual-review.json').read_text())
assert set(range(97,107))<=set(qa['identityReviewed'])&set(qa['animationReviewed'])
assert len(json.loads((R/'assets/pet-roster.json').read_text()))==96
backup=R/'build/pets106-before-runtime';assert not backup.exists()
files=['CMakeLists.txt','partitions.csv','core/game.h','core/game.c','core/moves.c','core/moves.h','core/pokedex_roster.h','main/game_ui.c','main/battle_view.c','main/image_assets.c','main/image_assets.h','preview/preview.c','assets/pet-roster.json','assets/evolution.json','assets/moves.json','assets/pet-animation.json','web/app.js','web/index.html','web/animation-player.js','tools/prepare_assets.py','tools/validate_assets.py','tools/prepare_builtin_assets.py','tools/validate_builtin_firmware.py','tools/package_firmware.py','tools/test.sh','tests/test_forms_087_096.c','tests/test_forms_087_096.mjs','assets/animation-templates/index.json','web/manifest.json','sdcard/pokemon-plus/manifest.json']
for file in files:
 dst=backup/file;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(R/file,dst)
dst=backup/'preview/CMakeLists.txt';shutil.copy2(R/'preview/CMakeLists.txt',dst)
subprocess.run([sys.executable,str(R/'tools/integrate_gen1_097_106.py')],check=True)
def edit(file,a,b):
 p=R/file;s=p.read_text();assert a in s,(file,a);p.write_text(s.replace(a,b))
edit('core/game.h','PP_SPECIES 42','PP_SPECIES 47')
edit('core/game.h','PP_EVOLUTIONS 54','PP_EVOLUTIONS 59')
edit('core/game.c','"大岩蛇", "催眠貘"};','"大岩蛇", "催眠貘", "大钳蟹", "霹雳电球", "蛋蛋", "卡拉卡拉", "飞腿郎"};')
edit('core/game.c','{39,0,1,93,"耿鬼"}};','{39,0,1,93,"耿鬼"},\n {41,26,0,96,"引梦貘人"}, {42,28,0,98,"巨钳蟹"}, {43,30,0,100,"顽皮雷弹"}, {44,0,0,102,"椰蛋树"}, {45,28,0,104,"嘎啦嘎啦"}};')
edit('core/game.c','t->species==17 || t->species==28)','t->species==17 || t->species==28 || t->species==44)')
edit('core/game.c','species==17 || species==28)','species==17 || species==28 || species==44)')
types={'一般':'PP_NORMAL','水':'PP_WATER','电':'PP_ELECTRIC','草':'PP_GRASS','超能力':'PP_PSYCHIC','地面':'PP_GROUND','格斗':'PP_FIGHTING','虫':'PP_BUG'}
new=json.loads((R/'assets/moves.json').read_text())['moves'][168:]
assert len(new)==20
edit('core/moves.c','\n};\nconst pp_move *pp_move_at',''.join('\n    {"'+v['name']+'", "'+v['type']+'", "'+v['description']+'", '+types[v['type']]+'},' for v in new)+'\n};\nconst pp_move *pp_move_at')
edit('core/moves.c',': 148+slot*5+species-37]',': species<42 ? 148+slot*5+species-37 : 168+slot*5+species-42]')
for f in ['main/game_ui.c','preview/preview.c']:
 edit(f,'0xc294df,0xc294df,0x9f87dc,0xc4b29a,0xd695ed','0xc294df,0xc294df,0x9f87dc,0xc4b29a,0xd695ed,0xf4ad7a,0xef9d9d,0xe9b4cb,0xd8bb98,0xd4b591')
edit('main/game_ui.c','p->species==41 ? "Lv.26进化 · 097待接入" : "最终形态"','"最终形态"')
for f in ['CMakeLists.txt','main/game_ui.c','web/index.html','web/app.js']:
 p=R/f;p.write_text(p.read_text().replace('0.16.0','0.17.0').replace('0160-pets96-ready1','0170-pets106-ready3'))
for a,b in [('<small>96</small>','<small>106</small>'),('<small>168</small>','<small>188</small>'),('95套动作模板','105套动作模板'),('001-096-animation-templates.zip','001-106-animation-templates.zip'),('42 个伙伴家庭，96 个已接入形态。','47 个伙伴家庭，106 个已接入形态。'),('<strong>96<small>','<strong>106<small>'),('001–096 · 共4584播放帧','001–106 · 共5064播放帧')]:edit('web/index.html',a,b)
for f in ['web/app.js','web/animation-player.js']:
 edit(f,"dex===95?'盘行':'移动'","dex===95?'盘行':[98,99].includes(dex)?'横移':[100,101].includes(dex)?'轻滚':dex===102?'倾摇':'移动'")
edit('main/image_assets.c','IMAGE_LIMIT 11200','IMAGE_LIMIT 12200')
edit('main/image_assets.c','(24u * 1024u * 1024u)','(26u * 1024u * 1024u)')
edit('tools/package_firmware.py','096的097后续形态未开放。','096在26级、098在28级、100在30级、104在28级进化；102用叶之石200金币进化103，106为独立形态。')
edit('tests/test_forms_087_096.c','PP_SPECIES==42 && PP_DEX_COUNT==96 && PP_EVOLUTIONS==54 && PP_MOVE_COUNT==168','PP_SPECIES>=42 && PP_DEX_COUNT>=96 && PP_EVOLUTIONS>=54 && PP_MOVE_COUNT>=168')
edit('tests/test_forms_087_096.c','for(unsigned s=40;s<=41;s++)','for(unsigned s=40;s<=40;s++)')
edit('tests/test_forms_087_096.mjs','[86,88,90,93,94,95]','[86,88,90,93,94,96]')
edit('tools/test.sh','node "$root/tests/test_evolution_demo.mjs"','''cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I "$root/core" "$root/core/game.c" "$root/core/moves.c" "$root/tests/test_forms_097_106.c" -lm -o "$root/tests/build/test_forms_097_106"
"$root/tests/build/test_forms_097_106"
node "$root/tests/test_forms_097_106.mjs"
node "$root/tests/test_evolution_demo.mjs"''')
subprocess.run([sys.executable,str(R/'tools/enable_native_pet_canvas.py')],check=True)
print('Staged v0.17.0:106 forms/47 families/59 transitions/188 moves. Full build/render QA required.')
