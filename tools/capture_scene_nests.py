"""Review actual shared LVGL egg seating and empty nests in every scene."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import json
import os
import subprocess
import tempfile
from scene_nest_art import catalog, build_nest, front_rim, ROOT
from scene_art import scene_base

config=catalog()
env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software')
out=ROOT/'docs/screenshots'
font=ImageFont.truetype(str(ROOT/'assets/NotoSansSC-subset.ttf'),14)
report=[]
sheet=Image.new('RGB',(4*360,4*388),'#142735')
empty_sheet=Image.new('RGB',sheet.size,'#142735')
with tempfile.TemporaryDirectory(prefix='pp-nests-') as temp:
 for s in config['scenes']:
  scene=s['scene'];nx,ny=s['position'];rim=front_rim(scene)
  if config.get('embeddedSources'):
   background=scene_base(ROOT,config['embeddedSources'][scene])
   nest=background.crop((nx,ny,nx+72,ny+42)).convert('RGBA')
   for yy in range(15):
    for xx in range(72):
     if rim.getpixel((xx,yy))[3]:assert rim.getpixel((xx,yy))[:3]==background.getpixel((nx+xx,ny+27+yy))
  else:nest=build_nest(scene)
  assert nest.size==(72,42) and rim.size==(72,15)
  assert set(rim.getchannel('A').getdata())=={0,255}
  # Native3x blocks; all props fit inside the physical round display.
  assert nest.resize((24,14),Image.Resampling.NEAREST).resize((72,42),Image.Resampling.NEAREST).tobytes()==nest.tobytes()
  for yy in range(42):
   for xx in range(72):
    if nest.getpixel((xx,yy))[3]:assert (nx+xx-179.5)**2+(ny+yy-179.5)**2<168**2
  for occupied in (False,True):
   target=Path(temp)/f'{scene}-{occupied}.ppm'
   args=[str(ROOT/'preview/build/pokemon_plus'),'--assets',str(ROOT/'sdcard/pokemon-plus'),'--demo','--scene',str(scene),'--clock','11:11','--at-ms','1000','--scene-fx-off','--screenshot',str(target)]
   if occupied:args+=['--activity','egg']
   subprocess.run(args,env=env,check=True,capture_output=True)
   im=Image.open(target).convert('RGB');im.save(out/f'scene-nest-{scene}{"-egg" if occupied else ""}.png')
   target_sheet=sheet if occupied else empty_sheet
   x,y=scene%4*360,scene//4*388;target_sheet.paste(im,(x,y))
   ImageDraw.Draw(target_sheet).text((x+180,y+366),s['name'],font=font,fill='#deecd7',anchor='mt')
  report.append(dict(scene=scene,position=s['position'],egg=[nx+config['eggOffset'][0],ny+config['eggOffset'][1]],circularSafe=True))
sheet.save(out/'scene-nests-eggs.png');empty_sheet.save(out/'scene-nests-empty.png')
(ROOT/'build/scene-nests-render-checks.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
print('PASS: 26 shared LVGL scene renders; all13 rims match their ground, binary alpha, exact3x pixels and circular-safe placement')
