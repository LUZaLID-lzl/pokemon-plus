"""Capture actual round-screen fishing states and the APP entry."""
from pathlib import Path
from PIL import Image
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'docs/screenshots'
cmd=[str(ROOT/'preview/build/pokemon_plus'),'--assets',str(ROOT/'sdcard/pokemon-plus')]
with tempfile.TemporaryDirectory(prefix='pp-fishing-') as temp:
    for phase in ('ready','wait','reel','win','miss','partner','dex','detail','dex-1','dex-2','dex-3','dex-4','dex-5','detail-last'):
        ppm=Path(temp)/(phase+'.ppm')
        subprocess.run(cmd+['--fishing',phase,'--screenshot',str(ppm)],check=True)
        Image.open(ppm).save(OUT/f'fishing-{phase}.png')
    for name,args in [('page-1',['--demo','--page','1']),('legend-apps',['--legend','apps'])]:
        ppm=Path(temp)/(name+'.ppm')
        subprocess.run(cmd+args+['--screenshot',str(ppm)],check=True)
        Image.open(ppm).save(OUT/f'{name}.png')
sheet=Image.new('RGB',(1080,360),'#091b24')
for i,name in enumerate(('fishing-ready','fishing-partner','fishing-dex-5')):
    sheet.paste(Image.open(OUT/f'{name}.png'),(i*360,0))
sheet.save(OUT/'fishing-preview.png')
print(OUT/'fishing-preview.png')

# Actual LVGL frames, with ambient water and companion poses.
with tempfile.TemporaryDirectory(prefix='pp-fishing-animation-') as temp:
    subprocess.run(cmd+['--fishing','ready','--frames-dir',temp,'--frame-count','90','--frame-step','120'],check=True)
    frames=[Image.open(p).convert('RGB') for p in sorted(Path(temp).glob('[0-9][0-9][0-9].ppm'))]
    frames[0].save(OUT/'fishing-ambient.gif',save_all=True,append_images=frames[1:],duration=120,loop=0)
