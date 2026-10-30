"""Review the real LVGL starry-hill comets and maple leaf silhouettes."""
from pathlib import Path
from PIL import Image,ImageChops,ImageDraw,ImageFont
import subprocess,tempfile,json,os
root=Path(__file__).resolve().parents[1];env={**os.environ,'SDL_VIDEODRIVER':'dummy','SDL_RENDER_DRIVER':'software'}
command=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo','--clock','11:11','--raw-display','--at-ms','5000','--frame-count','48','--frame-step','200']
font=ImageFont.truetype(str(root/'assets/NotoSansSC-subset.ttf'),18);review=[];report=[]
with tempfile.TemporaryDirectory(prefix='pp-comet-maple-') as temp:
 for scene,name in [(8,'星丘流星'),(4,'枫叶轻旋')]:
  sequences=[]
  for enabled in (False,True):
   folder=Path(temp)/f'{scene}-{enabled}';folder.mkdir()
   subprocess.run(command+['--scene',str(scene),'--frames-dir',str(folder)]+(['--ambient'] if enabled else ['--scene-fx-off']),env=env,check=True,capture_output=True)
   sequences.append([Image.open(p).convert('RGB') for p in sorted(folder.glob('*.ppm'))])
  foreground=0;changed=0;active=0
  for quiet,effect in zip(*sequences):
   diff=ImageChops.difference(quiet,effect);active+=diff.getbbox() is not None
   for y in range(360):
    for x in range(360):
     c=quiet.getpixel((x,y))
     if (35<=y<73 and 85<=x<260 and c==(213,198,255)) or (78<=y<100 and 85<=x<260 and c==(230,226,255)):
      assert effect.getpixel((x,y))==c;foreground+=1
     if diff.getpixel((x,y))!=(0,0,0):
      assert (x-179.5)**2+(y-179.5)**2<177**2;changed+=1
  assert changed>1000 and foreground>100 and active>20,(scene,changed,active)
  report.append({'scene':scene,'frames':48,'activeFrames':active,'changedPixels':changed,'unchangedHUDPixels':foreground})
  for im in sequences[1]:
   for y in range(360):
    for x in range(360):
     if (x-179.5)**2+(y-179.5)**2>=180**2:im.putpixel((x,y),(15,28,35))
  review.append((name,sequences[1]))
 frames=[]
 for i in range(48):
  im=Image.new('RGB',(720,394),'#0f1c23');draw=ImageDraw.Draw(im)
  for j,(name,sequence) in enumerate(review):
   im.paste(sequence[i],(j*360,0));draw.text((j*360+180,368),name,font=font,fill='#e0ebef',anchor='mt')
  frames.append(im)
 frames[0].save(root/'docs/screenshots/comet-maple.gif',save_all=True,append_images=frames[1:],duration=200,loop=0)
 frames[8].save(root/'docs/screenshots/comet-maple.png')
 sheet=Image.new('RGB',(1440,788))
 for j,i in enumerate((0,12,24,36)):sheet.paste(frames[i],(j%2*720,j//2*394))
 sheet.save(root/'build/comet-maple-contact.png')
(root/'build/comet-maple-render-checks.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS:96 actual RGB565 frames; continuous visible maple motion/intermittent background comets, HUD glyphs and outer rim protected')
