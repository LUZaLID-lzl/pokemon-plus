"""Shared LVGL v9 native image encoder."""
import struct

def write_bin(im,path,alpha=False,indexed=False):
    im=im.convert('RGBA');w,h=im.size
    if indexed:
        colors=sorted(set(im.getdata()))
        assert len(colors)<=256
        if len(colors)<=16:
            indices={c:i for i,c in enumerate(colors)}
            palette=bytes(v for r,g,b,a in colors for v in (b,g,r,a))+bytes((16-len(colors))*4)
            pixels=bytearray()
            for y in range(h):
                for x in range(0,w,2):
                    pixels.append((indices[im.getpixel((x,y))]<<4)|(indices[im.getpixel((x+1,y))] if x+1<w else 0))
            path.write_bytes(struct.pack('<BBHHHHH',0x19,0x09,0,w,h,(w+1)//2,0)+palette+pixels)
            return
        indices={c:i for i,c in enumerate(colors)}
        palette=bytes(v for r,g,b,a in colors for v in (b,g,r,a))+bytes((256-len(colors))*4)
        path.write_bytes(struct.pack('<BBHHHHH',0x19,0x0A,0,w,h,w,0)+palette+bytes(indices[c] for c in im.getdata()))
        return
    # LVGL v9 image header: magic/format/flags, width/height, stride/reserved.
    pixels=bytearray();a=bytearray()
    for r,g,b,v in im.getdata():
        pixels+=struct.pack('<H',((r>>3)<<11)|((g>>2)<<5)|(b>>3));a.append(v)
    path.write_bytes(struct.pack('<BBHHHHH',0x19,0x14 if alpha else 0x12,0,w,h,w*2,0)+pixels+(a if alpha else b''))

