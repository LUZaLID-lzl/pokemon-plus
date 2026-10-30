from pathlib import Path
import subprocess,tempfile,os
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
out=ROOT/'docs/screenshots';env={**os.environ,'SDL_VIDEODRIVER':'dummy'}
cmd=[str(ROOT/'preview/build/pokemon_plus'),'--assets',str(ROOT/'sdcard/pokemon-plus'),'--intro-seen','--training-party','3']
with tempfile.TemporaryDirectory() as temp:
 for phase in ['scene','team','search','alert','fight','result','complete','complete-all','complete-empty']:
  target=Path(temp)/(phase+'.ppm');subprocess.run(cmd+['--training',phase,'--screenshot',str(target)]+(['--at-ms','1200'] if phase=='fight' else []),env=env,check=True);Image.open(target).save(out/f'training-{phase}.png')
 for i in range(6):
  target=Path(temp)/(str(i)+'.ppm');subprocess.run(cmd+['--training','scene','--training-scene',str(i),'--screenshot',str(target)],env=env,check=True);Image.open(target).save(out/f'training-scene-{i}.png')
 sheet=Image.new('RGB',(1080,720),'#112b35')
 for i in range(6):sheet.paste(Image.open(out/f'training-scene-{i}.png'),(i%3*360,i//3*360))
 sheet.save(out/'training-scenes.png')
 sheet=Image.new('RGB',(1440,360),'#112b35')
 for i,phase in enumerate(['scene','team','search','fight']):sheet.paste(Image.open(out/f'training-{phase}.png'),(i*360,0))
 sheet.save(out/'training-preview.png')
 clips=[]
 for i in range(6):
  folder=Path(temp)/f'frames-{i}';folder.mkdir()
  subprocess.run(cmd+['--training','search','--training-scene',str(i),'--frames-dir',str(folder),'--frame-count','85','--frame-step','33','--screenshot',str(Path(temp)/'animation.ppm')],env=env,check=True)
  clips.append([Image.open(p).convert('RGB') for p in sorted(folder.glob('*.ppm')) if p.stem.isdigit()])
 clips[0][0].save(out/'training-search.gif',save_all=True,append_images=clips[0][1:],duration=[30 if i%3!=2 else 40 for i in range(len(clips[0]))],loop=0)
 frames=[]
 for frame in range(85):
  canvas=Image.new('RGB',(1080,720),'#112b35')
  for i in range(6):canvas.paste(clips[i][frame],(i%3*360,i//3*360))
  frames.append(canvas)
 frames[0].save(out/'training-ambient.gif',save_all=True,append_images=frames[1:],duration=[30 if i%3!=2 else 40 for i in range(len(frames))],loop=0)
 folder=Path(temp)/'encounter';folder.mkdir()
 subprocess.run(cmd+['--training','search','--frames-dir',str(folder),'--frame-count','667','--frame-step','33','--screenshot',str(Path(temp)/'encounter.ppm')],env=env,check=True)
 frames=[Image.open(p).convert('RGB') for p in sorted(folder.glob('*.ppm')) if p.stem.isdigit()]
 frames[0].save(out/'training-encounter.gif',save_all=True,append_images=frames[1:],duration=[30 if i%3!=2 else 40 for i in range(len(frames))],loop=0)
 for name,args in [('page-1',['--demo','--page','1']),('legend-apps',['--legend','apps'])]:
  target=Path(temp)/(name+'.ppm');subprocess.run(cmd+args+['--screenshot',str(target)],env=env,check=True);Image.open(target).save(out/(name+'.png'))
print('PASS: six training maps, team/search/fight/result and updated APP pages captured')
