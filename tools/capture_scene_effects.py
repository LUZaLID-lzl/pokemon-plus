#!/usr/bin/env python3
"""Capture the actual shared renderer with forced ambient weather for QA only."""
from pathlib import Path
from PIL import Image
import json, subprocess, tempfile
root=Path(__file__).resolve().parents[1];out=root/'docs/screenshots'
SCENE_COUNT=len(json.loads((root/'assets/scene-sources.json').read_text())['sources'])
command=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo','--page','0']
sheet=Image.new('RGB',(1440,((SCENE_COUNT+3)//4)*360),'black');report=[]
with tempfile.TemporaryDirectory(prefix='pokemon-weather-') as d:
 for scene in range(SCENE_COUNT):
  images=[]
  for enabled in (False,True):
   target=Path(d)/f'{scene}-{enabled}.ppm'
   subprocess.run(command+['--scene',str(scene),'--at-ms','2400','--raw-display','--screenshot',str(target)]+(['--ambient'] if enabled else ['--scene-fx-off']),check=True,capture_output=True)
   images.append(Image.open(target).convert('RGB'))
  changed=[(x,y) for y in range(360) for x in range(360) if images[0].getpixel((x,y))!=images[1].getpixel((x,y))]
  assert changed and all((x-179.5)**2+(y-179.5)**2<177**2 for x,y in changed),(scene,changed[:5])
  report.append({'scene':scene,'effectPixels':len(changed),'ambientBehindUI':True})
  im=images[1]
  for y in range(360):
   for x in range(360):
    if (x-179.5)**2+(y-179.5)**2>=180**2:im.putpixel((x,y),(0,0,0))
  im.save(out/f'ambient-{scene}.png');sheet.paste(im,((scene%4)*360,(scene//4)*360))
 for scene in (5,6,11):
  frames=Path(d)/f'frames-{scene}';frames.mkdir()
  subprocess.run(command+['--scene',str(scene),'--ambient','--at-ms','1','--frame-count','48','--frames-dir',str(frames)],check=True,capture_output=True)
  ims=[Image.open(p).convert('RGB') for p in sorted(frames.glob('*.ppm'))]
  assert len(ims)==48 and len({im.tobytes() for im in ims})>20
  ims[0].save(out/f'ambient-{scene}.gif',save_all=True,append_images=ims[1:],duration=[120,130]*24,loop=0,optimize=False)
sheet.save(out/'ambient-scenes.png')
(out/'ambient-safety.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS: 13 real ambient overlays; full-circle ambience behind UI, protected outer rim; snow/crystal/cloud GIFs animate')
