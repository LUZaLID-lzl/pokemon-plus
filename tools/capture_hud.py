#!/usr/bin/env python3
"""Capture the enlarged clock, BAG battery home and settings from shared LVGL."""
from pathlib import Path
from PIL import Image, ImageDraw
import json, os, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
out=root/'docs/screenshots'
env={**os.environ,'SDL_VIDEODRIVER':'dummy','SDL_RENDER_DRIVER':'software'}
cases=[('page-0',[]),('home-clock-0000',['--clock','00:00']),('home-clock-1111',['--clock','11:11']),('home-clock-2359',['--clock','23:59']),('home-clock-unknown',['--clock','--:--']),('page-11',['--page','11'])]
manifest=json.loads((root/'sdcard/pokemon-plus/manifest.json').read_text())
background=Image.open(root/'sdcard/pokemon-plus'/manifest['scenes'][0]['image']).convert('RGB')
with tempfile.TemporaryDirectory(prefix='pokemon-clock-') as temp:
 for name,args in cases:
  ppm=Path(temp)/(name+'.ppm')
  subprocess.run([str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo',*args,'--screenshot',str(ppm)],env=env,check=True,timeout=20,capture_output=True)
  im=Image.open(ppm).convert('RGB');im.save(out/(name+'.png'))
  if name!='page-11':
   for y in range(10,30):
    for x in range(165,195):
     assert max(abs(a-b) for a,b in zip(im.getpixel((x,y)),background.getpixel((x,y))))<=7,(name,x,y)
sheet=Image.new('RGB',(1080,772),'#000000');draw=ImageDraw.Draw(sheet)
for i,(name,_) in enumerate(cases):
 x,y=i%3*360,i//3*386
 sheet.paste(Image.open(out/(name+'.png')),(x,y));draw.text((x+12,y+365),name,fill='#c1b3ed')
sheet.save(out/'home-hud.png')
print('PASS: 38px clock variants/settings captured; former battery area contains only original background')
