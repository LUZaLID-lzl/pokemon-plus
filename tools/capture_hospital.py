"""Capture hospital states from the actual shared LVGL view."""
from pathlib import Path
import subprocess,tempfile,os
from PIL import Image
root=Path(__file__).resolve().parents[1];out=root/'docs/screenshots'
cmd=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--intro-seen']
with tempfile.TemporaryDirectory() as tmp:
 for phase in ['ready','play','win','miss']:
  target=Path(tmp)/(phase+'.ppm')
  subprocess.run(cmd+['--hospital',phase,'--screenshot',str(target)],check=True,env={**os.environ,'SDL_VIDEODRIVER':'dummy'})
  Image.open(target).save(out/f'hospital-{phase}.png')
 for name,args in [('page-1',['--demo','--page','1']),('legend-apps',['--legend','apps'])]:
  target=Path(tmp)/(name+'.ppm');subprocess.run(cmd+args+['--screenshot',str(target)],check=True,env={**os.environ,'SDL_VIDEODRIVER':'dummy'})
  Image.open(target).save(out/f'{name}.png')
 sheet=Image.new('RGB',(1080,360),'#091b24')
 for i,name in enumerate(['page-1','legend-apps','menu-scene-1-locked']):sheet.paste(Image.open(out/(name+'.png')),(i*360,0))
 sheet.save(out/'menu-design-preview.png')
 for name,tile in [('contact-sheet.png','page-1.png'),('legend-contact-sheet.png','legend-apps.png')]:
  p=out/name
  if p.exists():
   im=Image.open(p).convert('RGB');im.paste(Image.open(out/tile),(360,0) if name=='contact-sheet.png' else (0,0));im.save(p)
 sheet=Image.new('RGB',(1440,360),'#091b24')
 for i,phase in enumerate(['ready','play','win','miss']):sheet.paste(Image.open(out/f'hospital-{phase}.png'),(i*360,0))
 sheet.save(out/'hospital-preview.png')
print('PASS: hospital ready/play/win/miss and both APP pages captured')
