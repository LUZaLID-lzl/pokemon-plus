"""Extract reviewed MobiIoT 7x7 atlases, retaining measured cell bounds and QA."""
from pathlib import Path
from PIL import Image,ImageDraw
import json,hashlib,sys
from collections import deque
ROOT=Path(__file__).resolve().parents[1]
c=json.loads((ROOT/'assets/collection-catalog.json').read_text())
out=ROOT/'assets/collection';out.mkdir(exist_ok=True)
qa=[]
for b in c['batches']:
 source=ROOT/'assets'/b['source']
 if not source.exists():continue
 im=Image.open(source).convert('RGBA');w,h=im.size
 # Measure connected silhouettes rather than cutting across generated gutters.
 pixels=list(im.getdata());mask=bytearray(1 if a>=200 and not(r>210 and g<80 and blue>210) else 0 for r,g,blue,a in pixels)
 components={}
 for start in range(w*h):
  if not mask[start]:continue
  mask[start]=0;todo=[start];points=[]
  while todo:
   n=todo.pop();points.append(n);xx,yy=n%w,n//w
   for nx,ny in ((xx-1,yy),(xx+1,yy),(xx,yy-1),(xx,yy+1)):
    if 0<=nx<w and 0<=ny<h and mask[ny*w+nx]:mask[ny*w+nx]=0;todo.append(ny*w+nx)
  if len(points)<100:continue
  pending=[points]
  while pending:
   part=pending.pop();xs=[n%w for n in part];ys=[n//w for n in part]
   split=False
   for coords,length,axis in ((ys,h,'y'),(xs,w,'x')):
    lo,hi=min(coords),max(coords)
    if hi-lo<length/7*1.38:continue
    candidates=[round(k*length/7) for k in range(1,7) if lo+length/7*.35<k*length/7<hi-length/7*.35]
    if not candidates:continue
    seam=min(candidates,key=lambda v:abs(v-(lo+hi)/2))
    projection={v:0 for v in range(max(lo,int(seam-length/7*.22)),min(hi,int(seam+length/7*.22))+1)}
    for v in coords:
     if v in projection:projection[v]+=1
    cut=min(projection,key=lambda v:(projection[v],abs(v-seam)))
    left=[n for n in part if (n//w if axis=='y' else n%w)<cut]
    right=[n for n in part if (n//w if axis=='y' else n%w)>=cut]
    if min(len(left),len(right))<100:continue
    pending.extend((left,right));split=True;break
   if split:continue
   cx=sum(n%w for n in part)/len(part);cy=sum(n//w for n in part)/len(part)
   slot=min(6,int(cy*7/h))*7+min(6,int(cx*7/w))
   if slot not in components or len(part)>len(components[slot]):components[slot]=part

 for x in [i for i in c['items'] if i.get('batch')==b['id']]:
  col,row=x['cell']%7,x['cell']//7
  bounds=(round(col*w/7),round(row*h/7),round((col+1)*w/7),round((row+1)*h/7))
  points=components.get(x['cell'])
  if not points:raise ValueError(f'Missing silhouette {x["id"]}')
  left=min(n%w for n in points);top=min(n//w for n in points)
  right=max(n%w for n in points)+1;bottom=max(n//w for n in points)+1
  bounds=(left,top,right,bottom)
  cell=Image.new('RGBA',(right-left+4,bottom-top+4))
  dst=cell.load()
  for n in points:dst[n%w-left+2,n//w-top+2]=(*pixels[n][:3],255)
  box=cell.getbbox();margin=2
  cell=cell.crop(box);cell.thumbnail((28,28),Image.Resampling.NEAREST)
  sprite=Image.new('RGBA',(32,32));sprite.alpha_composite(cell,((32-cell.width)//2,(32-cell.height)//2))
  # At most 15 opaque colors plus transparent, exactly represented in I4.
  alpha=sprite.getchannel('A');rgb=sprite.convert('RGB').quantize(colors=15,method=Image.Quantize.MEDIANCUT).convert('RGBA');rgb.putalpha(alpha)
  rgb.putdata([(r,g,b,a) if a else (0,0,0,0) for r,g,b,a in rgb.getdata()])
  rgb.save(out/f'{x["id"]:03}.png')
  qa.append(dict(id=x['id'],source=b['source'],cellBounds=bounds,artBounds=box,margin=margin,sha256=hashlib.sha256(rgb.tobytes()).hexdigest()))
 sheet=Image.new('RGB',(7*96,7*100),'#10202c');draw=ImageDraw.Draw(sheet)
 for x in [i for i in c['items'] if i.get('batch')==b['id']]:
  col,row=x['cell']%7,x['cell']//7;sprite=Image.open(out/f'{x["id"]:03}.png').convert('RGBA').resize((64,64),Image.Resampling.NEAREST)
  sheet.paste(sprite,(col*96+16,row*100+8),sprite)
  color='#'+c['qualities'][x['quality']]['color'];draw.text((col*96+12,row*100+77),f'{x["id"]:03} / Q{x["quality"]}',fill=color)
 sheet.save(out/f'qa-{b["id"]:02}.png')
assert len({x['sha256'] for x in qa})==len(qa),'Duplicate icon sprites'
(ROOT/'assets/collection-qa.json').write_text(json.dumps(dict(exported=len(qa),unique=len(qa),icons=qa),indent=2)+'\n')
print('Exported unique collection icons:',len(qa))
if '--require-all' in sys.argv:assert len(qa)==583
