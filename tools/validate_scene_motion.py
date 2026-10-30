"""Check actual LVGL RGB565 scenery frames and export visible animated examples."""
from pathlib import Path
from PIL import Image,ImageChops,ImageDraw
import os,json,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[1]
SCENE_COUNT=len(json.loads((ROOT/'assets/scene-sources.json').read_text())['sources'])
ENV={**os.environ,'SDL_VIDEODRIVER':'dummy','SDL_RENDER_DRIVER':'software'}
def capture(scene,time,target,still=False):
 cmd=[str(ROOT/'preview/build/pokemon_plus'),'--assets',str(ROOT/'sdcard/pokemon-plus'),'--demo','--scene',str(scene),'--clock','11:11','--at-ms',str(time),'--raw-display','--screenshot',str(target)]
 if still:cmd+=['--scene-still']
 subprocess.run(cmd,check=True,capture_output=True,env=ENV)
 return Image.open(target).convert('RGB')
def display(im):
 im=im.copy()
 for y in range(360):
  for x in range(360):
   if (x-179.5)**2+(y-179.5)**2>=180**2:im.putpixel((x,y),(0,0,0))
 return im
results=[]
with tempfile.TemporaryDirectory(prefix='pp-scenery-') as folder:
 folder=Path(folder)
 for scene in range(SCENE_COUNT):
  still=capture(scene,2400,folder/'still.ppm',True);moving=capture(scene,2400,folder/'motion.ppm')
  diff=ImageChops.difference(still,moving);changed=0
  for y in range(360):
   for x in range(360):
    if diff.getpixel((x,y))!=(0,0,0):
     assert (x-179.5)**2+(y-179.5)**2<173**2,(scene,'rim',x,y)
     assert not(80<=x<280 and 22<=y<112),(scene,'HUD',x,y)
     assert not(98<=x<262 and 258<=y<328),(scene,'nav',x,y)
     changed+=1
  assert changed,(scene,'no motion')
  results.append(dict(scene=scene,changedPixels=changed,bounds=diff.getbbox()))
 # Native scenery only for review: existing pets/HUD remain readable and fixed.
 samples=[]
 for t in range(120,8120,400):
  sheet=Image.new('RGB',(1080,360),'black')
  for col,scene in enumerate((0,2,6)):
   im=capture(scene,t,folder/'frame.ppm');sheet.paste(display(im),(col*360,0))
  samples.append(sheet)
 samples[0].save(ROOT/'docs/screenshots/scenery-motion.gif',save_all=True,append_images=samples[1:],duration=400,loop=0)
 samples[5].save(ROOT/'docs/screenshots/scenery-motion.png')
(ROOT/'build/scene-motion-render-checks.json').write_text(json.dumps(results,indent=2)+'\n')
print('PASS:13 actual RGB565 scenes animate, protected HUD/nav/rim unchanged;8s tree/water/crystal GIF exported')
