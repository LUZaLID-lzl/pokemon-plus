#!/usr/bin/env python3
"""Inspect real RGB565 flush pixels, without the screenshot's circular mask."""
from pathlib import Path
from PIL import Image
import json, subprocess, tempfile, sys
root=Path(__file__).resolve().parents[1]
out=root/'docs/screenshots'
command=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo','--raw-display']
report=[]
home_only="--home-only" in sys.argv
manifest=json.loads((root/'sdcard/pokemon-plus/manifest.json').read_text())
home_background=Image.open(root/'sdcard/pokemon-plus'/manifest['scenes'][0]['image']).convert('RGB')
# wild_fixture uses the coastal regional pool (scene 2), including its background.
wild_background=Image.open(root/'sdcard/pokemon-plus'/manifest['scenes'][2]['image']).convert('RGB')
gym_maps={k:Image.open(root/f'sdcard/pokemon-plus/gyms/{v}.png').convert('RGB') for k,v in [('page-17','map'),('cerulean-map','cerulean-map'),('cerulean-locked','cerulean-map'),('vermilion-map','vermilion-map'),('vermilion-locked','vermilion-map'),('gym-cleared','map'),('cerulean-cleared','cerulean-map'),('vermilion-cleared','vermilion-map'),('celadon-map','celadon-map'),('celadon-locked','celadon-map'),('celadon-cleared','celadon-map'),('fuchsia-map','fuchsia-map'),('fuchsia-locked','fuchsia-map'),('fuchsia-cleared','fuchsia-map'),('saffron-map','saffron-map'),('saffron-locked','saffron-map'),('saffron-cleared','saffron-map'),('cinnabar-map','cinnabar-map'),('cinnabar-locked','cinnabar-map'),('cinnabar-cleared','cinnabar-map'),('viridian-map','viridian-map'),('viridian-locked','viridian-map'),('viridian-cleared','viridian-map')]}
with tempfile.TemporaryDirectory(prefix='pokemon-edges-') as d:
 for name,args in [('home-clock',[])]+([("page-11",["--page","11"])] if home_only else [(f'page-{p}',['--page',str(p)]) for p in range(1,22)]+[(f'travel-{a}',['--activity',a]) for a in ('travel','travel-ready','travel-receipt')]+[(f'intro-{t}',['--page','22','--at-ms',str(t)]) for t in (400,1400,4000,5500,6500,8500,10900)]+[('gym-battle',['--activity','gym-battle']),('gym-faint',['--activity','gym-faint']),('cerulean-locked',['--activity','cerulean-locked']),('cerulean-map',['--activity','cerulean-map']),('cerulean-detail',['--activity','cerulean-detail']),('cerulean-team',['--activity','cerulean-team']),('cerulean-battle',['--activity','cerulean-battle']),('cerulean-faint',['--activity','cerulean-faint']),('cerulean-win',['--activity','cerulean-win']),('cerulean-badges',['--activity','cerulean-badges'])]+[(f'vermilion-{a}',['--activity',f'vermilion-{a}']) for a in ('locked','map','detail','team','battle','faint','win','badges')]+[(f'{prefix}-cleared',['--activity',f'{prefix}-cleared']) for prefix in ('gym','cerulean','vermilion','celadon','fuchsia','saffron','cinnabar','viridian')]+[*[(f'viridian-{a}',['--activity',f'viridian-{a}']) for a in ('locked','map','detail','team','battle','faint','win','badges')],*[(f'cinnabar-{a}',['--activity',f'cinnabar-{a}']) for a in ('locked','map','detail','team','battle','faint','win','badges')],*[(f'saffron-{a}',['--activity',f'saffron-{a}']) for a in ('locked','map','detail','team','battle','faint','win','badges')],*[(f'fuchsia-{a}',['--activity',f'fuchsia-{a}']) for a in ('locked','map','detail','team','battle','faint','win','badges')],*[(f'celadon-{a}',['--activity',f'celadon-{a}']) for a in ('locked','map','detail','team','battle','faint','win','badges')],*[(f'wild-{w}',['--wild',w]) for w in ('alert','pending','battle','caught','duplicate','full','lost')],('battle',['--activity','battle']),('evolution',['--activity','evolution','--at-ms','1200'])]):
  target=Path(d)/(name+'.ppm')
  subprocess.run(command+args+['--screenshot',str(target)],check=True,capture_output=True)
  im=Image.open(target).convert('RGB')
  if name=='page-9':
   # Scene selection is a centered native panorama extending to the panel arc.
   for y in range(108,252):
    for x in list(range(18))+list(range(342,360)):
     if (x-179.5)**2+(y-179.5)**2<180**2:
      assert max(abs(a-b) for a,b in zip(im.getpixel((x,y)),home_background.getpixel((x,y))))<=7,('scene panorama side',x,y)
  if name in gym_maps:
   gym_map=gym_maps[name]
   # Check the actual side scenery, including the panel arc, so a rectangular
   # inset or a dark cosmetic rim cannot pass as full-width artwork.
   for y in range(100,268):
    for x in list(range(36))+list(range(324,360)):
     if (x-179.5)**2+(y-179.5)**2<180**2:
      if 158<=y<202 and (18<=x<62 or 298<=x<342):continue # fixed side navigation targets, r168-safe
      assert max(abs(a-b) for a,b in zip(im.getpixel((x,y)),gym_map.getpixel((x,y-100))))<=7,('map side',x,y)
  rim=[(x,y,im.getpixel((x,y))) for y in range(360) for x in range(360) if (x-179.5)**2+(y-179.5)**2>=174**2]
  # Battle rails intentionally touch the panel arc. Permit only their exact
  # RGB565 colors in the eight rail rows inside the physical circular panel.
  def rail_pixel(x,y,c):
   return name in ('wild-battle','battle','gym-battle','gym-faint','cerulean-battle','cerulean-faint','vermilion-battle','vermilion-faint','celadon-battle','celadon-faint','fuchsia-battle','fuchsia-faint','saffron-battle','saffron-faint','cinnabar-battle','cinnabar-faint','viridian-battle','viridian-faint') and 84<=y<92 and (x<40 or x>=320) and (x-179.5)**2+(y-179.5)**2<180**2 and c in ((49,214,156),(246,89,139))
  # Home intentionally displays its artwork up to the physical panel edge.
  # Permit that exact background (RGB565 quantization <=7), never new UI pixels.
  def background_pixel(x,y,c):
   if name=='page-9' and 108<=y<252:
    return max(abs(a-b) for a,b in zip(c,home_background.getpixel((x,y))))<=7
   if name in gym_maps and 100<=y<268:
    return max(abs(a-b) for a,b in zip(c,gym_map.getpixel((x,y-100))))<=7
   if name=='wild-alert':
    return max(abs(a-b) for a,b in zip(c,wild_background.getpixel((x,y))))<=7
   return name.startswith('home-') and max(abs(a-b) for a,b in zip(c,home_background.getpixel((x,y))))<=7
  intentional=sum(sum(c)>384 and rail_pixel(x,y,c) for x,y,c in rim)
  background=sum(sum(c)>384 and background_pixel(x,y,c) for x,y,c in rim)
  bright=sum(sum(c)>384 and not rail_pixel(x,y,c) and not background_pixel(x,y,c) for x,y,c in rim)
  report.append({'page':name,'rawOuterPixels':len(rim),'brightPixels':bright,'intentionalBattleRailPixels':intentional,'originalBackgroundPixels':background})
  assert bright==0,(name,bright)
  if name=='page-3':im.save(out/'raw-edge-after-v073.png')
(out/('raw-edge-hud.json' if home_only else 'raw-edge-v073.json')).write_text(json.dumps(report,indent=2)+'\n')
print('PASS: HUD home/settings introduce no bright outer rim; original home artwork accounted for' if home_only else 'PASS: real flush pixels on home clock, 21 pages plus gym battle/faint, travel/journey/return/receipt, active battle and evolution; no unexpected bright pixels outside radius174; exact home/map artwork and colored battle rails are accounted for (no cosmetic mask)')
