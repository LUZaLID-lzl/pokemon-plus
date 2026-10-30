"""Update the enlarged home coin without rebuilding unrelated pet/scene art."""
from pathlib import Path
import hashlib,json
from PIL import Image
from image_binary import write_bin
root=Path(__file__).resolve().parents[1]
out=root/'sdcard/pokemon-plus'
im=Image.open(out/'icons/15.png').convert('RGBA').resize((24,24),Image.Resampling.NEAREST)
p=out/'hud/coin-large.png';im.save(p);write_bin(im,p.with_suffix('.bin'),True)
changed={}
for p in [p,p.with_suffix('.bin')]:
 key=p.relative_to(out).as_posix();changed[key]={'path':key,'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
for path in [out/'manifest.json',root/'web/manifest.json']:
 manifest=json.loads(path.read_text());files={x['path']:x for x in manifest['files']};files.update(changed)
 manifest['files']=[files[k] for k in sorted(files)]
 path.write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
print('Updated 24px home coin; existing 16px shop/bag icon preserved')
