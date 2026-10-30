#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Apply the reviewed 76-form batch; refuses premature or repeated publication."""
from pathlib import Path
import json,subprocess,sys
R=Path(__file__).resolve().parents[1]
qa=json.loads((R/'docs/gen1-067-076/visual-review.json').read_text())
assert set(range(67,77))<=set(qa['animationReviewed']),'Visual review incomplete'
assert len(json.loads((R/'assets/pet-roster.json').read_text()))==66,'Only publish onto 66-form runtime'
assert '#define PP_SPECIES 28' in (R/'core/game.h').read_text()
subprocess.run([sys.executable,str(R/'tools/integrate_gen1_067_076.py')],check=True)
def edit(name,a,b):
    p=R/name;s=p.read_text();assert a in s,(name,a);p.write_text(s.replace(a,b))
edit('core/game.h','PP_SPECIES 28','PP_SPECIES 31');edit('core/game.h','PP_EVOLUTIONS 38','PP_EVOLUTIONS 45')
edit('core/game.c','"凯西", "腕力"};','"凯西", "腕力", "喇叭芽", "玛瑙水母", "小拳石"};')
edit('core/game.c','{26,16,0,63,"勇基拉"}, {26,0,1,64,"胡地"}};','''{26,16,0,63,"勇基拉"}, {26,0,1,64,"胡地"},
 {27,28,0,66,"豪力"}, {27,0,1,67,"怪力"},
 {28,21,0,69,"口呆花"}, {28,0,1,70,"大食花"},
 {29,30,0,72,"毒刺水母"}, {30,25,0,74,"隆隆石"}, {30,0,1,75,"隆隆岩"}};''')
edit('core/game.c','t->species==17 ? "叶之石"','(t->species==17 || t->species==28) ? "叶之石"')
edit('core/game.c','t->species==26 ? "联系之绳"','(t->species==26 || t->species==27 || t->species==30) ? "联系之绳"')
edit('core/game.c','species==17 ? PP_LEAF_STONE_PRICE','(species==17 || species==28) ? PP_LEAF_STONE_PRICE')
edit('core/game.c','species==26 ? PP_LINK_CORD_PRICE','(species==26 || species==27 || species==30) ? PP_LINK_CORD_PRICE')
new=json.loads((R/'assets/moves.json').read_text())['moves'][112:];types={'草':'PP_GRASS','毒':'PP_POISON','一般':'PP_NORMAL','岩石':'PP_ROCK','地面':'PP_GROUND','水':'PP_WATER'}
edit('core/moves.c','\n};\nconst pp_move *pp_move_at',''.join('\n    {"'+v['name']+'", "'+v['type']+'", "'+v['description']+'", '+types[v['type']]+'},' for v in new)+'\n};\nconst pp_move *pp_move_at')
edit('core/moves.c',': 96+slot*4+species-24]',': species<28 ? 96+slot*4+species-24 : 112+slot*3+species-28]')
edit('main/game_ui.c','0xf69b68,0x74c7ed,0xc294df,0xe6a083','0xf69b68,0x74c7ed,0xc294df,0xe6a083,0x80cb83,0x74c7ed,0xc4b29a')
edit('main/game_ui.c','p->species==27 ? "Lv.28进化 · 067待接入" : "最终形态"','"最终形态"')
edit('preview/preview.c','0xf69b68,0x74c7ed,0xc294df,0xe6a083};','0xf69b68,0x74c7ed,0xc294df,0xe6a083,0x80cb83,0x74c7ed,0xc4b29a};')
for f in ['CMakeLists.txt','main/game_ui.c','web/index.html','web/app.js']:
    p=R/f;p.write_text(p.read_text().replace('0.13.0','0.14.0').replace('0130-pets66-ready1','0140-pets76-ready1'))
for a,b in [('<small>66</small>','<small>76</small>'),('<small>112</small>','<small>124</small>'),('65套动作模板','75套动作模板'),('001-066-animation-templates.zip','001-076-animation-templates.zip'),('28 个伙伴家庭，66 个已接入形态。','31 个伙伴家庭，76 个已接入形态。'),('<strong>66<small>','<strong>76<small>'),('001–066 · 共3144播放帧','001–076 · 共3624播放帧')]:edit('web/index.html',a,b)
# Older suites keep old move IDs and rule checks; their one former pending-family
# assertion becomes a final-form check once Machop's whole chain is available.
edit('tests/test_forms_037_046.c','init(27,0,PP_MAX_LEVEL)','init(29,1,PP_MAX_LEVEL)')
edit('tests/test_forms_037_046.c','==65','==72');edit('tests/test_forms_037_046.c','g.growth[0].stage=1;assert(!pp_valid(&g));','g.growth[0].stage=2;assert(!pp_valid(&g));')
edit('tests/test_forms_037_046.c','Machop future-art guard','Tentacruel final-form guard')
for f in ['tests/test_forms_047_056.c','tests/test_forms_057_066.c']:
    edit(f,'PP_SPECIES==28 && PP_DEX_COUNT==66 && PP_EVOLUTIONS==38 && PP_MOVE_COUNT==112','PP_SPECIES>=28 && PP_DEX_COUNT>=66 && PP_EVOLUTIONS>=38 && PP_MOVE_COUNT>=112')
edit('tests/test_forms_047_056.c','init(27,PP_MAX_LEVEL);','init(29,PP_MAX_LEVEL);g.growth[0].stage=1;')
edit('tests/test_forms_047_056.c','==65','==72');edit('tests/test_forms_047_056.c','g.growth[0].stage=1;assert(!pp_valid(&g));','g.growth[0].stage=2;assert(!pp_valid(&g));')
edit('tests/test_forms_047_056.c','067 future-art guard','final-form guard');edit('tests/test_forms_047_056.c','With 24 families','With 31 families');edit('tests/test_forms_047_056.c','28-family','31-family')
edit('tests/test_forms_057_066.c','init(27,0,PP_MAX_LEVEL);','init(29,1,PP_MAX_LEVEL);')
edit('tests/test_forms_057_066.c','==65','==72');edit('tests/test_forms_057_066.c','g.growth[0].stage=1;assert(!pp_valid(&g));','g.growth[0].stage=2;assert(!pp_valid(&g));')
edit('tests/test_forms_057_066.c','067 pending guard','final-form guard');edit('tests/test_forms_057_066.c','28-family','31-family')
edit('tests/test_forms_057_066.mjs','[56,58,61,64,65]','[56,58,61,64,72]');edit('tests/test_forms_057_066.mjs','final and pending guards','final form guards')
edit('tools/test.sh','node "$root/tests/test_evolution_demo.mjs"','''cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -I "$root/core" "$root/core/game.c" "$root/core/moves.c" "$root/tests/test_forms_067_076.c" -lm -o "$root/tests/build/test_forms_067_076"
"$root/tests/build/test_forms_067_076"
node "$root/tests/test_forms_067_076.mjs"
node "$root/tests/test_evolution_demo.mjs"''')
print('Updated runtime to v0.14.0; derive assets/fonts, build and validate before marking complete.')
