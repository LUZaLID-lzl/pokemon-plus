#!/usr/bin/env python3
"""Generate ten independent MobiIoT travel paintings; stop the batch on failure."""
from pathlib import Path
import json, subprocess, sys, hashlib
ROOT=Path(__file__).resolve().parents[1]
SKILL=Path('/Users/lzl/.codex/skills/mobiiot-imagegen/scripts/imagegen.py')
THEMES=[
 ('晨雾竹林','A winding earthen trail through tall jade bamboo, soft early morning mist, tiny stone lanterns at the edges, layered teal foliage and pale amber shafts of light.'),
 ('碧海沙湾','A pale sandy coastal trail above a turquoise bay, small seashells, distant white sailboats, palms framing the far left and right, gentle blue waves.'),
 ('赤岩峡谷','A warm terracotta canyon footpath, layered red sandstone mesas, natural stone arch far behind, small desert succulents, a narrow turquoise creek.'),
 ('极光雪原','A quiet blue snowfield trail, distant snowy pines and mountains, soft green aurora across a dark indigo sky, small warm cabin lights far to the left.'),
 ('萤火湿地','A mossy boardwalk through a dusk wetland, reeds and lily pads, a few fireflies at the edges, calm violet water, distant willow silhouettes.'),
 ('浮云群岛','A broad grassy path on a floating island, smaller floating islands with waterfalls in the distance, soft blue clouds below, a stone bridge on the far left.'),
 ('金色麦田','A broad dirt farm trail between golden wheat fields, distant small windmill on the far right, warm afternoon light, low green hills and a calm blue sky.'),
 ('古树遗迹','A safe mossy stone path in an ancient forest ruin, enormous old trees, broken carved stone columns and quiet stone archways, cool green canopy and soft warm light.'),
 ('熔岩山径','A safe dark basalt mountain footpath, glowing orange lava streams in the distant background only, dark red volcanic peaks, dusky plum sky, no lava on the foreground path.'),
 ('薰衣花丘','A broad grassy trail through rolling lavender flower hills, violet flower fields, distant small cream cottage, soft peach sunset, rounded trees far to the sides.')]
COMMON='''Create ONE finished panoramic pixel-art background for a small cozy creature travel game. Wide landscape aspect ratio 3:1, ideally 1536 by 512. It is an independent single background, never a collage, never multiple panels. Authentic hand-placed chunky 16-bit pixel art, unified coarse square pixel grid, limited approximately 40-color palette, flat color clusters, subtle depth, no photorealism, no vector outlines. Side-view game stage with a horizontal traversable foreground path across the lower third. Keep the CENTER LOWER THIRD open and quiet so a separately rendered 48px pet remains visible. Distant scenery and small environmental details should read at 360 by 112 pixels. Medium-low contrast background, rich but restrained colors, natural cohesive light. Full-bleed artwork edge to edge. No characters, creatures, people, UI, text, numbers, logo, border, frame or watermarks. Theme: '''
def main():
 records=[];status=ROOT/'assets/travel-generation.json'
 if status.exists():records=json.loads(status.read_text()).get('items',[])
 for i,(name,theme) in enumerate(THEMES):
  prompt=ROOT/f'assets/prompts/travel-v1/{i:02}.txt';image=ROOT/f'assets/source/travel-v1/{i:02}.png';log=prompt.with_suffix('.log')
  if image.exists():continue
  prompt.write_text(COMMON+theme+'\n')
  print(f'Generating {i+1}/10: {name}',flush=True)
  with log.open('w') as stream:
   result=subprocess.run([sys.executable,str(SKILL),'generate','--prompt-file',str(prompt),'--output',str(image)],stdout=stream,stderr=subprocess.STDOUT)
  record={'id':i,'name':name,'source':str(image.relative_to(ROOT/'assets')),'prompt':str(prompt.relative_to(ROOT/'assets')),'log':str(log.relative_to(ROOT/'assets')),'status':'generated-awaiting-review' if result.returncode==0 else 'failed-batch-stopped'}
  if image.exists():record['sha256']=hashlib.sha256(image.read_bytes()).hexdigest()
  records=[r for r in records if r['id']!=i]+[record]
  status.write_text(json.dumps({'generator':'mobiiot-imagegen','items':records},ensure_ascii=False,indent=2)+'\n')
  print(record['status'],flush=True)
  if result.returncode:raise SystemExit(result.returncode)
if __name__=='__main__':main()
