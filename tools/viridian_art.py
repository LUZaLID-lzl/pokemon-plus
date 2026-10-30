"""Measured, offline extraction of the MobiIoT Viridian gym assets."""
from PIL import Image, ImageOps, ImageEnhance
import json


def _sprite(atlas, box, size, threshold, colors=32):
    im=atlas.crop(tuple(box))
    im.putalpha(im.getchannel('A').point(lambda a:255 if a>=threshold else 0))
    im=im.crop(im.getbbox());im.thumbnail(size,Image.Resampling.NEAREST)
    im=im.quantize(colors=colors,method=Image.Quantize.FASTOCTREE).convert('RGBA')
    im.putalpha(im.getchannel('A').point(lambda a:255 if a>=128 else 0))
    return im.crop(im.getbbox())


def build_viridian(root):
    folder=root/'assets/gyms';recipe=json.loads((folder/'viridian-slices.json').read_text())
    atlas=Image.open(root/recipe['source']).convert('RGBA')
    for name,key,size in [('giovanni','giovanni',64),('earth','earth',64),('earth-display','earth',48)]:
        sprite=_sprite(atlas,recipe[key]['box'],(size-4,size-4),recipe['alphaThreshold'])
        out=Image.new('RGBA',(size,size));out.alpha_composite(sprite,((size-sprite.width)//2,(size-sprite.height)//2));out.save(folder/(name+'.png'))
    stamp=_sprite(atlas,recipe['stamp']['box'],(68,24),recipe['alphaThreshold'])
    out=Image.new('RGBA',(68,24));out.alpha_composite(stamp,((68-stamp.width)//2,(24-stamp.height)//2))
    out.resize((136,48),Image.Resampling.NEAREST).save(folder/'defeated-ground.png')
    for source,name,native,size,screen_y,radius,brightness in [
        ('viridian-city-v1','viridian-map',(180,84),(360,168),100,180,1),
        ('viridian-arena-v1','viridian-arena',(120,38),(360,112),108,173,.8)]:
        im=ImageOps.fit(Image.open(root/f'assets/source/gyms/{source}.png').convert('RGB'),native,method=Image.Resampling.LANCZOS)
        im=ImageEnhance.Brightness(im).enhance(brightness)
        im=im.quantize(colors=48,dither=Image.Dither.NONE).convert('RGB').resize(size,Image.Resampling.NEAREST)
        for y in range(im.height):
            for x in range(im.width):
                if (x-179.5)**2+(y+screen_y-179.5)**2>=radius**2:im.putpixel((x,y),(8,14,23))
        im.save(folder/(name+'.png'))


if __name__=='__main__':
    from pathlib import Path
    build_viridian(Path(__file__).resolve().parents[1])
