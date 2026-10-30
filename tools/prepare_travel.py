#!/usr/bin/env python3
"""Incremental travel-only export; preserve concurrent pet/UI work and release ZIP."""
from pathlib import Path
import json,hashlib,sys
from travel_art import load_catalog,write_header,export_travel
from image_binary import write_bin
root=Path(__file__).resolve().parents[1];out=root/'sdcard/pokemon-plus'
catalog=load_catalog(root)
if '--catalog-only' in sys.argv:
    write_header(root,catalog);raise SystemExit(0)
missing=[x['source'] for x in catalog['items'] if not (root/'assets'/x['source']).exists()]
if missing:raise SystemExit('Missing travel paintings; no runtime exports changed: '+', '.join(missing))
changed={}
def export(im,kind,name):
    target=out/kind;target.mkdir(exist_ok=True,parents=True)
    im.save(target/f'{name}.png');write_bin(im,target/f'{name}.bin',indexed=True)
    for suffix in ('.png','.bin'):
        p=target/(name+suffix);rel=str(p.relative_to(out))
        changed[rel]={'path':rel,'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
travel=export_travel(root,export)
for path in (out/'manifest.json',root/'web/manifest.json'):
    manifest=json.loads(path.read_text());manifest['travel']=travel
    files={f['path']:f for f in manifest['files']};files.update(changed)
    manifest['files']=[files[k] for k in sorted(files)]
    path.write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
print(f'Updated {len(changed)} travel files; preserved all other assets and release packages')
