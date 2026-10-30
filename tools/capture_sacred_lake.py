"""Export one complete24-second shared-LVGL patrol behind/in front of the tree."""
from pathlib import Path
from PIL import Image
import os, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='pp-sacred-lake-') as temp:
    subprocess.run([str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo','--ambient','--scene','12','--clock','11:11','--at-ms','100','--frames-dir',temp,'--frame-count','120','--frame-step','200'],env={**os.environ,'SDL_VIDEODRIVER':'dummy','SDL_RENDER_DRIVER':'software'},check=True,capture_output=True)
    frames=[Image.open(p).convert('RGB') for p in sorted(Path(temp).glob('*.ppm'))]
    assert len(frames)==120
    out=root/'docs/screenshots'
    frames[0].save(out/'sacred-lake.gif',save_all=True,append_images=frames[1:],duration=200,loop=0)
    frames[10].save(out/'sacred-lake.png')
print('PASS:24s /120 real LVGL frames; full back/front loop and lake effects exported')
