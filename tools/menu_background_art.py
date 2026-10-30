"""One quiet menu backdrop derived from the reviewed MobiIoT capture ball."""
from pathlib import Path
import hashlib
import json
import zipfile
from PIL import Image, ImageFilter
from capture_ball_art import build_icon
from image_binary import write_bin

ROOT = Path(__file__).resolve().parents[1]


def build_background(root=ROOT):
    ball = build_icon(root)
    ball = ball.crop(ball.getbbox()).resize((400, 400), Image.Resampling.NEAREST)
    backdrop = Image.new('RGBA', (360, 360))
    backdrop.alpha_composite(ball, (-20, -20))
    backdrop = backdrop.filter(ImageFilter.GaussianBlur(10))
    # Bake softness and dimming once; neither LVGL nor Canvas blurs per frame.
    pixels = []
    for y in range(360):
        t = y / 359
        base = (9 + 4*t, 20 + 12*t, 30 + 10*t)
        for x in range(360):
            r, g, b, a = backdrop.getpixel((x, y))
            opacity = a / 255 * .24
            vignette = 1 - .24 * min(1, ((x-179.5)**2 + (y-179.5)**2) / 180**2)
            pixels.append(tuple(round((c*(1-opacity) + v*opacity)*vignette)
                                for c, v in zip(base, (r, g, b))))
    out = Image.new('RGB', (360, 360))
    out.putdata(pixels)
    return out.quantize(colors=64, method=Image.Quantize.MEDIANCUT,
                        dither=Image.Dither.NONE).convert('RGBA')


def background_manifest():
    return {'image': 'ui/menu-bg.png', 'source': 'assets/source/capture-ball-v1.png',
            'size': [360, 360], 'ballSize': [400, 400], 'ballPosition': [-20, -20],
            'blurRadius': 10, 'ballOpacity': .24}


if __name__ == '__main__':
    # Publish only this derived asset; do not regenerate unrelated pet frames.
    folder = ROOT/'sdcard/pokemon-plus'
    target = folder/'ui'
    target.mkdir(exist_ok=True)
    im = build_background()
    im.save(target/'menu-bg.png')
    write_bin(im, target/'menu-bg.bin', indexed=True)
    manifest = json.loads((folder/'manifest.json').read_text())
    manifest['menuBackground'] = background_manifest()
    changed = ['ui/menu-bg.bin', 'ui/menu-bg.png']
    manifest['files'] = sorted([x for x in manifest['files'] if x['path'] not in changed] + [
        {'path': name, 'bytes': (folder/name).stat().st_size,
         'sha256': hashlib.sha256((folder/name).read_bytes()).hexdigest()} for name in changed
    ], key=lambda x: x['path'])
    for path in [folder/'manifest.json', ROOT/'web/manifest.json']:
        path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2))
    with zipfile.ZipFile(ROOT/'dist/pokemon-plus-sd.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
        for path in sorted(folder.rglob('*')):
            if path.is_file():
                archive.write(path, path.relative_to(folder.parent))
    print('Published full-circle menu backdrop: 400px ball, centered crop, 10px blur')
