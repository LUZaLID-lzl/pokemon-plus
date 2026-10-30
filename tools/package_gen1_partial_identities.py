#!/usr/bin/env python3
"""Package only reviewed single-pose art, never claim missing identities/animations."""
from pathlib import Path
import argparse,json,zipfile
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--scope',required=True);a=p.parse_args()
planfile=f'assets/pokemon/plan-{a.scope}.json';qfile=f'docs/gen1-{a.scope}/visual-review.json';plan=json.loads((R/planfile).read_text());qa=json.loads((R/qfile).read_text());folder=R/f'assets/pokemon/gen1-{a.scope}';identity=json.loads((folder/'manifest.json').read_text())
ids=sorted(qa['identityReviewed']);assert ids
scope=f'{ids[0]:03}-{ids[-1]:03}';name=f'{scope}-identity-art.zip'
remaining=json.loads((R/'assets/pokemon/remaining-gen1-plan.json').read_text())
integrated=set(qa.get('integratedNationalIds',[]))
status=dict(kind='reviewed single-pose identities only',requestedRange=remaining['requestedRange'],includedIdentityNationalIds=ids,includedFullAnimationNationalIds=[],pendingIdentityNationalIds=remaining['pendingIdentityNationalIds'],runtimeIntegrated=set(ids)<=integrated,scopeCompleted=False,firmwareVersion=qa.get('firmwareVersion'),note='Integration applies only to included identities. Complete animation sets are separate packages; the entire151-form request remains incomplete.')
status.update(identityArtScopeCompleted=set(ids)=={x['nationalId'] for x in plan['items']},artScopeCompleted=remaining.get('artScopeCompleted',False))
if status['artScopeCompleted']:
 status['note']='Requested identity and animation art is complete; this archive contains single-pose identities only. Animation templates are separate packages. scopeCompleted=false refers to pending runtime integration.'
files={planfile,qfile,f'docs/GEN1-{a.scope}-STATUS.md','assets/pokemon/remaining-gen1-plan.json'}
for x in identity['items']:
 if x['id']not in ids:continue
 files.update({f'assets/pokemon/gen1-{a.scope}/'+x['image'],f'assets/pokemon/gen1-{a.scope}/'+x['preview'],x['sourceImage'],x['prompt'],x['generationLog'],str(Path(x['sourceImage']).with_name(Path(x['sourceImage']).stem+'-box.json'))})
with zipfile.ZipFile(R/'dist'/name,'w',zipfile.ZIP_DEFLATED)as z:
 for f in sorted(files):z.write(R/f,f)
 z.writestr('manifest.json',json.dumps({**identity,'items':[x for x in identity['items']if x['id']in ids]},ensure_ascii=False,indent=2))
 z.writestr('PACKAGE-STATUS.json',json.dumps(status,ensure_ascii=False,indent=2))
print(R/'dist'/name)
