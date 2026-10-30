#!/usr/bin/env python3
"""Refresh only published battle previews from the shared LVGL renderer."""
from pathlib import Path
from PIL import Image
import argparse, json, subprocess, tempfile, struct
from image_binary import write_bin
parser=argparse.ArgumentParser();parser.add_argument('--start',type=int,default=0);parser.add_argument('--end',type=int,default=10000);parser.add_argument('--actor-only',action='store_true');args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
out=root/'docs/screenshots'
manifest=json.loads((root/'web/manifest.json').read_text())
command=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo','--activity','battle']
selected=[m for m in manifest['skills']if args.start<=m['id']<=args.end]
for move in selected:
 i=move['id']
 with tempfile.TemporaryDirectory(prefix='pokemon-story-') as directory:
  actor=next((p for p in manifest['pets'] if not p['stage'] and p['name']==move['species']),None)
  if actor:
   actor_args=['--species',str(actor['species'])]
  else:
   template=root/f'assets/animation-templates/{move["nationalIds"][0]:03}'
   profile=json.loads((template/'manifest.json').read_text())
   native=Path(directory)/'native';native.mkdir()
   durations=[];once_mask=0
   for a,action in enumerate(['idle','walk','happy','eat','sleep','attack']):
    row=profile['actions'][action];durations+=row['durationsMs']
    if row['mode']=='one-shot':once_mask|=1<<a
    for f,name in enumerate(row['frames']):
     # Approved source faces left; battle ally faces right, as in runtime.
     write_bin(Image.open(template/'right'/name),native/f'{a*8+f}.bin',indexed=True)
   (native/'profile.bin').write_bytes(struct.pack('<49H',*durations,once_mask))
   actor_args=['--catalog-family',str(move['familyId']),'--catalog-frames',str(native)]
   if args.actor_only:actor_args+=['--catalog-actor-check']
  if not actor and not args.actor_only:
   audit=Path(directory)/'actor-check';audit.mkdir()
   subprocess.run(command+actor_args+['--catalog-actor-check','--move-slot',str(move['slot']),'--at-ms','2000','--frames-dir',str(audit),'--frame-count','1'],check=True,capture_output=True)
   Image.open(audit/'000.ppm').convert('RGB').save(out/f'skill-{i}-actor.png')
  subprocess.run(command+actor_args+['--move-slot',str(move['slot']),'--at-ms','2000','--frames-dir',directory,'--frame-count','1' if args.actor_only else '48'],check=True,capture_output=True)
  frames=[Image.open(p).convert('RGB') for p in sorted(Path(directory).glob('*.ppm'))]
  if args.actor_only:
   frames[0].save(out/f'skill-{i}-actor.png')
   print(f'Captured pre-impact actor {i}',flush=True)
   continue
  assert len(frames)==48
  frames[0].save(out/f'skill-{i}.gif',save_all=True,append_images=frames[1:],duration=[120,130]*24,loop=0,optimize=False)
  frames[2].save(out/f'skill-{i}.png')
  print(f'Rendered skill {i}: {move["species"]} / {move["name"]}',flush=True)
  if i==0:
   frames[2].save(out/'activity-battle.png')
   frames[2].save(out/'battle-narration.png')
   frames[0].save(out/'battle-narration.gif',save_all=True,append_images=frames[1:],duration=[120,130]*24,loop=0,optimize=False)
print(f"PASS: {len(selected)} {'isolated actor checks' if args.actor_only else 'selected published skill GIFs use sequential narration'} (starting ID{args.start})")
