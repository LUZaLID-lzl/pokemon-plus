"""Refresh current shared battle previews after HUD changes, not historical art."""
from pathlib import Path
from PIL import Image,ImageDraw
import subprocess,tempfile,json,os
from battle_type_icons import ICONS,pixels
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'docs/screenshots';env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software')
cmd=[str(ROOT/'preview/build/pokemon_plus'),'--assets',str(ROOT/'sdcard/pokemon-plus'),'--intro-seen']
cases=[(prefix+'-battle',['--demo','--activity',prefix+'-battle'],100,180) for prefix in ['gym','cerulean','vermilion','celadon','fuchsia','saffron','cinnabar','viridian']]
cases += [(f'legend-{i}-battle',['--legend','battle','--legend-index',str(i)],64,125) for i in range(5)]
cases += [('wild-battle',['--wild','battle'],80,125),('training-fight',['--training','fight','--training-party','3'],0,0),('activity-battle',['--demo','--activity','battle'],0,0)]
report=[]
with tempfile.TemporaryDirectory(prefix='pp-types-') as temporary:
 tmp=Path(temporary)
 for name,args,count,step in cases:
  p=tmp/(name+'.ppm');subprocess.run(cmd+args+['--at-ms','1200','--screenshot',str(p)],env=env,check=True,stdout=subprocess.DEVNULL)
  Image.open(p).convert('RGB').save(out/(name+'.png'))
  if count:
   folder=tmp/name;folder.mkdir();subprocess.run(cmd+args+['--at-ms','1','--frames-dir',str(folder),'--frame-count',str(count),'--frame-step',str(step)],env=env,check=True,stdout=subprocess.DEVNULL)
   images=[Image.open(p).convert('RGB') for p in sorted(folder.glob('*.ppm'))];assert len(images)==count
   images[0].save(out/(name+'.gif'),save_all=True,append_images=images[1:],duration=step if step%10==0 else [120,130]*(count//2),loop=0,optimize=False)
  report.append({'name':name,'source':'shared LVGL','gifFrames':count,'staticAtMs':1200})
  print('Captured',name,flush=True)
# Current composite previews also contain the battle tile.
for prefix in ['gym','cerulean','vermilion','celadon','fuchsia','saffron','cinnabar','viridian']:
 for suffix in ['release','overview']:
  p=out/(prefix+'-'+suffix+'.png')
  if not p.exists():continue
  im=Image.open(p).convert('RGB')
  if suffix=='release':im.paste(Image.open(out/(prefix+'-battle.png')),(360 if prefix=='gym' else 720,0))
  else:
   names=([x['name'] for x in json.loads((out/(prefix+'-captures.json')).read_text())])
   i=names.index('battle');im.paste(Image.open(out/(prefix+'-battle.png')),(i%5*360,i//5*390))
  im.save(p)
p=out/'training-preview.png'
if p.exists():
 im=Image.open(p).convert('RGB');im.paste(Image.open(out/'training-fight.png'),(1080,0));im.save(p)
for file,xy in [('contact-sheet.png',(720,385)),('showcase.png',(360,360))]:
 p=out/file
 if p.exists():
  im=Image.open(p).convert('RGB');im.paste(Image.open(out/'activity-battle.png'),xy);im.save(p)
# Full current battle matrix for visual QA.
sheet=Image.new('RGB',(1440,4*384),'#102332');d=ImageDraw.Draw(sheet)
for i,(name,*_) in enumerate(cases):
 x=i%4*360;y=i//4*384;sheet.paste(Image.open(out/(name+'.png')),(x,y));d.text((x+14,y+364),name,fill='#d4e0d5')
sheet.save(out/'battle-types-all.png')
icons=Image.new('RGB',(6*120,3*84),'#102332');d=ImageDraw.Draw(icons)
for i,(key,label,color,mark) in enumerate(ICONS):
 im=Image.new('RGBA',(16,16));im.putdata([((p>>16)&255,(p>>8)&255,p&255,(p>>24)&255) for p in pixels(color,mark)])
 x=i%6*120;y=i//6*84;im=im.resize((48,48),Image.Resampling.NEAREST);icons.paste(im,(x+36,y+4),im);d.text((x+8,y+60),key,fill='#d4e0d5')
icons.save(out/'battle-type-icons.png')
(ROOT/'build/battle-types-captures.json').write_text(json.dumps(report,indent=2)+'\n')
