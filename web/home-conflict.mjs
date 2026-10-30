export const conflictRules=Object.freeze({minMs:180000,maxMs:300000,chance:25,impactMs:700,durationMs:1800,health:3,mood:5});
export const feistyNature=n=>[3,4,8,11,24].includes(n);
const palette=[0xffdf69,0x8fe68b,0xff9a63,0x6edaff,0xe6d2c1,0xb8d961,0xc78de0,0xb4e8ed,0xd6b685,0xbcb397,0xf4a8c5,0xd695ed,0xe6a083,0xa8e5f3,0xcbd8e2,0x9f87dc,0x796b99,0x9296ff];
export function createHomeConflict(seed,now=0){
 const s={rng:seed>>>0,nextCheck:0,started:0,attacker:0,target:0,slot:0,type:0,active:false,hit:false,suspended:false,healthLoss:0,moodLoss:0};
 const roll=()=>s.rng=(Math.imul(s.rng,1664525)+1013904223)>>>0;
 const schedule=now=>s.nextCheck=(now+180000+roll()%120001)>>>0;
 const available=(p,mask,i)=>p&&!!(mask&(1<<i))&&!p.sleeping&&!p.traveling&&p.health>0&&p.energy>=30;
 const valid=(pets,mask,a,b,hit=false)=>a!==b&&available(pets[a],mask,a)&&available(pets[b],mask,b)&&feistyNature(pets[a].nature)&&pets[a].health>=30&&(hit||pets[b].health>1);
 function pause(now){if(!s.suspended)schedule(now);s.suspended=true;s.active=false}
 function start(pets,now,a,b,slot){
  if(s.active||!valid(pets,31,a,b)||!pets[a].moves?.[slot]?.power)return false;
  schedule(now);Object.assign(s,{attacker:a,target:b,slot,type:pets[a].moves[slot].type,started:now>>>0,active:true,hit:false,suspended:false,healthLoss:0,moodLoss:0});return true;
 }
 function tick(pets,now,enabled,mask,onHit=()=>{}){
  now=now>>>0;
  if(!enabled){const active=s.active;pause(now);return active?'cancel':'none'}
  if(s.suspended){s.suspended=false;schedule(now)}
  if(s.active){
   if(!valid(pets,mask,s.attacker,s.target,s.hit)){s.active=false;return 'cancel'}
   const age=(now-s.started)>>>0;
   if(age>=1800){s.active=false;return 'end'}
   if(!s.hit&&age>=700){
    const p=pets[s.target],health=Math.min(3,p.health-1),mood=Math.min(5,p.mood);
    onHit(s.target,health,mood);Object.assign(s,{hit:true,healthLoss:health,moodLoss:mood});return 'hit';
   }
   return 'none';
  }
  if((now-s.nextCheck|0)<0)return 'none';schedule(now);if(roll()%100>=25)return 'none';
  const pairs=[];for(let a=0;a<pets.length;a++)for(let b=0;b<pets.length;b++)if(valid(pets,mask,a,b))pairs.push([a,b]);
  if(!pairs.length)return 'none';const [a,b]=pairs[roll()%pairs.length],slots=pets[a].moves.map((m,i)=>m.power?i:-1).filter(i=>i>=0);
  if(!slots.length)return 'none';return start(pets,now,a,b,slots[roll()%slots.length])?'begin':'none';
 }
 schedule(now);return {get state(){return s},pause,start,tick};
}
export function sampleHomeConflict(s,now,sx,sy,tx,ty){
 const age=(now-s.started)>>>0;if(!s.active||age<200||age>=1150)return [];
 const tint=palette[s.type]??palette[4],t=Math.min(age,700),x=sx+Math.trunc((tx-sx)*(t-200)/500),y=sy+Math.trunc((ty-sy)*(t-200)/500);
 const dirs=[[1,0],[1,1],[0,1],[-1,1],[-1,0],[-1,-1],[0,-1],[1,-1]];
 return Array.from({length:16},(_,i)=>{
  let dx,dy,w=3,h=3;
  if(age>=700){const radius=5+Math.floor((age-700)/35)+Math.floor(i/8)*4;dx=dirs[i%8][0]*radius;dy=dirs[i%8][1]*radius}
  else{dx=i%4*5-8;dy=Math.floor(i/4)*4-6+(Math.floor(age/60)+i)%3*2;if(s.type===0){dx+=i%2?3:-3;h=6}if([1,5].includes(s.type)){w=6;h=3}if(s.type===2)dy-=i%3*3}
  return {x:x+dx,y:y+dy,w,h,color:i%4?tint:0xfff2d5,opacity:age<700?230:Math.floor(230*(1150-age)/450)};
 });
}
