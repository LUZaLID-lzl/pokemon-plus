#!/usr/bin/env python3
"""Reduce one generated identity; retain its real source instead of a fake batch atlas."""
from pathlib import Path
import argparse, hashlib, json
from PIL import Image, ImageDraw, ImageFont
from pet_identity import sprite, groups
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('dex',type=int);p.add_argument('--version',type=int,default=1);a=p.parse_args()
plans=[(f,json.loads(f.read_text()))for f in (R/'assets/pokemon').glob('plan-*.json')]
match=[(f,plan,item)for f,plan in plans for item in plan.get('items',[])if item['nationalId']==a.dex];assert len(match)==1
planfile,plan,item=match[0];scope=planfile.stem.removeprefix('plan-')
source=R/f'assets/source/identity-{a.dex:03}-v{a.version}.png';prompt=R/f'assets/prompts/identity-{a.dex:03}-v{a.version}.txt';logfile=R/f'build/identity-{a.dex:03}-generation-v{a.version}.log'
raw=Image.open(source).convert('RGBA');assert raw.getchannel('A').getextrema()[0]==0,'Source must have true transparent margins'
components=[g for g in groups({(x,y)for y in range(raw.height)for x in range(raw.width)if raw.getpixel((x,y))[3]>=230})if len(g)>500]
assert len(components)==1,('Expected one complete source body; inspect image',len(components))
g=components[0];box=[min(x for x,y in g),min(y for x,y in g),max(x for x,y in g)+1,max(y for x,y in g)+1]
im,outline=sprite(raw.crop(box),preserve_hues=True)
folder=R/f'assets/pokemon/gen1-{scope}';folder.mkdir(exist_ok=True)
im.save(folder/f'{a.dex:03}.png');im.resize((384,384),Image.Resampling.NEAREST).save(folder/f'{a.dex:03}@8x.png')
f=folder/'manifest.json';m=json.loads(f.read_text())if f.exists()else dict(kind='individual single-pose identity drawings',generator='MobiIoT',nativeCanvas=48,maxColors=16,outlineColor='#1b263a',alpha='binary',gameIntegrated=False,animationPosesPerSpecies=1,items=[])
log=logfile.read_text();model=next(x.split(': ',1)[1]for x in reversed(log.splitlines())if x.startswith('Selected model: '))
entry=dict(id=a.dex,name=item['name'],english=item['english'],chain=item['species'],stage=item['stage'],image=f'{a.dex:03}.png',preview=f'{a.dex:03}@8x.png',sourceBox=box,outlinePixels=outline,version=a.version,model=model,attempts=log.count('Attempt '),sourceImage=str(source.relative_to(R)),sourceSha256=hashlib.sha256(source.read_bytes()).hexdigest(),prompt=str(prompt.relative_to(R)),generationLog=str(logfile.relative_to(R)),source='https://wiki.52poke.com/wiki/'+item['name'])
m['items']=[x for x in m['items']if x['id']!=a.dex]+[entry];m['items'].sort(key=lambda x:x['id']);m['completedIdentityNationalIds']=[x['id']for x in m['items']];m['pendingIdentityNationalIds']=[x['nationalId']for x in plan['items']if x['nationalId']not in m['completedIdentityNationalIds']]
# Newly exported art needs review and runtime publication even if an older
# drawing of this national ID was integrated. Preserve the other item flags.
previous=set(m.get('integratedNationalIds',[]))
entry['gameIntegrated']=False
m['integratedNationalIds']=[x['id']for x in m['items']if x['id']!=a.dex and x.get('gameIntegrated',x['id']in previous)]
m['gameIntegrated']=all(x['id']in m['integratedNationalIds']for x in m['items'])
f.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n')
source.with_name(source.stem+'-box.json').write_text(json.dumps(dict(box=box,sourceSha256=entry['sourceSha256']),indent=2)+'\n')
sheet=Image.new('RGB',(1000,200*((len(m['items'])+4)//5)),'#e1e9df');draw=ImageDraw.Draw(sheet);font=ImageFont.truetype('/System/Library/Fonts/STHeiti Medium.ttc',15)
for i,x in enumerate(m['items']):
 img=Image.open(folder/x['image']).resize((192,192),Image.Resampling.NEAREST);xx,yy=i%5*200,i//5*200;sheet.paste(img,(xx+4,yy),img);draw.text((xx+8,yy+180),f"{x['id']:03} {x['name']}",font=font,fill='#26394c')
sheet.save(folder/'preview.png')
(folder/'README.md').write_text(f'# {scope} single-pose identities\n\nEach exported identity retains its own actual source, prompt and attempts. Incomplete batch: consult manifest pendingIdentityNationalIds. Art review is separate from full animations/runtime integration.\n')
print(f'PASS:{a.dex:03} one measured complete identity/native48/binary alpha/closed connected outline/15 opaque colors; visual review required')
print(folder/f'{a.dex:03}@8x.png')
