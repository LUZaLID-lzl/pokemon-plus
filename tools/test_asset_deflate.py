"""Cross-language exact static-background decode and malformed-stream guards."""
from pathlib import Path
import subprocess,tempfile
from asset_deflate import STATIC_FOLDERS,static_path,encode,decode
R=Path(__file__).resolve().parents[1];binary=R/'tests/build/test_asset_deflate';binary.parent.mkdir(exist_ok=True)
subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-g','-DPP_ASSET_HOST_TEST','-I',str(R/'main'),str(R/'tests/test_asset_deflate.c'),'-lz','-o',str(binary)],check=True)
raw_bytes=packed_bytes=count=0
with tempfile.TemporaryDirectory(prefix='pokemon-static-zlib-') as folder:
 for kind in sorted(STATIC_FOLDERS|{"gyms","ui","fishing"}):
  for p in sorted((R/'sdcard/pokemon-plus'/kind).glob('*.bin')):
   if not static_path(p):continue
   raw=p.read_bytes();packed=encode(raw);assert decode(packed)==raw
   for bad in (packed[:-1],packed+b'\0'):
    try:decode(bad)
    except AssertionError:pass
    else:raise AssertionError('Malformed wrapper was accepted')
   tmp=Path(folder)/(kind+'-'+p.name);tmp.write_bytes(packed)
   subprocess.run([str(binary),str(tmp),str(p)],check=True)
   count+=1;raw_bytes+=len(raw);packed_bytes+=len(packed)
print(f'PASS: {count} exact Python/host-C static background roundtrips; ASan/UBSan truncation, checksum, trailing data, dimensions/stride, capacity and guard bytes; {raw_bytes-packed_bytes} bytes saved; decoded {raw_bytes} bytes; device ROM primitive requires hardware validation')
