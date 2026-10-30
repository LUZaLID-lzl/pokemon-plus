#!/usr/bin/env python3
"""Compare actual battle flush pixels with transparent sprite compositing.

Covers packed I4 and I8 frames, both facings and every integrated form;
transparent feet margins must show only the scene (no transformed-image stripe).
"""
from pathlib import Path
import json, subprocess, tempfile
from PIL import Image
root=Path(__file__).resolve().parents[1]
assets=root/'sdcard/pokemon-plus'
roster=json.loads((root/'assets/pet-roster.json').read_text())
bases={p['species']:p for p in roster if p['stage']==0}
bg=Image.open(assets/'battle-scenes/0.png').convert('RGB')
report=[]
with tempfile.TemporaryDirectory(prefix='battle-sprites-') as directory:
 for pet in roster:
  target=Path(directory)/'frame.ppm'
  subprocess.run([str(root/'preview/build/pokemon_plus'),'--assets',str(assets),'--demo','--activity','battle','--species',str(pet['species']),'--growth-stage',str(pet['stage']),'--at-ms','1','--raw-display','--screenshot',str(target)],check=True,capture_output=True)
  actual=Image.open(target).convert('RGB')
  for form,x,folder in [(pet,52,'pets'),(bases[(pet['species']+1)%len(bases)],252,'pets-left')]:
   sprite=Image.open(assets/f'{folder}/{form["id"]}-0.png').convert('RGBA')
   largest=0
   for y in range(56):
    for col in range(56):
     rgba=sprite.getpixel(((2*col+1)*sprite.width//112,(2*y+1)*sprite.height//112))
     expected=rgba[:3] if rgba[3] else bg.getpixel((x+col,52+y))
     got=actual.getpixel((x+col,160+y))
     error=max(abs(a-b) for a,b in zip(got,expected))
     assert error<=7,(pet['nationalId'],folder,col,y,got,expected)
     largest=max(largest,error)
   report.append({'nationalId':form['nationalId'],'facing':folder,'maxRgb565Error':largest})
(root/'docs/screenshots/battle-sprite-composite-v071.json').write_text(json.dumps(report,indent=2)+'\n')
print(f'PASS: {len(roster)} battle forms, both facings; all 56px sprite and transparent-margin pixels match source compositing within RGB565 precision')
