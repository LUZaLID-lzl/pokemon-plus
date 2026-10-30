"""Capture the shared LVGL renderer, including a deterministic 4s home loop."""
from pathlib import Path
import subprocess
import tempfile
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
out = root / 'docs/screenshots'
out.mkdir(exist_ok=True)
command = [str(root / 'preview/build/pokemon_plus'), '--assets',
           str(root / 'sdcard/pokemon-plus'), '--demo']
names = ['HOME', 'APPS', 'BAG', 'SHOP', 'STATUS', 'TRAVEL', 'BATTLE',
         'DIG', 'COLLECTION', 'SCENES', 'TAROT', 'SETTINGS', 'DETAIL']
sheet = Image.new('RGB', (1440, 1540), '#e8eee2')
draw = ImageDraw.Draw(sheet)

def capture(name, *args):
    target = out / f'{name}.ppm'
    subprocess.run(command + list(args) + ['--screenshot', str(target)], check=True)
    im = Image.open(target).convert('RGB')
    im.save(target.with_suffix('.png'))
    target.unlink()
    return im

for page, name in enumerate(names):
    im = capture(f'page-{page}', '--page', str(page))
    x, y = page % 4 * 360, page // 4 * 385
    sheet.paste(im, (x, y))
    draw.text((x + 140, y + 365), name, fill='#476547')
sheet.save(out / 'contact-sheet.png')
capture('activity-steps', '--activity', 'steps')
capture('pokedex', '--activity', 'pokedex')
capture('pokedex-demo', '--page', '15')
capture('starter-current', '--page', '13')
capture('battle-result', '--activity', 'battle', '--at-ms', '90000')
for activity in ['travel', 'battle', 'dig', 'egg', 'feed']:
    capture(f'activity-{activity}', '--activity', activity)
with tempfile.TemporaryDirectory(prefix='pokemon-frames-') as directory:
    subprocess.run(command + ['--frames-dir', directory], check=True)
    frames = [Image.open(p).convert('RGB') for p in sorted(Path(directory).glob('*.ppm'))]
    assert len(frames) == 32 and len({im.tobytes() for im in frames}) > 16
    # GIF stores centiseconds: alternate 120/130ms to average 125ms.
    frames[0].save(out / 'home-v2.gif', save_all=True, append_images=frames[1:],
                   duration=[120, 130] * 16, loop=0, optimize=False)
show = Image.new('RGB', (1080, 720), '#e8eee2')
for i, name in enumerate(['page-0', 'activity-feed', 'activity-egg',
                          'activity-travel', 'activity-battle', 'activity-dig']):
    show.paste(Image.open(out / f'{name}.png'), (i % 3 * 360, i // 3 * 360))
show.save(out / 'showcase.png')
print(out / 'contact-sheet.png')
print(out / 'home-v2.gif')

# Canonical full narration clips are produced by capture_battle_narration.py.
# Build the four-family montage from those files without overwriting any skill.
clips=[]
for move_id in range(4):
    clip=Image.open(out/f'skill-{move_id}.gif')
    frames=[]
    for frame in range(clip.n_frames):
        clip.seek(frame);frames.append(clip.convert('RGB'))
    clips.append(frames)
combined=[]
for frame in range(min(len(clip) for clip in clips)):
    tile=Image.new('RGB',(720,720),'#091521')
    for species in range(4):tile.paste(clips[species][frame],(species%2*360,species//2*360))
    combined.append(tile)
combined[0].save(out/'battle-skills.gif',save_all=True,append_images=combined[1:],duration=[120,130]*(len(combined)//2),loop=0,optimize=False)
print(out/'battle-skills.gif')

with tempfile.TemporaryDirectory(prefix='pokemon-evolution-') as directory:
    subprocess.run(command+['--activity','evolution','--frames-dir',directory],check=True)
    frames=[Image.open(p).convert('RGB') for p in sorted(Path(directory).glob('*.ppm'))]
    frames[0].save(out/'evolution.gif',save_all=True,append_images=frames[1:],duration=[120,130]*16,loop=0,optimize=False)
    frames[24].save(out/'evolution.png')

# Every theme must remain readable behind the same real clock/coin widgets.
scene_sheet = Image.new('RGB', (1080, 1536), '#e8eee2')
scene_labels = ['COURTYARD', 'SAKURA', 'COAST', 'WATERFALL', 'AUTUMN', 'SNOW',
                'CRYSTAL CAVE', 'MUSHROOMS', 'STARRY HILL', 'OASIS', 'RAIN', 'SKY ISLAND']
scene_draw = ImageDraw.Draw(scene_sheet)
for scene, name in enumerate(scene_labels):
    im = capture(f'scene-{scene}', '--page', '0', '--scene', str(scene))
    x, y = scene % 3 * 360, scene // 3 * 384
    scene_sheet.paste(im, (x, y))
    scene_draw.text((x + 12, y + 365), f'{scene:02}  {name}', fill='#476547')
scene_sheet.save(out / 'scenes-v073.png')

subprocess.run(["python3",str(root/"tools/capture_wild_preview.py")],check=True)
