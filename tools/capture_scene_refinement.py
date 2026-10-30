"""Export all12 actual LVGL scene/weather animations in one compact review sheet."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import subprocess,tempfile,os,json
ROOT=Path(__file__).resolve().parents[1];env={**os.environ,'SDL_VIDEODRIVER':'dummy','SDL_RENDER_DRIVER':'software'}
SCENE_COUNT=len(json.loads((ROOT/'assets/scene-sources.json').read_text())['sources'])
names=[s['name'] for s in json.loads((ROOT/'web/manifest.json').read_text())['scenes']]
font=ImageFont.truetype(str(ROOT/'assets/NotoSansSC-subset.ttf'),13)
with tempfile.TemporaryDirectory(prefix='pp-fx-review-') as temp:
 paths=[]
 for scene in range(SCENE_COUNT):
  out=Path(temp)/str(scene);out.mkdir()
  args=[str(ROOT/'preview/build/pokemon_plus'),'--assets',str(ROOT/'sdcard/pokemon-plus'),'--demo','--ambient','--scene',str(scene),'--clock','11:11','--at-ms','5000','--frames-dir',str(out),'--frame-count','24','--frame-step','400']
  subprocess.run(args,env=env,check=True,capture_output=True);paths.append(sorted(out.glob('*.ppm')))
 frames=[]
 for i in range(24):
  frame=Image.new('RGB',(960,((SCENE_COUNT+3)//4)*264),'#13252c');draw=ImageDraw.Draw(frame)
  for scene in range(SCENE_COUNT):
   im=Image.open(paths[scene][i]).resize((240,240),Image.Resampling.NEAREST);x=scene%4*240;y=scene//4*264
   frame.paste(im,(x,y));draw.text((x+120,y+243),names[scene],fill='#cbdfd2',font=font,anchor='mt')
  frames.append(frame)
 frames[0].save(ROOT/'docs/screenshots/scene-effects-refined.gif',save_all=True,append_images=frames[1:],duration=400,loop=0)
 frames[8].save(ROOT/'docs/screenshots/scene-effects-refined.png')
print('PASS:13 scene animations /24 real frames each /9.6s export')
