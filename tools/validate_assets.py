from pathlib import Path
import json,hashlib,struct
from PIL import Image, ImageOps
root=Path(__file__).resolve().parents[1];folder=root/'sdcard/pokemon-plus';manifest=json.loads((folder/'manifest.json').read_text())
roster=json.loads((root/'assets/pet-roster.json').read_text())
form_count=len(roster);families={p['species'] for p in roster};frame_count=sum(len(p['frames']) for p in manifest['pets'])
assert len(manifest['pets'])==form_count
for item in manifest['files']:
 p=folder/item['path'];data=p.read_bytes();assert len(data)==item['bytes'] and hashlib.sha256(data).hexdigest()==item['sha256'],p
 if p.suffix=='.bin':
  if p.parent.name in ('pets','pets-left'):
   from asset_pet_canvas import decode
   data=decode(data)
  magic,cf,flags,w,h,stride,reserved=struct.unpack('<BBHHHHH',data[:12]);assert magic==0x19 and cf in [0x09,0x0A,0x12,0x14],p
  assert stride==((w+1)//2 if cf==0x09 else w if cf==0x0A else w*2),p
  assert len(data)==(12+64+stride*h if cf==0x09 else 12+1024+w*h if cf==0x0A else 12+w*h*(3 if cf==0x14 else 2)),p
  if cf in (0x09,0x0A):
   count=16 if cf==0x09 else 256
   colors=[tuple(data[12+i*4+j] for j in (2,1,0,3)) for i in range(count)]
   pixels=data[12+count*4:]
   decoded=[colors[(pixels[y*stride+x//2]>>(4 if x%2==0 else 0))&15] if cf==0x09 else colors[pixels[y*stride+x]] for y in range(h) for x in range(w)]
   assert decoded==list(Image.open(p.with_suffix('.png')).convert('RGBA').getdata()),p
  if cf==0x14:
   png=Image.open(p.with_suffix('.png'));alpha=png.getchannel('A');assert alpha.getextrema()[0]==0 and alpha.getextrema()[1]>=240,p
assert manifest==json.loads((root/'web/manifest.json').read_text())
from fishing_art import build_sprites,metadata as fishing_metadata
assert manifest['fishing']==fishing_metadata(root)
for name,expected in build_sprites(root).items():
 actual=Image.open(folder/f'fishing/{name}.png').convert(expected.mode)
 assert actual.size==expected.size and actual.tobytes()==expected.tobytes(),name
 if name!='pond':assert set(actual.getchannel('A').getdata())=={0,255} and len(set(actual.getdata()))<=16
font=set((root/'assets/font-characters.txt').read_text());chars=set()
for f in ('main','core'):
 for p in (root/f).glob('*.[ch]'):
  if p.name.startswith('pp_font'):continue
  chars.update(c for c in p.read_text() if ord(c)>127)
assert not chars-font,'Missing font glyphs: '+''.join(sorted(chars-font))
print(f'PASS: {len(manifest["files"])} image files, device headers, alpha, SHA256, web/SD manifest, Chinese font coverage')

animation=manifest['animation']
assert animation==json.loads((root/'assets/pet-animation.json').read_text())
assert animation==json.loads((root/'web/animation.json').read_text())
assert animation['framesPerPet']==48 and animation['frameMs']==125
assert sorted(f for frames in animation['actions'].values() for f in frames)==list(range(48))
for pet in manifest['pets']:
 count=24 if pet['nationalId']==25 else 48
 assert len(pet['frames'])==count and pet['keyPoses']==(1 if pet['nationalId']==25 else 48)
 images=[Image.open(folder/p).convert('RGBA') for p in pet['frames']]
 for im in images:
  assert im.size==(animation['canvas'],animation['canvas'])
  assert set(im.getchannel('A').getdata())=={0,255}
  assert im.getbbox()[2]-im.getbbox()[0]<=animation['silhouetteMax']
  assert im.getbbox()[3]-im.getbbox()[1]<=animation['silhouetteMax']
  assert len({p[:3] for p in im.getdata() if p[3]})<=animation['paletteColors']+1
 source=Image.open(root/'assets'/pet['source']).convert('RGBA')
 # Compare left-facing playback to the reviewed master without mirroring offsets.
 images=[ImageOps.mirror(im) for im in images]
 assert hashlib.sha256((root/'assets'/pet['source']).read_bytes()).hexdigest()==pet['sourceSha256']
 if pet['nationalId']==25:
  from pet_frames import build_frames
  assert all(a.tobytes()==b.tobytes() for a,b in zip(images,build_frames(source)))
  continue
 template_root=root/f'assets/animation-templates/{pet["nationalId"]:03}'
 template=json.loads((template_root/'manifest.json').read_text())
 assert pet['animation']==animation['speciesOverrides'][str(pet['id'])]
 for a,conf in template['actions'].items():
  assert pet['animation']['durationsMs'][a]==conf['durationsMs']
  poses=[]
  for index,file in zip(pet['animation']['actions'][a],conf['frames']):
   assert images[index].tobytes()==Image.open(template_root/file).convert('RGBA').tobytes()
   poses.append(images[index].tobytes())
  assert len(set(poses))==8,(pet['name'],a,'duplicate poses')
header=(root/'main/pet_animation.h').read_text()
assert '#define PP_FRAME_COUNT 48' in header and 'pp_frame_durations' in header
from animation_contract import write_header
import tempfile
with tempfile.TemporaryDirectory() as tmp:
 generated=Path(tmp)/'pet_animation.h';write_header(generated,animation);assert generated.read_text()==header
print(f'PASS: {form_count-1} exact 48-frame templates and variable durations; 025 preserved source-exact 24 slots; generated C playback contract')

moves=json.loads((root/'assets/moves.json').read_text())['moves']
source=(root/'core/moves.c').read_text()
for move,skill in zip(moves,manifest['skills']):
 assert move['name']==skill['name'] and move['name'] in source
 assert skill['status']=='animated' and skill['source']==move['source']
for size in [12,14,18]:assert (root/f'main/pp_font_{size}.c').exists()
print('PASS: sourced move names, animated skill manifest, 12/14/18px font hierarchy')

from move_catalog import load_catalog, skill_manifest
catalog=load_catalog()
assert manifest['skills']==skill_manifest(roster)
assert len(moves)==len(manifest['skills'])==len(catalog['families'])*4
assert sum(bool(m['runtimeIntegrated']) for m in manifest['skills'])==len(families)*4
for species in [p['name'] for p in roster if not p['stage']]:
 assert sorted(m['slot'] for m in moves if m['species']==species)==[0,1,2,3]
evolution=manifest['evolution']
assert evolution==json.loads((root/'assets/evolution.json').read_text())
assert [f.get('level') for f in evolution['forms'][1:]]==[16,36]
for element in range(4):
 for phase in range(4):assert Image.open(folder/f'effects/{element}-{phase}.png').size==(48,48)
print('PASS: 4 slots per species, evolution template, 16 generated effect frames')

from PIL import ImageOps
for pet in range(form_count):
 for frame in range(24 if pet==0 else 48):
  original=Image.open(folder/f'pets/{pet}-{frame}.png')
  mirrored=Image.open(folder/f'pets-left/{pet}-{frame}.png')
  assert mirrored.tobytes()==ImageOps.mirror(original).tobytes()
for dex in [1,4,7]:
 assert Image.open(folder/f'starters-large/{dex:03}.png').tobytes()==Image.open(root/f'assets/pokemon/gen1-001-009/{dex:03}.png').resize((40,40),Image.Resampling.NEAREST).resize((80,80),Image.Resampling.NEAREST).tobytes()
print(f'PASS: {frame_count} exact horizontal mirrors and 80px 001/004/007 starter portraits')

assert sorted(p['nationalId'] for p in manifest['pets'])==list(range(1,form_count+1))
expected_chains=[[p['nationalId'] for p in sorted(roster,key=lambda p:p['stage']) if p['species']==species] for species in sorted(families,key=lambda f:min(p['nationalId'] for p in roster if p['species']==f))]
assert [chain['nationalIds'] for chain in evolution['chains']]==[c for c in expected_chains if len(c)>1]
print(f'PASS: all {form_count} national-dex forms, {len(evolution["chains"])} evolution chains, exact I4/I8 roundtrip')

# Evolution data and firmware must agree on every gate/art mapping.
import re
roster=json.loads((root/'assets/pet-roster.json').read_text())
block=(root/'core/game.c').read_text().split('pp_evolutions[PP_EVOLUTIONS] =',1)[1].split('};',1)[0]
templates={(int(s),int(stage)):(int(level),int(art),name) for s,level,stage,art,name in re.findall(r'\{\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*"([^"]+)"',block)}
assert len(templates)==sum(len(c['levels']) for c in evolution['chains'])
for chain in evolution['chains']:
 for stage,level in enumerate(chain['levels']):
  pet=next(p for p in roster if p['nationalId']==chain['nationalIds'][stage+1])
  assert (pet['species'],pet['stage'],pet['id'])==(chain['species'],stage+1,chain['artIds'][stage+1])
  assert templates[(chain['species'],stage)]==(level,pet['id'],pet['name'])
print(f'PASS: all {len(templates)} evolution gates and dedicated forms match firmware, roster and Web data')

stone=next(c for c in evolution['chains'] if c['nationalIds']==[25,26])
assert stone['trigger']=='thunder-stone' and stone['levels']==[0] and stone['price']==200
for pending in evolution.get('pendingChains',[]):
 assert any(dex not in {p['nationalId'] for p in roster} for dex in pending['nationalIds'])
for chain in evolution['chains']:
 if 'steps' not in chain:continue
 assert len(chain['steps'])==len(chain['levels'])
 for step,level in zip(chain['steps'],chain['levels']):
  assert (step['trigger']=='level')==bool(level)
  if not level:assert step['price']==200 and step['item'] in ('雷之石','月之石','火之石','叶之石','水之石','联系之绳')
print('PASS: per-stage level/stone conditions, atomic stone prices and no pending chain exposing missing art')
for chain in evolution['chains']:
 if not chain.get('branches'):continue
 assert chain['species']==67 and chain['nationalIds']==[133,134,135,136] and chain['levels']==[]
 assert [(b['fromNationalId'],b['toNationalId'],b['item'],b['price']) for b in chain['branches']]==[(133,134,'水之石',200),(133,135,'雷之石',200),(133,136,'火之石',200)]
 assert all(next(p for p in roster if p['nationalId']==b['toNationalId'])['stage']==i+1 for i,b in enumerate(chain['branches']))
print('PASS: Eevee three mutually exclusive base-to-final branches; no linear evolution edges')


collection=manifest['collection']
assert collection['total']==586 and collection['travelSlots']==583 and collection['imagesIntegrated']==586
assert collection['special'][0]['id']==583 and collection['special'][0]['name']=='纯净之心'
heart=Image.open(folder/collection['special'][0]['image']).convert('RGBA')
assert heart.size==(32,32) and set(heart.getchannel('A').getdata())=={0,255}
assert len({p[:3] for p in heart.getdata() if p[3]})<=12
assert (folder/'collection/pure-heart.bin').is_file()
assert [x['id'] for x in collection['special']]==[583,584,585]
for item in collection['special'][1:]:
 im=Image.open(folder/item['image']).convert('RGBA')
 assert im.size==(32,32) and set(im.getchannel('A').getdata())=={0,255}
 assert len(set(im.getdata()))<=16
 assert Image.open(folder/item['smallImage']).size==(20,20)
 assert (folder/item['image'].replace('.png','.bin')).is_file()
assert [q['count'] for q in collection['qualities']]==[350,146,58,23,6,3]
assert [x['id'] for x in collection['items']]==list(range(586))
assert [sum(x['quality']==q for x in collection['items']) for q in range(6)]==[350,146,58,23,6,3]
unique=set()
for item in collection['items'][:583]:
 im=Image.open(folder/item['image']).convert('RGBA')
 assert im.size==(32,32) and set(im.getchannel('A').getdata())=={0,255}
 assert len(set(im.getdata()))<=16
 assert (folder/item['smallImage']).is_file() and Image.open(folder/item['smallImage']).size==(20,20)
 unique.add(im.tobytes())
 assert (folder/item['image'].replace('.png','.bin')).is_file()
assert len(unique)==583
print('PASS: 583 unique 32px/20px travel icons, exact quantity qualities, stable IDs and unique special gift')


# Travel art is an independent ten-destination pool, with lossless runtime files.
from travel_art import load_catalog as load_travel_catalog, background, write_header as write_travel_header
travel=load_travel_catalog(root)
assert len(manifest['travel']['destinations'])==10
unique=set()
for item,destination in zip(travel['items'],manifest['travel']['destinations']):
 assert item['id']==destination['id'] and item['name']==destination['name']
 im=Image.open(folder/destination['image']).convert('RGB')
 assert im.size==(360,112) and len(set(im.getdata()))<=65
 assert im.tobytes()==background(root,item).tobytes()
 unique.add(im.tobytes())
assert len(unique)==10
old=(root/'core/travel_catalog.h').read_text();write_travel_header(root,travel)
assert (root/'core/travel_catalog.h').read_text()==old
print('PASS: 10 independent travel paintings, stable catalog/names, exact offline processing, lossless runtime images')

# Fourth gym uses only integrated defenders and lossless, boot-preloaded art.
gyms=manifest['gyms'];celadon=gyms['celadon']
assert celadon['requiresBadge']==2 and celadon['qualityRates']==[45,40,15]
assert [(p['nationalId'],p['level']) for p in celadon['enemies']]==[(114,34),(71,35),(45,36)]
for enemy in celadon['enemies']:
 assert roster[enemy['art']]['nationalId']==enemy['nationalId']
 assert roster[enemy['art']]['name']==enemy['name']
assert len(gyms['doorTeams']['images'])==len(gyms['clearedStamp']['images'])==8
assert len(gyms['doorTeams']['positions'])==8
for name,size in [('erika',(64,64)),('rainbow',(64,64)),('rainbow-display',(48,48)),('celadon-map',(360,168)),('celadon-arena',(360,112)),('defeated-grass',(136,48)),('door-team-3',(128,72))]:
 p=folder/f'gyms/{name}.bin';assert p.read_bytes()[1] in (9,10),p
 im=Image.open(p.with_suffix('.png')).convert('RGBA');assert im.size==size
 if name not in ('celadon-map','celadon-arena'):assert set(im.getchannel('A').getdata())=={0,255}
print('PASS: fourth gym integrated defenders/level/reward metadata, seven stamps/teams, native lossless indexed art and binary alpha')

fuchsia=gyms['fuchsia']
assert fuchsia['requiresBadge']==3 and fuchsia['qualityRates']==[35,45,20]
assert [(p['nationalId'],p['level']) for p in fuchsia['enemies']]==[(109,42),(89,43),(110,44)]
for enemy in fuchsia['enemies']:
 assert roster[enemy['art']]['nationalId']==enemy['nationalId'] and roster[enemy['art']]['name']==enemy['name']
for name,size in [('koga',(64,64)),('soul',(64,64)),('soul-display',(48,48)),('fuchsia-map',(360,168)),('fuchsia-arena',(360,112)),('defeated-poison',(136,48)),('door-team-4',(128,72))]:
 p=folder/f'gyms/{name}.bin';assert p.read_bytes()[1] in (9,10),p
 im=Image.open(p.with_suffix('.png')).convert('RGBA');assert im.size==size
 if name not in ('fuchsia-map','fuchsia-arena'):assert set(im.getchannel('A').getdata())=={0,255}
print('PASS: fifth gym integrated poison defenders, sequential unlock, rewards, seven teams/stamps and lossless native indexed art')

saffron=gyms['saffron']
assert saffron['requiresBadge']==4 and saffron['qualityRates']==[25,50,25]
assert [(p['nationalId'],p['level']) for p in saffron['enemies']]==[(122,48),(64,49),(65,50)]
for enemy in saffron['enemies']:
 assert roster[enemy['art']]['nationalId']==enemy['nationalId'] and roster[enemy['art']]['name']==enemy['name']
for name,size in [('sabrina',(64,64)),('marsh',(64,64)),('marsh-display',(48,48)),('saffron-map',(360,168)),('saffron-arena',(360,112)),('defeated-psychic',(136,48)),('door-team-5',(128,72))]:
 p=folder/f'gyms/{name}.bin';assert p.read_bytes()[1] in (9,10),p
 im=Image.open(p.with_suffix('.png')).convert('RGBA');assert im.size==size
 if name not in ('saffron-map','saffron-arena'):assert set(im.getchannel('A').getdata())=={0,255}
print('PASS: sixth gym integrated psychic defenders, sequential unlock/reward metadata, seven teams/stamps and lossless indexed art')

cinnabar=gyms['cinnabar']
assert cinnabar['requiresBadge']==5 and cinnabar['qualityRates']==[20,50,30]
assert [(p['nationalId'],p['level']) for p in cinnabar['enemies']]==[(38,53),(78,54),(59,55)]
for enemy in cinnabar['enemies']:
 assert roster[enemy['art']]['nationalId']==enemy['nationalId'] and roster[enemy['art']]['name']==enemy['name']
for name,size in [('blaine',(64,64)),('volcano',(64,64)),('volcano-display',(48,48)),('cinnabar-map',(360,168)),('cinnabar-arena',(360,112)),('defeated-fire',(136,48)),('door-team-6',(128,72))]:
 p=folder/f'gyms/{name}.bin';assert p.read_bytes()[1] in (9,10),p
 im=Image.open(p.with_suffix('.png')).convert('RGBA');assert im.size==size
 if name not in ('cinnabar-map','cinnabar-arena'):assert set(im.getchannel('A').getdata())=={0,255}
print('PASS: seventh gym integrated fire defenders, sequential unlock/reward metadata, seven teams/stamps and lossless indexed art')

viridian=gyms['viridian']
assert viridian['requiresBadge']==6 and viridian['qualityRates']==[15,50,35]
assert [(p['nationalId'],p['level']) for p in viridian['enemies']]==[(31,58),(34,59),(112,60)]
for enemy in viridian['enemies']:
 assert roster[enemy['art']]['nationalId']==enemy['nationalId'] and roster[enemy['art']]['name']==enemy['name']
for name,size in [('giovanni',(64,64)),('earth',(64,64)),('earth-display',(48,48)),('viridian-map',(360,168)),('viridian-arena',(360,112)),('defeated-ground',(136,48)),('door-team-7',(128,72))]:
 p=folder/f'gyms/{name}.bin';assert p.read_bytes()[1] in (9,10),p
 im=Image.open(p.with_suffix('.png')).convert('RGBA');assert im.size==size
 if name not in ('viridian-map','viridian-arena'):assert set(im.getchannel('A').getdata())=={0,255}
print('PASS: eighth gym integrated ground defenders, sequential unlock/reward metadata, eight teams/stamps and lossless indexed art')

from gym_balance import sync_balance
expected=json.loads(json.dumps(gyms));sync_balance(root,expected,roster);assert expected==gyms
assert gyms["levelCap"]==manifest["maxLevel"]==60
print("PASS: all eight Web gym teams/rewards match core;60 cap and151 form powers")

from balance_catalog import load as load_balance
balance=load_balance()
assert [x['price'] for x in manifest['scenes']]==[None]*13
assert [x['unlock'] for x in manifest['scenes']]==[{'type':'prosperity','requiredPets':n} for n in balance['sceneRequirements']]
print('PASS: all13 scenes unlock free by distinct Pokédex forms / prosperity thresholds')

from hospital_art import sprites as hospital_sprites,metadata as hospital_metadata
assert manifest['hospital']==hospital_metadata(root)
for name,expected in hospital_sprites(root).items():
 actual=Image.open(folder/f'hospital/{name}.png').convert(expected.mode)
 assert actual.size==expected.size and actual.tobytes()==expected.tobytes()
 if name in ('icon','joy'):assert set(actual.getchannel('A').getdata())=={0,255} and len(set(actual.getdata()))<=16
assert manifest['appMenu']['pages'][0][-1]=='hospital' and 'pokedex' in manifest['appMenu']['pages'][1] and manifest['appMenu']['pages'][0][1]=='training' and manifest['appMenu']['pages'][0][7]=='tarot'
print('PASS: hospital deterministic clinic/icon, source hashes, health-only recovery metadata and hospital/encyclopedia menu entries')

from hospital_scenery import generate as generate_hospital_scenery
with tempfile.TemporaryDirectory() as tmp:
 path=Path(tmp);(path/'main').mkdir();(path/'web').mkdir();generate_hospital_scenery(path)
 for name in ['main/hospital_scenery.h','web/hospital-scenery.mjs']:assert (path/name).read_text()==(root/name).read_text()
print('PASS: hospital pixel clinic furnishings, supply paths and patient positions agree in C and Web')

from training_art import sprites as training_sprites
for name,expected in training_sprites(root).items():
 actual=Image.open(folder/f'training/{name}.png').convert(expected.mode)
 assert actual.size==(180,180) and actual.tobytes()==expected.tobytes()
 assert len(set(actual.getdata()))<=32
print('PASS: all six training landscapes match reviewed source and native palette budget')

from battle_entrance import generate as generate_battle_entrance
with tempfile.TemporaryDirectory() as tmp:
 path=Path(tmp);(path/'core').mkdir();(path/'web').mkdir();generate_battle_entrance(path)
 for name in ['core/battle_entrance.h','web/battle-entrance.mjs']:assert (path/name).read_text()==(root/name).read_text()
print('PASS: native and Web ball release effects match their shared generator')

from item_drop import generate as generate_item_drop
with tempfile.TemporaryDirectory() as tmp:
 path=Path(tmp);(path/'core').mkdir();(path/'web').mkdir();generate_item_drop(path)
 for name in ['core/item_drop.h','web/item-drop.mjs']:assert (path/name).read_text()==(root/name).read_text()
small=Image.open(folder/'items/tree-essence-small.png').convert('RGBA')
assert small.size==(24,24) and small.tobytes()==Image.open(folder/'items/tree-essence.png').convert('RGBA').resize((24,24),Image.Resampling.NEAREST).tobytes()
assert Image.open(folder/'icons/5.png').size==(32,32)
print('PASS: shared item fall/bounce, 24px essence derivation and unchanged 32px egg')

# Compact result icons retain the reviewed shop artwork at its offline native size.
for i,item in enumerate(manifest['items']):
    expected=f'icons-small/{i}.png' if i<6 else item['image'].replace('.png','-small.png')
    assert item['summaryImage']==expected
    image=Image.open(folder/expected).convert('RGBA')
    assert image.size==(24,24)
    assert image.tobytes()==Image.open(folder/item['image']).convert('RGBA').resize((24,24),Image.Resampling.NEAREST).tobytes()
print('PASS: all nine training-result icons match native 24px shop derivations')
