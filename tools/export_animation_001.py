#!/usr/bin/env python3
"""Export reviewed 001 animation from measured alpha components, fixed scale/palette."""
from pathlib import Path
from collections import Counter
from statistics import median
from PIL import Image, ImageDraw, ImageFont, ImageOps
import json,hashlib,zipfile
R=Path(__file__).resolve().parents[1]
SOURCE=R/'assets/source/animation-001-master-v1.png'
BASE=R/'assets/pokemon/gen1-001-009/001.png'
OUT=R/'assets/animation-templates/001';OUT.mkdir(parents=True,exist_ok=True)
ACTIONS=['idle','walk','happy','eat','sleep','attack']
LABELS=['待机·呼吸眨眼','行走·交替迈腿','开心·起跳落地','进食·低头咀嚼','睡眠·闭眼呼吸','攻击·蓄力收回']
INK=(27,38,58,255)
def groups(points):
    points=set(points);out=[]
    while points:
        p=points.pop();q=[p];component={p}
        while q:
            x,y=q.pop()
            for n in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
                if n in points:points.remove(n);component.add(n);q.append(n)
        out.append(component)
    return out

def neighbors(p):
    x,y=p;return [(x-1,y),(x+1,y),(x,y-1),(x,y+1)]

source=Image.open(SOURCE).convert('RGBA')
components=[g for g in groups({(x,y) for y in range(source.height) for x in range(source.width) if source.getpixel((x,y))[3]>=160}) if len(g)>500]
assert len(components)==48,'Expected exactly 48 separate characters; inspect atlas'
rows=[]
for row in range(6):
    entries=[g for g in components if int(sum(y for x,y in g)/len(g)*6/source.height)==row]
    assert len(entries)==8,('Expected 8 measured characters in row',row,len(entries))
    rows.append(sorted(entries,key=lambda g:min(x for x,y in g)))
