#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build device-native RGB565(A8) binary images and a truthful resource manifest."""
from pathlib import Path
from pet_dialogue import generate as generate_pet_dialogue
from PIL import Image, ImageEnhance, ImageOps, ImageFilter
import struct,json,hashlib,zipfile
from pet_frames import build_frames
from animation_contract import write_header
from image_binary import write_bin
from move_catalog import skill_manifest, write_core as write_moves_core
from asset_pet_canvas import encode as trim_pet_canvas
from scene_art import scene_variants
from sacred_lake_art import build_front
from collection_assets import export_collection, load_catalog
from travel_art import export_travel, load_catalog as load_travel_catalog
from hatch_boost_art import build_icon as build_hatch_boost_icon
from capture_ball_art import build_icon as build_capture_ball_icon
from tree_essence_art import build_icon as build_tree_essence_icon, ITEM as TREE_ESSENCE_ITEM
from shop_art import export_shop
from gym_icon_art import build_icon as build_gym_icon
from menu_icon_art import build_icon as build_menu_icon
from menu_icon_art import ICON_REVISIONS
from menu_background_art import build_background as build_menu_background
from menu_background_art import background_manifest
from fishing_art import export_fishing
from hospital_art import export_hospital
from training_art import export_training,patch_manifest as patch_training_manifest
from gym_art import build_map
from cerulean_art import build_cerulean
from vermilion_art import build_vermilion
from celadon_art import build_celadon
from fuchsia_art import build_fuchsia
from saffron_art import build_saffron
from cinnabar_art import build_cinnabar
from viridian_art import build_viridian
from gym_stamp_art import build_gym_stamp
from gym_door_art import build_gym_doors
from tumbleweed_art import export_frames as export_tumbleweed
from habitat_catalog import load_catalog as load_habitats, write_header as write_habitats
from scene_nest_art import export_nests, manifest_nests
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'sdcard/pokemon-plus'
exported=set()
roster=json.loads((ROOT/'assets/pet-roster.json').read_text())
# Reject incomplete batches before modifying generated headers or published assets.
missing=[]
for pet in roster:
    dex=pet['nationalId']
    template_root=ROOT/f'assets/animation-templates/{dex:03}'
    template_path=template_root/'manifest.json'
    if not template_path.exists() and dex==25:
        continue # Preserve existing Pikachu until its new image service task succeeds.
    if not template_path.exists():
        missing.append(f'{dex:03}: missing reviewed 48-frame template')
        continue
    template=json.loads(template_path.read_text())
    actions=template.get('actions',{})
    if set(actions)!={'idle','walk','happy','eat','sleep','attack'} or any(len(a.get('frames',[]))!=8 for a in actions.values()):
        missing.append(f'{dex:03}: incomplete action rows')
    else:
        for action in actions.values():
            for file in action['frames']:
                if not (template_root/file).is_file(): missing.append(f'{dex:03}: missing {file}')
for x in (x for x in load_catalog()['items'] if x['id']!=583):
    if not (ROOT/f'assets/collection/{x["id"]:03}.png').is_file(): missing.append(f'collection {x["id"]:03}: missing reviewed image')
for x in load_travel_catalog(ROOT)['items']:
    if not (ROOT/'assets'/x['source']).is_file(): missing.append('travel background: missing '+x['source'])
if not (ROOT/'assets/source/hatch-boost-v1.png').is_file(): missing.append('hatch boost: missing reviewed icon')
if not (ROOT/'assets/source/capture-ball-v1.png').is_file(): missing.append('capture ball: missing reviewed icon')
if not (ROOT/'assets/source/gym-icon-v1.png').is_file(): missing.append('gym: missing reviewed menu icon')
if not (ROOT/'assets/source/oasis-tumbleweed-v2.png').is_file(): missing.append('tumbleweed: missing reviewed sprite')
for kind in ('warehouse','scene'):
    if not (ROOT/f'assets/source/{kind}-icon-v{ICON_REVISIONS[kind]}.png').is_file(): missing.append(f'{kind}: missing reviewed menu icon')
if missing:
    raise SystemExit('Asset build stopped before publishing:\n'+'\n'.join(missing))
