#!/usr/bin/env python3
"""Package single-pose identities with actual review/integration status."""
from pathlib import Path
import json,zipfile
R=Path(__file__).resolve().parents[1]
qa=json.loads((R/'docs/gen1-107-116/visual-review.json').read_text())
assert set(range(107,117))<=set(qa['identityReviewed'])
common=['assets/pokemon/plan-107-116.json','docs/gen1-107-116/visual-review.json','docs/GEN1-107-116-STATUS.md']
if (R/'assets/animation-generation-107-116.json').exists():common+=['assets/animation-generation-107-116.json']
status=dict(kind='single-pose identity art',includedIdentityNationalIds=list(range(107,117)),includedFullAnimationNationalIds=[],runtimeIntegrated=qa['runtimeIntegrated'],note='Ten single-pose identity drawings only; complete reviewed animation sets are packaged separately.')
if qa['runtimeIntegrated']:status['firmwareVersion']=qa['firmwareVersion']
folder=R/'assets/pokemon/gen1-107-116'
with zipfile.ZipFile(R/'dist/gen1-107-116-pets.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(folder.iterdir()):
  if p.is_file():z.write(p,'gen1-107-116/'+p.name)
 for f in common:z.write(R/f,'provenance/'+f)
 z.writestr('PACKAGE-STATUS.json',json.dumps(status,ensure_ascii=False,indent=2))
files=list(folder.iterdir())+list((R/'assets/source').glob('gen1-107-116-v*'))+list((R/'assets/prompts').glob('gen1-107-116-v*.txt'))+list((R/'build').glob('gen1-107-116-generation-v*.log'))+[R/f for f in common]+[R/'tools/export_gen1_107_116.py']
with zipfile.ZipFile(R/'dist/107-116-identity-art.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(set(files)):
  if p.is_file():z.write(p,p.relative_to(R).as_posix())
 z.writestr('PACKAGE-STATUS.json',json.dumps(status,ensure_ascii=False,indent=2))
print('Packaged ten107–116 single-pose identities with current review and integration status')
