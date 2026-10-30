"""Offline Pewter town artwork: native pixels, full-width circular panel arc."""
from PIL import Image, ImageOps, ImageEnhance

MAP_Y = 100
MAP_SIZE = (360, 168)


def build_map(root):
    source = root / 'assets/source/gyms/pewter-city-v2.png'
    native = ImageOps.fit(Image.open(source).convert('RGB'), (180, 84),
                          method=Image.Resampling.LANCZOS)
    native = ImageEnhance.Contrast(native).enhance(1.08)
    native = native.quantize(colors=48, dither=Image.Dither.NONE).convert('RGB')
    scene = native.resize(MAP_SIZE, Image.Resampling.NEAREST)
    # Only pixels outside the actual physical panel are dark. Scenery itself
    # reaches radius180; there is no inset card, frame or artificial dark rim.
    for y in range(scene.height):
        for x in range(scene.width):
            if (x - 179.5)**2 + (y + MAP_Y - 179.5)**2 >= 180**2:
                scene.putpixel((x, y), (8, 14, 23))
    scene.save(root / 'assets/gyms/map.png')
    return scene
