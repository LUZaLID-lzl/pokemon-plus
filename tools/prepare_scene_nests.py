"""Incremental nest export; keep the full asset builder on the same recipe."""
from pathlib import Path
import hashlib
import json
from scene_nest_art import ROOT, export_nests, manifest_nests, record_generation
from image_binary import write_bin

out = ROOT / 'sdcard/pokemon-plus'
changed = {}
stale = {f'nests/{i}{suffix}' for i in range(13) for suffix in ('.png', '.bin')}
for name in stale:
    (out / name).unlink(missing_ok=True)

def export(im, kind, name, alpha=False, indexed=True):
    target = out / kind / name
    target.parent.mkdir(parents=True, exist_ok=True)
    im.save(target.with_suffix('.png'))
    write_bin(im, target.with_suffix('.bin'), alpha=alpha, indexed=indexed)
    for suffix in ('.png', '.bin'):
        file = target.with_suffix(suffix)
        key = file.relative_to(out).as_posix()
        changed[key] = dict(path=key, bytes=file.stat().st_size,
                            sha256=hashlib.sha256(file.read_bytes()).hexdigest())

export_nests(export)
record_generation()
for path in (out / 'manifest.json', ROOT / 'web/manifest.json'):
    manifest = json.loads(path.read_text())
    files = {f['path']: f for f in manifest['files'] if f['path'] not in stale}
    files.update(changed)
    manifest['files'] = sorted(files.values(), key=lambda f: f['path'])
    manifest['sceneNests'] = manifest_nests()
    path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n')
print(f'Updated {len(changed)} nest image files and shared C/Web scene anchors')
