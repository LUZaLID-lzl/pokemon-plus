#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Publish only the complete reviewed batch, retaining save/ID compatibility."""
from pathlib import Path
import json,subprocess,sys,shutil
R=Path(__file__).resolve().parents[1]
qa=json.loads((R/'docs/gen1-087-096/visual-review.json').read_text())
assert set(range(87,97))<=set(qa['identityReviewed'])&set(qa['animationReviewed'])
assert len(json.loads((R/'assets/pet-roster.json').read_text()))==86
backup=R/'build/pets96-before-runtime';assert not backup.exists()
files=['CMakeLists.txt','partitions.csv','core/game.h','core/game.c','core/moves.c','core/moves.h','main/game_ui.c','main/battle_view.c','main/image_assets.c','preview/preview.c','assets/pet-roster.json','assets/evolution.json','assets/moves.json','assets/pet-animation.json','web/app.js','web/index.html','web/animation-player.js','tools/prepare_builtin_assets.py','tools/validate_builtin_firmware.py','tools/package_firmware.py','tools/export_pet_animation.py','tools/build_animation_001_preview.py','tools/test.sh','tests/test_forms_077_086.c','tests/test_forms_077_086.mjs']
for file in files:
 dst=backup/file;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(R/file,dst)
for file in ['assets/animation-templates/index.json','web/manifest.json','sdcard/pokemon-plus/manifest.json']:
 dst=backup/file;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(R/file,dst)
subprocess.run([sys.executable,str(R/'tools/integrate_gen1_087_096.py')],check=True)
def edit(file,a,b):
 p=R/file;s=p.read_text();assert a in s,(file,a);p.write_text(s.replace(a,b))
edit('core/game.h','PP_SPECIES 37','PP_SPECIES 42');edit('core/game.h','PP_EVOLUTIONS 49','PP_EVOLUTIONS 54')
edit('core/game.c','"嘟嘟", "小海狮"};','"嘟嘟", "小海狮", "臭泥", "大舌贝", "鬼斯", "大岩蛇", "催眠貘"};')
edit('core/game.c','{35,31,0,84,"嘟嘟利"}};','{35,31,0,84,"嘟嘟利"},\n {36,34,0,86,"白海狮"}, {37,38,0,88,"臭臭泥"}, {38,0,0,90,"刺甲贝"}, {39,25,0,92,"鬼斯通"}, {39,0,1,93,"耿鬼"}};')
edit('core/game.c','t->species==25 ? "水之石"','(t->species==25 || t->species==38) ? "水之石"')
edit('core/game.c','t->species==27 || t->species==30)','t->species==27 || t->species==30 || t->species==39)')
edit('core/game.c','species==25 ? PP_WATER_STONE_PRICE','(species==25 || species==38) ? PP_WATER_STONE_PRICE')
edit('core/game.c','species==27 || species==30)','species==27 || species==30 || species==39)')
edit('core/moves.h','PP_ICE, PP_STEEL','PP_ICE, PP_STEEL, PP_GHOST')
types={'一般':'PP_NORMAL','毒':'PP_POISON','冰':'PP_ICE','幽灵':'PP_GHOST','岩石':'PP_ROCK','地面':'PP_GROUND','超能力':'PP_PSYCHIC'}
new=json.loads((R/'assets/moves.json').read_text())['moves'][148:]
edit('core/moves.c','\n};\nconst pp_move *pp_move_at',''.join('\n    {"'+v['name']+'", "'+v['type']+'", "'+v['description']+'", '+types[v['type']]+'},' for v in new)+'\n};\nconst pp_move *pp_move_at')
edit('core/moves.c',': 124+slot*6+species-31]',': species<37 ? 124+slot*6+species-31 : 148+slot*5+species-37]')
edit('main/battle_view.c','0xa8e5f3, 0xcbd8e2};','0xa8e5f3, 0xcbd8e2, 0x9f87dc};')
edit('main/battle_view.c','} else if (type == PP_PSYCHIC) {','''} else if (type == PP_GHOST) {
      int x=source+(int)((dest-source)*progress);
      for(unsigned i=0;i<24;i++) {
        float a=i*3.141593f/12;
        int r=6+(int)(i%3)*3;
        pixel(x+(int)(cosf(a)*r),116+(int)(sinf(a)*r),4,4,
              i%3?tint:0xd5c7f5,235);
      }
    } else if (type == PP_PSYCHIC) {''')