generate_pet_dialogue()
names=[p['name'] for p in roster]
# Core/UI share the same national-dex order and legacy art/save mapping.
dex_roster=sorted(roster,key=lambda p:p['nationalId'])
dex_header='#pragma once\n/* Generated from assets/pet-roster.json by prepare_assets.py. */\n#define PP_DEX_COUNT '+str(len(dex_roster))+'\n#define PP_DEX_ENTRIES '+chr(92)+'\n'
dex_header+=(', '+chr(92)+'\n').join('  {%u, %u, %u, %u, "%s"}'%(p['nationalId'],p['species'],p['stage'],p['id'],p['name']) for p in dex_roster)+'\n'
(ROOT/'core/pokedex_roster.h').write_text(dex_header)
write_habitats()
scenes=['清晨庭院','樱花小院','晴日海岸','林间瀑布','秋日木林','雪落小镇','水晶洞穴','蘑菇秘境','星空山丘','沙漠绿洲','雨中村庄','天空岛','神树镜湖']


def export(im,kind,name,alpha=False,indexed=None):
    target=OUT/kind;target.mkdir(parents=True,exist_ok=True)
    im.save(target/(name+'.png'));write_bin(im,target/(name+'.bin'),alpha,indexed=kind in ('pets','pets-left','scenes','thumbs','strips','collection','travel-scenes') if indexed is None else indexed)
    if kind in ('pets','pets-left'):
        native=target/(name+'.bin');native.write_bytes(trim_pet_canvas(native.read_bytes()))
    exported.update([target/(name+'.png'),target/(name+'.bin')])

layout=json.loads((ROOT/'assets/atlas-layout.json').read_text())
# Remove obsolete four-pose avatars left by earlier builds.
for stale in (OUT/'avatars').glob('*'):
    if stale.is_file():stale.unlink()
# Stable legacy/save IDs map to reviewed national-dex master art.
pet_sources=[p['source'] for p in roster]
animation=json.loads((ROOT/'assets/pet-animation.json').read_text())
animation['framesPerPet']=48
animation['actions']={a:list(range(i*8,i*8+8)) for i,a in enumerate(('idle','walk','happy','eat','sleep','attack'))}
animation.pop('motionOffsets',None)
animation['speciesOverrides']={}
templates=[]
for pet in roster:
    row=pet['id'];source=pet['source'];dex=pet['nationalId']
    template_root=ROOT/f'assets/animation-templates/{dex:03}'
    if dex==25 and not (template_root/'manifest.json').exists():
        frames=build_frames(Image.open(ROOT/'assets'/source))
        animation['speciesOverrides'][str(row)]={'nationalId':25,'framesPerPet':24,'actions':{a:list(range(i*4,i*4+4)) for i,a in enumerate(animation['actions'])},'source':source,'status':'legacy-source-translation; new 48-pose master pending service recovery'}
        for frame,canvas in enumerate(frames):
            export(ImageOps.mirror(canvas),'pets',f'{row}-{frame}',True)
            export(canvas,'pets-left',f'{row}-{frame}',True)
        export(Image.open(ROOT/'assets'/source).resize((40,40),Image.Resampling.NEAREST),'avatars',f'{row}-0',True,indexed=dex>=137)
        continue
    template=json.loads((template_root/'manifest.json').read_text())
    frames=[Image.open(template_root/f).convert('RGBA') for a in template['actions'].values() for f in a['frames']]
    animation['speciesOverrides'][str(row)]={'nationalId':dex,'framesPerPet':48,'actions':{a:list(range(i*8,i*8+8)) for i,a in enumerate(template['actions'])},'durationsMs':{a:x['durationsMs'] for a,x in template['actions'].items()},'modes':{a:x['mode'] for a,x in template['actions'].items()},'source':f'animation-templates/{dex:03}/manifest.json'}
    templates.append({'nationalId':dex,'name':pet['name'],'artId':row,'path':f'../assets/animation-templates/{dex:03}/'})
    for frame,canvas in enumerate(frames):
        export(ImageOps.mirror(canvas),'pets',f'{row}-{frame}',True)
        export(canvas,'pets-left',f'{row}-{frame}',True)
    export(Image.open(ROOT/'assets'/source).resize((40,40),Image.Resampling.NEAREST),'avatars',f'{row}-0',True,indexed=dex>=137)
