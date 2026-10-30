"""Reproducible offline extraction of the MobiIoT Vermilion artwork."""
from PIL import Image, ImageOps, ImageEnhance
import json

def build_vermilion(root):
    folder=root/'assets/gyms'
    slices=json.loads((folder/'vermilion-slices.json').read_text())
    atlas=Image.open(root/slices['source']).convert('RGBA')
    for name,size in [('surge',64),('thunder',64),('thunder-display',48)]:
        spec=slices['surge' if name=='surge' else 'thunder']
        sprite=atlas.crop(tuple(spec['box']))
        sprite.putalpha(sprite.getchannel('A').point(lambda a:255 if a>=slices['alphaThreshold'] else 0))
        sprite=sprite.crop(sprite.getbbox())
        sprite.thumbnail((size-4,size-4),Image.Resampling.NEAREST)
        sprite=sprite.quantize(colors=32,method=Image.Quantize.FASTOCTREE).convert('RGBA')
        sprite.putalpha(sprite.getchannel('A').point(lambda a:255 if a>=128 else 0))
        canvas=Image.new('RGBA',(size,size));canvas.alpha_composite(sprite,((size-sprite.width)//2,(size-sprite.height)//2))
        canvas.save(folder/(name+'.png'))
    for source,name,native,size,screen_y,radius,brightness in [
        ('vermilion-city-v1','vermilion-map',(180,84),(360,168),100,180,1),
        ('vermilion-arena-v1','vermilion-arena',(120,38),(360,112),108,173,.8)]:
        im=ImageOps.fit(Image.open(root/f'assets/source/gyms/{source}.png').convert('RGB'),native,method=Image.Resampling.LANCZOS)
        im=ImageEnhance.Brightness(im).enhance(brightness)
        im=im.quantize(colors=48,dither=Image.Dither.NONE).convert('RGB').resize(size,Image.Resampling.NEAREST)
        for y in range(im.height):
            for x in range(im.width):
                if (x-179.5)**2+(y+screen_y-179.5)**2>=radius**2:im.putpixel((x,y),(8,14,23))
        im.save(folder/(name+'.png'))

if __name__=='__main__':
    from pathlib import Path
    build_vermilion(Path(__file__).resolve().parents[1])