for f in ['main/game_ui.c','preview/preview.c']:
 edit(f,'0xd9b67c,0xd9b67c,0xcde7ef','0xd9b67c,0xd9b67c,0xcde7ef,0xc294df,0xc294df,0x9f87dc,0xc4b29a,0xd695ed')
edit('main/game_ui.c','p->species==36 ? "Lv.34进化 · 087待接入"','p->species==41 ? "Lv.26进化 · 097待接入"')
for f in ['CMakeLists.txt','main/game_ui.c','web/index.html','web/app.js']:
 p=R/f;p.write_text(p.read_text().replace('0.15.0','0.16.0').replace('0150-pets86-ready1','0160-pets96-ready1'))
edit('web/app.js','animation-player.js?v=0150-pets86-ready1','animation-player.js?v=0160-pets96-ready1') if 'animation-player.js?v=0150-pets86-ready1' in (R/'web/app.js').read_text() else None
for a,b in [('<small>86</small>','<small>96</small>'),('<small>148</small>','<small>168</small>'),('85套动作模板','95套动作模板'),('001-086-animation-templates.zip','001-096-animation-templates.zip'),('37 个伙伴家庭，86 个已接入形态。','42 个伙伴家庭，96 个已接入形态。'),('<strong>86<small>','<strong>96<small>'),('001–086 · 共4104播放帧','001–096 · 共4584播放帧')]:edit('web/index.html',a,b)
for f in ['web/app.js','web/animation-player.js']:
 edit(f,"dex===86?'滑行':'移动'","[86,87].includes(dex)?'滑行':[88,89,90,91].includes(dex)?'挪动':[92,93].includes(dex)?'悬浮':dex===95?'盘行':'移动'")
# Capacity: the 1.12MiB application fits a1.5MiB reservation; all pet bytes remain native.
edit('partitions.csv','factory,app,factory,0x10000,2M\nassets,data,0x40,0x210000,0xDF0000','factory,app,factory,0x10000,0x180000\nassets,data,0x40,0x190000,0xE70000')
edit('tools/prepare_builtin_assets.py','<=0xDF0000','<=0xE70000')
edit('tools/validate_builtin_firmware.py',"(0, 0, 0x10000, 0x200000)","(0, 0, 0x10000, 0x180000)")
edit('tools/validate_builtin_firmware.py',"(1, 0x40, 0x210000, 0xDF0000)","(1, 0x40, 0x190000, 0xE70000)")
edit('main/image_assets.c','IMAGE_LIMIT 10200','IMAGE_LIMIT 11200');edit('main/image_assets.c','(22u * 1024u * 1024u)','(24u * 1024u * 1024u)')
edit('tools/package_firmware.py','预留2MiB、assets起点0x210000/大小0xDF0000','预留1.5MiB、assets起点0x190000/大小0xE70000')
edit('tools/package_firmware.py','086的087后续形态未开放。','086在34级进化087；088在38级、090用水之石、092在25级后用联系之绳进化，096的097后续形态未开放。')
# Historical tests retain old guarantees while using current final-form guards.
edit('tests/test_forms_077_086.c','PP_SPECIES==37 && PP_DEX_COUNT==86 && PP_EVOLUTIONS==49 && PP_MOVE_COUNT==148','PP_SPECIES>=37 && PP_DEX_COUNT>=86 && PP_EVOLUTIONS>=49 && PP_MOVE_COUNT>=148')
edit('tests/test_forms_077_086.c','for(unsigned s=34;s<=36;s+=2)','for(unsigned s=34;s<=40;s+=6)')
edit('tests/test_forms_077_086.mjs','[77,79,81,82,84,85]','[77,79,81,82,84,86]')
edit('tools/test.sh','node "$root/tests/test_evolution_demo.mjs"','''cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I "$root/core" "$root/core/game.c" "$root/core/moves.c" "$root/tests/test_forms_087_096.c" -lm -o "$root/tests/build/test_forms_087_096"
"$root/tests/build/test_forms_087_096"
node "$root/tests/test_forms_087_096.mjs"
node "$root/tests/test_evolution_demo.mjs"''')
print('Staged v0.16.0:96 forms/42 families/54 transitions/168 moves. Build/QA required.')
