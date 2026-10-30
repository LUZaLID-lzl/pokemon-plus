#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Measure ten source identities, export native art without guessing equal crops."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import argparse,hashlib,json,zipfile
from pet_identity import sprite,groups
R=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--version',type=int,default=1);args=parser.parse_args();version=args.version
SOURCE=R/f'assets/source/gen1-107-116-v{version}.png'
OUT=R/'assets/pokemon/gen1-107-116';OUT.mkdir(exist_ok=True)
plan=json.loads((R/'assets/pokemon/plan-107-116.json').read_text())
original=Image.open(SOURCE).convert('RGBA')
components=[g for g in groups({(x,y) for y in range(original.height) for x in range(original.width) if original.getpixel((x,y))[3]>=230}) if len(g)>500]
measure=[dict(box=[min(x for x,y in g),min(y for x,y in g),max(x for x,y in g)+1,max(y for x,y in g)+1],pixels=len(g)) for g in components]
SOURCE.with_name(SOURCE.stem+'-components.json').write_text(json.dumps(measure,indent=2)+'\n')
assert len(components)==10,('Expected ten complete isolated characters; inspect source',len(components))
rows=[[g for g in components if int(sum(y for x,y in g)/len(g)*2/original.height)==row] for row in range(2)]
assert all(len(row)==5 for row in rows),'Expected five measured bodies in each source row'
ordered=[g for row in rows for g in sorted(row,key=lambda g:min(x for x,y in g))]
boxes=[[min(x for x,y in g),min(y for x,y in g),max(x for x,y in g)+1,max(y for x,y in g)+1] for g in ordered]
SOURCE.with_name(SOURCE.stem+'-boxes.json').write_text(json.dumps(boxes)+'\n')
sheet=Image.new('RGB',(1000,400),'#e1e9df');draw=ImageDraw.Draw(sheet)
font=ImageFont.truetype('/System/Library/Fonts/STHeiti Medium.ttc',15);items=[]
for i,(pet,box) in enumerate(zip(plan['items'],boxes)):
 im,outline=sprite(original.crop(box),preserve_hues=True);dex=pet['nationalId'];number=f'{dex:03}'
 im.save(OUT/f'{number}.png');im.resize((384,384),Image.Resampling.NEAREST).save(OUT/f'{number}@8x.png')
 large=im.resize((192,192),Image.Resampling.NEAREST);x=i%5*200;y=i//5*200
 sheet.paste(large,(x+4,y),large);draw.text((x+8,y+180),number+' '+pet['name'],font=font,fill='#26394c')
 items.append(dict(id=dex,name=pet['name'],english=pet['english'],chain=pet['species'],stage=pet['stage'],image=number+'.png',preview=number+'@8x.png',sourceBox=box,outlinePixels=outline,source='https://wiki.52poke.com/wiki/'+pet['name']))
sheet.save(OUT/'preview.png')
log=(R/f'build/gen1-107-116-generation-v{version}.log').read_text()
manifest=dict(version=version,kind='single-pose art review',generator='MobiIoT',model=next(l.split(': ',1)[1] for l in reversed(log.splitlines()) if l.startswith('Selected model: ')),attempts=log.count('Attempt '),sourceAtlas=f'../../source/gen1-107-116-v{version}.png',sourceSha256=hashlib.sha256(SOURCE.read_bytes()).hexdigest(),prompt=f'../../prompts/gen1-107-116-v{version}.txt',nativeCanvas=48,maxColors=16,outlineColor='#1b263a',alpha='binary',gameIntegrated=False,animationPosesPerSpecies=1,items=items)
(OUT/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
(OUT/'README.md').write_text('# 全国图鉴107–116 · 单姿态形象\n\n身份图导出；完整动作与运行接入状态另行验收，不把单姿态计为完整模板。\n')
with zipfile.ZipFile(R/'dist/gen1-107-116-pets.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(OUT.glob('*')):
  if p.is_file():z.write(p,'gen1-107-116/'+p.name)
print('PASS: ten measured native48px identities, binary alpha, <=15 opaque colors, connected closed outlines; visual review still required')
print(OUT/'preview.png')
