"""Lossless native indexed pet storage with a logical 48px canvas.

USER1 marks a symmetric transparent border omitted from the native I4/I8
bitmap. Palette and remaining pixel indices stay native LVGL bytes, suitable
for Flash mmap; consumers must center it on the original 48px canvas.
This module alone does not enable cropping in the runtime asset pipeline.
"""
import struct

CANVAS = 48
CENTERED_PET = 0x0100


def layout(data):
    assert len(data) >= 12
    magic, cf, flags, w, h, stride, reserved = struct.unpack_from('<BBHHHHH', data)
    assert magic == 0x19 and cf in (9, 10) and flags in (0, CENTERED_PET)
    assert not reserved and w and h
    assert stride == ((w + 1) // 2 if cf == 9 else w)
    prefix = 12 + (64 if cf == 9 else 1024)
    assert len(data) == prefix + stride * h
    if flags:
        assert w <= CANVAS and h <= CANVAS and not (w % 2 or h % 2)
        assert data[12:16] == bytes(4)
    return cf, flags, w, h, stride, prefix


def encode(data):
    cf, flags, w, h, stride, prefix = layout(data)
    assert not flags and (w, h) == (CANVAS, CANVAS)
    if data[12:16] != bytes(4):
        return data
    indices = [[((data[prefix+y*stride+x//2] >> (0 if x % 2 else 4)) & 15)
                if cf == 9 else data[prefix+y*stride+x] for x in range(w)] for y in range(h)]
    points = [(x,y) for y in range(h) for x in range(w) if indices[y][x]]
    if not points:
        return data
    dx = min(min(x for x,y in points), w-1-max(x for x,y in points))
    dy = min(min(y for x,y in points), h-1-max(y for x,y in points))
    if not (dx or dy):
        return data
    cw, ch = w-2*dx, h-2*dy
    payload = bytearray()
    for row in indices[dy:h-dy]:
        row = row[dx:w-dx]
        if cf == 9:
            payload.extend((row[i] << 4) | row[i+1] for i in range(0, cw, 2))
        else:
            payload.extend(row)
    out = struct.pack('<BBHHHHH',0x19,cf,CENTERED_PET,cw,ch,cw//2 if cf==9 else cw,0)+data[12:prefix]+payload
    assert decode(out) == data
    return out


def decode(data):
    cf, flags, w, h, stride, prefix = layout(data)
    if not flags:
        return data
    dx, dy = (CANVAS-w)//2, (CANVAS-h)//2
    full_stride = CANVAS//2 if cf == 9 else CANVAS
    pixels = bytearray(full_stride*CANVAS)
    for y in range(h):
        for x in range(w):
            value = ((data[prefix+y*stride+x//2] >> (0 if x%2 else 4)) & 15) if cf==9 else data[prefix+y*stride+x]
            offset = (y+dy)*full_stride + ((x+dx)//2 if cf==9 else x+dx)
            pixels[offset] |= value << (0 if (x+dx)%2 else 4) if cf==9 else value
    return struct.pack('<BBHHHHH',0x19,cf,0,CANVAS,CANVAS,full_stride,0)+data[12:prefix]+pixels
