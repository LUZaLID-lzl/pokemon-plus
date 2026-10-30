"""Export a complete2.4s dot cycle from the actual shared LVGL renderer."""
from pathlib import Path
import json
import os
import subprocess
import tempfile
from PIL import Image, ImageDraw, ImageFont

root=Path(__file__).resolve().parents[1]
out=root/'docs/screenshots'
config=json.loads((root/'assets/scene-nests.json').read_text())
font=ImageFont.truetype(str(root/'assets/NotoSansSC-subset.ttf'),14)
frames=[];details=[]
with tempfile.TemporaryDirectory(prefix='pp-egg-dots-') as temp:
 for time in range(0,2400,100):
  full=Image.new('RGB',(720,388),'#142735')
  zoom=Image.new('RGB',(552,292),'#142735')
  for col,scene in enumerate((3,12)):
   target=Path(temp)/f'{scene}-{time}.ppm'
   subprocess.run([str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),
    '--demo','--activity','egg','--scene',str(scene),'--clock','11:11','--scene-fx-off',
    '--at-ms',str(time),'--screenshot',str(target)],env=dict(os.environ,SDL_VIDEODRIVER='dummy'),
    check=True,capture_output=True)
   image=Image.open(target).convert('RGB')
   full.paste(image,(col*360,0))
   x,y=config['scenes'][scene]['position'];x+=config['eggOffset'][0];y+=config['eggOffset'][1]
   zoom.paste(image.crop((x-28,y-44,x+64,y+44)).resize((276,264),Image.Resampling.NEAREST),(col*276,0))
   name=config['scenes'][scene]['name']
   ImageDraw.Draw(full).text((col*360+180,365),name,font=font,fill='#e1e6d9',anchor='mt')
   ImageDraw.Draw(zoom).text((col*276+138,270),name,font=font,fill='#e1e6d9',anchor='mt')
  frames.append(full);details.append(zoom)
for images,name in ((frames,'egg-dots.gif'),(details,'egg-dots-detail.gif')):
 images[0].save(out/name,save_all=True,append_images=images[1:],duration=100,loop=0,optimize=False)
frames[6].save(out/'egg-dots.png')
details[6].save(out/'egg-dots-detail.png')
print('PASS:24 actual LVGL frames per scene; full2.4s egg-dot cycle and nearest3x detail exported')
