"""Derive the reviewed MobiIoT hatch potion as a native shop-size icon."""
from pathlib import Path
from PIL import Image
import hashlib
import json

ROOT = Path(__file__).resolve().parents[1]


def build_icon(root=ROOT):
    icon = Image.open(root / 'assets/source/hatch-boost-v1.png').convert('RGBA')
    icon.putalpha(icon.getchannel('A').point(lambda a: 255 if a >= 200 else 0))
    bounds = icon.getbbox()
    if not bounds:
        raise ValueError('Hatch potion has no visible silhouette')
    icon = icon.crop(bounds)
    icon.thumbnail((29, 29), Image.Resampling.NEAREST)
    icon = icon.quantize(colors=15, method=Image.Quantize.FASTOCTREE).convert('RGBA')
    icon.putdata([(r, g, b, 255) if a else (0, 0, 0, 0)
                  for r, g, b, a in icon.getdata()])
    canvas = Image.new('RGBA', (32, 32))
    canvas.alpha_composite(icon, ((32-icon.width)//2, (32-icon.height)//2))
    assert set(canvas.getchannel('A').getdata()) == {0, 255}
    assert len(set(canvas.getdata())) <= 16
    return canvas


if __name__ == '__main__':
    target = ROOT / 'assets/items/hatch-boost.png'
    target.parent.mkdir(exist_ok=True)
    icon = build_icon()
    icon.save(target)
    record = {
        'service': 'MobiIoT', 'model': 'gpt-image-1.5', 'attempts': 1,
        'source': 'source/hatch-boost-v1.png',
        'prompt': 'prompts/items/hatch-boost-v1.txt',
        'log': 'build/hatch-icon-generation-v1.log',
        'references': ['sdcard/pokemon-plus/icons/2.png', 'sdcard/pokemon-plus/icons/5.png'],
        'sourceSha256': hashlib.sha256((ROOT/'assets/source/hatch-boost-v1.png').read_bytes()).hexdigest(),
        'output': 'items/hatch-boost.png',
        'sha256': hashlib.sha256(target.read_bytes()).hexdigest(),
        'recipe': {'alphaThreshold': 200, 'maxSilhouette': 29, 'canvas': 32,
                   'resampling': 'nearest', 'quantizeColors': 15},
    }
    (ROOT/'assets/items/hatch-boost.json').write_text(json.dumps(record, indent=2)+'\n')
    print(target)
