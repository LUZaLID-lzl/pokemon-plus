"""Derive the reviewed MobiIoT warehouse and scene menu icons as a native 32px APP icon."""
from pathlib import Path
import hashlib
import json
import re
import argparse
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ICON_REVISIONS = {'warehouse': 2, 'scene': 1}


def build_icon(kind, root=ROOT):
    if kind not in ("warehouse", "scene"):
        raise ValueError("Unknown menu icon")
    icon = Image.open(root / f'assets/source/{kind}-icon-v{ICON_REVISIONS[kind]}.png').convert('RGBA')
    icon.putalpha(icon.getchannel('A').point(lambda a: 255 if a >= 200 else 0))
    bounds = icon.getbbox()
    if not bounds:
        raise ValueError('Menu icon has no visible silhouette')
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
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kind',choices=tuple(ICON_REVISIONS))
    args=parser.parse_args()
    for kind in ([args.kind] if args.kind else ICON_REVISIONS):
        revision=ICON_REVISIONS[kind]
        target = ROOT / f'assets/hud/{kind}.png'
        target.parent.mkdir(parents=True, exist_ok=True)
        icon = build_icon(kind)
        icon.save(target)
        log = (ROOT / f'build/{kind}-icon-generation-v{revision}.log').read_text()
        models = re.findall(r'Selected model: (gpt-image-[^\s]+)', log)
        attempts = re.findall(r'Attempt (\d+)', log)
        record = {
            'service': 'MobiIoT', 'model': models[-1] if models else None,
            'attempts': int(attempts[-1]) if attempts else None,
            'source': f'source/{kind}-icon-v{revision}.png',
            'prompt': f'prompts/{kind}-icon-v{revision}.txt',
            'log': f'build/{kind}-icon-generation-v{revision}.log',
            'references': ['sdcard/pokemon-plus/hud/gym.png',
                           'sdcard/pokemon-plus/icons/13.png'],
            'sourceSha256': hashlib.sha256((ROOT/f'assets/source/{kind}-icon-v{revision}.png').read_bytes()).hexdigest(),
            'output': f'hud/{kind}.png',
            'sha256': hashlib.sha256(target.read_bytes()).hexdigest(),
            'recipe': {'alphaThreshold': 200, 'maxSilhouette': 29, 'canvas': 32,
                       'resampling': 'nearest', 'quantizeColors': 15},
        }
        (ROOT/f'assets/hud/{kind}.json').write_text(json.dumps(record, indent=2)+'\n')
        print(target)
