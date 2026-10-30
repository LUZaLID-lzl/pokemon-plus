"""Lossless row encoding for static indexed backgrounds, never pet frames.

Native files remain unchanged. The bank wrapper stores exact header/palette,
then each row as a literal, byte runs, or a copy of the previous row.
"""
import struct
MAGIC=b'PPRLE1\0\0'

def layout(data):
    assert len(data)>=12
    magic,cf,flags,w,h,stride,reserved=struct.unpack_from('<BBHHHHH',data)
    assert magic==0x19 and not flags and not reserved and w and h
    assert cf in (9,10) and stride==((w+1)//2 if cf==9 else w)
    prefix=12+(64 if cf==9 else 1024)
    assert len(data)==prefix+stride*h
    return prefix,stride,h

def encode(data):
    prefix,stride,h=layout(data)
    output=bytearray(MAGIC+struct.pack('<I',len(data))+data[:prefix]);previous=None
    for y in range(h):
        row=data[prefix+y*stride:prefix+(y+1)*stride]
        if row==previous:output.append(0)
        else:
            runs=bytearray();i=0
            while i<stride:
                j=i+1
                while j<stride and row[j]==row[i] and j-i<255:j+=1
                runs.extend((j-i,row[i]));i=j
            output.append(2 if len(runs)<stride else 1)
            output.extend(runs if len(runs)<stride else row)
        previous=row
    return bytes(output)

def decode(data):
    if not data.startswith(MAGIC):return data
    assert len(data)>=24
    expected=struct.unpack_from('<I',data,8)[0]
    _,cf,flags,w,h,stride,reserved=struct.unpack_from('<BBHHHHH',data,12)
    assert data[12]==0x19 and cf in (9,10) and not flags and not reserved and w and h
    assert stride==((w+1)//2 if cf==9 else w)
    prefix=12+(64 if cf==9 else 1024)
    assert expected==prefix+stride*h and len(data)>=12+prefix
    result=bytearray(data[12:12+prefix]);pos=12+prefix
    for y in range(h):
        assert pos<len(data);op=data[pos];pos+=1
        if op==0:
            assert y;result.extend(result[-stride:])
        elif op==1:
            assert pos+stride<=len(data);result.extend(data[pos:pos+stride]);pos+=stride
        else:
            assert op==2;row=bytearray()
            while len(row)<stride:
                assert pos+2<=len(data);n,value=data[pos:pos+2];pos+=2
                assert n and len(row)+n<=stride;row.extend(bytes([value])*n)
            result.extend(row)
    assert pos==len(data) and len(result)==expected
    layout(result)
    return bytes(result)
