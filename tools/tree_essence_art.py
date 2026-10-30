"""Native 32px item derived from the reviewed MobiIoT essence master."""
from pathlib import Path
from PIL import Image
import hashlib
import json
ROOT = Path(__file__).resolve().parents[1]
ITEM = {'name':'神树精华','price':200,'image':'items/tree-essence.png',
        'effect':'fullRestore','description':'恢复场景内总体状态最低的宠物，饱腹、清洁、健康、心情、精力全部恢复至100'}
def build_icon(root=ROOT):
    im=Image.open(root/'assets/source/tree-essence-v1.png').convert('RGBA')
    im.putalpha(im.getchannel('A').point(lambda a:255 if a>=200 else 0))
    if not im.getbbox(): raise ValueError('Empty tree essence')
    im=im.crop(im.getbbox());im.thumbnail((29,29),Image.Resampling.NEAREST)
    im=im.quantize(colors=15,method=Image.Quantize.FASTOCTREE).convert('RGBA')
    im.putdata([(r,g,b,255) if a else (0,0,0,0) for r,g,b,a in im.getdata()])
    canvas=Image.new('RGBA',(32,32));canvas.alpha_composite(im,((32-im.width)//2,(32-im.height)//2))
    return canvas
if __name__=='__main__':
    from image_binary import write_bin
    im=build_icon();target=ROOT/'assets/items/tree-essence.png';im.save(target)
    folder=ROOT/'sdcard/pokemon-plus/items';im.save(folder/'tree-essence.png');write_bin(im,folder/'tree-essence.bin',True,indexed=True)
    small=im.resize((24,24),Image.Resampling.NEAREST);small.save(folder/'tree-essence-small.png');write_bin(small,folder/'tree-essence-small.bin',True,indexed=True)
    log=(ROOT/'build/tree-essence-generation.log').read_text()
    record={'service':'MobiIoT','model':log.split('Selected model: ')[-1].splitlines()[0],
        'attempts':log.count('Selected model:'),'source':'source/tree-essence-v1.png',
        'prompt':'prompts/items/tree-essence-v1.txt','log':'build/tree-essence-generation.log',
        'references':['sdcard/pokemon-plus/icons/2.png','assets/items/hatch-boost.png'],
        'sourceSha256':hashlib.sha256((ROOT/'assets/source/tree-essence-v1.png').read_bytes()).hexdigest(),
        'sha256':hashlib.sha256(target.read_bytes()).hexdigest(),
        'recipe':{'alphaThreshold':200,'maxSilhouette':29,'canvas':32,'resampling':'nearest','quantizeColors':15}}
    (ROOT/'assets/items/tree-essence.json').write_text(json.dumps(record,indent=2)+'\n')
    for manifest_path in [ROOT/'sdcard/pokemon-plus/manifest.json',ROOT/'web/manifest.json']:
        m=json.loads(manifest_path.read_text());m['items']=[x for x in m['items'] if x.get('effect')!='fullRestore']+[ITEM]
        for file in sorted(folder.glob('tree-essence*.*')):
            path=str(file.relative_to(folder.parent));m['files']=[x for x in m['files'] if x['path']!=path]
            m['files'].append({'path':path,'bytes':file.stat().st_size,'sha256':hashlib.sha256(file.read_bytes()).hexdigest()})
        m['files'].sort(key=lambda x:x['path']);manifest_path.write_text(json.dumps(m,ensure_ascii=False,indent=2))
    print('Exported tree essence PNG + I4 BIN and synchronized manifests')
