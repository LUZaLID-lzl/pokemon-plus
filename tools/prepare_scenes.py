#!/usr/bin/env python3
"""Update scenes without publishing or modifying an unfinished pet batch."""
from pathlib import Path
import json, hashlib
from scene_art import scene_variants
from sacred_lake_art import build_front
from image_binary import write_bin
root=Path(__file__).resolve().parents[1];out=root/'sdcard/pokemon-plus'
changed={}
for i,source in enumerate(json.loads((root/'assets/scene-sources.json').read_text())['sources']):
    variants=scene_variants(root,source)
    if i==12:variants['scene-fronts']=build_front(variants['scenes'])
    for kind,im in variants.items():
        p=out/kind/str(i)
        p.parent.mkdir(parents=True,exist_ok=True)
        im.save(p.with_suffix('.png'))
        write_bin(im,p.with_suffix('.bin'),alpha=kind=='scene-fronts',indexed=kind in ('scenes','thumbs','strips','scene-fronts'))
        for suffix in ('.png','.bin'):
            file=p.with_suffix(suffix);rel=str(file.relative_to(out))
            changed[rel]={'path':rel,'bytes':file.stat().st_size,'sha256':hashlib.sha256(file.read_bytes()).hexdigest()}
for p in (out/'manifest.json',root/'web/manifest.json'):
    manifest=json.loads(p.read_text())
    files={f['path']:f for f in manifest['files']}
    files.update(changed)
    manifest['files']=sorted(files.values(),key=lambda f:f['path'])
    manifest['scenes']=[{'id':i,'name':n,'image':f'scenes/{i}.png','price':None if i==12 else 0 if i==0 else 100+i*20,**({'unlock':{'type':'all-scenes','requires':list(range(12))}} if i==12 else {}),'status':'integrated'} for i,n in enumerate(['清晨庭院','樱花小院','晴日海岸','林间瀑布','秋日木林','雪落小镇','水晶洞穴','蘑菇秘境','星空山丘','沙漠绿洲','雨中村庄','天空岛','神树镜湖'])]
    manifest['sceneDisplay']=json.loads((root/'assets/scene-display.json').read_text())
    p.write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
print(f'Updated {len(changed)} scene files; pet roster/assets and release packages preserved')
