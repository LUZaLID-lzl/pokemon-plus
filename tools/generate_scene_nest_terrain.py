"""Sequential MobiIoT edits, preserving successes and the client's retry limit."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys
from scene_nest_terrain import ROOT, terrain_directory

p=argparse.ArgumentParser();p.add_argument('--client',type=Path,required=True);p.add_argument('--scenes',type=int,nargs='+',default=[12]+list(range(12)));args=p.parse_args()
directory=terrain_directory()
log=ROOT/f'assets/{directory}-generation.json'
record=json.loads(log.read_text()) if log.exists() else {'service':'MobiIoT','edits':{}}
for scene in args.scenes:
    output=ROOT/f'assets/source/{directory}/{scene:02}-edit.png'
    if output.exists():
        print(f'PRESERVED {scene:02}: existing generated edit',flush=True);continue
    command=[sys.executable,str(args.client),'edit','--prompt-file',str(ROOT/f'assets/prompts/{directory}/{scene:02}.txt'),'--image',str(ROOT/f'assets/reference/{directory}/{scene:02}.png'),'--image',str(ROOT/f'assets/reference/{directory}/{scene:02}-context.png'),'--output',str(output)]
    print(f'GENERATING ground edit {scene:02}',flush=True)
    run=subprocess.run(command,capture_output=True,text=True)
    print(run.stderr,flush=True)
    entry={'scene':scene,'success':run.returncode==0,'attempts':len(re.findall(r'Attempt \d+/6',run.stderr)),'prompt':f'prompts/{directory}/{scene:02}.txt','references':[f'reference/{directory}/{scene:02}.png',f'reference/{directory}/{scene:02}-context.png'],'output':f'source/{directory}/{scene:02}-edit.png'}
    if run.returncode==0:entry['model']=json.loads(run.stdout)['model']
    record['edits'][str(scene)]=entry;log.write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n')
    if run.returncode:raise SystemExit('Stopped after image client failure; earlier successful edits retained')
    print(f'COMPLETE {scene:02}',flush=True)
