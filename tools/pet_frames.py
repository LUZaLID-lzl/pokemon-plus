"""Derive gentle motion from reviewed native art without deforming its pixels.

A single source pose supplies 24 playback slots, not 24 hand-drawn poses.
Resting keeps the source eyes/shape; no closed-eye sleeping art is claimed.
"""
from PIL import Image

# Integer translations only; all loops return smoothly to their first frame.
MOTION = {
    'idle': [(0, 0), (0, 0), (0, -1), (0, 0)],
    'walk': [(0, 0), (0, -1), (0, 0), (0, -1)],
    'happy': [(0, 0), (0, -1), (0, -3), (0, -1)],
    'eat': [(0, 0), (0, 1), (0, 1), (0, 0)],
    'sleep': [(0, 0)] * 4,
    'attack': [(0, 0), (-1, 0), (-2, 0), (-1, 0)],
}


def build_frames(source):
    source = source.convert('RGBA')
    if source.size != (48, 48) or not source.getbbox():
        raise ValueError('Expected nonempty reviewed 48px native sprite')
    if set(source.getchannel('A').getdata()) != {0, 255}:
        raise ValueError('Expected binary transparent source')
    x0, y0, x1, y1 = source.getbbox()
    frames = []
    for offsets in MOTION.values():
        for dx, dy in offsets:
            if not (0 <= x0 + dx < x1 + dx <= 48 and
                    0 <= y0 + dy < y1 + dy <= 48):
                raise ValueError('Motion would clip reviewed art')
            frame = Image.new('RGBA', (48, 48))
            frame.paste(source, (dx, dy))
            frames.append(frame)
    return frames
