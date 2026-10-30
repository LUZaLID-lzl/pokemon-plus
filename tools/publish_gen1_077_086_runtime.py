#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Apply reviewed 86-form batch without touching device or legacy save layout."""
from pathlib import Path
import json,subprocess,sys,shutil
R=Path(__file__).resolve().parents[1]
qa=json.loads((R/'docs/gen1-077-086/visual-review.json').read_text())
assert set(range(77,87))<=set(qa['identityReviewed'])&set(qa['animationReviewed'])
assert len(json.loads((R/'assets/pet-roster.json').read_text()))==76,'Only publish onto76-form runtime'
backup=R/'build/pets86-before-runtime';assert not backup.exists()
for file in ['CMakeLists.txt','partitions.csv','core/game.h','core/game.c','core/moves.c','core/moves.h','main/game_ui.c','main/battle_view.c','main/image_assets.c','preview/preview.c','assets/pet-roster.json','assets/evolution.json','assets/moves.json','assets/pet-animation.json','web/app.js','web/index.html','web/animation-player.js','tools/prepare_builtin_assets.py','tools/validate_builtin_firmware.py','tools/export_pet_animation.py','tools/build_animation_001_preview.py','tools/test.sh','tests/test_forms_067_076.c']:
 dst=backup/file;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(R/file,dst)
subprocess.run([sys.executable,str(R/'tools/integrate_gen1_077_086.py')],check=True)
def edit(file,a,b):
 p=R/file;s=p.read_text();assert a in s,(file,a);p.write_text(s.replace(a,b))
edit('core/game.h','PP_SPECIES 31','PP_SPECIES 37');edit('core/game.h','PP_EVOLUTIONS 45','PP_EVOLUTIONS 49')
edit('core/game.c','"玛瑙水母", "小拳石"};','"玛瑙水母", "小拳石", "小火马", "呆呆兽", "小磁怪", "大葱鸭", "嘟嘟", "小海狮"};')
edit('core/game.c','{30,0,1,75,"隆隆岩"}};','{30,0,1,75,"隆隆岩"},\n {31,40,0,77,"烈焰马"}, {32,37,0,79,"呆壳兽"}, {33,30,0,81,"三合一磁怪"}, {35,31,0,84,"嘟嘟利"}};')
# Append types to preserve every old enum and move ID.
edit('core/moves.h','PP_PSYCHIC, PP_FIGHTING','PP_PSYCHIC, PP_FIGHTING, PP_ICE, PP_STEEL')
types={'火':'PP_FIRE','一般':'PP_NORMAL','水':'PP_WATER','电':'PP_ELECTRIC','飞行':'PP_FLYING','虫':'PP_BUG','超能力':'PP_PSYCHIC','冰':'PP_ICE','钢':'PP_STEEL'}
new=json.loads((R/'assets/moves.json').read_text())['moves'][124:]
edit('core/moves.c','\n};\nconst pp_move *pp_move_at',''.join('\n    {"'+v['name']+'", "'+v['type']+'", "'+v['description']+'", '+types[v['type']]+'},' for v in new)+'\n};\nconst pp_move *pp_move_at')
edit('core/moves.c',': 112+slot*3+species-28]',': species<31 ? 112+slot*3+species-28 : 124+slot*6+species-31]')
edit('core/game.c','type==PP_GRASS || type==PP_FIGHTING','type==PP_GRASS || type==PP_FIGHTING || type==PP_ICE || type==PP_STEEL')
edit('main/battle_view.c','0xf4a8c5, 0xd695ed, 0xe6a083};','0xf4a8c5, 0xd695ed, 0xe6a083, 0xa8e5f3, 0xcbd8e2};')
edit('main/battle_view.c','} else if (type == PP_PSYCHIC) {','''} else if (type == PP_ICE) {
      int x=source+(int)((dest-source)*progress);
      for(unsigned i=0;i<12;i++) {
        int dx=(int)i*3-18,dy=(int)(sinf(ms*.015f+i)*10);
        pixel(x+dx,116+dy,3,9,i%2?tint:0xf4ffff,240);
        pixel(x+dx-3,119+dy,9,3,tint,240);
      }
    } else if (type == PP_STEEL) {
      for(unsigned i=0;i<32;i++) {
        float t=i/31.0f;if(t>progress)break;
        pixel(source+(int)((dest-source)*t),114+(int)(i%3)*3,7,5,i%3?tint:0xffffff,245);
      }
    } else if (type == PP_PSYCHIC) {''')
