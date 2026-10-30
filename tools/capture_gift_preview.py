"""Export actual LVGL gift receipts and all five quality layouts."""
from pathlib import Path
import subprocess, tempfile
from PIL import Image
root=Path(__file__).resolve().parents[1]
cmd=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--intro-seen','--gift']
with tempfile.TemporaryDirectory(prefix='pp-gift-') as tmp:
    ppm=Path(tmp)/'gift.ppm'
    def capture(name,*args):
        subprocess.run(cmd+list(args)+['--screenshot',str(ppm)],check=True)
        im=Image.open(ppm).copy()
        im.save(root/'docs/screenshots'/name)
        return im
    capture('gift-pending.png')
    sheet=Image.new('RGB',(1800,360))
    for q in range(5):
        sheet.paste(capture(f'gift-quality-{q}.png','--gift-quality',str(q)),(q*360,0))
    sheet.save(root/'docs/screenshots/gift-quality-sheet.png')
