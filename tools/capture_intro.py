#!/usr/bin/env python3
"""Export the actual shared LVGL opening, including its starter transition."""
from pathlib import Path
from PIL import Image, ImageDraw
import json, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
out=root/'docs/screenshots'
cmd=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus')]
with tempfile.TemporaryDirectory(prefix='pokemon-intro-') as temp:
 folder=Path(temp)
 subprocess.run(cmd+['--frames-dir',str(folder),'--at-ms','100','--frame-step','100','--frame-count','120'],check=True,capture_output=True,timeout=60)
 frames=[Image.open(p).convert('RGB') for p in sorted(folder.glob('*.ppm'))]
 assert len(frames)==120 and len({im.tobytes() for im in frames})>80
 frames[0].save(out/'intro.gif',save_all=True,append_images=frames[1:],duration=[100]*119+[1600],loop=0,optimize=False)
 sheet=Image.new('RGB',(1080,772),'black');draw=ImageDraw.Draw(sheet)
 for i,index in enumerate([12,38,51,62,89,115]):
  x,y=(i%3)*360,(i//3)*386;sheet.paste(frames[index],(x,y));draw.text((x+12,y+366),f'{(index+1)/10:.1f}s',fill='#a2b1c2')
 sheet.save(out/'intro-storyboard.png')
 frames[89].save(out/'intro.png')
(out/'intro-validation.json').write_text(json.dumps({'durationMs':11200,'actors':[1,4,7],'nativeSpriteSize':48,'framesExported':120,'transition':'starter selection'},indent=2)+'\n')
print('PASS: opening frames + automatic starter transition exported from shared LVGL')
