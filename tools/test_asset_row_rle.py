"""Cross-language exact decode and bounds checks for real static backgrounds."""
from pathlib import Path
import subprocess,tempfile
from asset_row_rle import encode,decode
R=Path(__file__).resolve().parents[1]
binary=R/'tests/build/test_asset_row_rle';binary.parent.mkdir(exist_ok=True)
subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-g','-I',str(R/'main'),str(R/'tests/test_asset_row_rle.c'),'-o',str(binary)],check=True)
raw_bytes=packed_bytes=0
with tempfile.TemporaryDirectory(prefix='pokemon-row-rle-') as folder:
    for p in sorted((R/'sdcard/pokemon-plus/scenes').glob('*.bin')):
        raw=p.read_bytes();packed=encode(raw);assert decode(packed)==raw
        tmp=Path(folder)/p.name;tmp.write_bytes(packed)
        subprocess.run([str(binary),str(tmp),str(p)],check=True)
        raw_bytes+=len(raw);packed_bytes+=len(packed)
print(f'PASS: all 12 exact Python/C background roundtrips; ASan/UBSan truncation, bad opcode/run, first-row-copy, wrong size and capacity guards; {raw_bytes-packed_bytes} bytes saved')
