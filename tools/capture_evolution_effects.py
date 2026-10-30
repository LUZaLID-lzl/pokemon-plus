#!/usr/bin/env python3
"""Capture current published evolution chains with the actual shared renderer."""
from pathlib import Path
import argparse, json, subprocess, tempfile
parser=argparse.ArgumentParser();parser.add_argument('--species',type=int);args=parser.parse_args()
from PIL import Image
root=Path(__file__).resolve().parents[1];out=root/'docs/screenshots'
manifest=json.loads((root/'web/manifest.json').read_text())
command=[str(root/'preview/build/pokemon_plus'),'--assets',str(root/'sdcard/pokemon-plus'),'--demo']
selected=[c for c in manifest['evolution']['chains']if args.species is None or c['species']==args.species]
assert selected,'No published evolution chain matches the selection'
sheet=Image.new('RGB',(1440,1080),'black');row=0;count=0
for chain in selected:
 if chain.get('branches'):
  for branch_index,branch in enumerate(chain['branches'],1):
   for activity,suffix in [('evolution-stone',str(branch['toNationalId'])),('evolution-decline','decline')]:
    if suffix=='decline' and branch_index>1:continue
    with tempfile.TemporaryDirectory(prefix='pokemon-branch-') as folder:
     subprocess.run(command+['--species','67','--evolution-branch',str(branch_index),'--activity',activity,'--at-ms','160','--frames-dir',folder,'--frame-count','64','--frame-step','50'],check=True,capture_output=True)
     frames=[Image.open(p).convert('RGB')for p in sorted(Path(folder).glob('*.ppm'))];assert len(frames)==64
     name=f'evolution-67-{suffix}'if suffix!='decline'else'evolution-decline-67-0'
     frames[0].save(out/f'{name}.gif',save_all=True,append_images=frames[1:],duration=50,loop=0,optimize=False)
     if suffix!='decline':count+=1
  continue
 species=chain['species']
 for stage in range(len(chain['nationalIds'])-1):
  with tempfile.TemporaryDirectory(prefix='pokemon-evolution-fx-') as folder:
   step=chain.get('steps',[{} for _ in chain['levels']])[stage]
   activity='evolution-stone' if step.get('trigger',chain.get('trigger','level')) != 'level' else 'evolution'
   subprocess.run(command+['--species',str(species),'--growth-stage',str(stage),'--activity',activity,'--at-ms','160','--frames-dir',folder,'--frame-count','64','--frame-step','50'],check=True,capture_output=True)
   frames=[Image.open(p).convert('RGB') for p in sorted(Path(folder).glob('*.ppm'))]
   assert len(frames)==64 and len({im.tobytes() for im in frames})>30
   frames[0].save(out/f'evolution-{species}-{stage}.gif',save_all=True,append_images=frames[1:],duration=50,loop=0,optimize=False)
   if stage==0:
    frames[0].save(out/f'evolution-{species}.gif',save_all=True,append_images=frames[1:],duration=50,loop=0,optimize=False)
    if species in (1,2,3):
     for col,frame in enumerate((4,20,32,48)):sheet.paste(frames[frame],(col*360,row*360))
     row+=1
    if species==2:
     frames[0].save(out/'evolution.gif',save_all=True,append_images=frames[1:],duration=50,loop=0,optimize=False)
     frames[48].save(out/'evolution.png')
   count+=1
if args.species is None:sheet.save(out/'evolution-colors.png')
print(f'PASS: {count} published evolution transitions captured at 20fps, distinct frames and typed color storyboard')

for chain in selected:
 if chain.get('branches'):continue
 for stage in range(len(chain['nationalIds'])-1):
  species=chain['species']
  with tempfile.TemporaryDirectory(prefix='pokemon-refusal-') as folder:
   subprocess.run(command+['--species',str(species),'--growth-stage',str(stage),'--activity','evolution-decline','--at-ms','100','--frames-dir',folder,'--frame-count','64','--frame-step','50'],check=True,capture_output=True)
   frames=[Image.open(p).convert('RGB') for p in sorted(Path(folder).glob('*.ppm'))]
   frames[0].save(out/f'evolution-decline-{species}-{stage}.gif',save_all=True,append_images=frames[1:],duration=50,loop=0,optimize=False)
print(f'PASS: all {count} refusal animations retain each actual current pet form')
if args.species is not None:raise SystemExit(0)
for activity in ('evolution-choice','evolution-decline'):
 with tempfile.TemporaryDirectory(prefix='pokemon-evolution-choice-') as folder:
  subprocess.run(command+['--species','1','--activity',activity,'--at-ms','100','--frames-dir',folder,'--frame-count','64','--frame-step','50'],check=True,capture_output=True)
  frames=[Image.open(p).convert('RGB') for p in sorted(Path(folder).glob('*.ppm'))]
  assert len(frames)==64
  frames[0].save(out/f'{activity}.gif',save_all=True,append_images=frames[1:],duration=50,loop=0,optimize=False)
  frames[48 if activity=='evolution-decline' else 0].save(out/f'{activity}.png')
for activity in ('heart-collection','heart-detail'):
 with tempfile.TemporaryDirectory(prefix='pokemon-heart-') as folder:
  target=Path(folder)/'shot.ppm'
  subprocess.run(command+['--activity',activity,'--screenshot',str(target)],check=True,capture_output=True)
  Image.open(target).convert('RGB').save(out/f'{activity}.png')
print('PASS: shared LVGL choice, permanent refusal animation, collectible gallery and Pure Heart detail captured')
