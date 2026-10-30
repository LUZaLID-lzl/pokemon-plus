"""Export the reviewed MobiIoT atlas as nine native, outlined 48px sprites.

Does not replace the game's four-species model or claim additional poses.
"""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import json,hashlib,zipfile
R=Path(__file__).resolve().parents[1]
SOURCE=R/'assets/source/gen1-019-027-v11.png'
OUT=R/'assets/pokemon/gen1-019-027';OUT.mkdir(parents=True,exist_ok=True)
INK=(27,38,58,255)
NAMES=['小拉达','拉达','烈雀','大嘴雀','阿柏蛇','阿柏怪','皮卡丘','雷丘','穿山鼠']
EN=['Rattata','Raticate','Spearow','Fearow','Ekans','Arbok','Pikachu','Raichu','Sandshrew']
# Measured alpha component boxes, not assumed equal cells.
BOXES=json.loads((R/'assets/source/gen1-019-027-v11-boxes.json').read_text())
def groups(points):
 points=set(points);out=[]
 while points:
  start=points.pop();q=[start];g={start}
  while q:
   x,y=q.pop()
   for n in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
    if n in points:points.remove(n);g.add(n);q.append(n)
  out.append(g)
 return out

def neighbors(p):
 x,y=p;return [(x-1,y),(x+1,y),(x,y-1),(x,y+1)]

