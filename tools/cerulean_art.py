"""Offline MobiIoT Cerulean extraction; no runtime resizing or image processing."""
from PIL import Image, ImageOps, ImageEnhance, ImageDraw
import json

def build_cerulean(root):
    folder=root/'assets/gyms'
    slices=json.loads((folder/'cerulean-slices.json').read_text())
    atlas=Image.open(root/slices['source']).convert('RGBA')
    for name,size in [('misty',64),('cascade',64),('cascade-display',48)]:
        spec=slices['cascade' if name.startswith('cascade') else name]
        mask=Image.new('L',atlas.size);draw=ImageDraw.Draw(mask)
        draw.polygon([tuple(p) for p in spec['polygon']],fill=255)
        for hole in spec['holes']:draw.polygon([tuple(p) for p in hole],fill=0)
        sprite=atlas.copy();sprite.putalpha(mask);sprite=sprite.crop(tuple(spec['box']))
        sprite.thumbnail((size-4,size-4),Image.Resampling.NEAREST)
        sprite=sprite.quantize(colors=32,method=Image.Quantize.FASTOCTREE).convert('RGBA')
        sprite.putalpha(sprite.getchannel('A').point(lambda a:255 if a>=128 else 0))
        canvas=Image.new('RGBA',(size,size));canvas.alpha_composite(sprite,((size-sprite.width)//2,(size-sprite.height)//2))
        canvas.save(folder/(name+'.png'))
    for source,name,native,size,screen_y,radius,brightness in [
        ('cerulean-city-v1','cerulean-map',(180,84),(360,168),100,180,1),
        ('cerulean-arena-v1','cerulean-arena',(120,38),(360,112),108,173,.8)]:
        im=ImageOps.fit(Image.open(root/f'assets/source/gyms/{source}.png').convert('RGB'),native,method=Image.Resampling.LANCZOS)
        im=ImageEnhance.Brightness(im).enhance(brightness)
        im=im.quantize(colors=48,dither=Image.Dither.NONE).convert('RGB').resize(size,Image.Resampling.NEAREST)
        for y in range(im.height):
            for x in range(im.width):
                if (x-179.5)**2+(y+screen_y-179.5)**2>=radius**2:im.putpixel((x,y),(8,14,23))
        im.save(folder/(name+'.png'))

if __name__=='__main__':
    from pathlib import Path
    build_cerulean(Path(__file__).resolve().parents[1])
