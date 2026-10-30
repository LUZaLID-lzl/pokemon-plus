"""Capture the actual shared LVGL APP and every scene menu state."""
from pathlib import Path
import json,subprocess,tempfile
from PIL import Image
root=Path(__file__).resolve().parents[1]
out=root/'docs/screenshots'
cmd=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--intro-seen']
count=len(json.loads((root/'web/manifest.json').read_text())['scenes'])
with tempfile.TemporaryDirectory(prefix='pp-menu-') as temp:
 def capture(name,*args):
  ppm=Path(temp)/(name+'.ppm')
  subprocess.run(cmd+list(args)+['--screenshot',str(ppm)],check=True,stdout=subprocess.DEVNULL)
  Image.open(ppm).save(out/(name+'.png'))
 capture('page-1','--demo','--page','1')
 capture('legend-apps','--legend','apps')
 for scene in range(count):
  for state in ['active','owned','locked']:
   capture(f'menu-scene-{scene}-{state}','--scene',str(scene),'--menu-scene-state',state)
 Image.open(out/'menu-scene-0-active.png').save(out/'page-9.png')
 Image.open(out/'menu-scene-1-locked.png').save(out/'scene-selection-preview.png')
 sheet=Image.new('RGB',(1080,360),'#091b24')
 for i,name in enumerate(['page-1','legend-apps','menu-scene-1-locked']):sheet.paste(Image.open(out/(name+'.png')),(i*360,0))
 sheet.save(out/'menu-design-preview.png')
# Refresh menu tiles inside existing documentation contact sheets only.
p=out/'contact-sheet.png'
if p.exists():
 im=Image.open(p).convert('RGB')
 for page in (1,9):im.paste(Image.open(out/f'page-{page}.png'),(page%4*360,page//4*385))
 im.save(p)
p=out/'legend-contact-sheet.png'
if p.exists():
 im=Image.open(p).convert('RGB');im.paste(Image.open(out/'legend-apps.png'),(0,0));im.save(p)
p=out/'menu-background-preview.png'
if p.exists():
 im=Image.open(p).convert('RGB');im.paste(Image.open(out/'page-9.png'),(360,384));im.save(p)
print(f'PASS: APP two pages and {count} scenes × three states captured from shared LVGL')