def sprite(cell):
 # Remove detached source specks, keeping the complete visible character.
 alpha=cell.getchannel('A');pts={(x,y) for y in range(cell.height) for x in range(cell.width) if alpha.getpixel((x,y))>=160}
 main=max(groups(pts),key=len)
 mask=Image.new('L',cell.size)
 for p in main:mask.putpixel(p,255)
 cell.putalpha(mask);cell.thumbnail((40,40),Image.Resampling.NEAREST)
 cell.putalpha(cell.getchannel('A').point(lambda a:255 if a>=128 else 0))
 # 15 opaque colors plus the one common outline color. Binary alpha only.
 rgb=cell.convert('RGB').quantize(colors=15,method=Image.Quantize.MEDIANCUT).convert('RGB')
 cell=Image.merge('RGBA',(*rgb.split(),cell.getchannel('A')))
 image=Image.new('RGBA',(48,48));image.alpha_composite(cell,((48-cell.width)//2,(48-cell.height)//2))
 mask={(x,y) for y in range(48) for x in range(48) if image.getpixel((x,y))[3]}
 # Reconnect subpixel necks/antennae lost during nearest-neighbor reduction.
 components=sorted(groups(mask),key=len,reverse=True)
 while len(components)>1:
  main=components[0];other=set().union(*components[1:])
  a,b=min(((a,b) for a in main for b in other),key=lambda t:abs(t[0][0]-t[1][0])+abs(t[0][1]-t[1][1]))
  assert abs(a[0]-b[0])+abs(a[1]-b[1])<=4,'Review disconnected source features'
  x,y=a
  while (x,y)!=b:
   if x!=b[0]:x+=1 if b[0]>x else -1
   else:y+=1 if b[1]>y else -1
   mask.add((x,y));image.putpixel((x,y),INK)
  components=sorted(groups(mask),key=len,reverse=True)
 # Fill enclosed alpha pinholes, using a neighboring body color.
 exterior={(0,0)};q=[(0,0)]
 while q:
  for p in neighbors(q.pop()):
   x,y=p
   if 0<=x<48 and 0<=y<48 and p not in mask and p not in exterior:exterior.add(p);q.append(p)
 holes={(x,y) for y in range(48) for x in range(48)}-mask-exterior
 fallback=next(p for p in image.getdata() if p[3] and max(p[:3])>100)
 for p in holes:
  nearby=[image.getpixel(n) for n in neighbors(p) if n in mask]
  image.putpixel(p,nearby[0] if nearby else fallback)
 mask|=holes
 # Bridge corner-only contacts in the visible mask, then color the complete
 # silhouette boundary. No full-radius dilation or second external halo.
 for y in range(47):
  for x in range(47):
   a,b,c,d=(x,y),(x+1,y),(x,y+1),(x+1,y+1)
   if a in mask and d in mask and b not in mask and c not in mask:
    p=min((b,c),key=lambda p:(p[0]-23.5)**2+(p[1]-23.5)**2);mask.add(p);image.putpixel(p,INK)
   elif b in mask and c in mask and a not in mask and d not in mask:
    p=min((a,d),key=lambda p:(p[0]-23.5)**2+(p[1]-23.5)**2);mask.add(p);image.putpixel(p,INK)
 outline={p for p in mask if any(n not in mask for n in neighbors(p))}
 # Join diagonal boundary steps with an existing interior pixel. Outline
 # thickening is limited to these corners; the alpha silhouette stays intact.
 for unused in range(4):
  added=set()
  for y in range(47):
   for x in range(47):
    a,b,c,d=(x,y),(x+1,y),(x,y+1),(x+1,y+1)
    options=[]
    if a in outline and d in outline and b not in outline and c not in outline:options=[p for p in (b,c) if p in mask]
    if b in outline and c in outline and a not in outline and d not in outline:options=[p for p in (a,d) if p in mask]
    if options:added.add(min(options,key=lambda p:(p[0]-23.5)**2+(p[1]-23.5)**2))
  if not added:break
  outline|=added
 for p in outline:image.putpixel(p,INK)
 assert len(groups(mask))==1,'Disconnected silhouette'
 assert len(groups(outline))==1,'Disconnected outline'
 assert all(image.getpixel(p)==INK for p in mask if any(n not in mask for n in neighbors(p)))
 assert set(image.getchannel('A').getdata())=={0,255}
 assert len({p for p in image.getdata() if p[3]})<=16
 return image,len(outline)

assert len(BOXES)==9,'Expected nine reviewed source boxes'
original=Image.open(SOURCE).convert('RGBA');items=[]
assert all(0<=b[0]<b[2]<=original.width and 0<=b[1]<b[3]<=original.height for b in BOXES)
sheet=Image.new('RGB',(720,720),'#e1e9df');draw=ImageDraw.Draw(sheet)
font=ImageFont.truetype('/System/Library/Fonts/STHeiti Medium.ttc',16)
for i,box in enumerate(BOXES):
 im,outline=sprite(original.crop(box));number=f'{i+19:03}'
 assert im.getbbox() and max(im.getbbox()[2]-im.getbbox()[0],im.getbbox()[3]-im.getbbox()[1])<=40
 im.save(OUT/f'{number}.png');im.resize((384,384),Image.Resampling.NEAREST).save(OUT/f'{number}@8x.png')
 x=i%3*240;y=i//3*240
 large=im.resize((192,192),Image.Resampling.NEAREST);sheet.paste(large,(x+24,y+6),large)
 label=f'{number}  {NAMES[i]}';draw.text((x+120,y+207),label,font=font,fill='#26394c',anchor='mt')
 items.append({'id':i+19,'name':NAMES[i],'english':EN[i],'chain':6+i//2,'stage':i%2,'evolvesTo':i+20 if i%2==0 else None,'image':f'{number}.png','preview':f'{number}@8x.png','sourceBox':list(box),'outlinePixels':outline,'source':'https://wiki.52poke.com/wiki/'+NAMES[i]})
sheet.save(OUT/'preview.png')
manifest={'version':1,'kind':'single-pose art review','generator':'MobiIoT','model':__import__('os').environ['IMAGEGEN_MODEL'],'attempts':int(__import__('os').environ['IMAGEGEN_ATTEMPTS']),'sourceAtlas':'../../source/gen1-019-027-v11.png','sourceSha256':hashlib.sha256(SOURCE.read_bytes()).hexdigest(),'prompt':'../../prompts/gen1-019-027-v11.txt','nativeCanvas':48,'upscale':8,'maxColors':16,'outlineColor':'#1b263a','alpha':'binary','outline':'one connected 4-neighbor boundary; local corner bridges','processing':'MobiIoT atlas split by measured source boxes, nearest-neighbor reduction, palette quantization and deterministic closed-outline repair','gameIntegrated':False,'animationPosesPerSpecies':1,'items':items}
(OUT/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2))
(OUT/'README.md').write_text('# 全国图鉴019–027像素宠物\n\n按全国图鉴编号排列；48×48真实透明PNG，附8倍最近邻放大版。MobiIoT生成原图，经切图、限色和闭合轮廓整理。单姿态画稿，尚未接入游戏或动画。来源为全国图鉴编号与各物种特征。\n')
with zipfile.ZipFile(R/'dist/gen1-019-027-pets.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(OUT.glob('*')):z.write(p,'gen1-019-027/'+p.name)
print('PASS: nine 48x48 transparent sprites; <=16 colors; connected silhouette and closed outline; 8x nearest-neighbor exports')
print(OUT/'preview.png')
