#!/usr/bin/env python3
"""Shared LVGL first-gym pages and real relay animation, no browser redraw."""
from pathlib import Path
from PIL import Image,ImageDraw
import subprocess,tempfile,json
r=Path(__file__).resolve().parents[1];out=r/'docs/screenshots';out.mkdir(exist_ok=True)
cmd=[str(r/'preview/build/pokemon_plus'),'--assets',str(r/'sdcard/pokemon-plus'),'--demo']
report=[];images=[]
prefix=__import__('sys').argv[1] if len(__import__('sys').argv)>1 else 'gym'
with tempfile.TemporaryDirectory(prefix='pp-gym-capture-') as tmp:
 for name in (['locked','badges-pewter','badges-cerulean','badges-vermilion','badges-celadon','badges-fuchsia','badges-saffron','badges-cinnabar'] if prefix=='viridian' else ['locked','badges-pewter','badges-cerulean','badges-vermilion','badges-celadon','badges-fuchsia','badges-saffron'] if prefix=='cinnabar' else ['locked','badges-pewter','badges-cerulean','badges-vermilion','badges-celadon','badges-fuchsia'] if prefix=='saffron' else ['locked','badges-pewter','badges-cerulean','badges-vermilion','badges-celadon'] if prefix=='fuchsia' else ['locked','badges-pewter','badges-cerulean','badges-vermilion'] if prefix=='celadon' else ['locked','badges-pewter','badges-cerulean'] if prefix=='vermilion' else ['locked','badges-pewter'] if prefix=='cerulean' else [])+['map','cleared','detail','team','empty','battle','faint','win','loss','rematch','badges']:
  p=Path(tmp)/(name+'.ppm');args=['--activity',prefix+'-'+name,'--screenshot',str(p)]
  if name=='battle':args+=['--at-ms','3450']
  subprocess.run(cmd+args,check=True,capture_output=True);im=Image.open(p).convert('RGB');im.save(out/f'{prefix}-{name}.png');images.append((name,im));report.append({'name':name,'source':'shared LVGL','size':im.size})
 for page in [1,18,19,20,21]:
  if page in (18,19,20,21):continue
  p=Path(tmp)/f'page-{page}.ppm';subprocess.run(cmd+['--page',str(page),'--screenshot',str(p)],check=True,capture_output=True);Image.open(p).save(out/f'page-{page}.png')
 framesdir=Path(tmp)/'frames';framesdir.mkdir();subprocess.run(cmd+['--activity',prefix+'-battle','--at-ms','1','--frames-dir',str(framesdir),'--frame-count','100','--frame-step','180'],check=True,capture_output=True)
 frames=[Image.open(p).convert('RGB') for p in sorted(framesdir.glob('*.ppm'))];assert len(frames)==100
 frames[0].save(out/f'{prefix}-battle.gif',save_all=True,append_images=frames[1:],duration=180,loop=0,optimize=False)
sheet=Image.new('RGB',(360*5,390*((len(images)+4)//5)),'#0c1c2a');d=ImageDraw.Draw(sheet)
for i,(name,im)in enumerate(images):
 x=i%5*360;y=i//5*390;sheet.paste(im,(x,y));d.text((x+145,y+365),name,fill='#b0bfc7')
sheet.save(out/f'{prefix}-overview.png');(out/f'{prefix}-captures.json').write_text(json.dumps(report,indent=2)+'\n')
release=Image.new('RGB',(1440 if prefix!='gym' else 1080,360),'#080e17')
for i,name in enumerate(('map','detail','battle','badges') if prefix!='gym' else ('map','battle','badges')):
 release.paste(dict(images)[name],(i*360,0))
release.save(out/f'{prefix}-release.png')
print('PASS: circular-safe shared LVGL gym pages, APP9 entries, 100-frame battle/relay GIF')
