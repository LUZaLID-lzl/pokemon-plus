#!/usr/bin/env python3
"""Package the ten identity drawings, keeping single-pose scope explicit."""
from pathlib import Path
import json, zipfile
R=Path(__file__).resolve().parents[1]
q=json.loads((R/'docs/gen1-097-106/visual-review.json').read_text())
assert q['runtimeIntegrated'] and set(range(97,107))<=set(q['identityReviewed'])&set(q['animationReviewed'])
common=['assets/pokemon/plan-097-106.json','assets/animation-generation-097-106.json','docs/gen1-097-106/visual-review.json','docs/GEN1-097-106-STATUS.md']
status=dict(kind='single-pose identity art',includedIdentityNationalIds=list(range(97,107)),includedFullAnimationNationalIds=[],completeAnimationsPackage='097-106-animation-templates.zip',runtimeIntegrated=True,firmwareVersion='v0.17.0',note='Ten identity drawings only; complete six-action sets are in the separate animation package.')
folder=R/'assets/pokemon/gen1-097-106'
with zipfile.ZipFile(R/'dist/gen1-097-106-pets.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(folder.iterdir()):
  if p.is_file():z.write(p,'gen1-097-106/'+p.name)
 for f in common:z.write(R/f,'provenance/'+f)
 z.writestr('PACKAGE-STATUS.json',json.dumps(status,ensure_ascii=False,indent=2))
files=list(folder.iterdir())+list((R/'assets/source').glob('gen1-097-106-v2*'))+list((R/'assets/prompts').glob('gen1-097-106-v*.txt'))+list((R/'build').glob('gen1-097-106-generation-v*.log'))+[R/f for f in common]+[R/'tools/export_gen1_097_106.py']
for d in (100,104):files+=list((R/f'assets/prompts/animation-{d}').glob('*'))
with zipfile.ZipFile(R/'dist/097-106-identity-art.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(set(files)):
  if p.is_file():z.write(p,p.relative_to(R).as_posix())
 z.writestr('PACKAGE-STATUS.json',json.dumps(status,ensure_ascii=False,indent=2))
print('Packaged ten single-pose identity drawings and provenance; six-action sets remain in097-106-animation-templates.zip')
