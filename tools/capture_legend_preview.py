"""Export actual shared LVGL legendary views for the Web firmware preview."""
from pathlib import Path
import subprocess
import tempfile
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
out = root / 'docs/screenshots'
out.mkdir(exist_ok=True)
cmd = [str(root/'preview/build/pokemon_plus'), '--assets', str(root/'sdcard/pokemon-plus'), '--intro-seen']
with tempfile.TemporaryDirectory(prefix='pp-legend-') as tmp:
    tmp = Path(tmp)
    def capture(name, *args):
        ppm = tmp/(name+'.ppm')
        subprocess.run(cmd+list(args)+(['--at-ms','1200'] if name.endswith('-battle') else [])+['--screenshot', str(ppm)], check=True)
        Image.open(ppm).save(out/(name+'.png'))
    for phase in ['apps', 'locked', 'list', 'cleared', 'bag']:
        capture('legend-'+phase, '--legend', phase)
    for page in [1, 2]:
        capture('page-'+str(page), '--demo', '--page', str(page))
    for index in range(5):
        for phase in ['detail', 'locked-detail', 'party', 'battle', 'won', 'lost', 'egg', 'hatched']:
            capture(f'legend-{index}-{phase}', '--legend', phase, '--legend-index', str(index))
        frames = tmp/str(index)
        frames.mkdir()
        subprocess.run(cmd+['--legend', 'battle', '--legend-index', str(index), '--at-ms', '1', '--frames-dir', str(frames), '--frame-count', '64', '--frame-step', '125'], check=True)
        images = [Image.open(p).convert('RGB') for p in sorted(frames.glob('*.ppm'))]
        assert len(images) == 64 and len({im.tobytes() for im in images}) > 25
        images[0].save(out/f'legend-{index}-battle.gif', save_all=True, append_images=images[1:], duration=[120, 130]*32, loop=0, optimize=False)
    names = ['legend-apps', 'legend-locked', 'legend-list', 'legend-3-detail', 'legend-3-won', 'legend-bag', 'legend-3-egg', 'legend-3-hatched']
    sheet = Image.new('RGB', (1440, 780), '#e8eee2')
    draw = ImageDraw.Draw(sheet)
    for i, name in enumerate(names):
        x, y = i%4*360, i//4*390
        sheet.paste(Image.open(out/(name+'.png')), (x, y))
        draw.text((x+16, y+366), name, fill='#244556')
    sheet.save(out/'legend-contact-sheet.png')
    names = ['legend-locked', 'legend-list', 'legend-cleared', 'legend-3-detail', 'legend-0-party', 'legend-3-won', 'legend-3-lost', 'legend-bag']
    sheet = Image.new('RGB', (1440, 780), '#e8eee2')
    draw = ImageDraw.Draw(sheet)
    for i, name in enumerate(names):
        x, y = i%4*360, i//4*390
        sheet.paste(Image.open(out/(name+'.png')), (x, y))
        draw.text((x+16, y+366), name, fill='#244556')
    sheet.save(out/'legend-style-sheet.png')
print('PASS: five legendary targets, locked/list/APP/bag views, exact eggs/hatches and five real battle GIFs')
