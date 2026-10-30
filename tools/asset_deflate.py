"""Lossless boot-only compression for static backgrounds; native files stay exact."""
import struct,zlib
MAGIC=b'PPZLIB1\0'
STATIC_FOLDERS={'training','scene-fronts','thumbs','scenes','strips','battle-scenes','travel-scenes'}
def static_path(path):
    return (path.parent.name=='icons-large' and path.name in {'10.bin','12.bin'}) or (path.parent.name=='hospital' and path.name=='room.bin') or path.parent.name in STATIC_FOLDERS or (path.parent.name=='fishing' and (path.name=='pond.bin' or path.name.startswith(('fish-','thumb-')))) or (path.parent.name=='ui' and path.name=='menu-bg.bin') or (path.parent.name=='gyms' and (path.name in {'map.bin','arena.bin'} or path.name.endswith('-map.bin') or path.name.endswith('-arena.bin')))
def native_size(header):
    assert len(header)>=12
    magic,cf,flags,w,h,stride,reserved=struct.unpack_from('<BBHHHHH',header)
    assert magic==0x19 and not flags and not reserved and w and h
    assert cf in (9,10,18,20)
    assert stride==((w+1)//2 if cf==9 else w if cf==10 else w*2)
    size=12+(64 if cf==9 else 1024 if cf==10 else 0)+stride*h+(w*h if cf==20 else 0)
    assert size<=2*1024*1024
    return size

def encode(data):
    assert native_size(data)==len(data)
    return MAGIC+struct.pack('<I',len(data))+data[:12]+zlib.compress(data[12:],9)

def decode(data):
    if not data.startswith(MAGIC):return data
    assert len(data)>24
    n=struct.unpack_from('<I',data,8)[0]
    assert n==native_size(data[12:24])
    stream=zlib.decompressobj();payload=stream.decompress(data[24:],n-12+1)
    assert stream.eof and not stream.unused_data and not stream.unconsumed_tail and len(payload)==n-12
    return data[12:24]+payload
