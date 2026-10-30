#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Export reference-grounded animation from measured alpha components, fixed scale/palette."""
from pathlib import Path
import argparse
parser=argparse.ArgumentParser();parser.add_argument("dex",type=int);parser.add_argument("--version",type=int,default=1);parser.add_argument('--alpha-threshold',type=int,default=160);parser.add_argument('--identity',help='Project-relative reviewed native art for an unpublished batch');parser.add_argument('--name',help='Name of an unpublished form');args=parser.parse_args();DEX=args.dex;VERSION=args.version
NAMES=["妙蛙种子","妙蛙草","妙蛙花","小火龙","火恐龙","喷火龙","杰尼龟","卡咪龟","水箭龟"]
R=Path(__file__).resolve().parents[1]
import json
roster=json.loads((R/"assets/pet-roster.json").read_text())
NAME=args.name or next(p["name"] for p in roster if p["nationalId"]==DEX)
from collections import Counter
from statistics import median
from PIL import Image, ImageDraw, ImageFont, ImageOps
import json,hashlib,zipfile
R=Path(__file__).resolve().parents[1]
SOURCE=R/f'assets/source/animation-{DEX:03}-master-v{VERSION}.png'
BASE=R/args.identity if args.identity else R/'assets'/next(p['source'] for p in roster if p['nationalId']==DEX)
OUT=R/f'assets/animation-templates/{DEX:03}';OUT.mkdir(parents=True,exist_ok=True)
ACTIONS=['idle','walk','happy','eat','sleep','attack']
LABELS=['待机·呼吸眨眼','行走·交替迈腿','开心·起跳落地','进食·低头咀嚼','睡眠·闭眼呼吸','攻击·蓄力收回']
if DEX in (11,14): LABELS=['待机·轻摇','移动·摇摆挪动','开心·轻弹','进食·轻微脉动','睡眠·闭眼静息','攻击·前倾撞击']
if DEX in (10,13): LABELS[1]='移动·分节爬行'
if DEX in (12,15): LABELS[1]='移动·振翅飞行'
if DEX==49: LABELS[1]='移动·振翅飞行'; LABELS[4]='睡眠·合翼闭眼'
if DEX in (50,51): LABELS=['待机·探头眨眼','移动·探身挪动','开心·轻摆探头','进食·轻动口鼻','睡眠·闭眼静息','攻击·前倾收回']
if DEX in (41,42): LABELS=['待机·轻振翅','移动·振翅飞行','开心·扬翅','进食·轻启口','睡眠·合翼静息' if DEX==41 else '睡眠·合翼闭眼','攻击·俯冲收回']
if DEX in (23,24): LABELS[1]='移动·蛇形滑行'; LABELS[2]='开心·抬头摇摆'
if DEX==63: LABELS[0]='待机·闭目冥想'; LABELS[4]='睡眠·蜷身静息'
if DEX in (70,71): LABELS[1]='移动·摆叶悬移'; LABELS[2]='开心·扬叶轻摆'
if DEX in (72,73): LABELS[1]='移动·触手游动'; LABELS[2]='开心·轻卷触手'; LABELS[4]='睡眠·闭眼浮休'
if DEX==74: LABELS[1]='移动·悬浮摆臂'; LABELS[2]='开心·抬臂轻摆'; LABELS[4]='睡眠·闭眼悬休'
if DEX in (81,82): LABELS[1]='移动·磁体悬浮'; LABELS[2]='开心·扬磁轻摆'; LABELS[3]='进食·磁体轻动'; LABELS[4]='睡眠·闭眼悬休'
if DEX in (86,87): LABELS[1]='移动·鳍肢滑行'; LABELS[2]='开心·抬鳍轻摆'; LABELS[4]='睡眠·闭眼低卧'
if DEX in (88,89): LABELS[1]='移动·泥褶挪动'; LABELS[2]='开心·抬臂轻摆'
if DEX in (90,91): LABELS[1]='移动·摇壳挪动'; LABELS[2]='开心·轻启贝壳'; LABELS[4]='睡眠·合壳静息'
if DEX==91: LABELS[4]='睡眠·闭眼静息'
if DEX in (92,93): LABELS[1]='移动·幽灵悬浮'; LABELS[2]='开心·轻摆'; LABELS[4]='睡眠·闭眼悬休'
if DEX==95: LABELS[1]='移动·岩节盘行'; LABELS[2]='开心·抬头轻摆'; LABELS[4]='睡眠·盘身闭眼'
if DEX in (98,99): LABELS[1]='移动·蟹足横移'; LABELS[2]='开心·抬钳轻摆'; LABELS[4]='睡眠·低伏闭眼'; LABELS[5]='攻击·合钳收回'
if DEX in (100,101): LABELS[1]='移动·球身轻滚'; LABELS[2]='开心·神情变化'; LABELS[4]='睡眠·闭眼静息'; LABELS[5]='攻击·倾身收回'
if DEX in (100,106): LABELS[3]='进食·专注轻动'
if DEX==102: LABELS[1]='移动·蛋群倾摇'; LABELS[2]='开心·表情轻动'; LABELS[4]='睡眠·蛋群闭眼'
if DEX==103: LABELS[1]='移动·迈步摆叶'; LABELS[4]='睡眠·三头闭眼'
if DEX==104: LABELS[4]='睡眠·低坐闭眼'; LABELS[5]='攻击·凝神收回'
if DEX==105: LABELS[4]='睡眠·抱骨闭眼'; LABELS[5]='攻击·挥骨收回'
if DEX==106: LABELS[1]='移动·屈膝迈步'; LABELS[4]='睡眠·屈腿闭眼'; LABELS[5]='攻击·抬腿收回'
if DEX==107: LABELS[4]='睡眠·低坐闭眼'; LABELS[5]='攻击·挥拳收回'
if DEX==108: LABELS[4]='睡眠·低坐闭眼'; LABELS[5]='攻击·舌探收回'
if DEX in (109,110): LABELS[1]='移动·轻摇悬浮'; LABELS[2]='开心·神情轻动'; LABELS[4]='睡眠·闭眼悬休'; LABELS[5]='攻击·倾身收回'
if DEX==111: LABELS[1]='移动·四足交替'; LABELS[4]='睡眠·低卧闭眼'; LABELS[5]='攻击·俯身收回'
if DEX==112: LABELS[4]='睡眠·低坐闭眼'
if DEX==113: LABELS[4]='睡眠·抱蛋闭眼'; LABELS[5]='攻击·护蛋收回'
if DEX==114: LABELS[1]='移动·藤足挪动'; LABELS[2]='开心·藤蔓轻摆'; LABELS[3]='进食·专注轻动'; LABELS[4]='睡眠·藤身闭眼'; LABELS[5]='攻击·摆藤收回'
if DEX==115: LABELS[4]='睡眠·亲子闭眼'; LABELS[5]='攻击·护袋收回'
if DEX==116: LABELS[1]='移动·摆鳍游动'; LABELS[2]='开心·轻摆卷尾'; LABELS[4]='睡眠·闭眼浮休'; LABELS[5]='攻击·探头收回'
if DEX==117: LABELS[1]='移动·摆鳍游动'; LABELS[4]='睡眠·闭眼浮休'; LABELS[5]='攻击·探头收回'
if DEX in (118,119): LABELS[1]='移动·摆尾游动'; LABELS[2]='开心·扬鳍轻摆'; LABELS[4]='睡眠·闭眼浮休'; LABELS[5]='攻击·倾身收回'
if DEX in (120,121): LABELS=['待机·星臂轻动','移动·星臂游动','开心·扬臂轻摆','进食·凝神轻动','睡眠·收臂暗心','攻击·倾星收回']
if DEX==122: LABELS[4]='睡眠·低坐闭眼'; LABELS[5]='攻击·掌势收回'
if DEX==123: LABELS[1]='移动·迈步振翅'; LABELS[4]='睡眠·合翼闭眼'; LABELS[5]='攻击·前臂收回'
if DEX==124: LABELS[1]='移动·迈步摆袖'; LABELS[4]='睡眠·低坐闭眼'; LABELS[5]='攻击·抬掌收回'
if DEX in (125,126): LABELS[4]='睡眠·低坐闭眼'; LABELS[5]='攻击·摆臂收回'
if DEX==127: LABELS[4]='睡眠·低坐闭眼'; LABELS[5]='攻击·摆臂收回'
if DEX in (128,133,134,135,136): LABELS[1]='移动·四足交替'; LABELS[2]='开心·抬头轻摆'; LABELS[4]='睡眠·低卧闭眼'; LABELS[5]='攻击·俯身收回'
if DEX==129: LABELS[1]='移动·摆尾游动'; LABELS[2]='开心·扬鳍轻摆'; LABELS[4]='睡眠·闭眼浮休'; LABELS[5]='攻击·倾身收回'
if DEX in (130,147,148): LABELS[1]='移动·蛇身游动'; LABELS[2]='开心·抬头轻摆'; LABELS[4]='睡眠·盘身闭眼'; LABELS[5]='攻击·探身收回'
if DEX==131: LABELS[1]='移动·鳍肢划水'; LABELS[2]='开心·抬头轻摆'; LABELS[4]='睡眠·伏颈闭眼'; LABELS[5]='攻击·探头收回'
if DEX==132: LABELS[1]='移动·软体挪动'; LABELS[2]='开心·侧瓣轻摆'; LABELS[4]='睡眠·低伏闭眼'; LABELS[5]='攻击·凝神收回'
if DEX==137: LABELS[1]='移动·多边足交替'; LABELS[2]='开心·抬头轻摆'; LABELS[4]='睡眠·低伏闭眼'; LABELS[5]='攻击·探头收回'
if DEX in (138,139): LABELS[1]='移动·触足挪动'; LABELS[2]='开心·抬触轻摆'; LABELS[4]='睡眠·收触闭眼'; LABELS[5]='攻击·触足收回'
if DEX==140: LABELS[1]='移动·四足交替'; LABELS[2]='开心·摇壳轻摆'; LABELS[3]='进食·凝神轻动'; LABELS[4]='睡眠·伏壳闭眼'; LABELS[5]='攻击·足撑收回'
if DEX in (141,143,149,150): LABELS[4]='睡眠·低坐闭眼'; LABELS[5]='攻击·摆臂收回'
if DEX in (142,144,145,146): LABELS[1]='移动·振翅飞行'; LABELS[2]='开心·扬翅轻摆'; LABELS[4]='睡眠·合翼闭眼'; LABELS[5]='攻击·扬翅收回'
if DEX==151: LABELS[1]='移动·摆尾悬浮'; LABELS[2]='开心·抬臂轻摆'; LABELS[4]='睡眠·蜷身闭眼'; LABELS[5]='攻击·凝神收回'
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
components=[g for g in groups({(x,y) for y in range(source.height) for x in range(source.width) if source.getpixel((x,y))[3]>=args.alpha_threshold}) if len(g)>500]
assert len(components)==48,'Expected exactly 48 separate characters; inspect atlas'
rows=[]
for row in range(6):
    entries=[g for g in components if int(sum(y for x,y in g)/len(g)*6/source.height)==row]
    assert len(entries)==8,('Expected 8 measured characters in row',row,len(entries))
    rows.append(sorted(entries,key=lambda g:min(x for x,y in g)))
