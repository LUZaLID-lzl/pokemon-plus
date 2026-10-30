#!/usr/bin/env python3
from pathlib import Path
import json,subprocess,tempfile
from PIL import Image,ImageDraw
import argparse
parser=argparse.ArgumentParser();parser.add_argument('--start',type=int,default=1);parser.add_argument('--end',type=int,default=9);args=parser.parse_args();start,end=args.start,args.end
R=Path(__file__).resolve().parents[1];out=R/'docs/screenshots';out.mkdir(exist_ok=True)
cmd=[str(R/'preview/build/pokemon_plus'),'--assets',str(R/'sdcard/pokemon-plus'),'--demo']
roster=sorted(json.loads((R/'assets/pet-roster.json').read_text()),key=lambda p:p['nationalId'])
chains=json.loads((R/'assets/evolution.json').read_text())['chains']
count=sum(start<=p['nationalId']<=end for p in roster)
sheet=Image.new('RGB',(1080,384*((count+2)//3)),'#e8eee2');d=ImageDraw.Draw(sheet)
for i,pet in enumerate(p for p in roster if start<=p['nationalId']<=end):
    shot=out/f'form-{pet["nationalId"]:03}.ppm'
    flags=['--species',str(pet['species']),'--growth-stage',str(pet['stage'])]
    subprocess.run(cmd+flags+['--at-ms','1000','--screenshot',str(shot)],check=True)
    im=Image.open(shot).convert('RGB');im.save(shot.with_suffix('.png'));shot.unlink()
    sheet.paste(im,(i%3*360,i//3*384));d.text((i%3*360+16,i//3*384+364),f'{pet["nationalId"]:03}',fill='#345840')
    for activity in ['sleep','travel','battle']:
        target=out/f'form-{pet["nationalId"]:03}-{activity}.ppm'
        subprocess.run(cmd+flags+['--activity',activity,'--screenshot',str(target)],check=True)
        Image.open(target).save(target.with_suffix('.png'));target.unlink()
sheet.save(out/f'forms-{start:03}-{end:03}.png')
for family in sorted({p['species'] for p in roster if start<=p['nationalId']<=end}):
    chain=next((c for c in chains if c['species']==family),None)
    for stage in range(sum(p['species']==family for p in roster)-1):
        step=chain.get('steps',[{} for _ in chain['levels']])[stage]
        stone=step.get('trigger',chain.get('trigger','level')) != 'level'
        with tempfile.TemporaryDirectory(prefix='pokemon-evo-') as folder:
            subprocess.run(cmd+['--species',str(family),'--growth-stage',str(stage),'--activity','evolution-stone' if stone else 'evolution','--frames-dir',folder],check=True)
            frames=[Image.open(p).convert('RGB') for p in sorted(Path(folder).glob('*.ppm'))]
            frames[0].save(out/f'evolution-{family}-{stage}.gif',save_all=True,append_images=frames[1:],duration=[120,130]*16,loop=0,optimize=False)
            assert len(frames)==32
            if stage==0:frames[0].save(out/f'evolution-{family}.gif',save_all=True,append_images=frames[1:],duration=[120,130]*16,loop=0,optimize=False)
print(out/f'forms-{start:03}-{end:03}.png')
