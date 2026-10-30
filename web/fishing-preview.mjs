import {fishCatalog} from './fish-catalog.js?v=fishingdex1';
export const fishWeight=(id,quality)=>fishCatalog[id].weight*[[100,100,100,100],[100,120,150,175],[100,150,250,400]][Math.min(2,quality)][fishCatalog[id].rarity];
export const fishWeightTotal=q=>fishCatalog.reduce((s,_,i)=>s+fishWeight(i,q),0);
export function pickFish(roll,quality){roll%=fishWeightTotal(quality);for(let i=0;i<fishCatalog.length;i++){const w=fishWeight(i,quality);if(roll<w)return i;roll-=w}return fishCatalog.length-1}
export function createFishing(seed=42,{onCatch=()=>undefined}={}){
 let phase=0,rng=seed>>>0||1,started=0,waitMs=0,target=50,reward=0,fish=0,needle=0,quality=0,caught=false,pausedAt=null;
 function random(){let x=rng;x^=x<<13;x^=x>>>17;x^=x<<5;return rng=x>>>0||1}
 const state=()=>({phase,started,waitMs,target,reward,fish,needle,quality,caught});
 function tick(time){if(pausedAt!==null)return state();const now=time>>>0;let age=(now-started)>>>0;
  if(phase===1&&age>=waitMs){phase=2;started=(started+waitMs)>>>0;age=(now-started)>>>0}
  if(phase===2){const p=age%2000;needle=Math.floor((p<=1000?p:2000-p)/10);if(age>=6000){phase=3;reward=0;caught=false}}
  return state();
 }
 return {get state(){return state()},tick,
  press(time){if(pausedAt!==null)return 0;tick(time);const now=time>>>0;
   if(phase===0||phase===3){phase=1;started=now;reward=0;fish=0;needle=0;caught=false;quality=0;waitMs=6000+random()%4001;target=30+random()%41;return 0}
   if(phase!==2)return 0;
   const d=Math.abs(needle-target);if(d>18){phase=3;caught=false;return 0}
   const savedRng=rng,q=d<=4?2:d<=10?1:0,total=fishWeightTotal(q),limit=0xffffffff-0xffffffff%total;let roll;
   do{roll=random()}while(roll>=limit);
   const id=pickFish(roll,q),amount=Math.max(2,Math.floor(fishCatalog[id].coins/6))+q;let granted;
   try{granted=onCatch(id,amount)}catch(e){rng=savedRng;throw e}
   fish=id;quality=q;reward=granted??amount;caught=true;phase=3;return reward;
  },screen(on,time){if(!on&&pausedAt===null)pausedAt=time>>>0;else if(on&&pausedAt!==null){started=(started+((time>>>0)-pausedAt))>>>0;pausedAt=null}}
 };
}
