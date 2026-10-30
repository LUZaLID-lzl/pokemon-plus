#!/usr/bin/env python3
"""Publish data only after all ten real identities and48-pose sets are accepted."""
from pathlib import Path
import json,hashlib
R=Path(__file__).resolve().parents[1]
def read(f):return json.loads((R/f).read_text())
def write(f,o):(R/f).write_text(json.dumps(o,ensure_ascii=False,indent=2)+'\n')
plan=read('assets/pokemon/plan-117-126.json');qa=read('docs/gen1-117-126/visual-review.json')
assert set(range(117,127))<=set(qa['identityReviewed'])&set(qa['animationReviewed']),'Full visual QA required'
identity=read('assets/pokemon/gen1-117-126/manifest.json')
for pet in plan['items']:
 d=pet['nationalId'];folder=R/f'assets/animation-templates/{d:03}';m=read(folder.relative_to(R)/'manifest.json')
 assert m['frames']==48 and m['framesPerAction']==8 and m['source']==f"assets/source/animation-{d:03}-master-v{qa['acceptedVersions'][str(d)]}.png"
 assert hashlib.sha256((R/m['source']).read_bytes()).hexdigest()==m['sourceSha256']
 ident=next(x for x in identity['items']if x['id']==d)
 assert hashlib.sha256((R/ident['sourceImage']).read_bytes()).hexdigest()==ident['sourceSha256']
 for a in m['actions'].values():
  assert len(a['frames'])==len(a['durationsMs'])==8
  assert len({hashlib.sha256((folder/f).read_bytes()).hexdigest()for f in a['frames']})==8
 for frame in m['items']:assert hashlib.sha256((folder/frame['image']).read_bytes()).hexdigest()==frame['sha256']
roster=read('assets/pet-roster.json');assert len(roster)==116
for p in plan['items']:roster.append(dict(id=p['artId'],nationalId=p['nationalId'],name=p['name'],species=p['species'],stage=p['stage'],source=f"pokemon/gen1-117-126/{p['nationalId']:03}.png"))
assert [x['id']for x in roster]==list(range(126))
e=read('assets/evolution.json')
for target in plan['evolution']:
 steps=target['steps'];chain=dict(species=target['species'],nationalIds=target['nationalIds'],levels=[s.get('level',0)for s in steps],artIds=[next(p['id']for p in roster if p['nationalId']==d)for d in target['nationalIds']],trigger=steps[0]['trigger'],steps=steps)
 e['chains']=[c for c in e['chains']if c['species']!=target['species']]+[chain]
e.update(version=15,imageFallback='001-024 and026-126 dedicated48 poses;025 retains original24 translation slots',pendingChains=[])
assert len(e['chains'])==49 and sum(len(x['steps'])for x in e['chains'])==64
moves=read('assets/moves.json');assert len(moves['moves'])==220
new=read('assets/moves-plan-117-126.json')['moves'];assert [x['id']for x in new]==list(range(220,248));moves['moves']+=new
write('assets/pet-roster.json',roster);write('assets/evolution.json',e);write('assets/moves.json',moves)
plan['status']='visual QA passed;runtime validation pending';write('assets/pokemon/plan-117-126.json',plan)
print('Prepared126 forms/62 families/64 transitions/248 moves;runtime QA/build still required')
