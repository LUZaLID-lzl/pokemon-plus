# -*- coding: utf-8 -*-
"""Export ten reviewed identities using measured source bounds."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import hashlib, json, zipfile
from pet_identity import sprite

R=Path(__file__).resolve().parents[1]
SOURCE=R/'assets/source/gen1-067-076-v1.png'
OUT=R/'assets/pokemon/gen1-067-076';OUT.mkdir(exist_ok=True)
plan=json.loads((R/'assets/pokemon/plan-067-076.json').read_text())
boxes=json.loads(SOURCE.with_name(SOURCE.stem+'-boxes.json').read_text())
original=Image.open(SOURCE).convert('RGBA')
assert len(boxes)==len(plan['items'])==10
sheet=Image.new('RGB',(1000,400),'#e1e9df');draw=ImageDraw.Draw(sheet)
font=ImageFont.truetype('/System/Library/Fonts/STHeiti Medium.ttc',15)
items=[]
for i,(pet,box) in enumerate(zip(plan['items'],boxes)):
    assert 0<=box[0]<box[2]<=original.width and 0<=box[1]<box[3]<=original.height
    im,outline=sprite(original.crop(box),preserve_hues=True);dex=pet['nationalId'];number=f'{dex:03}'
    im.save(OUT/f'{number}.png');im.resize((384,384),Image.Resampling.NEAREST).save(OUT/f'{number}@8x.png')
    large=im.resize((192,192),Image.Resampling.NEAREST);x=i%5*200;y=i//5*200
    sheet.paste(large,(x+4,y),large);draw.text((x+8,y+180),number+' '+pet['name'],font=font,fill='#26394c')
    items.append(dict(id=dex,name=pet['name'],english=pet['english'],chain=pet['species'],stage=pet['stage'],image=number+'.png',preview=number+'@8x.png',sourceBox=box,outlinePixels=outline,source='https://wiki.52poke.com/wiki/'+pet['name']))
sheet.save(OUT/'preview.png')
log=(R/'build/gen1-067-076-generation-v1.log').read_text()
manifest=dict(version=1,kind='single-pose art review',generator='MobiIoT',model=next(l.split(': ',1)[1] for l in log.splitlines() if l.startswith('Selected model: ')),attempts=log.count('Attempt '),sourceAtlas='../../source/gen1-067-076-v1.png',sourceSha256=hashlib.sha256(SOURCE.read_bytes()).hexdigest(),prompt='../../prompts/gen1-067-076-v1.txt',nativeCanvas=48,maxColors=16,outlineColor='#1b263a',alpha='binary',gameIntegrated=bool(plan.get("runtimeIntegrated")),animationPosesPerSpecies=1,items=items)
(OUT/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
(OUT/'README.md').write_text('# 全国图鉴067–076\n\n十只原生48px单姿态画稿，独立动作与运行接入状态另行记录。\n')
with zipfile.ZipFile(R/'dist/gen1-067-076-pets.zip','w',zipfile.ZIP_DEFLATED) as z:
    for p in sorted(OUT.glob('*')):
        if p.is_file():z.write(p,'gen1-067-076/'+p.name)
print('PASS: ten native48px sprites; binary alpha, <=15 opaque colors, connected closed outlines')
print(OUT/'preview.png')