boxes=[[(min(x for x,y in g),min(y for x,y in g),max(x for x,y in g)+1,max(y for x,y in g)+1) for g in row] for row in rows]
(R/f'assets/source/animation-{DEX:03}-master-v{VERSION}-boxes.json').write_text(json.dumps(boxes)+'\n')
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
        assert len(groups(mask))==1
        # An enclosed gap (e.g. tail against belly) has its own closed inner outline.
        outside={(-1,-1)};queue=[(-1,-1)]
        while queue:
            point=queue.pop()
            for n in neighbors(point):
                if -1<=n[0]<=48 and -1<=n[1]<=48 and n not in mask and n not in outside:
                    outside.add(n);queue.append(n)
        outer_groups=[g for g in groups(outline) if any(n in outside for p in g for n in neighbors(p))]
        assert len(outer_groups)==1,(DEX,action,col,'disconnected exterior outline')
        assert len({p for p in frame.getdata() if p[3]})<=16
        assert max(frame.getbbox()[2]-frame.getbbox()[0],frame.getbbox()[3]-frame.getbbox()[1])<=40
        path=f'{action}-{col:02}.png';frame.save(OUT/path)
        folder=OUT/'right';folder.mkdir(exist_ok=True);ImageOps.mirror(frame).save(folder/path)
        frames.append(frame);items.append({'action':action,'frame':col,'image':path,'sourceBox':box,'sha256':hashlib.sha256((OUT/path).read_bytes()).hexdigest()})
        large=frame.resize((144,144),Image.Resampling.NEAREST);sheet.paste(large,(col*144,row*156),large)
    d.text((8,row*156+135),LABELS[row],font=font,fill='#26394c')
    all_frames.append(frames)
