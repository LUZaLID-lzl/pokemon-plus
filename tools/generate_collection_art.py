"""Sequential MobiIoT requests; the skill owns retry limits. Stop on exhausted batch."""
from pathlib import Path
import json,subprocess,sys,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
SCRIPT=Path('/Users/lzl/.codex/skills/mobiiot-imagegen/scripts/imagegen.py')
p=ROOT/'assets/collection-generation.json'
data=json.loads(p.read_text()) if p.exists() else {'service':'mobiiot-imagegen','batches':[]}
for batch in range(12):
 target=ROOT/f'assets/source/collection-v1/{batch:02}.png'
 if not target.exists():
  result=subprocess.run([sys.executable,str(SCRIPT),'generate','--prompt-file',str(ROOT/f'assets/prompts/collection-v1/{batch:02}.txt'),'--output',str(target)])
  if result.returncode:
   data['failedBatch']=batch;data['status']='stopped-after-skill-retry-limit';p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n');sys.exit(result.returncode)
 if not any(x['id']==batch for x in data['batches']):
  data['batches'].append(dict(id=batch,source=str(target.relative_to(ROOT/'assets')),sha256=hashlib.sha256(target.read_bytes()).hexdigest(),prompt=f'prompts/collection-v1/{batch:02}.txt',status='generated-awaiting-export-review'))
 data['status']='generating' if batch<11 else 'generated-awaiting-export-review'
 p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')
 print(f'Collection art batch {batch:02}/11 saved',flush=True)
