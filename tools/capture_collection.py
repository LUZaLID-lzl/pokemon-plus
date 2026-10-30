"""Capture actual shared LVGL collection pages, quality filters and item detail."""
from pathlib import Path
from PIL import Image,ImageDraw
import subprocess,tempfile
root=Path(__file__).resolve().parents[1];out=root/'docs/screenshots'
command=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo']
def capture(name,args):
 with tempfile.TemporaryDirectory() as d:
  p=Path(d)/'frame.ppm';subprocess.run(command+args+['--screenshot',str(p)],check=True,capture_output=True)
  image=Image.open(p).convert('RGB');image.save(out/f'{name}.png');return image
# Refresh every shared static page because the collection header/version changed.
for page in range(16):capture(f'page-{page}',['--page',str(page)])
images=[]
for q in [6,0,1,2,3,4,5]:images.append(capture(f'collection-quality-{q}',['--activity','collection-demo','--collection-quality',str(q)]))
images.append(capture('collection-detail',['--activity','collection-detail']))
images.append(capture('collection-heart',['--activity','heart-detail']))
sheet=Image.new('RGB',(1080,1080),'#080e17')
for i,image in enumerate(images):sheet.paste(image,(i%3*360,i//3*360))
sheet.save(out/'collection-ui.png')
# Five quality rows from actual generated sprites, with native integer 2x scale.
import json
catalog=json.loads((root/'assets/collection-catalog.json').read_text());gallery=Image.new('RGB',(672,380),'#10202c');draw=ImageDraw.Draw(gallery)
for q in range(5):
 items=[x for x in catalog['items'] if x['quality']==q]
 # High qualities show all, lower qualities show six distinct themes.
 picks=items if q==4 else [items[round(i*(len(items)-1)/5)] for i in range(6)]
 draw.text((12,q*76+30),['COMMON','FINE','RARE','EPIC','LEGEND'][q],fill='#'+catalog['qualities'][q]['color'])
 for i,x in enumerate(picks[:6]):
  im=Image.open(root/'assets/collection'/f'{x["id"]:03}.png').resize((64,64),Image.Resampling.NEAREST);gallery.paste(im,(96+i*94,q*76+6),im)
gallery.save(out/'collection-art.png')
print(out/'collection-ui.png')
