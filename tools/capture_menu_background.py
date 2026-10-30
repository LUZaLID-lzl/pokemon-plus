"""Refresh shared LVGL menu screenshots after changing the common backdrop."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import subprocess
import tempfile
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'docs/screenshots'
CMD = [str(ROOT/'preview/build/pokemon_plus'), '--assets',
       str(ROOT/'sdcard/pokemon-plus'), '--demo', '--intro-seen']
jobs = [(f'page-{p}', ['--page', str(p)])
        for p in [1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 14, 15, 16, 18, 19, 20, 26, 27, 28, 29]]
jobs += [('activity-steps', ['--activity', 'steps']),
         ('pokedex-demo', ['--page', '15']), ('pokedex', ['--activity', 'pokedex'])]
jobs += [(f'collection-quality-{q}', ['--activity', 'collection-demo', '--collection-quality', str(q)]) for q in range(7)]
jobs += [('collection-detail', ['--activity', 'collection-detail']),
         ('collection-heart', ['--activity', 'heart-detail'])]
jobs += [('legend-'+state, ['--legend', state]) for state in ['apps', 'locked', 'list', 'cleared', 'bag']]
jobs += [(f'legend-{i}-{state}', ['--legend', state, '--legend-index', str(i)])
         for i in range(5) for state in ['detail', 'locked-detail', 'party']]
prefixes = ['gym', 'cerulean', 'vermilion', 'celadon', 'fuchsia', 'saffron', 'cinnabar', 'viridian']
for i, prefix in enumerate(prefixes):
    states = ['detail', 'team', 'empty', 'rematch', 'badges']
    states += ['badges-'+p for p in ['pewter']+prefixes[1:i]] if i else []
    jobs += [(prefix+'-'+state, ['--activity', prefix+'-'+state]) for state in states]


def capture(job):
    name, args = job
    with tempfile.TemporaryDirectory(prefix='pp-menu-bg-') as tmp:
        ppm = Path(tmp)/'frame.ppm'
        subprocess.run(CMD+args+['--screenshot', str(ppm)], check=True,
                       capture_output=True, timeout=45)
        Image.open(ppm).save(OUT/(name+'.png'))


if __name__ == '__main__':
    with ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(capture, jobs))
    # Scene states and travel clips also display the same menu backdrop.
    for script in ['capture_menu_preview.py', 'capture_travel.py']:
        subprocess.run(['python3', str(ROOT/'tools'/script)], check=True)
    names = ['page-1', 'page-2', 'page-3', 'page-11', 'page-9', 'legend-locked']
    sheet = Image.new('RGB', (1080, 768), '#080e17')
    draw = ImageDraw.Draw(sheet)
    for i, name in enumerate(names):
        x, y = i % 3 * 360, i // 3 * 384
        sheet.paste(Image.open(OUT/(name+'.png')), (x, y))
        draw.text((x+150, y+365), ['APP', 'BAG', 'SHOP', 'SETTINGS', 'SCENES', 'LEGEND'][i], fill='#b8c9ce')
    sheet.save(OUT/'menu-background-preview.png')
    p = OUT/'contact-sheet.png'
    if p.exists():
        contact = Image.open(p).convert('RGB')
        for page in [1, 2, 3, 4, 5, 8, 9, 10, 11, 12]:
            contact.paste(Image.open(OUT/f'page-{page}.png'), (page % 4*360, page // 4*385))
        contact.save(p)
    print(f'PASS: {len(jobs)} menu views, all scene states and travel clips refreshed')
