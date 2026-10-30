"""Native, binary-alpha sprites derived from the reviewed MobiIoT fishing atlas."""
from pathlib import Path
from PIL import Image
import hashlib
import json
from image_binary import write_bin
ROOT=Path(__file__).resolve().parents[1]

def write_catalog(root=ROOT):
    data=json.loads((root/'assets/fish-catalog.json').read_text())
    (root/'web/fish-catalog.js').write_text('export const fishCatalog='+json.dumps(data,ensure_ascii=False,separators=(',',':'))+';\n')
    (root/'core/fish_catalog.h').write_text('#pragma once\n#define PP_FISH_COUNT 27\ntypedef struct {const char *name;unsigned weight,coins,rarity;} pp_fish_entry;\nstatic const pp_fish_entry pp_fish_catalog[PP_FISH_COUNT]={\n'+''.join(' {"%s",%u,%u,%u},\n'%(x['name'],x['weight'],x['coins'],x['rarity'])for x in data)+'};\n')

def build_sprites(root=ROOT):
    sheet=Image.open(root/'assets/source/fishing-atlas-v1.png').convert('RGBA')
    result={}
    for i,name in enumerate(('icon','fish-0','fish-1','fish-2')):
        x,y=i%2,i//2
        cell=sheet.crop((x*sheet.width//2,y*sheet.height//2,(x+1)*sheet.width//2,(y+1)*sheet.height//2))
        cell.putalpha(cell.getchannel('A').point(lambda a:255 if a>=200 else 0))
        assert cell.getbbox()
        cell=cell.crop(cell.getbbox());cell.thumbnail((29,29) if i==0 else (24,16),Image.Resampling.NEAREST)
        cell=cell.quantize(colors=15,method=Image.Quantize.FASTOCTREE).convert('RGBA')
        cell.putdata([(r,g,b,255) if a>=128 else (0,0,0,0) for r,g,b,a in cell.getdata()])
        canvas=Image.new('RGBA',(32,32) if i==0 else (24,16))
        canvas.alpha_composite(cell,((canvas.width-cell.width)//2,(canvas.height-cell.height)//2))
        if i:canvas=canvas.resize((96,64),Image.Resampling.NEAREST)
        assert len(set(canvas.getdata()))<=16
        result[name]=canvas
    # Generated art is not an exact grid: isolate whole connected fish first,
    # then use its centre to assign a slot. This preserves fins across cell edges.
    for version,rows,offset in [('v2',3,3),('v3',5,12)]:
        atlas=Image.open(root/f'assets/source/fishing-fish-{version}.png').convert('RGBA')
        w,h=atlas.size
        mask=bytearray(1 if a>=200 else 0 for a in atlas.getchannel('A').getdata())
        found={}
        for first in range(w*h):
            if not mask[first]:continue
            mask[first]=0;queue=[first];part=[]
            while queue:
                point=queue.pop();part.append(point);x,y=point%w,point//w
                for xx,yy in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
                    n=yy*w+xx
                    if 0<=xx<w and 0<=yy<h and mask[n]:mask[n]=0;queue.append(n)
            if len(part)<2000:continue
            x1=min(n%w for n in part);y1=min(n//w for n in part)
            x2=max(n%w for n in part)+1;y2=max(n//w for n in part)+1
            slot=int((y1+y2)*rows/(2*h))*3+int((x1+x2)*3/(2*w))
            assert slot not in found,(version,slot)
            cell=atlas.crop((x1,y1,x2,y2));alpha=Image.new('L',cell.size)
            pixels=alpha.load()
            for n in part:pixels[n%w-x1,n//w-y1]=255
            cell.putalpha(alpha);cell.thumbnail((24,16),Image.Resampling.NEAREST)
            cell=cell.quantize(colors=15,method=Image.Quantize.FASTOCTREE).convert('RGBA')
            cell.putdata([(r,g,b,255) if a>=128 else (0,0,0,0) for r,g,b,a in cell.getdata()])
            canvas=Image.new('RGBA',(24,16));canvas.alpha_composite(cell,((24-cell.width)//2,(16-cell.height)//2))
            found[slot]=canvas.resize((96,64),Image.Resampling.NEAREST)
        assert set(found)==set(range(rows*3)),(version,found.keys())
        for slot,canvas in found.items():result[f'fish-{slot+offset}']=canvas
    for i in range(27):result[f'thumb-{i}']=result[f'fish-{i}'].resize((48,32),Image.Resampling.NEAREST)
    controls=Image.open(root/'assets/source/fishing-controls-v1.png').convert('RGBA')
    for i,name in enumerate(('partner-icon','dex-icon')):
        cell=controls.crop((i*controls.width//2,0,(i+1)*controls.width//2,controls.height))
        cell.putalpha(cell.getchannel('A').point(lambda a:255 if a>=200 else 0))
        assert cell.getbbox(),name
        cell=cell.crop(cell.getbbox());cell.thumbnail((28,28),Image.Resampling.NEAREST)
        cell=cell.quantize(colors=15,method=Image.Quantize.FASTOCTREE).convert('RGBA')
        cell.putdata([(r,g,b,255) if a>=128 else (0,0,0,0) for r,g,b,a in cell.getdata()])
        canvas=Image.new('RGBA',(32,32));canvas.alpha_composite(cell,((32-cell.width)//2,(32-cell.height)//2))
        assert len(set(canvas.getdata()))<=16
        result[name]=canvas
    pond=Image.open(root/'assets/source/fishing-pond-v1.png').convert('RGB')
    pond=pond.resize((120,120),Image.Resampling.NEAREST).quantize(colors=48,method=Image.Quantize.MEDIANCUT).convert('RGB')
    result['pond']=pond.resize((360,360),Image.Resampling.NEAREST)
    return result

def metadata(root=ROOT):
    return dict(source='source/fishing-atlas-v1.png',prompt='prompts/fishing-atlas-v1.txt',
                sourceSha256=hashlib.sha256((root/'assets/source/fishing-atlas-v1.png').read_bytes()).hexdigest(),
                background='fishing/pond.png',backgroundSource='source/fishing-pond-v1.png',
                backgroundSha256=hashlib.sha256((root/'assets/source/fishing-pond-v1.png').read_bytes()).hexdigest(),
                controls={'source':'source/fishing-controls-v1.png','sourceSha256':hashlib.sha256((root/'assets/source/fishing-controls-v1.png').read_bytes()).hexdigest(),'images':['fishing/partner-icon.png','fishing/dex-icon.png'],'generation':'fishing-controls-generation.json'},
                icon='fishing/icon.png',fish=json.loads((root/'assets/fish-catalog.json').read_text()),
                moreSource='source/fishing-fish-v3.png',moreSourceSha256=hashlib.sha256((root/'assets/source/fishing-fish-v3.png').read_bytes()).hexdigest(),extraSource='source/fishing-fish-v2.png',extraSourceSha256=hashlib.sha256((root/'assets/source/fishing-fish-v2.png').read_bytes()).hexdigest())

def export_fishing(export,root=ROOT):
    write_catalog(root)
    for name,image in build_sprites(root).items():export(image,'fishing',name,True,indexed=True)
    return metadata(root)

if __name__=='__main__':
    write_catalog()
    out=ROOT/'sdcard/pokemon-plus/fishing';out.mkdir(parents=True,exist_ok=True)
    for name,image in build_sprites().items():
        image.save(out/f'{name}.png');write_bin(image,out/f'{name}.bin',True,True)
    record=dict(service='MobiIoT',model='gpt-image-1.5',attempts=1,reviewed=True,**metadata(),
                backgroundModel='gpt-image-1.5',backgroundAttempts=1,backgroundPrompt='prompts/fishing-pond-v1.txt',
                recipe='2x2 atlas; alpha>=200; whole sprites nearest-fit;15 colors+transparent; icon32;fish24x16 integer4x; pond120/48 colors/integer3x')
    record.update(moreModel='gpt-image-1.5',moreAttempts=1,morePrompt='prompts/fishing-fish-v3.txt',extraModel='gpt-image-1.5',extraAttempts=1,extraPrompt='prompts/fishing-fish-v2.txt',extraRecipe='whole connected fish by atlas slot; alpha>=200;24x16 nearest fit;15 colors+binary alpha;96x64 native display and48x32 thumbnails')
    (ROOT/'assets/fishing-generation.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n')
    manifest=json.loads((ROOT/'sdcard/pokemon-plus/manifest.json').read_text());manifest['fishing']=metadata()
    old=[f for f in manifest['files'] if not f['path'].startswith('fishing/')]
    for file in sorted(out.iterdir()):
        old.append(dict(path='fishing/'+file.name,bytes=file.stat().st_size,sha256=hashlib.sha256(file.read_bytes()).hexdigest()))
    manifest['files']=sorted(old,key=lambda f:f['path'])
    for target in ('sdcard/pokemon-plus/manifest.json','web/manifest.json'):
        (ROOT/target).write_text(json.dumps(manifest,ensure_ascii=False,indent=2))
