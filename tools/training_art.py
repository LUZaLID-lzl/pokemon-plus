"""Reviewed MobiIoT side-scrolling landscapes and a dedicated hiking icon."""
from pathlib import Path
from PIL import Image
import hashlib,json,re
ROOT=Path(__file__).resolve().parents[1]
def sprites(root=ROOT):
 source=Image.open(root/'assets/source/training/scroll-v1.png').convert('RGB')
 result={}
 for i in range(6):
  w,h=source.width//2,source.height//3
  tile=source.crop((i%2*w,i//2*h,(i%2+1)*w,(i//2+1)*h)).resize((180,60),Image.Resampling.BOX)
  # Match both edges progressively; the terrain stays level across every loop.
  for y in range(60):
   for d in range(8):
    a=tile.getpixel((d,y));b=tile.getpixel((179-d,y));weight=(8-d)/16
    tile.putpixel((d,y),tuple(round(x*(1-weight)+z*weight) for x,z in zip(a,b)))
    tile.putpixel((179-d,y),tuple(round(z*(1-weight)+x*weight) for x,z in zip(a,b)))
  tile=tile.quantize(colors=31,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE).convert('RGB')
  canvas=Image.new('RGB',(180,180),'#102b34');canvas.paste(tile,(0,54));result[str(i)]=canvas
 return result
def icon(root=ROOT):
 im=Image.open(root/'assets/source/training/icon-v1.png').convert('RGBA')
 im.putalpha(im.getchannel('A').point(lambda a:255 if a>=200 else 0))
 if not im.getbbox():raise ValueError('Empty training icon')
 im=im.crop(im.getbbox());im.thumbnail((30,30),Image.Resampling.NEAREST)
 im=im.quantize(colors=15,method=Image.Quantize.FASTOCTREE).convert('RGBA')
 im.putdata([(r,g,b,255) if a else (0,0,0,0) for r,g,b,a in im.getdata()])
 canvas=Image.new('RGBA',(32,32));canvas.alpha_composite(im,((32-im.width)//2,(32-im.height)//2));return canvas
def export_training(export,root=ROOT):
 from training_catalog import generate
 from training_scenery import generate as generate_scenery
 generate(root);generate_scenery(root)
 for name,im in sprites(root).items():export(im,'training',name,False,indexed=True)
 export(icon(root),'hud','training',True,indexed=True)
 meta={'source':'source/training/scroll-v1.png','sourceSha256':hashlib.sha256((root/'assets/source/training/scroll-v1.png').read_bytes()).hexdigest(),'sceneCount':6,'teamSize':[1,3],'imageSize':[180,180],'panorama':[0,54,180,60],'scale':2,'scrollPixelsPerSecond':40,'icon':'hud/training.png'}
 records=[]
 for kind,source,prompt in [('scroll','scroll-v1.png','training-scroll-v1.txt'),('icon','icon-v1.png','training-icon-v1.txt')]:
  log=(root/f'build/training-{kind}-generation.log').read_text()
  records.append({'service':'MobiIoT','model':re.findall(r'Selected model: (\S+)',log)[-1],'attempts':int(re.findall(r'Attempt (\d+)',log)[-1]),'reviewed':True,'source':'source/training/'+source,'prompt':'prompts/'+prompt,'sourceSha256':hashlib.sha256((root/'assets/source/training'/source).read_bytes()).hexdigest()})
 (root/'assets/training-generation.json').write_text(json.dumps({'images':records,**meta},ensure_ascii=False,indent=2)+'\n')
 return meta
def patch_manifest(m):
 m['appMenu']['pages']=[['travel','training','battle','warehouse','scenes','fishing','steps','tarot','hospital'],['badges','legend','settings','pokedex','collection']]
 m['appMenu']['labels']['training']='历练';m['appMenu']['icons']['training']='hud/training.png'
