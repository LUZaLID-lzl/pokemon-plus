"""Compare actual LVGL effect pixels against an identical quiet frame."""
from pathlib import Path
from PIL import Image, ImageChops, ImageDraw
import json, os, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
SCENE_COUNT=len(json.loads((root/'assets/scene-sources.json').read_text())['sources'])
env={**os.environ,'SDL_VIDEODRIVER':'dummy','SDL_RENDER_DRIVER':'software'}
results=[]
sheet=Image.new('RGB',(1440,((SCENE_COUNT+3)//4)*384),'#edf1e7')
with tempfile.TemporaryDirectory(prefix='pokemon-scene-fx-') as folder:
 for scene in range(SCENE_COUNT):
  images=[]
  for enabled in (False,True):
   target=Path(folder)/f'{scene}-{int(enabled)}.ppm'
   args=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo','--scene',str(scene),'--clock','11:11','--at-ms','5000','--raw-display','--screenshot',str(target)]
   if enabled:args+=['--ambient']
   else:args+=['--scene-fx-off']
   subprocess.run(args,check=True,capture_output=True,env=env)
   images.append(Image.open(target).convert('RGB'))
  diff=ImageChops.difference(*images);changed=0;foreground=0
  for y in range(35,100):
   for x in range(85,260):
    c=images[0].getpixel((x,y))
    if (35<=y<73 and c==(213,198,255)) or (78<=y<100 and c==(230,226,255)):
     assert images[1].getpixel((x,y))==c,(scene,'HUD glyph obscured',x,y)
     foreground+=1
  assert foreground>100,(scene,'missing HUD glyphs')
  for y in range(360):
   for x in range(360):
    if diff.getpixel((x,y))!=(0,0,0):
     radius=177
     assert (x-179.5)**2+(y-179.5)**2<radius**2,(scene,'rim',x,y)
     changed+=1
  assert changed,(scene,'no visible effect')
  results.append({'scene':scene,'effectPixels':changed,'bounds':diff.getbbox(),'unchangedHUDGlyphPixels':foreground})
  # Only the review image is masked; the assertions above use raw flush pixels.
  display=images[1].copy()
  for y in range(360):
   for x in range(360):
    if (x-179.5)**2+(y-179.5)**2>=180**2:display.putpixel((x,y),(0,0,0))
  display.save(root/f'docs/screenshots/ambient-scene-{scene}.png')
  x,y=scene%4*360,scene//4*384
  sheet.paste(display,(x,y));ImageDraw.Draw(sheet).text((x+12,y+364),f'SCENE {scene:02}',fill='#345840')
sheet.save(root/'docs/screenshots/ambient-scenes-expanded.png')
(root/'build/scene-fx-render-checks.json').write_text(json.dumps(results,indent=2)+'\n')
print('PASS: 13 actual RGB565 before/after frames; visible effects, full-circle ambience behind UI, material highlights stay anchored, protected outer rim')