boxes=[[(min(x for x,y in g),min(y for x,y in g),max(x for x,y in g)+1,max(y for x,y in g)+1) for g in row] for row in rows]
(R/'assets/source/animation-001-master-v1-boxes.json').write_text(json.dumps(boxes)+'\n')
scale=40/max(max(b[2]-b[0],b[3]-b[1]) for row in boxes for b in row)
colors=sorted({p[:3] for p in Image.open(BASE).convert('RGBA').getdata() if p[3]})
palette=Image.new('P',(1,1));palette.putpalette([c for rgb in colors for c in rgb]+list(colors[0])*(256-len(colors)))
font=ImageFont.truetype('/System/Library/Fonts/STHeiti Medium.ttc',15)
sheet=Image.new('RGB',(1152,936),'#e1e9df');d=ImageDraw.Draw(sheet)
all_frames=[];items=[]
for row,action in enumerate(ACTIONS):
    baseline=max(b[3] for b in boxes[row]);frames=[]
    for col,g in enumerate(rows[row]):
        box=boxes[row][col];cell=source.crop(box)
        mask=Image.new('L',cell.size)
        for x,y in g:mask.putpixel((x-box[0],y-box[1]),255)
        cell.putalpha(mask)
        cell=cell.resize((round(cell.width*scale),round(cell.height*scale)),Image.Resampling.NEAREST)
        rgb=cell.convert('RGB').quantize(palette=palette,dither=Image.Dither.NONE).convert('RGB')
        cell=Image.merge('RGBA',(*rgb.split(),cell.getchannel('A')))
        delta=max(-3,min(0,round((box[3]-baseline)*scale)))
        frame=Image.new('RGBA',(48,48));frame.alpha_composite(cell,((48-cell.width)//2,43-cell.height+delta))
        mask={(x,y) for y in range(48) for x in range(48) if frame.getpixel((x,y))[3]}
        # Join only adjacent native fragments lost during reduction; no scaling.
        chunks=sorted(groups(mask),key=len,reverse=True)
        while len(chunks)>1:
            main=chunks[0];other=set().union(*chunks[1:]);a,b=min(((a,b) for a in main for b in other),key=lambda t:abs(t[0][0]-t[1][0])+abs(t[0][1]-t[1][1]))
            assert abs(a[0]-b[0])+abs(a[1]-b[1])<=3,'Review detached native feature'
            x,y=a
            while (x,y)!=b:
                if x!=b[0]:x+=1 if b[0]>x else -1
                else:y+=1 if b[1]>y else -1
                mask.add((x,y));frame.putpixel((x,y),INK)
            chunks=sorted(groups(mask),key=len,reverse=True)
        outline={p for p in mask if any(n not in mask for n in neighbors(p))}
        # Connect diagonal boundary steps through an existing interior pixel.
        for unused in range(4):
            added=set()
            for y in range(47):
                for x in range(47):
                    a,b,c,e=(x,y),(x+1,y),(x,y+1),(x+1,y+1);options=[]
                    if a in outline and e in outline and b not in outline and c not in outline:options=[p for p in (b,c) if p in mask]
                    if b in outline and c in outline and a not in outline and e not in outline:options=[p for p in (a,e) if p in mask]
                    if options:added.add(min(options,key=lambda p:(p[0]-23.5)**2+(p[1]-23.5)**2))
            if not added:break
            outline|=added
        for p in outline:frame.putpixel(p,INK)
        assert len(groups(mask))==1 and len(groups(outline))==1
        assert len({p for p in frame.getdata() if p[3]})<=16
        assert max(frame.getbbox()[2]-frame.getbbox()[0],frame.getbbox()[3]-frame.getbbox()[1])<=40
        path=f'{action}-{col:02}.png';frame.save(OUT/path)
        folder=OUT/'right';folder.mkdir(exist_ok=True);ImageOps.mirror(frame).save(folder/path)
        frames.append(frame);items.append({'action':action,'frame':col,'image':path,'sourceBox':box,'sha256':hashlib.sha256((OUT/path).read_bytes()).hexdigest()})
        large=frame.resize((144,144),Image.Resampling.NEAREST);sheet.paste(large,(col*144,row*156),large)
    d.text((8,row*156+135),LABELS[row],font=font,fill='#26394c')
    all_frames.append(frames)
sheet.save(OUT/'contact-sheet.png')
atlas=Image.new('RGBA',(48*8,48*6))
for row,frames in enumerate(all_frames):
    for col,frame in enumerate(frames):atlas.alpha_composite(frame,(col*48,row*48))
atlas.save(OUT/'atlas.png');atlas.resize((1536,1152),Image.Resampling.NEAREST).save(OUT/'atlas@4x.png')
# 8 phases at 12fps (centisecond durations total 670ms), slower idle/eat/sleep.
durations={'idle':[140]*8,'walk':[80,90,80,80,90,80,80,90],'happy':[90]*8,'eat':[130]*8,'sleep':[240]*8,'attack':[80,70,70,80,70,80,90,130]}
for row,action in enumerate(ACTIONS):
    frames=[]
    for frame in all_frames[row]:
        im=Image.new('RGB',(288,288),'#e1e9df');large=frame.resize((288,288),Image.Resampling.NEAREST);im.paste(large,(0,0),large);frames.append(im)
    frames[0].save(OUT/f'{action}.gif',save_all=True,append_images=frames[1:],duration=durations[action],loop=0,disposal=2,optimize=False)
manifest={'version':1,'nationalId':1,'name':'妙蛙种子','kind':'generated animation template','gameIntegrated':True,'source':str(SOURCE.relative_to(R)),'sourceSha256':hashlib.sha256(SOURCE.read_bytes()).hexdigest(),'identityReference':str(BASE.relative_to(R)),'prompt':'assets/prompts/animation-001/master-v1.txt','generator':'mobiiot-imagegen','model':'gpt-image-2.5-sunburst','attempts':1,'nativeCanvas':48,'silhouetteMax':40,'framesPerAction':8,'frames':48,'sourceFacing':'left','directions':{'left':'.','right':'right'},'palette':'shared 16-color palette from approved 001 native sprite','scale':scale,'actions':{a:{'frames':[f'{a}-{i:02}.png' for i in range(8)],'durationsMs':durations[a],'mode':'loop' if a in ('idle','walk','sleep','eat') else 'one-shot'} for a in ACTIONS},'processing':'Measured alpha components, one fixed global scale, shared reference palette, anchored baseline, connected native navy outline; no whole-body stretch or cut-body deformation','items':items}
(OUT/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
(OUT/'README.md').write_text('# 001 妙蛙种子动作模板\n\n48×48透明PNG，六动作各8帧，共48个MobiIoT生成姿态；统一原画色板、固定缩放与脚底基线，附左右朝向、图集、GIF和场景演示。动作图集仍需对未来物种逐只检查，不能用一套肢体结构套用所有宠物。\n\nv0.6.7已接入固件与资源工坊：001每动作8帧，逐帧时长取assets/pet-animation.json，开心与攻击播完保持末帧；其他三只保留4帧动作。左右朝向均在启动时预建描述符。\n')
print('PASS: 48 measured source components, 48 native frames, binary alpha, connected navy outlines, shared <=16 colors and exact mirrors')
print(OUT/'contact-sheet.png')
