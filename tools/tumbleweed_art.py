"""Native transparent rolling frames from the reviewed MobiIoT dry bramble sprite."""
from pathlib import Path
import hashlib
import json
import re
from PIL import Image
from image_binary import write_bin

ROOT = Path(__file__).resolve().parents[1]
SIZES = (32, 36, 40)


def build_frames(root=ROOT):
    source = Image.open(root/'assets/source/oasis-tumbleweed-v2.png').convert('RGBA')
    source.putalpha(source.getchannel('A').point(lambda a: 255 if a >= 160 else 0))
    source = source.crop(source.getbbox())
    palette = ((97, 64, 35), (146, 97, 52), (190, 138, 71), (223, 177, 106))
    source.putdata([(*min(palette, key=lambda c: (c[0]-r)**2+(c[1]-g)**2+(c[2]-b)**2), 255)
                    if a else (0, 0, 0, 0) for r, g, b, a in source.getdata()])
    for size in SIZES:
        grid = size//2
        twig = source.copy()
        twig.thumbnail((grid*3//4, grid*3//4), Image.Resampling.NEAREST)
        base = Image.new('RGBA', (grid, grid))
        base.alpha_composite(twig, ((grid-twig.width)//2, (grid-twig.height)//2))
        for frame in range(8):
            im = base.rotate(-45*frame, resample=Image.Resampling.NEAREST).resize((size, size), Image.Resampling.NEAREST)
            assert set(im.getchannel('A').getdata()) == {0, 255}
            assert len(set(im.getdata())) <= 5
            yield size, frame, im


def export_frames(export, root=ROOT):
    for size, frame, im in build_frames(root):
        export(im, 'effects', f'tumble-{size}-{frame}', True, indexed=True)


if __name__ == '__main__':
    folder = ROOT/'sdcard/pokemon-plus/effects'
    folder.mkdir(parents=True, exist_ok=True)
    records = []
    for size, frame, im in build_frames():
        target = folder/f'tumble-{size}-{frame}.png'
        im.save(target)
        write_bin(im, target.with_suffix('.bin'), True, indexed=True)
        for p in (target, target.with_suffix('.bin')):
            records.append({'path': p.relative_to(ROOT/'sdcard/pokemon-plus').as_posix(),
                            'bytes': p.stat().st_size, 'sha256': hashlib.sha256(p.read_bytes()).hexdigest()})
    for path in (ROOT/'web/manifest.json', ROOT/'sdcard/pokemon-plus/manifest.json'):
        manifest = json.loads(path.read_text())
        files = {x['path']: x for x in manifest['files']}
        files.update({x['path']: x for x in records})
        manifest['files'] = [files[k] for k in sorted(files)]
        path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2)+'\n')
    log = (ROOT/'build/tumbleweed-imagegen-v2.log').read_text()
    (ROOT/'assets/tumbleweed-art.json').write_text(json.dumps({
        'service': 'MobiIoT', 'model': re.findall(r'Selected model: (gpt-image-\S+)', log)[-1],
        'attempts': int(re.findall(r'Attempt (\d+)/', log)[-1]),
        'source': 'source/oasis-tumbleweed-v2.png', 'prompt': 'prompts/oasis-tumbleweed-v2.txt',
        'log': 'build/tumbleweed-imagegen-v2.log',
        'sourceSha256': hashlib.sha256((ROOT/'assets/source/oasis-tumbleweed-v2.png').read_bytes()).hexdigest(),
        'recipe': {'sizes': SIZES, 'frames': 8, 'angleStep': -45, 'alphaThreshold': 160,
                   'colors': 4, 'pixelBlock': 2, 'designGrids': [16, 18, 20],
                   'resampling': 'nearest', 'precomputed': True},
        'references': ['source/oasis-tumbleweed-v1.png'], 'outputs': records,
    }, ensure_ascii=False, indent=2)+'\n')
    print('Exported 24 native transparent tumbleweed frames, three sizes/eight rotations')