animation.update(version=9,artCount=len(roster),maxFramesPerPet=48,sources=pet_sources,derivation=f'001-024 and 026-{max(p["nationalId"] for p in roster):03}: articulated 48-frame animation; 025: preserved original 24 translation slots pending new master; native lossless I4/I8 storage retains all RGBA pixels')
(ROOT/'assets/pet-animation.json').write_text(json.dumps(animation,ensure_ascii=False,indent=2)+'\n')
(ROOT/'assets/animation-templates/index.json').write_text(json.dumps(sorted(templates,key=lambda x:x['nationalId']),ensure_ascii=False,indent=2)+'\n')
write_header(ROOT/'main/pet_animation.h',animation)
for dex in [1,4,7]:
    portrait=Image.open(ROOT/f'assets/pokemon/gen1-001-009/{dex:03}.png')
    export(portrait.resize((40,40),Image.Resampling.NEAREST).resize((80,80),Image.Resampling.NEAREST),'starters-large',f'{dex:03}',True,indexed=True)
scene_sources=json.loads((ROOT/'assets/scene-sources.json').read_text())
for i,source in enumerate(scene_sources['sources']):
    for kind,im in scene_variants(ROOT,source).items():
        export(im,kind,str(i))
        if i==12 and kind=="scenes":export(build_front(im),"scene-fronts","12",True,indexed=True)
