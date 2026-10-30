"""Capture the real shared renderer; do not fabricate wild combat screenshots."""
from pathlib import Path
from PIL import Image, ImageDraw
import subprocess, tempfile
root=Path(__file__).resolve().parents[1]
out=root/'docs/screenshots'
cmd=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--intro-seen']
phases=['alert','pending','battle','caught','caught-ball','success','duplicate','full','lost']
with tempfile.TemporaryDirectory(prefix='pp-wild-') as temporary:
 d=Path(temporary)
 for phase in phases:
  ppm=d/(phase+'.ppm')
  subprocess.run(cmd+['--wild',phase]+(['--at-ms','1200'] if phase=='battle' else [])+['--screenshot',str(ppm)],check=True)
  Image.open(ppm).save(out/('wild-'+phase+'.png'))
 frames=d/'frames';frames.mkdir()
 subprocess.run(cmd+['--wild','battle','--at-ms','1','--frames-dir',str(frames),'--frame-count','80','--frame-step','125'],check=True)
 images=[Image.open(p).convert('RGB') for p in sorted(frames.glob('*.ppm'))]
 assert len(images)==80 and len({im.tobytes() for im in images})>40
 images[0].save(out/'wild-battle.gif',save_all=True,append_images=images[1:],duration=[120,130]*40,loop=0,optimize=False)
 encounter=d/'encounter';encounter.mkdir()
 subprocess.run(cmd+['--wild','alert','--frames-dir',str(encounter),'--frame-count','28','--frame-step','250'],check=True)
 cue=[Image.open(p).convert('RGB') for p in sorted(encounter.glob('*.ppm'))]
 assert len(cue)==28 and cue[0].tobytes()!=cue[20].tobytes()
 cue[0].save(out/'wild-encounter.gif',save_all=True,append_images=cue[1:],duration=250,loop=0,optimize=False)
 capture=d/'capture';capture.mkdir()
 subprocess.run(cmd+['--wild','capture','--at-ms','1','--frames-dir',str(capture),'--frame-count','60','--frame-step','60'],check=True)
 caught=[Image.open(p).convert('RGB') for p in sorted(capture.glob('*.ppm'))]
 assert len(caught)==60 and len({im.tobytes() for im in caught})>25
 # Deliberately omit loop: the real catch is a single committed event.
 caught[0].save(out/'wild-capture.gif',save_all=True,append_images=caught[1:],duration=60,optimize=False)
 film=Image.new('RGB',(360*4,390),'#e8eee2');fd=ImageDraw.Draw(film)
 for i,(frame,name) in enumerate([(0,'THROW'),(11,'GATHER'),(26,'SHAKE'),(39,'CAUGHT')]):
  film.paste(caught[frame],(360*i,0));fd.text((360*i+14,365),name,fill='#244556')
 film.save(out/'wild-capture-sequence.png')
 sheet=Image.new('RGB',(1080,((len(phases)+2)//3)*390),'#e8eee2');draw=ImageDraw.Draw(sheet)
 for i,phase in enumerate(phases):
  x=i%3*360;y=i//3*390;sheet.paste(Image.open(out/('wild-'+phase+'.png')),(x,y));draw.text((x+14,y+365),phase,fill='#244556')
 sheet.save(out/'wild-contact-sheet.png')
print('PASS: nine wild shared LVGL views, battle/encounter GIFs, single-play capture timeline, success receipt and contact sheets')
