"""One offline scene pipeline for full builds and scene-only updates."""
from PIL import Image, ImageEnhance, ImageFilter
import json
from scene_nest_art import decorate_scene

def scene_base(root, source):
    config=json.loads((root/'assets/scene-display.json').read_text())
    im=Image.open(root/'assets'/source).convert('RGB').resize((120,120),Image.Resampling.NEAREST)
    im=im.quantize(colors=48,dither=Image.Dither.NONE).convert('RGB')
    # Compress palette differences, not sprite/UI contrast; no runtime filtering.
    im=ImageEnhance.Contrast(im).enhance(config['contrast'])
    return im.resize((360,360),Image.Resampling.NEAREST)

def scene_variants(root, source):
    im=scene_base(root,source)
    sources=json.loads((root/'assets/scene-sources.json').read_text())['sources']
    if source in sources:
        im=decorate_scene(im,sources.index(source),root)
    battle=im.crop((0,90,360,202)).filter(ImageFilter.GaussianBlur(radius=0.85))
    for y in range(112):
        for x in range(360):
            if (x-179.5)**2+(y+108-179.5)**2>=173**2:
                battle.putpixel((x,y),(8,14,23))
    return {'scenes':im,'thumbs':im.resize((152,120),Image.Resampling.NEAREST),
            'strips':im.crop((0,90,360,225)).resize((324,112),Image.Resampling.NEAREST),
            'battle-scenes':battle}
