// Mirrors core/scene_fx.c; recipes come from assets/scene-effects.json.
const MAX_PARTICLES=48;
function mix(x){x>>>=0;x^=x>>>16;x=Math.imul(x,0x7feb352d);x^=x>>>15;x=Math.imul(x,0x846ca68b);x^=x>>>16;return x>>>0}
export function sceneParticleSafe(p){
 if(p.w<1||p.h<1)return false;
 const overlaps=(x,y,w,h)=>p.x<x+w&&p.x+p.w>x&&p.y<y+h&&p.y+p.h>y;
 if(!p.full_screen&&(overlaps(80,22,200,90)||overlaps(98,258,164,70)))return false;
 const radius=p.full_screen?176:172;
 for(let c=0;c<4;c++){const dx=p.x+(c&1?p.w:0)-180,dy=p.y+(c&2?p.h:0)-180;if(dx*dx+dy*dy>radius*radius)return false}
 return true;
}
export function sceneParticleParts(p){
 const {x,y,w,h,opacity:a}=p,f=Math.floor;let rects=[[0,0,w,h,a]];
 switch(p.shape){
  case 15:case 16:case 17:case 18:case 19:case 20:case 21:case 22:return [];
  case 12:case 13:case 14:{const my=p.shape===12?1:p.shape===13?3:4,ty=p.shape===12?0:p.shape===13?2:6,mh=p.shape===13?2:3;
   const blocks=[[6,2,1,5],[3,3,3,2],[1,my,3,mh],[0,ty,2,2],[7,3,3,2],[9,my,3,mh],[11,ty,2,2]];
   rects=blocks.map(([bx,by,bw,bh],j)=>{const x=f(bx*w/13),y=f(by*h/9);return [x,y,Math.max(1,f((bx+bw)*w/13)-x),Math.max(1,f((by+bh)*h/9)-y),j?f(a*9/10):a]});break}
  case 1:rects=[[0,f(h/2),w,w>=7?2:1,a],[f(w/2),0,w>=7?2:1,h,a]];break;
  case 2:rects=[[0,f(h/2),w-2,h-f(h/2),a],[2,0,w-2,f(h/2)+1,f(a*4/5)]];break;
  case 3:rects=[[2,0,w-4,1,a],[0,1,2,h-1,f(a*2/3)],[w-2,1,2,h-1,f(a*2/3)]];break;
  case 11:rects=[[3,0,w-6,1,a],[3,h-1,w-6,1,a],[1,1,2,h-2,f(a*2/3)],[w-3,1,2,h-2,f(a*2/3)],[0,2,1,h-4,f(a/2)],[w-1,2,1,h-4,f(a/2)]];break;
  case 4:rects=[[w-3,h-2,3,2,a],[f(w/3),f(h/3),f(w/2),2,f(a*2/3)],[0,0,f(w/2),1,f(a/3)]];break;
  case 5:{const blocks=[[1,6,17,3,55],[0,5,5,3,70],[2,3,5,4,85],[7,0,6,2,85],[5,2,10,6,95],[13,3,5,5,80],[16,5,4,3,65]];
   rects=blocks.map(([bx,by,bw,bh,alpha])=>[f(bx*w/20),f(by*h/10),f(bw*w/20),f(bh*h/10),f(a*alpha/100)]);break}
  case 7:rects=[[1,0,w-2,h,f(a*3/4)],[0,1,w,h-2,a],[f(w/2),h-1,1,1,f(a/2)]];break;
  case 8:rects=[[0,0,w,h,f(a/6)],[2,2,w-4,h-4,f(a/2)],[f(w/2),f(h/2),2,2,a]];break;
  case 9:{const blocks=[[4,0,1,3],[3,1,3,6],[0,2,2,3],[7,2,2,3],[1,4,7,3],[2,7,5,1],[4,8,1,3]];rects=blocks.map(([bx,by,bw,bh],j)=>{const x=f(bx*w/9),y=f(by*h/11);return [x,y,Math.max(1,f((bx+bw)*w/9)-x),Math.max(1,f((by+bh)*h/11)-y),j===6?f(a*3/4):a]});break}
  case 10:rects=Array.from({length:6},(_,j)=>[j*6,j*2,8,j<2?2:3,f(a*(j+1)/7)]);rects.push([36,12,4,4,a]);break;
  case 6:rects=[[1,0,w-1,f(h/2),f(a*2/3)],[0,f(h/2),w-1,h-f(h/2),a]];break;
 }
 return rects.map(([dx,dy,w,h,opacity])=>({x:x+dx,y:y+dy,w,h,opacity}));
}
function breathe(age,period,offset){const t=(age%period+offset%period)%period,v=t<Math.floor(period/2)?Math.floor(t*510/period):Math.floor((period-t)*510/period);return Math.floor(v*v/255)}
function edgeAlpha(p){let far=0;for(let c=0;c<4;c++){const x=p.x+(c&1?p.w:0)-180,y=p.y+(c&2?p.h:0)-180;far=Math.max(far,x*x+y*y)}if(p.full_screen)return far<170*170?255:Math.floor((176*176-far)*255/(176*176-170*170));let a=far<157*157?255:Math.floor((172*172-far)*255/(172*172-157*157));for(const [x,y,w,h] of [[80,22,200,90],[98,258,164,70]]){const dx=p.x>=x+w?p.x-x-w:x-p.x-p.w,dy=p.y>=y+h?p.y-y-h:y-p.y-p.h,gap=Math.max(dx,dy);if(gap<14&&gap>=0)a=Math.min(a,Math.floor(gap*255/14))}return a}
export function sampleSceneEffects(configs,scene,seed,elapsed,preview=false){
 const c=configs[scene];if(!c)return [];
 elapsed=Math.floor(elapsed)>>>0;
 const f=Math.floor,weather=mix(seed^Math.imul(scene,2654435761)^Math.imul(f(elapsed/45000),2246822519));
 const start=1200+weather%3000,eventPhase=elapsed%45000;let burst=0;
 if(preview)burst=255;
 else if(weather%100<c.chance&&eventPhase>=start&&eventPhase<start+c.durationMs){const age=eventPhase-start,remaining=c.durationMs-age;burst=age<1200?f(age*255/1200):remaining<1200?f(remaining*255/1200):255}
 if(!burst)return [];
 const age=(2000+elapsed)>>>0,fade=elapsed<800?f(elapsed*255/800):255,out=[];
 for(let i=0;i<Math.min(c.count,MAX_PARTICLES);i++){
  const r=mix(seed^Math.imul(scene,3266489917)^Math.imul(i,668265263)),speed=f(c.speed*(75+r%60)/100);
  let phase=(f(age*speed/1000)+r%360)%360,density=burst;
  const glow=breathe(age,3200+(r%5)*640,r),wave=(age+r%7000)%8000,t=f((wave<4000?wave:8000-wave)/20)-100,sway=Math.trunc((6+r%9)*t*(30000-t*t)/2000000),x=20+(f(i*313/c.count)+(r>>>8)%17)%320;
  const p={x:x+sway,y:phase-8,w:2,h:2,color:c.color,opacity:130,shape:0,full_screen:scene!==5&&scene!==10};
  const anchor=c.anchors?.length?c.anchors[(r>>>3)%c.anchors.length]:[x,180];
  switch(c.kind){
   case 0:case 10:{
    const layer=i%3;phase=(f(age*f(speed*(layer+3)/4)/1000)+i*149+r%23)%408;
    p.x=(f(age*(4+r%6)/1000)+i*73+(r>>>8)%29+360+sway)%360;p.y=phase-24;
    const flutter=breathe(age,1600+r%4*400,r);p.w=layer===0?4:layer===1?6:8;p.h=3+f(flutter/85);
    p.shape=scene===1?7:2;p.opacity=layer===0?125:layer===1?185:225;
    if(scene===3&&i%3!==2){p.w=2+r%2;p.h=2;p.shape=0;p.opacity=135}
    if(c.kind===10){p.shape=9;p.w=layer===0?7:9;p.h=flutter<128?10:11;p.color=r%3===0?'e66f41':r%3===1?'f5b450':'c74b37'}
    if(scene===1)p.color=r%3===0?'ffd8e5':r%3===1?'efa6c5':'f2bacd';
    break}
   case 1:p.x=anchor[0]-1;p.y=anchor[1]-1;p.w=9+f(glow/85);p.h=3;p.shape=3;p.opacity=60+f(glow*185/255);if(i%4===0){p.w=p.h=3+r%2*2;p.shape=1}break;
   case 2:{
    if(i<8){p.x=anchor[0]-1;p.y=anchor[1]-1;p.w=p.h=3+(r%2)*2;p.shape=1;p.opacity=65+f(glow*160/255);density=burst}
    else{const layer=i%3;phase=(f(age*f(speed*(layer+2)/3)/1000)+(i-8)*149+r%23)%408;p.y=phase-24;p.x=((i-8)*73+(r>>>8)%29+360+Math.trunc(sway/(3-layer)))%360;p.full_screen=true;p.w=p.h=layer===2?7:layer===1?4:3;if(layer===2)p.shape=1;p.color=layer===0?'b2cede':layer===1?'e0edf5':'f2f8fc';p.opacity=layer===0?145:layer===1?205:240;density=burst}
    break}
   case 3:{
    if(scene===6&&i>=36){const slot=i-36,batAge=preview?elapsed:eventPhase-start,flight=(batAge+14000-slot*280)%14000;if(flight>=6200)continue;
     const wing=f(((batAge+r%400)>>>0)/100)%4;p.shape=wing===0?12:wing===2?14:13;p.w=slot<4?17:23;p.h=slot<4?11:15;
     p.x=f(flight*420/6200)-32;if(mix(seed^0x614ba71)&1)p.x=360-p.x-p.w;
     const arc=flight<3100?flight:6200-flight;p.y=126+slot%3*23+f(slot/3)*4-f(arc*24/3100)+Math.trunc(sway/4);
     p.color=slot<4?'a79acb':'1b1834';p.opacity=slot<4?210:250;break}
    p.x=anchor[0]-1;p.y=anchor[1]-1;p.w=p.h=5+2*f(glow/100);p.shape=1;p.opacity=55+f(glow*200/255);p.color=i%3===0?'dcc5ff':i%3===1?'b9e5ff':'a8f3ef';break}
   case 4:
    phase=(f(age*(5+r%7)/1000)+i*149+r%23)%408;
    p.x=(i*73+(r>>>8)%29+360+sway)%360;p.y=384-phase;
    p.w=p.h=5+(i%3)*2;p.shape=8;p.color=r%3?'b9e8a6':'e6ce8c';p.opacity=40+f(glow*210/255);break;
   case 5:
    p.x=anchor[0]-1;p.y=anchor[1]-1;p.w=p.h=r%3?3:5;p.shape=1;p.opacity=45+f(glow*200/255);
    if(i%8===0){const streak=(age+r%15000)%15000;if(streak>1800)continue;p.x=anchor[0]-20+f(streak/20);p.y=anchor[1]-15+f(streak/45);p.w=28;p.h=11;p.shape=4;if(p.y+p.h>138)continue;const life=streak<300?f(streak*255/300):f((1800-streak)*255/1500);p.opacity=f(life*230/255)}
    break;
   case 6:
    if(scene===9&&i>=37){const slot=i-37,eventAge=preview?elapsed:eventPhase-start,roll=(eventAge+18000-slot*3200)%18000;if(roll>=8800)continue;
     const reverse=mix(seed^0x9a511)&1;let turn=f(roll/180)%8;if(reverse)turn=7-turn;
     p.w=p.h=slot===0?32:slot===1?40:36;p.x=f(roll*424/8800)-32;if(reverse)p.x=360-p.x-p.w;
     const hopPhase=(roll+slot*211)%960,hop=f((hopPhase<480?hopPhase:960-hopPhase)*5/480);
     p.y=228+slot*10-p.h-hop;p.shape=15+turn;p.color='866037';p.opacity=255;break}
    p.x=anchor[0]-10+sway;p.y=anchor[1]-1+Math.trunc(sway/4);p.w=i%4===0?10:2+r%3;p.h=i%4===0?1:2;p.opacity=50+f(glow*150/255);break;
   case 7:
    if(i<8){p.x=anchor[0]-1;p.y=anchor[1]-1;p.w=7+f(glow/64);p.h=3;p.shape=3;p.opacity=65+f(glow*155/255);density=burst}
    else{phase=(f(age*speed/1000)+(i-8)*149+r%23)%408;p.y=phase-24;p.x=((i-8)*73+(r>>>8)%29+360-f(phase/12))%360;p.full_screen=true;p.w=3;p.h=14+r%8;p.shape=6;p.opacity=i%3===0?220:165;density=burst}
    break;
   case 9:{
    if(i>=8){p.x=anchor[0]-1;p.y=anchor[1]-1;p.w=p.h=r%3?3:5;p.shape=1;p.opacity=45+f(glow*200/255);density=burst;break}
    const streak=(age+r%10000)%10000;if(streak>=3200)continue;
    p.x=f(streak*420/3200)-48;p.y=10+(r>>>8)%30+f(streak*78/3200);
    p.w=40;p.h=16;p.shape=10;p.color=i%2?'c9e9ff':'e8e4ff';
    const life=streak<200?f(streak*255/200):streak>2800?f((3200-streak)*255/400):255;p.opacity=f((170+r%55)*life/255);break}
   case 11:{
    if(i<28){const life=(age+r%5200)%5200;p.w=12+f(life*24/5200);p.h=6+f(life*6/5200);p.x=anchor[0]-f(p.w/2);p.y=anchor[1]-f(p.h/2);p.shape=11;p.opacity=f((5200-life)*150/5200);density=burst}
    else{const life=(age+r%10000)%10000,fromX=105+(r>>>10)%150,fromY=132+(r>>>12)%28,toX=(r&1?247:68)+(r>>>16)%35,toY=206+(r>>>20)%48;
     if(life<6000){p.x=fromX+Math.trunc((toX-fromX)*life/6000)+Math.trunc(sway/3);p.y=fromY+Math.trunc((toY-fromY)*life/6000);p.w=5+(r%2)*2;p.h=4+f(glow/128);p.shape=2;p.color=r&1?'91ddff':'67c9f2';p.opacity=life<400?f(life*225/400):225}
     else if(life<8000){const landing=life-6000;p.w=12+f(landing*26/2000);p.h=6+f(landing*6/2000);p.x=toX-f(p.w/2);p.y=toY-f(p.h/2);p.shape=11;p.opacity=f((2000-landing)*195/2000)}else continue;
     density=burst}break}
   case 8:{const layer=i%3,drift=c.speed+layer*8+r%5;
    p.w=40+layer*20;p.h=p.w/2;p.x=(f(age*drift/1000)+i*71+(r>>>8)%73)%480-100;
    p.y=96+(i*47+(r>>>16)%45)%180+Math.trunc(sway/8);p.shape=5;
    p.color=layer===0?'c4dfed':layer===1?'e2eff5':'f4f8f2';p.opacity=layer===0?115:layer===1?150:180;break}
  }
  if(scene===8&&p.y+p.h>138)continue;
  if(sceneParticleSafe(p)){p.opacity=f(f(f(p.opacity*density/255)*fade/255)*edgeAlpha(p)/255);if(p.x>105&&p.x<250&&p.y>124&&p.y<244)p.opacity=f(p.opacity*7/8);if(p.opacity)out.push(p)}
 }
 return out;
}
export function drawSceneEffects(ctx,particles,drawSprite){
 for(const p of particles){if(p.shape>=15&&p.shape<=22){ctx.globalAlpha=p.opacity/255;drawSprite?.(`effects/tumble-${p.w}-${p.shape-15}.png`,p.x,p.y,p.w,p.h);continue}ctx.fillStyle='#'+p.color;for(const r of sceneParticleParts(p)){ctx.globalAlpha=r.opacity/255;ctx.fillRect(r.x,r.y,r.w,r.h)}}
 ctx.globalAlpha=1;
}
