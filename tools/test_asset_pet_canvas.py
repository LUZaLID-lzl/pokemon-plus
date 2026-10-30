#!/usr/bin/env python3
"""Verify native crop storage against every original playback PNG.

Read-only: does not change the generated BINs, bank or runtime configuration.
"""
from pathlib import Path
import struct
from PIL import Image
from asset_pet_canvas import encode, decode, CENTERED_PET

root = Path(__file__).resolve().parents[1]
count = saved = 0
formats = set()
for path in sorted((root/'sdcard/pokemon-plus').glob('pets*/*.bin')):
    stored = path.read_bytes()
    raw = decode(stored)
    packed = encode(raw)
    assert decode(packed) == raw, path
    if stored != raw:
        assert packed == stored, path
    _, cf, flags, w, h, stride, _ = struct.unpack_from('<BBHHHHH', packed)
    formats.add(cf)
    palette_size = 16 if cf == 9 else 256
    palette = [tuple(packed[12+i*4+j] for j in (2,1,0,3)) for i in range(palette_size)]
    payload = packed[12+palette_size*4:]
    dx, dy = ((48-w)//2, (48-h)//2) if flags == CENTERED_PET else (0,0)
    pixels = []
    for y in range(48):
        for x in range(48):
            sx, sy = x-dx, y-dy
            if not (0 <= sx < w and 0 <= sy < h):
                pixels.append((0,0,0,0))
            else:
                index = (payload[sy*stride+sx//2] >> (0 if sx%2 else 4)) & 15 if cf==9 else payload[sy*stride+sx]
                pixels.append(palette[index])
    original = Image.open(path.with_suffix('.png')).convert('RGBA')
    assert original.size == (48,48) and pixels == list(original.getdata()), path
    count += 1
    saved += len(raw)-len(packed)
    for invalid in (packed[:11], packed[:-1], packed+b'\0'):
        try:
            decode(invalid)
        except (AssertionError, struct.error):
            pass
        else:
            raise AssertionError(('malformed size accepted', path))
assert count and formats == {9,10}
print(f'PASS: {count} I4/I8 pet frames; original48px PNG pixels, centered placement, palette/indices and native BIN bytes exact; malformed sizes rejected; {saved}bytes saved. Read-only validation; no assets modified.')
