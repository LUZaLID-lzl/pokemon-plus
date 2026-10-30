# -*- coding: utf-8 -*-
"""Self-contained, offline action and courtyard preview for the 001 template."""
from pathlib import Path
import base64,json,zipfile,argparse
parser=argparse.ArgumentParser();parser.add_argument("--dex",type=int,default=1);args=parser.parse_args();dex=args.dex
R=Path(__file__).resolve().parents[1];folder=R/f'assets/animation-templates/{dex:03}';out=R/f'docs/animation-{dex:03}';out.mkdir(exist_ok=True)
def data(path):return 'data:image/png;base64,'+base64.b64encode(path.read_bytes()).decode()
manifest=json.loads((folder/'manifest.json').read_text())
html='''<!doctype html><html lang="zh-CN"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>001 妙蛙种子 · 动作模板</title><style>
*{box-sizing:border-box}body{margin:0;background:#f3f5ee;color:#24473b;font:15px -apple-system,BlinkMacSystemFont,"PingFang SC",sans-serif}main{max-width:1140px;margin:0 auto;padding:36px 28px}header{display:flex;justify-content:space-between;align-items:center;margin-bottom:26px}.eyebrow{letter-spacing:.18em;font-size:11px;color:#82917e}h1{font-size:30px;letter-spacing:-1px;margin:7px 0}p{color:#738374;line-height:1.7;margin:4px 0}button,select,input{font:inherit}button,select{border:1px solid #ccd8c6;border-radius:12px;padding:10px 14px;background:white;color:#325c45;cursor:pointer}button.active{background:#315f49;color:white;border-color:#315f49}.hero{display:grid;grid-template-columns:1fr 1fr;gap:20px}.panel{background:#fffdf8;border:1px solid #dee5d5;border-radius:24px;padding:22px}.panel h2{margin:0;font-size:17px}.screen{width:min(360px,100%);display:block;aspect-ratio:1;margin:18px auto;border-radius:50%;box-shadow:0 0 0 8px #dfe6d6,0 15px 25px #2b463515;image-rendering:pixelated}.closeup{width:240px;height:240px;display:block;max-width:100%;margin:15px auto;image-rendering:pixelated}.label{display:flex;justify-content:space-between;align-items:center}.badge{background:#e9efdf;padding:5px 9px;font-size:12px;border-radius:20px;color:#5c7656}.actions{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin:15px 0}.controls{display:flex;gap:12px;align-items:center;flex-wrap:wrap;margin:18px 0}.controls label{display:flex;align-items:center;gap:8px}.progress{height:4px;background:#e5ebdf;border-radius:10px;overflow:hidden}.progress i{display:block;height:100%;background:#71965a;transition:width .06s}.frames{display:grid;grid-template-columns:repeat(8,1fr);gap:7px;margin-top:15px}.frame{border:1px solid #dfe7d6;background:#edf2e6;border-radius:10px;text-align:center;padding:5px}.frame canvas{width:100%;image-rendering:pixelated}.frame small{color:#8b9984}.frame.active{border-color:#6e925e;background:#e1ebd3}.foot{margin:22px 0;font-size:12px;color:#85927e}.instruction{font-size:12px}.demo-status{min-height:24px;text-align:center}.sheet{margin-top:20px}.sheet img{display:block;width:100%;image-rendering:pixelated;border-radius:12px;margin-top:14px}.sr{position:absolute;left:-10000px}@media(max-width:700px){main{padding:22px 16px}.hero{grid-template-columns:1fr}.panel{padding:17px}header{align-items:flex-start}h1{font-size:24px}.frames{gap:3px}.frame{padding:2px}.frame small{font-size:10px}}
</style><main><header><div><div class="eyebrow">POKÉMON PLUS / ANIMATION STUDY</div><h1>001 妙蛙种子</h1><p>同一个伙伴，六种日常。</p></div><span class="badge">动作模板</span></header><section class="hero"><article class="panel"><div class="label"><h2>庭院里的一天</h2><span class="badge">实际比例</span></div><canvas id="scene" class="screen" width="360" height="360" aria-label="妙蛙种子庭院场景动作"></canvas><div class="demo-status" id="scene-label">散步中</div><p class="instruction">切换动作，观察它在庭院里的表现。</p></article><article class="panel"><div class="label"><h2 id="action-name">行走</h2><span class="badge" id="counter">1 / 8</span></div><canvas id="closeup" class="closeup" width="240" height="240" aria-label="动作像素放大预览"></canvas><div class="actions" id="actions"></div><div class="progress"><i id="progress"></i></div><div class="controls"><button id="pause">暂停</button><button id="tour">连续演示</button><label>速度<select id="speed"><option value="0.5">0.5×</option><option value="1" selected>1×</option><option value="1.5">1.5×</option></select></label></div><p class="instruction" id="description"></p></article></section><article class="panel sheet"><div class="label"><h2>逐帧查看</h2><span class="badge">8帧循环</span></div><div class="frames" id="frames"></div></article><article class="panel sheet"><h2>完整动作图集</h2><img id="sheet" alt="六动作48帧联系表"></article><p class="foot">48×48透明像素 · 六动作各8帧 · 基于确认原画补绘；已接入v0.7.0固件与资源工坊。</p></main><script>
const config=__CONFIG__,atlas=new Image(),background=new Image();atlas.src=__ATLAS__;background.src=__BG__;document.querySelector('#sheet').src=__SHEET__;
const names=['待机',([41,42,49].includes(config.nationalId)?'飞行':'移动'),'开心','进食','睡眠','攻击'],descriptions=['轻轻呼吸，偶尔眨眨眼。',([41,42,49].includes(config.nationalId)?'轻轻振翅，飞行后收回翅膀。':'按身体结构自然移动，附属部位跟随。'),'轻蹲、起跳、落地，回到熟悉的姿态。','低头、咀嚼、吞咽，再抬起头。',(config.nationalId===41?'合翼休息，慢慢呼吸。':'闭上眼睛休息，慢慢呼吸。'),'蓄力、前冲、收回，恢复站姿。'],keys=Object.keys(config.actions);
if([50,51].includes(config.nationalId)){descriptions[0]='轻轻探头，偶尔眨眼。';descriptions[1]='在相连的土堆中交替探身、倾头。';descriptions[2]='轻摆探头，逐渐回到静息姿态。';descriptions[3]='小幅活动口鼻，随后恢复。';descriptions[4]='闭眼歇息，保持头部与土堆相连。';descriptions[5]='前倾、收回，恢复探头姿态。'}
if(config.nationalId===49){descriptions[2]='轻轻扬翅，恢复悬停姿态。';descriptions[4]='合起翅膀、闭眼歇息。';descriptions[5]='小幅振翅前探，随后收回。'}
if(config.nationalId===63){descriptions[0]='闭目冥想，轻轻呼吸。';descriptions[4]='蜷身闭目安睡，尾巴保持相连。'}
if([70,71].includes(config.nationalId)){names[1]='悬移';descriptions[1]='叶片交替轻摆，身体缓缓悬移，藤蔓保持相连。';descriptions[2]='轻轻扬叶与倾身，随后收回。';descriptions[4]='低姿闭目歇息，叶片自然放松。';descriptions[5]='嘴部与叶片轻轻前探，随后收回。'}
if([72,73].includes(config.nationalId)){names[1]='游动';descriptions[1]='触手交替弯曲，带动身体缓缓游动。';descriptions[2]='轻轻卷起触手，再缓缓放松。';descriptions[4]='闭目浮休，触手轻轻摆动。';descriptions[5]='相连的触手前探，随后收回。'}
if(config.nationalId===74){names[1]='悬浮';descriptions[1]='双臂交替屈伸，岩石身体缓缓悬移。';descriptions[2]='双臂轻抬，回到悬浮姿态。';descriptions[4]='闭目悬休，双臂自然放松。'}
if([81,82].includes(config.nationalId)){names[1]='悬浮';descriptions[1]='磁体轻转，身体缓缓悬移。';descriptions[2]='轻扬磁体，回到悬浮姿态。';descriptions[3]='磁体轻动，缓缓恢复。';descriptions[4]='闭目悬休，磁体放松。';descriptions[5]='磁体前探，随后收回。'}
if(config.nationalId===86){names[1]='滑行';descriptions[1]='前鳍交替划动，身体贴地滑行。';descriptions[2]='抬头扬鳍，轻轻落下。';descriptions[4]='闭眼低卧，鳍肢自然放松。';descriptions[5]='前鳍支撑身体，探头后收回。'}
if(config.nationalId===87){names[1]='滑行';descriptions[1]='前鳍交替划动，身体贴地滑行。';descriptions[2]='轻轻抬头扬鳍，再放松。';descriptions[4]='低卧闭目，鳍肢自然放松。'}
if([88,89].includes(config.nationalId)){names[1]='挪动';descriptions[1]='泥褶交替起伏，手臂轻摆，身体缓缓挪动。';descriptions[2]='轻轻抬起手臂，再收回。';descriptions[4]='低姿沉静，闭目休息。'}
if([90,91].includes(config.nationalId)){names[1]='挪动';descriptions[1]='贝壳轻轻摇摆开合，带动身体挪动。';descriptions[2]='轻启贝壳，再缓缓收回。';descriptions[3]='嘴部与贝壳轻动，随后恢复。';descriptions[4]='贝壳低姿静息，闭眼休息。'}
if([92,93].includes(config.nationalId)){names[1]='悬浮';descriptions[1]='轮廓轻轻起伏，身体缓缓悬移。';descriptions[2]='轻轻摆动，恢复悬浮姿态。';descriptions[4]='闭目悬休，保持完整轮廓。'}
if(config.nationalId===95){names[1]='盘行';descriptions[1]='相连的岩节交替弯曲，身体缓缓盘行。';descriptions[2]='轻轻抬头，岩节保持相连。';descriptions[4]='盘身闭目，头部放松。';descriptions[5]='抬头蓄力，前探后收回。'}
if([98,99].includes(config.nationalId)){names[1]='横移';descriptions[1]='蟹足交替屈伸，贝壳与相连双钳轻摆。';descriptions[2]='轻轻抬钳，再缓缓放松。';descriptions[4]='低伏闭眼，双钳放松。';descriptions[5]='抬钳蓄力，合钳后收回。'}
if([100,101].includes(config.nationalId)){names[1]='轻滚';descriptions[1]='圆球身体轻轻倾摇，五官随动作变化。';descriptions[2]='神情轻动，恢复静息姿态。';descriptions[4]='闭眼静息，缓缓呼吸。';descriptions[5]='倾身蓄力，再轻轻收回。'}
if([100,106].includes(config.nationalId))descriptions[3]='专注轻动，随后恢复静息姿态。';
if(config.nationalId===102){names[1]='倾摇';descriptions[1]='六枚相连的蛋轻轻倾摇，保持紧凑蛋群。';descriptions[2]='蛋群神情轻动，再缓缓放松。';descriptions[4]='蛋群全部闭眼，低姿静息。'}
if(config.nationalId===103){descriptions[1]='双腿交替迈步，三个头与叶冠跟随轻摆。';descriptions[2]='轻轻抬叶，双腿自然弯曲。';descriptions[4]='低坐休息，三个头全部闭眼。'}
if([104,105].includes(config.nationalId)){descriptions[4]='低坐闭眼，骨棒靠在身侧。';descriptions[5]='紧握骨棒，轻轻挥动再收回。'}
if(config.nationalId===104)descriptions[5]='凝神准备，重心轻移后收回。';
if(config.nationalId===106){descriptions[1]='分节双腿交替屈膝迈步，手臂跟随平衡。';descriptions[4]='屈腿低坐，闭眼安睡。';descriptions[5]='屈膝抬腿，轻踢后收回。'}
if(config.nationalId===107){descriptions[4]='低坐闭眼，拳套收在身前。';descriptions[5]='抬拳准备，短促挥拳后收回。'}
if(config.nationalId===108){descriptions[4]='低坐闭眼，舌头靠近身体。';descriptions[5]='相连的舌头轻探，再缓缓收回。'}
if([109,110].includes(config.nationalId)){names[1]='悬浮';descriptions[1]='身体轻倾，神情变化，保持相连悬浮。';descriptions[2]='嘴部与神情轻动，缓缓恢复。';descriptions[4]='低姿浮休，所有眼睛闭合。';descriptions[5]='轻轻前倾，凝神后收回。'}
if(config.nationalId===111){descriptions[1]='四足交替屈伸，背甲与尾巴轻摆。';descriptions[2]='轻轻屈前腿，再恢复。';descriptions[4]='四腿收拢，低卧闭眼。';descriptions[5]='前腿支撑，探头后收回。'}
if(config.nationalId===112){descriptions[4]='低坐闭眼，双臂与尾巴放松。'}
if(config.nationalId===113){descriptions[4]='抱蛋低坐，闭眼休息。';descriptions[5]='双臂轻护腹袋，再恢复站姿。'}
if(config.nationalId===114){names[1]='藤足挪动';descriptions[1]='红鞋交替迈步，相连藤蔓轻摆。';descriptions[2]='轻轻扬藤，再恢复。';descriptions[3]='专注轻动，藤蔓缓缓恢复。';descriptions[4]='收拢藤蔓，双眼闭合。';descriptions[5]='相连藤蔓前探，再轻轻收回。'}
if(config.nationalId===115){descriptions[4]='亲子低坐，妈妈和宝宝全部闭眼。';descriptions[5]='轻护腹袋，手臂收回，宝宝保持在袋内。'}
if(config.nationalId===116){names[1]='游动';descriptions[1]='背鳍轻摆、卷尾弯曲，身体缓缓游动。';descriptions[2]='轻轻抬头，卷尾自然弯曲。';descriptions[4]='低姿闭眼浮休，卷尾收拢。';descriptions[5]='探头摆鳍，再缓缓收回。'}
if(config.nationalId===117){names[1]='游动';descriptions[1]='背鳍交替轻摆，卷尾自然弯曲。';descriptions[4]='卷尾收拢，闭眼低姿浮休。';descriptions[5]='探头摆鳍，再缓缓收回。'}
if([118,119].includes(config.nationalId)){names[1]='游动';descriptions[1]='相连尾鳍交替摆动，胸鳍轻轻划水。';descriptions[2]='轻轻扬鳍，回到游动姿态。';descriptions[4]='鳍尾放松，闭眼浮休。';descriptions[5]='倾身准备，轻轻收回。'}
if([120,121].includes(config.nationalId)){names[1]='星臂游动';descriptions[0]='相连星臂轻轻起伏，保持宝石与身体完整。';descriptions[1]='星臂交替弯曲，缓缓游动。';descriptions[2]='轻轻扬起星臂，再缓缓放松。';descriptions[3]='凝神轻动，星臂缓缓恢复。';descriptions[4]='星臂低伏放松，红色宝石暗淡静息；海星没有眼睛。';descriptions[5]='星臂准备，倾身后收回。'}
if(config.nationalId===122){descriptions[4]='屈膝低坐，闭眼休息，手套靠在身体旁。';descriptions[5]='抬起相连的手掌，再轻轻收回。'}
if(config.nationalId===123){descriptions[1]='双腿交替迈步，相连双翼轻振。';descriptions[4]='低姿闭眼，双翼与前臂收拢。';descriptions[5]='相连前臂轻摆，再恢复姿态。'}
if(config.nationalId===124){descriptions[1]='小步交替，衣袖与长发自然跟随。';descriptions[4]='低坐闭眼，双手贴近衣袖。';descriptions[5]='轻轻抬掌，再缓缓收回。'}
if([125,126].includes(config.nationalId)){descriptions[4]='低坐闭眼，手臂与尾巴自然放松。';descriptions[5]='屈膝准备，摆臂后收回。'}
if(config.nationalId===127){descriptions[1]='双腿交替迈步，双臂与头角自然跟随。';descriptions[4]='低坐闭眼，双臂放松。';descriptions[5]='双臂轻摆，准备后收回。'}
if([128,133,134,135,136].includes(config.nationalId)){descriptions[1]='四足交替迈步，头部与尾巴自然跟随。';descriptions[2]='轻屈前腿，抬头轻摆，再恢复。';descriptions[4]='四腿收拢，低卧闭眼。';descriptions[5]='前腿支撑，俯身后收回。'}
if(config.nationalId===129){names[1]='游动';descriptions[1]='相连尾鳍交替摆动，侧鳍轻轻划水。';descriptions[2]='轻轻扬鳍，保持须尾相连。';descriptions[4]='鳍尾放松，闭眼低姿浮休。';descriptions[5]='倾身摆鳍，再缓缓收回。'}
if([130,147,148].includes(config.nationalId)){names[1]='游动';descriptions[1]='相连蛇身交替弯曲，尾巴自然跟随。';descriptions[2]='轻轻抬头，保持盘身完整。';descriptions[4]='盘身低伏，闭眼休息。';descriptions[5]='轻轻探身，再收回。'}
if(config.nationalId===131){names[1]='游动';descriptions[1]='四只相连鳍肢交替划水，颈部自然跟随。';descriptions[2]='轻轻抬颈，保持背壳相连。';descriptions[4]='伏颈低卧，闭眼休息。';descriptions[5]='鳍肢支撑，探头后收回。'}
if(config.nationalId===132){descriptions[1]='相连软体侧瓣自然挪动。';descriptions[2]='微笑轻摆，保持身体完整。';descriptions[4]='低伏闭眼，侧瓣放松。';descriptions[5]='轻轻凝神，再恢复姿态。'}
if(config.nationalId===137){descriptions[1]='多边足交替挪动，保持棱角形体。';descriptions[2]='轻轻抬头，双足自然轻摆。';descriptions[4]='低伏闭眼，双足收拢。';descriptions[5]='轻轻探头，再缓缓收回。'}
if([138,139].includes(config.nationalId)){descriptions[1]='相连触足交替弯曲，背壳保持完整。';descriptions[2]='触足轻扬，抬头轻摆。';descriptions[4]='收拢触足，低姿闭眼。';descriptions[5]='触足支撑，探头后收回。'}
if(config.nationalId===140){descriptions[1]='壳下四足交替挪动。';descriptions[2]='背壳轻摆，四足跟随。';descriptions[3]='轻轻凝神，保持壳足相连。';descriptions[4]='伏壳收足，壳下红眼闭合。';descriptions[5]='四足轻撑，再恢复。'}
if([141,143,149,150].includes(config.nationalId)){descriptions[4]='低坐闭眼，双臂与尾巴自然放松。';descriptions[5]='轻轻准备，摆臂后收回。'}
if([142,144,145,146].includes(config.nationalId)){names[1]='飞行';descriptions[1]='相连双翼交替抬落，双足自然收拢。';descriptions[2]='抬头扬翅，再缓缓收回。';descriptions[4]='合翼低栖，闭眼休息。';descriptions[5]='轻轻扬翅准备，再恢复。'}
if(config.nationalId===151){names[1]='悬浮';descriptions[1]='短臂、后足与相连长尾轻摆悬浮。';descriptions[2]='轻轻抬臂，长尾自然跟随。';descriptions[4]='收足蜷身，闭眼低姿浮休。';descriptions[5]='抬臂凝神，长尾轻摆后恢复。'}
let selected=1,elapsed=0,playing=true,tour=false,previous=0,tourTime=0;const view=document.querySelector('#scene'),ctx=view.getContext('2d'),close=document.querySelector('#closeup'),big=close.getContext('2d');ctx.imageSmoothingEnabled=big.imageSmoothingEnabled=false;
const buttons=names.map((name,i)=>{const b=document.createElement('button');b.textContent=name;b.onclick=()=>{tour=false;document.querySelector('#tour').classList.remove('active');choose(i)};document.querySelector('#actions').append(b);return b});
const frameCanvases=Array.from({length:8},(_,i)=>{const b=document.createElement('button');b.className='frame';b.setAttribute('aria-label','查看第'+(i+1)+'帧');const c=document.createElement('canvas');c.width=c.height=48;b.append(c);const s=document.createElement('small');s.textContent=String(i+1).padStart(2,'0');b.append(s);b.onclick=()=>{playing=false;document.querySelector('#pause').textContent='播放';elapsed=config.actions[keys[selected]].durationsMs.slice(0,i).reduce((a,b)=>a+b,0)+1;draw()};document.querySelector('#frames').append(b);return c});
function choose(i){selected=i;elapsed=0;tourTime=0;buttons.forEach((b,j)=>{b.classList.toggle('active',j===i);b.setAttribute('aria-pressed',String(j===i))});document.querySelector('#action-name').textContent=names[i];document.querySelector('#description').textContent=descriptions[i];document.querySelector('#scene-label').textContent=names[i]+'中';if(atlas.complete)draw()}
function sprite(c,index,x,y,size,right=false){c.save();c.imageSmoothingEnabled=false;if(right){c.translate(x+size,y);c.scale(-1,1);x=y=0}c.drawImage(atlas,index*48,selected*48,48,48,Math.round(x),Math.round(y),size,size);c.restore()}
function draw(){if(!atlas.complete||!atlas.naturalWidth||!background.complete||!background.naturalWidth)return;const d=config.actions[keys[selected]].durationsMs,total=d.reduce((a,b)=>a+b,0);let t=elapsed%total,index=0;while(t>=d[index]&&index<7)t-=d[index++];ctx.clearRect(0,0,360,360);ctx.drawImage(background,0,0,360,360);ctx.fillStyle='#d7c9ff';ctx.font='bold 34px monospace';ctx.textAlign='center';ctx.fillText('08:30',180,65);let x=156,y=184,right=false;if(selected===1){const phase=(elapsed%6000)/6000;right=phase>.5;x=right?104+(phase-.5)*208:208-phase*208;y=184+Math.sin(phase*Math.PI*2)*9}sprite(ctx,index,x,y,48,right);big.fillStyle='#edf2e6';big.fillRect(0,0,240,240);sprite(big,index,0,0,240);document.querySelector('#counter').textContent=(index+1)+' / 8';document.querySelector('#progress').style.width=(index+1)/8*100+'%';frameCanvases.forEach((c,i)=>{const cc=c.getContext('2d');cc.clearRect(0,0,48,48);sprite(cc,i,0,0,48);c.parentElement.classList.toggle('active',i===index)})}
function animate(now){const dt=previous?Math.min(100,now-previous):0;previous=now;if(playing){const step=dt*Number(document.querySelector('#speed').value);elapsed+=step;tourTime+=step;if(tour&&tourTime>2500)choose((selected+1)%6)}draw();requestAnimationFrame(animate)}
document.querySelector('#pause').onclick=()=>{playing=!playing;document.querySelector('#pause').textContent=playing?'暂停':'播放'};document.querySelector('#tour').onclick=()=>{tour=!tour;document.querySelector('#tour').classList.toggle('active',tour);tourTime=0;playing=true;document.querySelector('#pause').textContent='暂停'};choose(1);requestAnimationFrame(animate);
</script></html>'''
integration_note=('已接入'+manifest.get('firmwareVersion','现有')+'固件与资源工坊。' if manifest.get('gameIntegrated') else '动作模板已导出；固件与资源工坊接入待本批资源补齐。')
html=html.replace('已接入v0.7.0固件与资源工坊。',integration_note)
html=html.replace('__CONFIG__',json.dumps(manifest,ensure_ascii=False)).replace('__ATLAS__',json.dumps(data(folder/'atlas.png'))).replace('__BG__',json.dumps(data(R/'sdcard/pokemon-plus/scenes/0.png'))).replace('__SHEET__',json.dumps(data(folder/'contact-sheet.png')))
html=html.replace('001 妙蛙种子',f'{dex:03} '+manifest['name']).replace('妙蛙种子庭院',manifest['name']+'庭院')
(out/'index.html').write_text(html)
with zipfile.ZipFile(R/f'dist/{dex:03}-animation-template.zip','w',zipfile.ZIP_DEFLATED) as z:
    for p in sorted(folder.rglob('*')):
        if p.is_file():z.write(p,f'{dex:03}/'+p.relative_to(folder).as_posix())
    z.write(out/'index.html',f'{dex:03}/scene-preview.html')
    provenance=[manifest['source'],manifest['identityReference'],manifest['prompt'],manifest['prompt'].replace('master-','generation-').replace('.txt','.log')]+manifest.get('additionalProvenance',[])+([manifest['assemblyProvenance']] if manifest.get('assemblyProvenance') else [])
    for file in provenance:z.write(R/file,'provenance/'+file)
    for review in sorted((R/'docs').glob('gen1-*/visual-review.json')):
        if dex in json.loads(review.read_text()).get('animationReviewed',[]):
            z.write(review,'provenance/'+review.relative_to(R).as_posix())
    for p in (R/'assets/templates/pet-animation-48').rglob('*'):
        if p.is_file():z.write(p,'template/'+p.relative_to(R/'assets/templates/pet-animation-48').as_posix())
print(out/'index.html')
