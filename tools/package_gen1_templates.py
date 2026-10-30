#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Package accepted templates and their canonical provenance."""
from pathlib import Path
import json,zipfile
from PIL import Image,ImageDraw,ImageFont
import argparse
parser=argparse.ArgumentParser();parser.add_argument('--start',type=int,default=1);parser.add_argument('--end',type=int,default=9);parser.add_argument('--skip',type=int,nargs='*',default=[]);args=parser.parse_args();start,end=args.start,args.end;scope=f'{start:03}-{end:03}'
R=Path(__file__).resolve().parents[1];root=R/'assets/animation-templates'
font=ImageFont.truetype('/System/Library/Fonts/STHeiti Medium.ttc',13)
dexes=[dex for dex in range(start,end+1) if dex not in args.skip]
config=[json.loads((root/f'{dex:03}/manifest.json').read_text()) for dex in dexes]
registry={p['nationalId']:p for p in json.loads((root/'index.json').read_text())}
# Unpublished, reviewed templates need a useful package index too. Keep the
# runtime registry untouched until a complete batch has passed integration QA.
package_index=[]
for pet in config:
    dex=pet['nationalId']
    if dex in registry:package_index.append(registry[dex]);continue
    matches=[item for plan in sorted((R/'assets/pokemon').glob('plan-*.json')) for item in json.loads(plan.read_text()).get('items',[]) if item.get('nationalId')==dex]
    assert len(matches)==1,('Missing or ambiguous unpublished art mapping',dex)
    package_index.append(dict(nationalId=dex,name=pet['name'],artId=matches[0]['artId'],path=f'../assets/animation-templates/{dex:03}/',gameIntegrated=False))
frames=[];times=[]
labels={'idle':'待机','walk':'移动','happy':'开心','eat':'进食','sleep':'睡眠','attack':'攻击'}
for action,label in labels.items():
    for index in range(8):
        sheet=Image.new('RGB',(432,178*((len(config)+2)//3)),'#e1e9df');draw=ImageDraw.Draw(sheet)
        for i,pet in enumerate(config):
            x,y=i%3*144,i//3*178
            im=Image.open(root/f'{pet["nationalId"]:03}'/pet['actions'][action]['frames'][index]).resize((144,144),Image.Resampling.NEAREST)
            sheet.paste(im,(x,y+13),im)
            draw.text((x+8,y+2),f'{pet["nationalId"]:03} {pet["name"]}',font=font,fill='#24473b')
            draw.text((x+8,y+159),f'{label} {index+1}/8',font=font,fill='#47745a')
        frames.append(sheet);times.append(config[0]['actions'][action]['durationsMs'][index])
frames[0].save(root/f'{scope}-preview.gif',save_all=True,append_images=frames[1:],duration=times,loop=0,optimize=False)
frames[0].save(root/f'{scope}-overview.png')
with zipfile.ZipFile(R/f'dist/{scope}-animation-templates.zip','w',zipfile.ZIP_DEFLATED) as z:
    package_status={'includedNationalIds':dexes,'excludedNationalIds':args.skip,'runtimeIntegrated':all(p.get('gameIntegrated',False) for p in config),'note':'025 retains original 24 translation slots; its 48-pose template is pending image service' if 25 in args.skip else 'All templates in this archive range included'}
    if not package_status['runtimeIntegrated']:
        remaining=json.loads((R/'assets/pokemon/remaining-gen1-plan.json').read_text())
        art_complete=remaining.get('artScopeCompleted',False)
        package_status.update(requestedRange=remaining['requestedRange'],artScopeCompleted=art_complete,pendingAnimationNationalIds=remaining.get('pendingAnimationNationalIds',[]),note='Requested animation art is complete; firmware/Web integration is pending.' if art_complete else 'Reviewed animation art only; firmware/Web integration and the remaining requested forms are pending.')
        z.write(R/'assets/pokemon/remaining-gen1-plan.json','provenance/assets/pokemon/remaining-gen1-plan.json')
    z.writestr('PACKAGE-STATUS.json',json.dumps(package_status,ensure_ascii=False,indent=2))
    for item,dex in enumerate(dexes):
        folder=root/f'{dex:03}'
        for p in sorted(folder.rglob('*')):
            if p.is_file():z.write(p,p.relative_to(root))
        z.write(R/f'docs/animation-{dex:03}/index.html',f'{dex:03}/scene-preview.html')
        for file in [config[item]['source'],config[item]['identityReference'],config[item]['prompt'],config[item]['prompt'].replace('master-','generation-').replace('.txt','.log')]+config[item].get('additionalProvenance',[])+([config[item]['assemblyProvenance']] if config[item].get('assemblyProvenance') else []):
            if 'provenance/'+file not in z.namelist():z.write(R/file,'provenance/'+file)
    for file in ['assets/evolution.json','assets/pet-roster.json','docs/EVOLUTION.md']:
        z.write(R/file,'gameplay/'+file)
    for review in sorted((R/'docs').glob('gen1-*/visual-review.json')):
        if set(json.loads(review.read_text()).get('animationReviewed',[])) & set(dexes):
            z.write(review,'provenance/'+review.relative_to(R).as_posix())
    z.writestr('index.json',json.dumps(package_index,ensure_ascii=False,indent=2))
    for file in [f'{scope}-overview.png',f'{scope}-preview.gif']:z.write(root/file,file)
    for p in (R/'assets/templates/pet-animation-48').rglob('*'):
        if p.is_file():z.write(p,'template/'+p.relative_to(R/'assets/templates/pet-animation-48').as_posix())
print(R/f'dist/{scope}-animation-templates.zip')