for file in ['main/game_ui.c','preview/preview.c']:edit(file,'0x80cb83,0x74c7ed,0xc4b29a','0x80cb83,0x74c7ed,0xc4b29a,0xf69b68,0xf3a7bc,0xc5d3dc,0xd9b67c,0xd9b67c,0xcde7ef')
edit('main/game_ui.c','label(body, "最终形态", 80,','label(body, p->species==36 ? "Lv.34进化 · 087待接入" : "最终形态", 80,')
for f in ['CMakeLists.txt','main/game_ui.c','web/index.html','web/app.js']:
 p=R/f;p.write_text(p.read_text().replace('0.14.0','0.15.0').replace('0140-pets76-ready1','0150-pets86-ready1'))
for a,b in [('<small>76</small>','<small>86</small>'),('<small>124</small>','<small>148</small>'),('75套动作模板','85套动作模板'),('001-076-animation-templates.zip','001-086-animation-templates.zip'),('31 个伙伴家庭，76 个已接入形态。','37 个伙伴家庭，86 个已接入形态。'),('<strong>76<small>','<strong>86<small>'),('001–076 · 共3624播放帧','001–086 · 共4104播放帧')]:edit('web/index.html',a,b)
for f in ['web/app.js','web/animation-player.js']:
 edit(f,"dex===74?'悬浮':'移动'","[74,81,82].includes(dex)?'悬浮':dex===86?'滑行':'移动'")
# Capacity: new application remains at0x10000; NVS/PHY stay byte-identical.
# Reserve2MiB for ~1.1MiB app. Reassign unused app padding to native image bank.
edit('partitions.csv','factory,app,factory,0x10000,4M\nassets,data,0x40,0x410000,0xBF0000','factory,app,factory,0x10000,2M\nassets,data,0x40,0x210000,0xDF0000')
edit('tools/prepare_builtin_assets.py','<=0xBF0000','<=0xDF0000')
edit('tools/validate_builtin_firmware.py',"assert parts['assets'] == (1, 0x40, 0x410000, 0xBF0000)","assert parts['factory'] == (0, 0, 0x10000, 0x200000)\nassert parts['assets'] == (1, 0x40, 0x210000, 0xDF0000)\nassert parts['assets'][2]+parts['assets'][3]==16*1024*1024\nassert len((folder/'pokemon_plus.bin').read_bytes())<=parts['factory'][3]")
edit('main/image_assets.c','IMAGE_LIMIT 9200','IMAGE_LIMIT 10200');edit('main/image_assets.c','(20u * 1024u * 1024u)','(22u * 1024u * 1024u)')
edit('tests/test_forms_067_076.c','PP_SPECIES==31 && PP_DEX_COUNT==76 && PP_EVOLUTIONS==45 && PP_MOVE_COUNT==124','PP_SPECIES>=31 && PP_DEX_COUNT>=76 && PP_EVOLUTIONS>=45 && PP_MOVE_COUNT>=124')
edit('tools/test.sh','node "$root/tests/test_evolution_demo.mjs"','''cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I "$root/core" "$root/core/game.c" "$root/core/moves.c" "$root/tests/test_forms_077_086.c" -lm -o "$root/tests/build/test_forms_077_086"
"$root/tests/build/test_forms_077_086"
node "$root/tests/test_forms_077_086.mjs"
node "$root/tests/test_evolution_demo.mjs"''')
print('v0.15.0 staged:86 forms/37 families/148 moves/49 transitions; build and QA required.')
