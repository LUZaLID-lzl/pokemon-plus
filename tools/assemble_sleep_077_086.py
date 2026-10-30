"""Compose five original full rows and one visually reviewed AI sleep row."""
from pathlib import Path
import argparse,hashlib,json
from statistics import median
from PIL import Image
from pet_identity import groups
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('dex',type=int,choices=[80,82]);p.add_argument('--version',type=int,default=2);a=p.parse_args();dex=a.dex;version=a.version
base=R/f'assets/source/animation-{dex:03}-master-v1.png';fix=R/f'assets/source/animation-{dex:03}-sleep-v{version}.png';out=R/f'assets/source/animation-{dex:03}-master-v{version}.png'
original=Image.open(base).convert('RGBA');sleep=Image.open(fix).convert('RGBA')
boxfile=base.with_name(base.stem+'-boxes.json')
if dex==56:
    pts={(x,y) for y in range(original.height) for x in range(original.width) if original.getpixel((x,y))[3]>=230}
    gs=[g for g in groups(pts) if len(g)>500];assert len(gs)==40
    rows=[]
    for row in range(5):
        rr=[g for g in gs if int(sum(y for x,y in g)/len(g)*5/original.height)==row];assert len(rr)==8
        rows.append(sorted(rr,key=lambda g:min(x for x,y in g)))
    boxes=[[(min(x for x,y in g),min(y for x,y in g),max(x for x,y in g)+1,max(y for x,y in g)+1) for g in row] for row in rows]
    boxes.insert(4,boxes[3]) # missing sleep band will be generated, never reused
else:boxes=json.loads(boxfile.read_text())
points={(x,y) for y in range(sleep.height) for x in range(sleep.width) if sleep.getpixel((x,y))[3]>=230}
components=[g for g in groups(points) if len(g)>500];assert len(components)==8
components.sort(key=lambda g:min(x for x,y in g))
bs=[(min(x for x,y in g),min(y for x,y in g),max(x for x,y in g)+1,max(y for x,y in g)+1) for g in components]
# Scale whole bodies uniformly to the same camera scale as original idle row.
scale=median(b[2]-b[0] for b in boxes[0])*1.0/median(b[2]-b[0] for b in bs)
row_h=max(max(b[3] for b in row)-min(b[1] for b in row) for row in boxes)+50
sheet=Image.new('RGBA',(original.width,row_h*6));parts=[]
for row,action in enumerate(['idle','walk','happy','eat','sleep','attack']):
    if action=='sleep':
        baseline=row_h*(row+1)-25
        for i,(b,g) in enumerate(zip(bs,components)):
            body=sleep.crop(b);mask=Image.new('L',body.size)
            for x,y in g:mask.putpixel((x-b[0],y-b[1]),255)
            body.putalpha(mask);sz=(round(body.width*scale),round(body.height*scale));body=body.resize(sz,Image.Resampling.NEAREST)
            pos=(round((i+.5)*sheet.width/8-body.width/2),baseline-body.height)
            assert 0<=pos[0] and pos[0]+sz[0]<=sheet.width and sz[0]<sheet.width/8*.92
            sheet.alpha_composite(body,pos);parts.append(dict(source=str(fix.relative_to(R)),sourceBox=b,destination=pos,destinationSize=sz,uniformScale=scale,action=action,frame=i))
    else:
        b=(0,min(b[1] for b in boxes[row]),original.width,max(b[3] for b in boxes[row]));body=original.crop(b);pos=(0,row_h*(row+1)-25-body.height)
        sheet.alpha_composite(body,pos);parts.append(dict(source=str(base.relative_to(R)),sourceBox=b,destination=pos,uniformScale=1,action=action))
sheet.save(out)
record=dict(method='Five pixel-exact original action bands plus eight complete AI-generated closed-eye sleep bodies, one uniform nearest-neighbor scale across the sleep row. No body-part deformation, painting, duplication or synthetic poses.',parts=parts,sources=[dict(path=str(f.relative_to(R)),sha256=hashlib.sha256(f.read_bytes()).hexdigest()) for f in (base,fix)],outputSha256=hashlib.sha256(out.read_bytes()).hexdigest(),additionalProvenance=[str(f.relative_to(R)) for f in (base,fix,R/f'assets/prompts/animation-{dex:03}/master-v1.txt',R/f'assets/prompts/animation-{dex:03}/generation-v1.log')])
out.with_name(out.stem+'-assembly.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n')
print(out)
