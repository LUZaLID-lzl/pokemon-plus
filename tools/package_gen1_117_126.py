#!/usr/bin/env python3
"""Package single-pose identities with actual review/integration status."""
from pathlib import Path
import json,zipfile
R=Path(__file__).resolve().parents[1]
qa=json.loads((R/'docs/gen1-117-126/visual-review.json').read_text())
assert set(range(117,127))<=set(qa['identityReviewed'])
common=['assets/pokemon/plan-117-126.json','docs/gen1-117-126/visual-review.json','docs/GEN1-117-126-STATUS.md']
if (R/'assets/animation-generation-117-126.json').exists():common+=['assets/animation-generation-117-126.json']
status=dict(kind='single-pose identity art',includedIdentityNationalIds=list(range(117,127)),includedFullAnimationNationalIds=[],runtimeIntegrated=qa['runtimeIntegrated'],note='Ten single-pose identity drawings only; complete reviewed animation sets are packaged separately.')
if qa['runtimeIntegrated']:status['firmwareVersion']=qa['firmwareVersion']
folder=R/'assets/pokemon/gen1-117-126'
with zipfile.ZipFile(R/'dist/gen1-117-126-pets.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(folder.iterdir()):
  if p.is_file():z.write(p,'gen1-117-126/'+p.name)
 for f in common:z.write(R/f,'provenance/'+f)
 z.writestr('PACKAGE-STATUS.json',json.dumps(status,ensure_ascii=False,indent=2))
identity=json.loads((folder/'manifest.json').read_text())
files=list(folder.iterdir())+list((R/'assets/source').glob('gen1-117-126-v*'))+list((R/'assets/prompts').glob('gen1-117-126-v*.txt'))+list((R/'build').glob('gen1-117-126-generation-v*.log'))+[R/f for f in common]+[R/'tools/export_gen1_identity.py']
for item in identity['items']:
 source=R/item['sourceImage']
 files.extend([source,source.with_name(source.stem+'-box.json'),R/item['prompt'],R/item['generationLog']])
with zipfile.ZipFile(R/'dist/117-126-identity-art.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(set(files)):
  if p.is_file():z.write(p,p.relative_to(R).as_posix())
 z.writestr('PACKAGE-STATUS.json',json.dumps(status,ensure_ascii=False,indent=2))
print('Packaged ten117–126 single-pose identities with current review and integration status')
