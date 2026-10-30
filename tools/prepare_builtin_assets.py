#!/usr/bin/env python3
"""Build a versioned, indexed image bank; no filesystem lookups on device."""
from pathlib import Path
import hashlib,json,struct
import re
from asset_deflate import encode, decode, static_path
root=Path(__file__).resolve().parents[1]
source=root/'sdcard/pokemon-plus'
cmake=(root/'CMakeLists.txt').read_text()
match=re.search(r'set\(PROJECT_VER\s+"([^"]+)"\)',cmake)
assert match, 'PROJECT_VER missing from CMakeLists.txt'
app_version=match.group(1)
version_bytes=app_version.encode('ascii')
assert 0<len(version_bytes)<16, 'Asset package version must fit the 16-byte header field'
files=sorted(source.rglob('*.bin'))
assert files
HEADER_SIZE=32
index_size=HEADER_SIZE+len(files)*40
# PPASSET3: magic / entry count / package size / NUL-terminated app version.
# The application rejects a package whose version differs from PP_APP_VERSION.
payload=bytearray(b'PPASSET3'+struct.pack('<II',len(files),0)+version_bytes.ljust(16,b'\0')+bytes(len(files)*40))
records=[]
saved=0
aliases={}
for i,file in enumerate(files):
    name=file.relative_to(source).as_posix().encode()
    assert len(name)<32
    payload.extend(bytes((-len(payload))%4))
    data=file.read_bytes();stored=data
    if static_path(file):
        compressed=encode(data)
        assert decode(compressed)==data
        if len(compressed)<len(data):stored=compressed
    saved+=len(data)-len(stored)
    if stored in aliases: offset=aliases[stored]
    else:
        offset=len(payload);aliases[stored]=offset;payload.extend(stored)
    struct.pack_into('<32sII',payload,HEADER_SIZE+i*40,name,offset,len(stored))
    records.append({'path':name.decode(),'bytes':len(data),'storedBytes':len(stored),'encoding':'zlib-static' if stored is not data else 'native','sha256':hashlib.sha256(data).hexdigest()})
struct.pack_into('<I',payload,12,len(payload))
assert len(payload)<=0xE70000
# Include alignment and the largest target/host ROM work state in the boot pool.
background_bytes=sum((f.stat().st_size+3)&~3 for f in files if static_path(f))
assert ((background_bytes+7)&~7)+11000<=5*1024*1024+320*1024
(root/'build/assets.bin').write_bytes(payload)
(root/'build/builtin-assets-manifest.json').write_text(json.dumps({
    'storage':'indexed-mmap', 'format':'PPASSET3', 'version':app_version,
    'files':records
},indent=2)+'\n')
print(f'Image bank v{app_version}: {len(files)} images / {len(payload)} bytes; static-background lossless saving {saved} bytes; pet frames remain native mmap')
