"""Check real RGB565 rain/snow coverage and export the shared native renderer."""
from pathlib import Path
from PIL import Image, ImageChops, ImageDraw, ImageFont
import json, os, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
env={**os.environ,'SDL_VIDEODRIVER':'dummy','SDL_RENDER_DRIVER':'software'}
command=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo','--clock','11:11','--raw-display','--at-ms','5000','--frame-count','40','--frame-step','200']
font=ImageFont.truetype(str(root/'assets/NotoSansSC-subset.ttf'),18)
review=[];report=[]
with tempfile.TemporaryDirectory(prefix='pp-full-weather-') as temp:
 for scene,name in [(5,'雪落小镇'),(10,'雨中村庄')]:
  sequences=[]
  for enabled in (False,True):
   folder=Path(temp)/f'{scene}-{enabled}';folder.mkdir()
   subprocess.run(command+['--scene',str(scene),'--frames-dir',str(folder)]+(['--ambient'] if enabled else ['--scene-fx-off']),env=env,check=True,capture_output=True)
   frames=[Image.open(p).convert('RGB') for p in sorted(folder.glob('*.ppm'))]
   assert len(frames)==40;sequences.append(frames)
  sectors=[[0]*3 for _ in range(3)];coverage=Image.new('L',(360,360));foreground=0;regions={'top':0,'bottom':0,'left':0,'right':0,'HUD':0,'dock':0}
  for quiet,weather in zip(*sequences):
   diff=ImageChops.difference(quiet,weather)
   for y in range(360):
    for x in range(360):
     original=quiet.getpixel((x,y))
     # Exact foreground colours after RGB565 quantization: clock and money.
     if (120<=x<240 and 35<=y<73 and original==(213,198,255)) or (130<=x<245 and 78<=y<100 and original==(230,226,255)):
      assert weather.getpixel((x,y))==original,('weather obscures HUD glyph',scene,x,y)
      foreground+=1
     if diff.getpixel((x,y))==(0,0,0):continue
     assert (x-179.5)**2+(y-179.5)**2<177**2,('rim',scene,x,y)
     sectors[y//120][x//120]+=1;coverage.putpixel((x,y),255)
     regions['top']+=y<32;regions['bottom']+=y>328
     regions['left']+=x<32;regions['right']+=x>328
     regions['HUD']+=80<=x<280 and 22<=y<112
     regions['dock']+=98<=x<262 and 258<=y<328
  assert all(v>30 for row in sectors for v in row),(scene,sectors)
  assert all(v>20 for v in regions.values()),(scene,regions)
  assert foreground>100,(scene,foreground)
  report.append({'scene':scene,'frames':40,'regions':regions,'sectors':sectors,'unchangedHUDGlyphPixels':foreground,'coverageBounds':coverage.getbbox()})
  coverage.save(root/f'docs/screenshots/weather-full-{scene}-coverage.png')
  for im in sequences[1]:
   for y in range(360):
    for x in range(360):
     if (x-179.5)**2+(y-179.5)**2>=180**2:im.putpixel((x,y),(15,28,35))
  review.append((name,sequences[1]))
 frames=[]
 for i in range(40):
  im=Image.new('RGB',(720,394),'#0f1c23');draw=ImageDraw.Draw(im)
  for j,(name,sequence) in enumerate(review):
   im.paste(sequence[i],(360*j,0));draw.text((360*j+180,368),name,font=font,fill='#e0ebef',anchor='mt')
  frames.append(im)
 frames[0].save(root/'docs/screenshots/weather-fullscreen.gif',save_all=True,append_images=frames[1:],duration=200,loop=0)
 frames[10].save(root/'docs/screenshots/weather-fullscreen.png')
(root/'build/weather-full-render-checks.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n')
print('PASS:80 actual RGB565 weather frames cover all9 sectors, top/bottom/left/right, former HUD/dock holes; HUD glyphs unchanged, outer rim protected')
