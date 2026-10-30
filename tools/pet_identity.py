"""Native identity reduction and closed-outline repair shared by new batches."""
from PIL import Image
INK=(27,38,58,255)
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

def sprite(cell,preserve_hues=False):
 # Remove detached source specks, keeping the complete visible character.
 alpha=cell.getchannel('A');pts={(x,y) for y in range(cell.height) for x in range(cell.width) if alpha.getpixel((x,y))>=160}
 main=max(groups(pts),key=len)
 mask=Image.new('L',cell.size)
 for p in main:mask.putpixel(p,255)
 cell.putalpha(mask);cell.thumbnail((40,40),Image.Resampling.NEAREST)
 cell.putalpha(cell.getchannel('A').point(lambda a:255 if a>=128 else 0))
 # 14 opaque colors plus the one common outline color (15 total; I4 including transparency). Binary alpha only.
 if preserve_hues:
  # Transparent fringe RGB and many near-black shades must not consume the
  # small palette at the expense of identity colors (nose/jewel/eye whites).
  rgb=cell.convert('RGB')
  for y in range(cell.height):
   for x in range(cell.width):
    c=rgb.getpixel((x,y))
    if not cell.getpixel((x,y))[3] or max(c)<105:rgb.putpixel((x,y),INK[:3])
  opaque=[rgb.getpixel((x,y)) for y in range(cell.height) for x in range(cell.width) if cell.getpixel((x,y))[3]]
  # Protect distinct hue families before allocating shade detail. Rare red
  # forehead gems and pink noses must not be averaged into a dominant body hue.
  import colorsys
  buckets={}
  for c in opaque:
   if c==INK[:3]:continue
   h,s,v=colorsys.rgb_to_hsv(*(n/255 for n in c))
   key=('neutral' if s<.18 else 'red' if h<.04 or h>=.94 else
        'orange' if h<.11 else 'gold' if h<.20 else 'green' if h<.48 else
        'blue' if h<.72 else 'purple' if h<.88 else 'pink')
   buckets.setdefault(key,[]).append(c)
  allocation={key:1 for key in buckets}
  for _ in range(13-len(allocation)):
   key=max(buckets,key=lambda key:len(buckets[key])/(allocation[key]+1)**2)
   allocation[key]+=1
  colors=[INK[:3]]
  for key,values in buckets.items():
   samples=Image.new('RGB',(len(values),1));samples.putdata(values)
   quant=samples.quantize(colors=allocation[key],method=Image.Quantize.MEDIANCUT)
   table=quant.getpalette()
   colors.extend(tuple(table[i*3:i*3+3]) for _,i in quant.getcolors())
  assert len(colors)<=14
  palette=Image.new('P',(1,1));palette.putpalette([n for c in colors for n in c]+list(INK[:3])*(256-len(colors)))
  rgb=rgb.quantize(palette=palette,dither=Image.Dither.NONE).convert('RGB')
 else:
  rgb=cell.convert('RGB').quantize(colors=14,method=Image.Quantize.MEDIANCUT).convert('RGB')
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
 assert len({p for p in image.getdata() if p[3]})<=15
 return image,len(outline)

