#!/usr/bin/env python3
"""Read generated SPIFFS pages back and verify every packaged runtime file."""
from pathlib import Path
import hashlib
import json
import struct
import zipfile
from asset_row_rle import decode as decode_row
from asset_deflate import decode as decode_deflate

root = Path(__file__).resolve().parents[1]
folder = root / 'dist/firmware-all-in-one'
manifest = json.loads((folder / 'manifest.json').read_text())
merged = (folder / manifest['merged']['path']).read_bytes()
assert hashlib.sha256(merged).hexdigest() == manifest['merged']['sha256']
intervals = []
for item in manifest['flash_files']:
    data = (folder / item['path']).read_bytes()
    offset = int(item['offset'], 0)
    assert len(data) == item['bytes']
    assert hashlib.sha256(data).hexdigest() == item['sha256']
    assert merged[offset:offset + len(data)] == data, item['path']
    intervals.append((offset, offset + len(data)))
intervals.sort()
assert all(a[1] <= b[0] for a, b in zip(intervals, intervals[1:]))
assert len(merged) <= 16 * 1024 * 1024
table = (folder / 'partition_table/partition-table.bin').read_bytes()
parts = {}
for offset in range(0, len(table), 32):
    magic, typ, sub, start, size, name, flags = struct.unpack_from('<HBBII16sI', table, offset)
    if magic != 0x50AA:
        break
    parts[name.rstrip(b'\0').decode()] = (typ, sub, start, size)
assert parts['nvs'][2:] == (0x9000, 0x6000)
image = (folder / 'assets.bin').read_bytes()
assert parts['factory'] == (0, 0, 0x10000, 0x180000)
assert parts['assets'] == (1, 0x40, 0x190000, 0xE70000)
assert parts['assets'][2]+parts['assets'][3]==16*1024*1024
assert len((folder/'pokemon_plus.bin').read_bytes())<=parts['factory'][3]
assert len(image) <= parts['assets'][3]
assert image[:8] == b'PPASSET3'
version=image[16:32].split(b'\0',1)[0].decode('ascii')
assert version == manifest['version'], (version,manifest['version'])
names=32
entry_size=names+8
count,total=struct.unpack_from('<II',image,8)
assert total == len(image)
recovered={}
ends=[]
for i in range(count):
    name,offset,size=struct.unpack_from(f'<{names}sII',image,32+i*entry_size)
    name=name.split(b'\0',1)[0].decode()
    assert offset%4==0 and offset>=32+count*entry_size and offset+size<=total
    assert name not in recovered
    recovered[name]=decode_deflate(decode_row(image[offset:offset+size]))
    ends.append((offset,offset+size))
ends=sorted(set(ends))
assert all(a[1]<=b[0] for a,b in zip(ends,ends[1:]))
runtime={p.relative_to(root/'sdcard/pokemon-plus').as_posix():p.read_bytes() for p in (root/'sdcard/pokemon-plus').rglob('*.bin')}
assert recovered == runtime
with zipfile.ZipFile(root / 'dist/pokemon-plus-all-in-one.zip') as archive:
    assert archive.testzip() is None
    assert archive.read(manifest['merged']['path']) == merged
print(f'PASS: v{version} resource package recovered {len(runtime)} indexed images byte-for-byte; merged offsets, '
      f'partition bounds, NVS layout, SHA256 and ZIP verified ({len(merged)} bytes)')
