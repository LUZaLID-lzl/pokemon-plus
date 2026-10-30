"""Derive the reviewed MobiIoT capture ball as a native 32px shop icon."""
from pathlib import Path
import hashlib
import json
import re
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]


def build_icon(root=ROOT):
    icon = Image.open(root / 'assets/source/capture-ball-v1.png').convert('RGBA')
    icon.putalpha(icon.getchannel('A').point(lambda a: 255 if a >= 200 else 0))
    bounds = icon.getbbox()
    if not bounds:
        raise ValueError('Capture ball has no visible silhouette')
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
    target = ROOT / 'assets/items/capture-ball.png'
    icon = build_icon()
    icon.save(target)
    log = (ROOT / 'build/capture-ball-generation-v1.log').read_text()
    models = re.findall(r'Selected model: (gpt-image-[^\s]+)', log)
    attempts = re.findall(r'Attempt (\d+)', log)
    record = {
        'service': 'MobiIoT', 'model': models[-1] if models else None,
        'attempts': int(attempts[-1]) if attempts else None,
        'source': 'source/capture-ball-v1.png',
        'prompt': 'prompts/items/capture-ball-v1.txt',
        'log': 'build/capture-ball-generation-v1.log',
        'references': ['sdcard/pokemon-plus/icons/2.png',
                       'sdcard/pokemon-plus/icons/5.png', 'assets/items/hatch-boost.png'],
        'sourceSha256': hashlib.sha256((ROOT/'assets/source/capture-ball-v1.png').read_bytes()).hexdigest(),
        'output': 'items/capture-ball.png',
        'sha256': hashlib.sha256(target.read_bytes()).hexdigest(),
        'recipe': {'alphaThreshold': 200, 'maxSilhouette': 29, 'canvas': 32,
                   'resampling': 'nearest', 'quantizeColors': 15},
    }
    (ROOT/'assets/items/capture-ball.json').write_text(json.dumps(record, indent=2)+'\n')
    print(target)
