"""Offline pixel exports from the reviewed MobiIoT clinic paintings."""
from pathlib import Path
from PIL import Image,ImageEnhance
from hospital_scenery import generate as generate_scenery
import json,hashlib
ROOT=Path(__file__).resolve().parents[1]
def sprites(root=ROOT):
    room=Image.open(root/'assets/source/hospital-room-v1.png').convert('RGB')
    room=room.resize((120,120),Image.Resampling.NEAREST)
    room=ImageEnhance.Brightness(room).enhance(.68)
    room=room.quantize(colors=32,method=Image.Quantize.MEDIANCUT).convert('RGB').resize((360,360),Image.Resampling.NEAREST)
    icon=Image.open(root/'assets/source/hospital-icon-v1.png').convert('RGBA')
    icon.putalpha(icon.getchannel('A').point(lambda a:255 if a>=200 else 0))
    icon=icon.crop(icon.getbbox());icon.thumbnail((28,28),Image.Resampling.NEAREST)
    icon=icon.quantize(colors=15,method=Image.Quantize.FASTOCTREE).convert('RGBA')
    icon.putdata([(r,g,b,255) if a>=128 else (0,0,0,0) for r,g,b,a in icon.getdata()])
    canvas=Image.new('RGBA',(32,32));canvas.alpha_composite(icon,((32-icon.width)//2,(32-icon.height)//2))
    nurse=Image.open(root/'assets/source/hospital-joy-v1.png').convert('RGBA')
    nurse.putalpha(nurse.getchannel('A').point(lambda a:255 if a>=200 else 0))
    nurse=nurse.crop(nurse.getbbox());nurse.thumbnail((44,60),Image.Resampling.NEAREST)
    nurse=nurse.quantize(colors=15,method=Image.Quantize.FASTOCTREE).convert('RGBA')
    nurse.putdata([(r,g,b,255) if a>=128 else (0,0,0,0) for r,g,b,a in nurse.getdata()])
    nurse=nurse.resize((nurse.width*2,nurse.height*2),Image.Resampling.NEAREST)
    joy=Image.new('RGBA',(96,128));joy.alpha_composite(nurse,((96-nurse.width)//2,(128-nurse.height)//2))
    return {'room':room,'icon':canvas,'joy':joy}
def metadata(root=ROOT):
    return {'background':'hospital/room.png','icon':'hospital/icon.png','host':'hospital/joy.png','game':'heart-catch','goal':6,'durationMs':19200,'recovery':'health-only','freeHealth':60,'treatmentCoins':20,
            'sourceSha256':{name:hashlib.sha256((root/'assets/source'/name).read_bytes()).hexdigest() for name in ['hospital-room-v1.png','hospital-icon-v1.png','hospital-joy-v1.png']}}
def export_hospital(export,root=ROOT):
    generate_scenery(root)
    for name,im in sprites(root).items():export(im,'hospital',name,True,indexed=True)
    return metadata(root)
if __name__=='__main__':
    generate_scenery()
    from image_binary import write_bin
    folder=ROOT/'sdcard/pokemon-plus/hospital';folder.mkdir(exist_ok=True)
    for name,im in sprites().items():
        im.save(folder/(name+'.png'));write_bin(im,folder/(name+'.bin'),True,indexed=True)
    for file in ['sdcard/pokemon-plus/manifest.json','web/manifest.json']:
        p=ROOT/file;m=json.loads(p.read_text());m['hospital']=metadata()
        m['appMenu']['pages']=[['travel','tarot','battle','warehouse','scenes','fishing','steps','collection','hospital'],['badges','legend','settings','pokedex']]
        m['appMenu']['labels']['hospital']='宠物医院';m['appMenu']['icons']['hospital']='hospital/icon.png'
        from training_art import patch_manifest
        patch_manifest(m)
        m['files']=[x for x in m['files'] if not x['path'].startswith('hospital/')]
        for f in sorted(folder.iterdir()):m['files'].append({'path':'hospital/'+f.name,'bytes':f.stat().st_size,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()})
        m['files'].sort(key=lambda x:x['path']);p.write_text(json.dumps(m,ensure_ascii=False,indent=2))
    record={'service':'MobiIoT','reviewed':True,**metadata(),'recipe':'room120px/32colors/brightness0.68/nearest3x; icon32px/15colors/binary alpha200; joy96x128/logical60px/15colors/nearest2x'}
    record['generations']={}
    for kind in ['room','icon','joy']:
        log=(ROOT/f'build/hospital-{kind}-generation.log').read_text()
        record['generations'][kind]={'model':log.split('Selected model: ')[-1].splitlines()[0],'attempts':log.count('Selected model:'),'prompt':f'prompts/hospital-{kind}-v1.txt'}
    (ROOT/'assets/hospital-generation.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n')
    print('PASS: clinic room, icon and both manifests exported')
