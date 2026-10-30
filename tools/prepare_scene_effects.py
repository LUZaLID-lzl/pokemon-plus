"""Generate recipes and small material anchors from the existing pixel paintings."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter
from scene_materials import selected
from sacred_lake_art import build_front
import json
ROOT=Path(__file__).resolve().parents[1]
configs=json.loads((ROOT/'assets/scene-effects.json').read_text())
motion=json.loads((ROOT/'web/scene-motion.json').read_text())['scenes']
materials=json.loads((ROOT/'assets/scene-motion.json').read_text())['scenes']
def safe(x,y,scene):
 # Keep rain/snow ground ambience unchanged; other overlays sit behind the UI.
 return all((px-180)**2+(py-180)**2<169**2 and (scene not in (5,10) or (not(74<=px<286 and 16<=py<118) and not(92<=px<268 and 252<=py<334))) for px in (x-4,x+4) for py in (y-4,y+4))
anchors=[]
for scene in range(len(configs)):
 im=Image.open(ROOT/f'sdcard/pokemon-plus/scenes/{scene}.png').convert('RGB').resize((120,120),Image.Resampling.NEAREST)
 mask=Image.new('1',(120,120))
 kind={2:'water',5:'water',6:'crystal',7:'glow',11:'cloud',12:'water'}.get(scene)
 if scene==11:
  for y in range(120):
   for x in range(120):
    r,g,b=im.getpixel((x,y))
    if min(r,g,b)>100 and b>r*.92 and g>r*.90:mask.putpixel((x,y),1)
 elif scene in (2,6,7,12):
  # Read original material polygons before the scenery shader's HUD exclusions;
  # otherwise sea highlights would still be trapped in a narrow band below it.
  for feature in materials[scene]:
   if feature['kind']!=kind:continue
   region=Image.new('L',(120,120));ImageDraw.Draw(region).polygon(feature['polygon'],fill=255)
   for y in range(120):
    for x in range(120):
     if not selected(feature['gate'],*im.getpixel((x,y))):region.putpixel((x,y),0)
   if kind=='water':region=region.filter(ImageFilter.MaxFilter(3)).filter(ImageFilter.MinFilter(3)).filter(ImageFilter.MinFilter(3))
   for y in range(120):
    for x in range(120):
     if region.getpixel((x,y)):mask.putpixel((x,y),1)
 elif kind:
  for f in motion[scene]:
   if f['kind']==kind:
    for y,x,n in f['runs']:
     for xx in range(x,x+n):mask.putpixel((xx,y),1)
 elif scene==8:
  for y in range(46):
   for x in range(120):
    r,g,b=im.getpixel((x,y))
    if b>g*1.15 and b>r*1.1:mask.putpixel((x,y),1)
 elif scene in (9,10):
  polygon=([(37,41),(74,48),(85,68),(78,80),(102,116),(39,119),(10,81),(26,52)] if scene==9 else [(30,49),(85,44),(104,119),(14,119)])
  ImageDraw.Draw(mask).polygon(polygon,fill=1)
  if scene==9:
   for y in range(120):
    for x in range(120):
     r,g,b=im.getpixel((x,y))
     if not(r>g and g>b*1.15):mask.putpixel((x,y),0)
 if scene==12:
  alpha=build_front(im).getchannel('A')
  for y in range(120):
   for x in range(120):
    if alpha.getpixel((x,y)):mask.putpixel((x,y),0)
 points=[]
 for y in range(2,118):
  for x in range(2,118):
   if mask.getpixel((x,y)) and safe(x*3+1,y*3+1,scene):
    # A6px glyph fits on coherent material, preserving cliff/shore silhouettes.
    if all(mask.getpixel((xx,yy)) for xx in (x,x+1) for yy in (y,y+1)):
     if scene==2 and not all(mask.getpixel((xx,yy)) for xx in range(x,min(x+4,120)) for yy in (y,y+1)):continue
     if scene==12 and not all(mask.getpixel((xx,yy)) for xx in (x-6,x+6) for yy in (y-2,y+2) if 0<=xx<120 and 0<=yy<120):continue
     points.append([x*3+1,y*3+1])
 if points:
  # Deterministic stratification retains all sides without adding large images.
  points=[points[i*len(points)//min(len(points),96)] for i in range(min(len(points),96))]
 anchors.append(points)
configs_out=[]
for c,a in zip(configs,anchors):configs_out.append({**c,'anchors':a})
(ROOT/'web/scene-effects.json').write_text(json.dumps(configs_out,ensure_ascii=False,separators=(',',':'))+'\n')
h=['#pragma once','/* Generated from scene recipes and actual painted materials. */',f'#define PP_SCENE_RECIPE_COUNT {len(configs)}',f'static const pp_scene_fx_config pp_scene_fx_configs[{len(configs)}]={{']
h += [' {%u,%u,%u,%u,0x%s,%u},'%(c['kind'],c['chance'],c['durationMs'],c['count'],c['color'],c['speed']) for c in configs]
h += ['};','typedef struct {int x,y;} pp_scene_anchor;']
for i,points in enumerate(anchors):
 h+=['static const pp_scene_anchor pp_scene_anchors_%u[]={%s};'%(i,','.join('{%d,%d}'%tuple(p) for p in points) if points else '{0,0}')]
h+=[f'static const pp_scene_anchor *const pp_scene_anchors[{len(configs)}]={{'+','.join('pp_scene_anchors_%d'%i for i in range(len(configs)))+'};',f'static const unsigned pp_scene_anchor_counts[{len(configs)}]={{'+','.join(str(len(p)) for p in anchors)+'};']
(ROOT/'core/scene_fx_config.h').write_text('\n'.join(h)+'\n')
print('Actual material anchors:',[len(p) for p in anchors])
