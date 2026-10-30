"""Verify the checked-in, audited vendor files against the provenance manifest."""
from pathlib import Path
import hashlib
import json
root=Path(__file__).resolve().parents[1]
data=json.loads((root/'docs/vendor-manifest.json').read_text())
for item in data['files']:
    actual=hashlib.sha256((root/item['path']).read_bytes()).hexdigest()
    if actual!=item['project_sha256']:
        raise SystemExit('Vendor file changed; re-audit hardware provenance: '+item['path'])
print(f"Verified {len(data['files'])} vendor files, upstream {data['commit']}")
print('Documented source fixes: docs/vendor-changes.patch; no hardware parameter edits.')
