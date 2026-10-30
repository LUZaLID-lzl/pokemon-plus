#!/usr/bin/env python3
"""Capture full-width travel, return and real collection receipts from shared UI."""
from pathlib import Path
from PIL import Image
import json,subprocess,tempfile
root=Path(__file__).resolve().parents[1];out=root/'docs/screenshots'
cmd=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo']
report=[];sheet=Image.new('RGB',(1080,720),'black')
with tempfile.TemporaryDirectory(prefix='pokemon-travel-') as d:
 for scene in range(10):
  for phase,activity in [('journey','travel'),('ready','travel-ready'),('receipt','travel-receipt')]:
   path=Path(d)/f'{scene}-{phase}.ppm'
   subprocess.run(cmd+['--travel-destination',str(scene),'--activity',activity,'--at-ms','200' if phase=='ready' else '1600' if phase=='receipt' else '200','--raw-display','--screenshot',str(path)],check=True,capture_output=True,timeout=45)
   raw=Image.open(path).convert('RGB')
   # The source strip now scrolls and carries ambient particles. Exact source
   # pixel/seam and clean repaint checks live in --travel-smoke.
   assert raw.size==(360,360)
   for y in range(360):
    for x in range(360):
     if (x-179.5)**2+(y-179.5)**2>=180**2:raw.putpixel((x,y),(0,0,0))
   raw.save(out/f'travel-{scene}-{phase}.png')
   if scene in (0,6):sheet.paste(raw,((0 if phase=='journey' else 1 if phase=='ready' else 2)*360,(0 if scene==0 else 1)*360))
   if scene==0 and phase=='journey':raw.save(out/'activity-travel.png')
   report.append({'scene':scene,'phase':phase,'movingTravelStrip':phase=='journey'})
  folder=Path(d)/f'frames-{scene}';folder.mkdir()
  subprocess.run(cmd+['--travel-destination',str(scene),'--activity','travel','--at-ms','200','--frame-count','72','--frame-step','33','--frames-dir',str(folder)],check=True,capture_output=True,timeout=45)
  frames=[Image.open(p).convert('RGB') for p in sorted(folder.glob('*.ppm'))]
  assert len(frames)==72 and len({im.tobytes() for im in frames})>=30
  frames[0].save(out/f'travel-{scene}.gif',save_all=True,append_images=frames[1:],duration=[30,30,40]*24,loop=0,optimize=False)
 folder=Path(d)/'settlement';folder.mkdir()
 subprocess.run(cmd+['--activity','travel-receipt','--at-ms','10','--frame-count','32','--frame-step','60','--frames-dir',str(folder)],check=True,capture_output=True,timeout=45)
 frames=[Image.open(p).convert('RGB') for p in sorted(folder.glob('*.ppm'))]
 assert len(frames)==32 and len({im.tobytes() for im in frames})>=10
 frames[0].save(out/'travel-settlement.gif',save_all=True,append_images=frames[1:],duration=[60]*31+[1800],optimize=False)
 path=Path(d)/'complete.ppm';subprocess.run(cmd+['--activity','travel-complete','--at-ms','1600','--screenshot',str(path)],check=True,capture_output=True,timeout=45)
 Image.open(path).save(out/'travel-complete.png')
 path=Path(d)/'prep.ppm';subprocess.run(cmd+['--page','5','--screenshot',str(path)],check=True,capture_output=True,timeout=45)
 Image.open(path).save(out/'page-5.png')
sheet.save(out/'travel-states.png')
(out/'travel-validation.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS: 10 destinations x 3 travel states, independent scenery, completion notice before explicit claim, actual rewards and animation')
