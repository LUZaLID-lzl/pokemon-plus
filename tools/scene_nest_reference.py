"""Contact sheet of actual scene pixels for image generation and placement review."""
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]

def main():
    sheet = Image.new('RGB', (4 * 360, 4 * 384), '#172635')
    draw = ImageDraw.Draw(sheet)
    for scene in range(13):
        x, y = scene % 4 * 360, scene // 4 * 384
        sheet.paste(Image.open(ROOT / f'sdcard/pokemon-plus/scenes/{scene}.png'), (x, y))
        draw.text((x + 12, y + 364), f'SCENE {scene + 1:02}', fill='white')
    target = ROOT / 'assets/reference/scene-nests-backgrounds.png'
    target.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(target)
    print(target)

if __name__ == '__main__':
    main()