icons=Image.open(ROOT/'assets/source/icons.png').convert('RGBA')
xs=[0,314,627,940,1254];ys=[0,336,631,915,1254]
for i in range(16):
    c=i%4;r=i//4;im=icons.crop((xs[c],ys[r],xs[c+1],ys[r+1]));im.putalpha(im.getchannel('A').point(lambda a:255 if a>=200 else 0));im=im.crop(im.getbbox());im.thumbnail((29,29),Image.Resampling.NEAREST)
    canvas=Image.new('RGBA',(32,32));canvas.alpha_composite(im,((32-im.width)//2,(32-im.height)//2));export(canvas,'icons',str(i),True)
    export(canvas.resize((24,24),Image.Resampling.NEAREST),'icons-small',str(i),True)
    if i in (10,12):export(canvas.resize((80,80),Image.Resampling.NEAREST),'icons-large',str(i),True)
# Dedicated potion icon shares the ordinary 32px shop item treatment.
export(build_hatch_boost_icon(ROOT),'items','hatch-boost',True,indexed=True)
export(build_capture_ball_icon(ROOT),'items','capture-ball',True,indexed=True)
export(build_hatch_boost_icon(ROOT).resize((24,24),Image.Resampling.NEAREST),'items','hatch-boost-small',True,indexed=True)
export(build_capture_ball_icon(ROOT).resize((24,24),Image.Resampling.NEAREST),'items','capture-ball-small',True,indexed=True)
export(build_tree_essence_icon(ROOT),'items','tree-essence',True,indexed=True)
export(build_tree_essence_icon(ROOT).resize((24,24),Image.Resampling.NEAREST),'items','tree-essence-small',True,indexed=True)
export_shop(export, ROOT)
export(build_menu_background(ROOT),'ui','menu-bg',indexed=True)
export(build_gym_icon(ROOT),'hud','gym',True,indexed=True)
for kind in ('warehouse','scene'):
    export(build_menu_icon(kind,ROOT),'hud',kind,True,indexed=True)
# Exclusive evolution-choice gift, from reviewed MobiIoT master.
heart=Image.open(ROOT/'assets/source/pure-heart-v1.png').convert('RGBA')
heart.putalpha(heart.getchannel('A').point(lambda a:255 if a>=200 else 0))
heart=heart.crop(heart.getbbox());heart.thumbnail((28,28),Image.Resampling.NEAREST)
heart=heart.quantize(colors=12,method=Image.Quantize.FASTOCTREE).convert('RGBA')
heart_canvas=Image.new('RGBA',(32,32));heart_canvas.alpha_composite(heart,((32-heart.width)//2,(32-heart.height)//2))
export(heart_canvas,'collection','pure-heart',True)
export(heart_canvas.resize((20,20),Image.Resampling.NEAREST),'collection','pure-heart-small',True)
# Walking icon generated by MobiIoT; keep square pixels and binary alpha.
foot=Image.open(ROOT/'assets/source/pedometer-icon-v12.png').convert('RGBA')
foot.putalpha(foot.getchannel('A').point(lambda a:255 if a>=200 else 0))
foot=foot.crop(foot.getbbox());foot.thumbnail((28,28),Image.Resampling.NEAREST)
foot_canvas=Image.new('RGBA',(32,32));foot_canvas.alpha_composite(foot,((32-foot.width)//2,(32-foot.height)//2))
export(foot_canvas,'hud','steps',True)
# Dedicated MobiIoT Pokedex device icon; export one native 32px menu asset.
dex=Image.open(ROOT/'assets/source/pokedex-icon-v16.png').convert('RGBA')
dex.putalpha(dex.getchannel('A').point(lambda a:255 if a>=200 else 0))
dex=dex.crop(dex.getbbox());dex.thumbnail((28,28),Image.Resampling.NEAREST)
# Compact the generated colors without smoothing the pixel grid.
dex=dex.quantize(colors=16,method=Image.Quantize.FASTOCTREE).convert('RGBA')
dex_canvas=Image.new('RGBA',(32,32));dex_canvas.alpha_composite(dex,((32-dex.width)//2,(32-dex.height)//2))
export(dex_canvas,'hud','pokedex',True)
# Recolor existing generated icon silhouettes for the reference's flat HUD.
for n,(source,tint) in enumerate([(14,(164,252,70)),(6,(255,215,50)),(7,(31,218,255))]):
    src=Image.open(OUT/'icons'/f'{source}.png').convert('RGBA')
    image=Image.new('RGBA',(24,24));src.thumbnail((20,20),Image.Resampling.NEAREST)
    data=[]
    for r,g,b,a in src.getdata():
        bright=max(r,g,b);data.append((*tint,a) if bright>100 else (15,48,98,a))
    src.putdata(data);image.alpha_composite(src,((24-src.width)//2,(24-src.height)//2));export(image,'nav',str(n),True)
coin=Image.open(OUT/'icons/15.png').resize((16,16),Image.Resampling.NEAREST);export(coin,'hud','coin',True)
coin_large=Image.open(OUT/'icons/15.png').resize((24,24),Image.Resampling.NEAREST);export(coin_large,'hud','coin-large',True)
# Four generated elemental silhouettes, preprocessed to native16px icons.
types=Image.open(ROOT/'assets/source/starter-types-v15.png').convert('RGBA')
for i,name in enumerate(['grass','poison','fire','water']):
    cell=types.crop((i*types.width//4,0,(i+1)*types.width//4,types.height))
    cell.putalpha(cell.getchannel('A').point(lambda a:255 if a>=200 else 0))
    cell=cell.crop(cell.getbbox());cell.thumbnail((14,14),Image.Resampling.NEAREST)
    canvas=Image.new('RGBA',(16,16));canvas.alpha_composite(cell,((16-cell.width)//2,(16-cell.height)//2))
    if name!='poison':export(canvas.resize((32,32),Image.Resampling.NEAREST),'types-large',name,True)
# MobiIoT VFX atlas: 4 rows/elements, 4 columns/phases. Transparent
# gutters are retained; each device frame is 48px with binary alpha.
fx=Image.open(ROOT/'assets/source/move-effects-v4.png').convert('RGBA')
for element in range(4):
    for phase in range(4):
        cell=fx.crop((phase*fx.width//4,element*fx.height//4,
                      (phase+1)*fx.width//4,(element+1)*fx.height//4))
        cell=cell.resize((48,48),Image.Resampling.NEAREST)
        cell.putalpha(cell.getchannel('A').point(lambda a:255 if a>=160 else 0))
        export(cell,'effects',f'{element}-{phase}',True)
# First-gym UI/environment assets; opponents reuse the complete reviewed pet bank.
export_tumbleweed(export, ROOT)
export_nests(export, ROOT)
build_map(ROOT)
build_cerulean(ROOT)
build_vermilion(ROOT)
build_celadon(ROOT)
build_fuchsia(ROOT)
build_saffron(ROOT)
build_cinnabar(ROOT)
build_viridian(ROOT)
build_gym_stamp(ROOT)
gym_door_layout=build_gym_doors(ROOT)
for name in ('brock','boulder','pewter','badge-icon','badge-display','enemy-74','enemy-75','enemy-95','arena','map','misty','cascade','cascade-display','cerulean-map','cerulean-arena','surge','thunder','thunder-display','vermilion-map','vermilion-arena','defeated-rock','defeated-water','defeated-electric','door-team-0','door-team-1','door-team-2'):
    im=Image.open(ROOT/f'assets/gyms/{name}.png').convert('RGBA')
    export(im,'gyms',name,name not in ('arena','map','cerulean-map','cerulean-arena','vermilion-map','vermilion-arena'),indexed=name in ('map','cerulean-map','vermilion-map','brock','misty','surge','cerulean-arena','vermilion-arena','door-team-0','door-team-1','door-team-2','pewter','boulder','cascade','thunder','badge-icon','enemy-74','enemy-75','enemy-95','defeated-rock','defeated-water','defeated-electric'))
# New gym assets use the same lossless indexed format as the existing pet bank.
# Three old city maps also use exact indexed pixels to make room without changing the partition.
for name in ('erika','rainbow','rainbow-display','celadon-map','celadon-arena','defeated-grass','door-team-3','koga','soul','soul-display','fuchsia-map','fuchsia-arena','defeated-poison','door-team-4','sabrina','marsh','marsh-display','saffron-map','saffron-arena','defeated-psychic','door-team-5','blaine','volcano','volcano-display','cinnabar-map','cinnabar-arena','defeated-fire','door-team-6','giovanni','earth','earth-display','viridian-map','viridian-arena','defeated-ground','door-team-7'):
    im=Image.open(ROOT/f'assets/gyms/{name}.png').convert('RGBA')
    export(im,'gyms',name,name not in ('celadon-map','celadon-arena','fuchsia-map','fuchsia-arena','saffron-map','saffron-arena','cinnabar-map','cinnabar-arena','viridian-map','viridian-arena'),indexed=True)
animation=json.loads((ROOT/'assets/pet-animation.json').read_text())
(ROOT/'web/animation.json').write_text(json.dumps(animation,ensure_ascii=False,indent=2))
manifest={'version':2,'style':'reviewed-native-pixel-v6','generator':'mobiiot-imagegen','pets':[{'id':i,'name':n,'frames':[f'pets/{i}-{f}.png' for f in range(24 if roster[i]['nationalId']==25 and not (ROOT/'assets/animation-templates/025/manifest.json').exists() else 48)],'keyPoses':(1 if roster[i]['nationalId']==25 and not (ROOT/'assets/animation-templates/025/manifest.json').exists() else 48),'sourceFacing':'left','nationalId':roster[i]['nationalId'],'species':roster[i]['species'],'stage':roster[i]['stage'],'source':pet_sources[i],'sourceSha256':hashlib.sha256((ROOT/'assets'/pet_sources[i]).read_bytes()).hexdigest(),'animationSource':'integer translation of reviewed native art; no resizing, recoloring or body-part warping; sleep is still rest','status':'integrated'} for i,n in enumerate(names)],'scenes':[{'id':i,'name':n,'image':f'scenes/{i}.png','price':None if i==12 else 0 if i==0 else 100+i*20,**({'unlock':{'type':'all-scenes','requires':list(range(12))}} if i==12 else {}),'status':'integrated'} for i,n in enumerate(scenes)],'icons':[{'id':i,'image':f'icons/{i}.png'} for i in range(16)],'collection':export_collection(export),'skills':[],'items':[{'name':n,'price':p,'image':f'icons/{i}.png'} for i,(n,p) in enumerate([('树果',12),('肥皂',18),('药水',30),('玩具',25),('精力饼干',20),('宠物蛋',180)])]}
manifest['items'].append({'name':'孵化药剂','price':120,'image':'items/hatch-boost.png','effect':'hatchBoost','description':'存入背包，使用后立即孵化场上的宠物蛋；无蛋时不消耗'})
manifest['items'].append({'name':'咕噜球','price':30,'image':'items/capture-ball.png','effect':'captureBall','description':'野外获胜后使用1个，必定捕捉'})
manifest['items'].append(TREE_ESSENCE_ITEM)
moves=json.loads((ROOT/'assets/moves.json').read_text())
manifest['skills']=skill_manifest(roster)
manifest['maxLevel']=60
manifest['moveBalance']=json.loads((ROOT/'assets/moves.json').read_text())['balance']
write_moves_core()
for i,item in enumerate(manifest['items']):
    item['summaryImage']=f'icons-small/{i}.png' if i<6 else item['image'].replace('.png','-small.png')
manifest['walking']={'icon':'hud/steps.png','milestoneSteps':100,'coinRates':[5,8,12],'xpPerPetRates':[3,4,6],'tierStarts':[100,1000,5000]}
manifest['pokedex']={'icon':'hud/pokedex.png','pageSize':10}
manifest['habitats']=load_habitats()
manifest['egg']={'minSeconds':7200,'maxSeconds':14400}
manifest['sceneNests']=manifest_nests(ROOT)
manifest['legendaryChallenges']=json.loads((ROOT/'assets/legendary-challenges.json').read_text())
legend_ids={p['nationalId'] for p in manifest['legendaryChallenges']['pets']}
for pet in manifest['pets']:
    pet['acquisition']='legendary-egg' if pet['nationalId'] in legend_ids else 'ordinary'
manifest['egg']['ordinaryFamilies']=len({p['species'] for p in manifest['pets'] if p['nationalId'] not in legend_ids})
manifest['gyms']={'levelCap': 60, 'teamSize': 3, 'qualityRates': [75, 20, 5], 'qualityIds': [2, 3, 4], 'pewter': {'leader': '小刚', 'city': '尼比市', 'type': '岩石', 'badge': '灰色徽章'}}
manifest['gyms']['icon']='hud/gym.png'
manifest['appMenu']={'pages':[['travel','tarot','battle','warehouse','scenes','fishing','steps','collection','hospital'],['badges','legend','settings','pokedex']], 'labels':{'travel':'旅行','tarot':'塔罗牌','battle':'道馆','warehouse':'仓库','scenes':'场景','legend':'神兽','steps':'计步器','collection':'收藏品','pokedex':'宠物图鉴','badges':'徽章','settings':'设置','fishing':'钓鱼','hospital':'宠物医院'}, 'icons':{'warehouse':'hud/warehouse.png','scenes':'hud/scene.png','fishing':'fishing/icon.png','hospital':'hospital/icon.png'}}
manifest['menuBackground']=background_manifest()
manifest['fishing']=export_fishing(export)
manifest['hospital']=export_hospital(export)
manifest['training']=export_training(export)
patch_training_manifest(manifest)
manifest['gyms']['cerulean']={'leader': '小霞', 'city': '华蓝市', 'type': '水', 'badge': '蓝色徽章', 'requiresBadge': 0, 'qualityRates': [65, 28, 7]}
manifest['gyms']['vermilion']={'leader': '马志士', 'city': '枯叶市', 'type': '电', 'badge': '橙色徽章', 'requiresBadge': 1, 'qualityRates': [55, 35, 10]}
manifest['gyms']['celadon']={'leader': '莉佳', 'city': '玉虹市', 'type': '草', 'badge': '彩虹徽章', 'requiresBadge': 2, 'qualityRates': [45, 40, 15]}
manifest['gyms']['fuchsia']={'leader': '阿桔', 'city': '浅红市', 'type': '毒', 'badge': '粉红徽章', 'requiresBadge': 3, 'qualityRates': [35, 45, 20]}
manifest['gyms']['saffron']={'leader': '娜姿', 'city': '金黄市', 'type': '超能', 'badge': '金色徽章', 'requiresBadge': 4, 'qualityRates': [25, 50, 25]}
manifest['gyms']['cinnabar']={'leader': '夏伯', 'city': '红莲岛', 'type': '火', 'badge': '深红徽章', 'requiresBadge': 5, 'qualityRates': [20, 50, 30]}
manifest['gyms']['viridian']={'leader': '坂木', 'city': '常青市', 'type': '地面', 'badge': '绿色徽章', 'requiresBadge': 6, 'qualityRates': [15, 50, 35]}
# Share authoritative core gym balance with Web; keep artwork metadata above.
from gym_balance import sync_balance
sync_balance(ROOT,manifest['gyms'],roster)
manifest['gyms']['clearedStamp']={'images':['gyms/defeated-rock.png','gyms/defeated-water.png','gyms/defeated-electric.png','gyms/defeated-grass.png','gyms/defeated-poison.png','gyms/defeated-psychic.png','gyms/defeated-fire.png','gyms/defeated-ground.png'],'size':[136,48],'position':[112,86],'condition':'selected gym badge bit','sources':['assets/source/gyms/defeated-themed-atlas-v1.png','assets/source/gyms/celadon-atlas-v1.png','assets/source/gyms/fuchsia-atlas-v1.png','assets/source/gyms/saffron-atlas-v1.png','assets/source/gyms/cinnabar-atlas-v1.png','assets/source/gyms/viridian-atlas-v1.png']}
manifest['gyms']['mapNavigation']={'wrap':True,'left':[18,158,36,44],'right':[306,158,36,44]}
manifest['gyms']['doorTeams']={'images':[f'gyms/door-team-{i}.png' for i in range(len(gym_door_layout['gyms']))],'size':gym_door_layout['size'],'positions':gym_door_layout['positions'],'condition':'selected gym badge not obtained','members':'leader and three existing defenders','sources':[gym_door_layout['source'],'assets/source/gyms/celadon-atlas-v1.png','assets/source/gyms/fuchsia-atlas-v1.png','assets/source/gyms/saffron-atlas-v1.png','assets/source/gyms/cinnabar-atlas-v1.png','assets/source/gyms/viridian-atlas-v1.png'],'placement':'alpha-cropped individual foot anchors on measured dry floor, sorted by depth'}
# The roster-to-type mapping is also shared by host tests and browser preview.
pet_types=json.loads((ROOT/'assets/pet-types.json').read_text())['types']
manifest['petTypes']=pet_types
rows=[]
for pet in roster:
    ts=pet_types[str(pet['nationalId'])]
    rows.append('  {PP_'+ts[0]+', '+('PP_'+ts[1] if len(ts)>1 else '-1')+'}')
(ROOT/'core/pet_types.h').write_text('#pragma once\n/* Generated from assets/pet-types.json and pet-roster.json. */\nstatic const int8_t pp_pet_types[PP_DEX_COUNT][2]={\n'+',\n'.join(rows)+'\n};\n')
manifest['travel']=export_travel(ROOT,export)
for pet in manifest['pets']:
    if pet['keyPoses']==1:
        pet.update(animation=animation['speciesOverrides'][str(pet['id'])],animationSource='preserved original 24 translation slots; new 48-pose master pending image service')
        continue
    pet.update(animation=animation['speciesOverrides'][str(pet['id'])],template=f'assets/animation-templates/{pet["nationalId"]:03}/manifest.json',animationSource='generated articulated 48-frame animation from approved native reference')
manifest['animation']=animation
manifest['evolution']=json.loads((ROOT/'assets/evolution.json').read_text())
# Output directory is derived, not a source of truth. Prune stale exports so
# removed sizes/icons cannot silently remain in Flash or the SD package.
pruned=[]
for path in sorted(OUT.rglob('*')):
    if path.is_file() and path.suffix in ('.png','.bin') and path not in exported:
        pruned.append({'path':str(path.relative_to(OUT)),'bytes':path.stat().st_size})
        path.unlink()
for path in sorted(OUT.rglob('*'),reverse=True):
    if path.is_dir() and not list(path.iterdir()):path.rmdir()
if pruned:
    report_path=ROOT/'assets/cleanup-20261023.json'
    report=json.loads(report_path.read_text()) if report_path.exists() else {}
    report['prunedRuntimeExports']=pruned
    report['runtimeBytesRemoved']=sum(p['bytes'] for p in pruned)
    report_path.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
manifest['files']=[{'path':str(p.relative_to(OUT)),'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted(OUT.rglob('*')) if p.is_file() and p.name!='manifest.json']
from balance_catalog import generate as generate_balance, patch_manifest as patch_balance
generate_balance();patch_balance(manifest)
(OUT/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2))
(ROOT/'web/manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2))
from battle_type_icons import generate as generate_battle_types
generate_battle_types()
from battle_relation import generate as generate_battle_relations
generate_battle_relations()
(ROOT/'dist').mkdir(exist_ok=True)
with zipfile.ZipFile(ROOT/'dist/pokemon-plus-sd.zip','w',zipfile.ZIP_DEFLATED) as z:
    for p in sorted(OUT.rglob('*')):
        if p.is_file():z.write(p,p.relative_to(OUT.parent))
print(f'Packaged {len(manifest["files"])} images, {len(roster)} forms: {len(templates)} articulated templates, 025 legacy 24 slots, {len(scenes)} scenes')

# Masks/anchors are sampled only after all final scene paintings are exported.
import runpy
runpy.run_path(str(ROOT/'tools/prepare_scene_motion.py'))
runpy.run_path(str(ROOT/'tools/prepare_scene_effects.py'))
