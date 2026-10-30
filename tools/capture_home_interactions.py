#!/usr/bin/env python3
"""Export real shared LVGL interactions, never synthetic sprite compositions."""
from pathlib import Path
from PIL import Image,ImageDraw
import subprocess,tempfile
root=Path(__file__).resolve().parents[1];out=root/'docs/screenshots'
command=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--clock','12:34']
kinds=['pat','call','explore','hop','greet','stroll','rest','welcome']
contact=Image.new('RGB',(1440,770),'#102331');draw=ImageDraw.Draw(contact)
with tempfile.TemporaryDirectory(prefix='pp-home-interact-') as temporary:
 for i,kind in enumerate(kinds):
  frames=Path(temporary)/kind;frames.mkdir()
  subprocess.run(command+['--home-interaction',kind,'--at-ms','1','--frames-dir',str(frames),'--frame-count','110','--frame-step','90'],check=True,capture_output=True)
  images=[Image.open(p).convert('RGB') for p in sorted(frames.glob('*.ppm'))]
  assert len(images)==110 and len({im.tobytes() for im in images})>20
  images[0].save(out/f'home-{kind}.gif',save_all=True,append_images=images[1:],duration=90,loop=0,optimize=False)
  frame=images[5 if kind in ('pat','hop') else 38]
  frame.save(out/f'home-{kind}.png');contact.paste(frame,(i%4*360,i//4*385))
  draw.text((i%4*360+15,i//4*385+365),kind.upper(),fill='#e8e8d4')
contact.save(out/'home-interactions.png')
print('PASS: eight real shared-renderer GIFs, representative frames and contact sheet')
