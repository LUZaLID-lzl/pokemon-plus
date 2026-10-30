#!/usr/bin/env python3
"""Expected visible BGRA pixels from unchanged full48px playback PNGs."""
from pathlib import Path
import struct, zlib
from PIL import Image
R=Path(__file__).resolve().parents[1]
paths=sorted((R/'sdcard/pokemon-plus').glob('pets*/*.png'))
out=R/'build/pet-canvas-checks.bin'
# Preserve the established integer center-sampling contract. Pillow's floating
# resize can choose the other side of an exact56px tie for some coordinates.
offsets={size:[(((2*y+1)*48//(2*size))*48+(2*x+1)*48//(2*size))*4
               for y in range(size) for x in range(size)] for size in (48,56,96)}
with out.open('wb') as f:
    f.write(b'PPCANV1\0'+struct.pack('<I',len(paths)))
    for p in paths:
        name=p.with_suffix('.bin').relative_to(R/'sdcard/pokemon-plus').as_posix().encode()
        assert len(name)<64
        im=Image.open(p).convert('RGBA');assert im.size==(48,48)
        # Fully transparent colors have no visible content; native palette0=0.
        im.putdata([pixel if pixel[3] else (0,0,0,0) for pixel in im.getdata()])
        raw=im.tobytes('raw','BGRA')
        checks=[zlib.crc32(b''.join(raw[i:i+4] for i in offsets[size])) for size in (48,56,96)]
        f.write(struct.pack('<64sIII',name,*checks))
print(f'Prepared full48px PNG references for {len(paths)} frames at48/56/96px: {out}')
