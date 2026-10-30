#!/usr/bin/env python3
"""Assemble five accepted AI action rows and one AI attack row; never deform bodies."""
import hashlib,json
from pathlib import Path
from statistics import median
from PIL import Image
from pet_identity import groups
R=Path(__file__).resolve().parents[1]
def measured(path):
 im=Image.open(R/path).convert('RGBA')
 gs=[g for g in groups({(x,y) for y in range(im.height) for x in range(im.width) if im.getpixel((x,y))[3]>=230}) if len(g)>500]
 return im,[(min(x for x,y in g),min(y for x,y in g),max(x for x,y in g)+1,max(y for x,y in g)+1) for g in gs]
base='assets/source/animation-046-master-v1.png';attack='assets/source/animation-046-attack-v3.png'
a,boxes=measured(base);b,attack_boxes=measured(attack)
assert len(boxes)==40 and len(attack_boxes)==8
rows=[sorted([box for box in boxes if int((box[1]+box[3])/2*5/950)==r],key=lambda box:box[0]) for r in range(5)]
assert all(len(row)==8 for row in rows)
# Match drawing scale once for the entire attack strip, using resting body heights.
scale=median(box[3]-box[1] for box in rows[0])/median(box[3]-box[1] for box in attack_boxes)
master=Image.new('RGBA',(1448,6*200));parts=[]
for r,row in enumerate(rows):
 top=min(box[1] for box in row)-4;bottom=max(box[3] for box in row)+4
 dest_y=r*200+150-(max(box[3] for box in row)-top)
 band=a.crop((0,top,a.width,bottom));master.alpha_composite(band,(0,dest_y))
 parts.append({'source':base,'sourceBox':[0,top,a.width,bottom],'destination':[0,dest_y],'uniformScale':1,'action':['idle','walk','happy','eat','sleep'][r]})
for col,box in enumerate(sorted(attack_boxes,key=lambda box:box[0])):
 cell=b.crop(box);cell=cell.resize((round(cell.width*scale),round(cell.height*scale)),Image.Resampling.NEAREST)
 pos=(round((col+.5)*1448/8-cell.width/2),1150-cell.height)
 master.alpha_composite(cell,pos)
 parts.append({'source':attack,'sourceBox':box,'destination':pos,'destinationSize':cell.size,'uniformScale':scale,'action':'attack','frame':col})
out=R/'assets/source/animation-046-master-v3.png';master.save(out)
meta={'method':'Five pixel-exact original row bands plus eight complete AI-generated attack bodies; one uniform nearest-neighbor scale for the entire attack row. No cut-body deformation or duplicated frames.','parts':parts,'sources':[{'path':p,'sha256':hashlib.sha256((R/p).read_bytes()).hexdigest()} for p in (base,attack)],'outputSha256':hashlib.sha256(out.read_bytes()).hexdigest(),'additionalProvenance':[base,'assets/prompts/animation-046/master-v1.txt','assets/prompts/animation-046/generation-v1.log',attack,'assets/prompts/animation-046/attack-v3.txt']}
(R/'assets/source/animation-046-master-v3-assembly.json').write_text(json.dumps(meta,ensure_ascii=False,indent=2)+'\n')
prompt=(R/'assets/prompts/animation-046/attack-v3.txt').read_text()
(R/'assets/prompts/animation-046/master-v3.txt').write_text('COMPOSITE SOURCE: see assets/source/animation-046-master-v3-assembly.json. First five actions from v1; attack row generated with the following prompt and uniformly normalized, all complete bodies preserved.\n\n'+prompt)
print('PASS: assembled 40 original + 8 generated attack poses; attack uniform scale',scale)