assert all(len({f.tobytes() for f in frames})==8 for frames in all_frames),'Each action must have eight distinct poses'
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
# Six concurrent action previews; actual phase timing comes from each row.
preview=[]
for ms in range(0,2400,80):
    tile=Image.new('RGB',(432,312),'#e1e9df');draw=ImageDraw.Draw(tile)
    for row,action in enumerate(ACTIONS):
        d=durations[action];phase=ms%sum(d);index=0
        while index<7 and phase>=d[index]:phase-=d[index];index+=1
        im=all_frames[row][index].resize((144,144),Image.Resampling.NEAREST)
        tile.paste(im,(row%3*144,row//3*156),im)
        draw.text((row%3*144+5,row//3*156+138),LABELS[row],font=font,fill='#26394c')
    preview.append(tile)
preview[0].save(OUT/'preview.gif',save_all=True,append_images=preview[1:],duration=80,loop=0,optimize=False)
manifest={'version':1,'nationalId':DEX,'name':NAME,'kind':'generated animation template','gameIntegrated':False,'source':str(SOURCE.relative_to(R)),'sourceSha256':hashlib.sha256(SOURCE.read_bytes()).hexdigest(),'identityReference':str(BASE.relative_to(R)),'prompt':f'assets/prompts/animation-{DEX:03}/master-v{VERSION}.txt','generator':'mobiiot-imagegen','model':next(line.split(': ',1)[1] for line in reversed((R/f'assets/prompts/animation-{DEX:03}/generation-v{VERSION}.log').read_text().splitlines()) if line.startswith('Selected model: ')),'attempts':(R/f'assets/prompts/animation-{DEX:03}/generation-v{VERSION}.log').read_text().count('Attempt '),'alphaThreshold':args.alpha_threshold,'nativeCanvas':48,'silhouetteMax':40,'framesPerAction':8,'frames':48,'sourceFacing':'left','directions':{'left':'.','right':'right'},'palette':'shared palette from this species approved native sprite','scale':scale,'actions':{a:{'frames':[f'{a}-{i:02}.png' for i in range(8)],'durationsMs':durations[a],'mode':'loop' if a in ('idle','walk','sleep','eat') else 'one-shot'} for a in ACTIONS},'processing':'Measured alpha components, one fixed global scale, shared reference palette, anchored baseline, connected native navy outline; no whole-body stretch or cut-body deformation','items':items}
assembly=SOURCE.with_name(SOURCE.stem+'-assembly.json')
if assembly.is_file():
    provenance=json.loads(assembly.read_text())
    assert provenance['outputSha256']==manifest['sourceSha256']
    for part in provenance['sources']:assert hashlib.sha256((R/part['path']).read_bytes()).hexdigest()==part['sha256']
    manifest.update(sourceParts=provenance,assemblyProvenance=str(assembly.relative_to(R)),additionalProvenance=provenance['additionalProvenance'])
if 97<=DEX<=106:
    provenance=['assets/source/gen1-097-106-v2.png','assets/source/gen1-097-106-v2-boxes.json','assets/prompts/gen1-097-106-v2.txt','build/gen1-097-106-generation-v2.log','assets/prompts/gen1-097-106-v1.txt','build/gen1-097-106-generation-v1.log','assets/animation-templates/001/atlas@4x.png','assets/pokemon/plan-097-106.json']
    manifest['additionalProvenance']=list(dict.fromkeys(manifest.get('additionalProvenance',[])+provenance))
if 117<=DEX<=151:
    batchplans=[(f,json.loads(f.read_text()))for f in (R/'assets/pokemon').glob('plan-*.json')]
    matches=[(f,p)for f,p in batchplans if any(x['nationalId']==DEX for x in p.get('items',[]))];assert len(matches)==1
    planfile,plan=matches[0];scope=planfile.stem.removeprefix('plan-')
    identities=json.loads((R/f'assets/pokemon/gen1-{scope}/manifest.json').read_text())
    identity=next(x for x in identities['items']if x['id']==DEX)
    if 'sourceImage' in identity:
        source=R/identity['sourceImage'];assert hashlib.sha256(source.read_bytes()).hexdigest()==identity['sourceSha256']
        provenance=[identity['sourceImage'],str(source.with_name(source.stem+'-box.json').relative_to(R)),identity['prompt'],identity['generationLog']]
    else:
        version=identities['version']
        provenance=[f'assets/source/gen1-{scope}-v{version}.png',f'assets/source/gen1-{scope}-v{version}-boxes.json',f'assets/prompts/gen1-{scope}-v{version}.txt',f'build/gen1-{scope}-generation-v{version}.log']
    provenance+=['assets/animation-templates/001/atlas@4x.png',str(planfile.relative_to(R))]
    manifest['additionalProvenance']=list(dict.fromkeys(manifest.get('additionalProvenance',[])+provenance))
(OUT/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
(OUT/'README.md').write_text(f'# {DEX:03} {NAME}动作模板\n\n48×48透明PNG，六动作各8帧，共48个MobiIoT生成姿态；统一原画色板、固定缩放与脚底基线，附左右朝向、图集、GIF和场景演示。动作图集仍需对未来物种逐只检查，不能用一套肢体结构套用所有宠物。\n\n六动作×8帧；集成状态以manifest.json和运行资源校验为准。\n')
print('PASS: 48 measured source components, 48 native frames, binary alpha, connected navy outlines, shared <=16 colors and exact mirrors')
print(OUT/'contact-sheet.png')
