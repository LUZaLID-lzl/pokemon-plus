import {lakePatrol,lakeNearestPhase} from './sacred-lake.js?v=lake3';
import {groundProject,homeRoute,routeLength,routeSample,pathSample} from './home-navigation.mjs?v=nestwalk2';
/* Presentation-only mirror of core/home_behavior.c; no gameplay/storage writes. */
export const homeKinds={none:0,pat:1,call:2,explore:3,hop:4,greet:5,stroll:6,rest:7,welcome:8};
const clamp=(v,a,b)=>Math.max(a,Math.min(b,v)),due=(now,when)=>(now-when|0)>=0;
const reflect=(value,low,high)=>{const width=high-low,phase=((value-low)%(width*2)+width*2)%(width*2);return low+(phase>width?width*2-phase:phase)};
export function homePatrol(elapsed,slot,tired,originX,originY,facing){
 const rx=[0,62,88,24,0],ry=[0,30,68,54,0],cycle=(elapsed>>>0)%(tired?24000:20000);
 if(cycle>=14400)return {x:originX,y:originY,left:facing,pose:0};
 const segment=Math.floor(cycle/3600),t=cycle%3600,sign=slot%2?-1:1;
 const rawX=originX+sign*(rx[segment]+Math.trunc((rx[segment+1]-rx[segment])*t/3600));
 const rawY=originY+ry[segment]+Math.trunc((ry[segment+1]-ry[segment])*t/3600),x=reflect(rawX,48,272);
 return {x,y:reflect(rawY,115,208),left:reflect(rawX+(sign*(rx[segment+1]-rx[segment])<0?-1:1),48,272)<x,pose:4};
}
export function homeScenePatrol(scene,elapsed,slot,tired,originX,originY,facing){
 if(scene===12)return {...lakePatrol(elapsed),pose:4};
 const origin=groundProject(scene,originX,originY),cycle=(elapsed>>>0)%(tired?24000:20000);
 if(cycle>=14400)return {...origin,left:facing,pose:0};
 const points=[[origin.x,origin.y]];
 for(let segment=0;segment<4;segment++){
  const a=homePatrol(segment*3600,slot,tired,origin.x,origin.y,facing),b=homePatrol((segment+1)*3600,slot,tired,origin.x,origin.y,facing);
  points.push(...homeRoute(scene,a.x,a.y,b.x,b.y).slice(1));
 }
 return {...pathSample(scene,points,cycle,14400,facing),pose:4};
}
export function createHomeBehavior(seed=0,now=0,scene=0){
 const s={pets:Array.from({length:5},()=>({kind:0})),rng:seed>>>0,scene,called:false,lastCall:0};
 const roll=()=>s.rng=(Math.imul(s.rng,1664525)+1013904223)>>>0;
 const safe=(x,y)=>{x=clamp(x,s.scene===12?40:48,272);y=clamp(y,115,s.scene===12?214:208);if(s.scene===12){if(x+24>=86&&x+24<=274&&y>132&&y<214)y=214;const p=lakePatrol(lakeNearestPhase(x,y));x=p.x;y=p.y}return groundProject(s.scene,x,y)};
 const approach=(x,y,tx,ty)=>Math.max(1000,routeLength(homeRoute(s.scene,x,y,tx,ty))*35);
 s.nextSolo=(now+20000+roll()%20001)>>>0;s.nextPair=(now+60000+roll()%60001)>>>0;
 s.clear=()=>s.pets.forEach(p=>p.kind=0);
 s.cancel=p=>{if(p>=5)return;const a=s.pets[p],kind=a.kind,b=a.partner;a.kind=0;if(kind>=5&&kind<=7&&b<5)s.pets[b].kind=0};
 const begin=(now,kind,p,b,tx,ty,input,span)=>{const tail=kind===3?2600:kind===6?4400:kind===7?5200:kind===5?2800:1800;
  s.pets[p]={kind,partner:b,x:input[p].x,y:input[p].y,tx,ty,left:input[p].left,start:now>>>0,approach:span,duration:kind===1?2000:kind===4?1800:span+tail};};
 s.request=(now,kind,a,b,x,y,input)=>{
  const count=input.length;if(kind<1||kind>8||a>=count||a>=5||!input[a].available)return false;
  const pair=kind>=5&&kind<=7;
  if(pair&&(b>=count||b>=5||a===b||!input[b].available))return false;
  if(pair&&s.pets.some(p=>p.kind>=5&&p.kind<=7))return false;
  s.cancel(a);if(pair)s.cancel(b);
  if(kind===1||kind===4){x=input[a].x;y=input[a].y}else if(pair){x=clamp(Math.trunc((input[a].x+input[b].x)/2),76,216)-26;y=Math.trunc((input[a].y+input[b].y)/2)}
  if(kind!==1&&kind!==4)({x,y}=safe(x,y));const bp=safe(x+54,y+(kind===6?8:0));
  let span=approach(input[a].x,input[a].y,x,y);if(pair)span=Math.max(span,approach(input[b].x,input[b].y,bp.x,bp.y));
  begin(now,kind,a,pair?b:5,x,y,input,span);if(pair)begin(now,kind,b,a,bp.x,bp.y,input,span);return true;
 };
 s.call=(now,x,y,input)=>{
  if(x<62||x>298||y<135||y>248||s.called&&((now-s.lastCall)>>>0)<900)return -1;
  const t=safe(x-24,y-24);
  if(input.some(p=>(p.x-t.x)**2+(p.y-t.y)**2<34**2))return -1;
  let chosen=5,best=1000000;input.slice(0,5).forEach((p,i)=>{if(p.available&&p.energy>=20){const d=(p.x-t.x)**2+(p.y-t.y)**2;if(d<best){best=d;chosen=i}}});
  if(chosen===5)return -1;s.request(now,2,chosen,0,t.x,t.y,input);s.called=true;s.lastCall=now>>>0;return chosen;
 };
 const move=(a,age)=>{
  return a.kind===1||a.kind===4?{x:a.x,y:a.y,left:a.left}:routeSample(s.scene,a.x,a.y,a.tx,a.ty,age,a.approach,a.left);
 };
 const schedule=(now,input)=>{
  const count=input.length;
  if(due(now,s.nextSolo)){
   s.nextSolo=(now+20000+roll()%20001)>>>0;const start=roll()%5;
   for(let n=0;n<count;n++){const p=(start+n)%count;if(!input[p].available||s.pets[p].kind||input[p].energy<30)continue;
    const kind=input[p].mood>=60&&roll()%3===0?4:3,t=safe(input[p].x+roll()%101-50,input[p].y+roll()%55-27);
    if(!input.some((v,j)=>j!==p&&Math.abs(v.x-t.x)<38&&Math.abs(v.y-t.y)<28))s.request(now,kind,p,0,t.x,t.y,input);break;
   }
  }
  if(due(now,s.nextPair)){
   s.nextPair=(now+60000+roll()%60001)>>>0;let a=5,b=5,best=140**2;
   for(let i=0;i<count;i++)for(let j=i+1;j<count;j++){
    if(!input[i].available||!input[j].available||input[i].energy<30||input[j].energy<30||s.pets[i].kind||s.pets[j].kind)continue;
    const d=(input[i].x-input[j].x)**2+(input[i].y-input[j].y)**2;if(d<best){best=d;a=i;b=j;}
   }
   if(a<5)s.request(now,5+roll()%3,a,b,0,0,input);
  }
 };
 s.tick=(now,input,automatic=true)=>{
  now>>>=0;input=input.slice(0,5);const count=input.length,out=Array.from({length:5},()=>({active:false,release:false,left:false,x:0,y:0,jump:0,pose:0,symbol:0,kind:0}));
  for(let p=0;p<5;p++)if(s.pets[p].kind&&(p>=count||!input[p].available)){const b=s.pets[p].partner;out[p].release=true;if(b<count)out[b].release=true;s.cancel(p)}
  if(automatic&&count)schedule(now,input);else{if(due(now,s.nextSolo))s.nextSolo=(now+20000)>>>0;if(due(now,s.nextPair))s.nextPair=(now+60000)>>>0}
  for(let p=0;p<count;p++){
   const a=s.pets[p],o=out[p];if(o.release)Object.assign(o,{x:input[p].x,y:input[p].y,left:input[p].left});if(!a.kind)continue;
   const age=(now-a.start)>>>0,stationary=a.kind===1||a.kind===4;
   Object.assign(o,{active:true,kind:a.kind,left:a.left,pose:0},move(a,stationary?0:age));
   if(!stationary&&age<a.approach)o.pose=o.x!==a.tx||o.y!==a.ty?4:0;
   else{
    const t=stationary?age:age-a.approach;
    if([1,2,4,8].includes(a.kind)){o.pose=8;o.left=o.x>156;o.symbol=a.kind===1?1:2;const phase=t%650;if(t<1300)o.jump=Math.trunc(16*phase*(650-phase)/(650*650))}
    else if(a.kind===3){o.left=t<800?a.left:t<1700?!a.left:a.left;o.symbol=t<1600?3:0}
    else if(a.kind===5){o.left=a.tx>s.pets[a.partner].tx;const offset=p<a.partner?0:1000;if(t>=offset&&t<offset+1000){o.pose=8;o.symbol=2}}
    else if(a.kind===7)o.left=a.tx>s.pets[a.partner].tx;
    else if(a.kind===6){const anchor=Math.min(a.tx,s.pets[a.partner].tx),direction=anchor<150?1:-1,delay=p<a.partner?800:1200;if(t>=delay){const walk=Math.min(2800,t-delay);Object.assign(o,routeSample(s.scene,a.tx,a.ty,a.tx+direction*54,a.ty,walk,2800,o.left));o.pose=walk<2800?4:0}}
   }
   if(age>=a.duration){o.active=false;o.release=true;o.jump=o.symbol=0;o.pose=0;a.kind=0}
  }
  return out;
 };
 return s;
}
