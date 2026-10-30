export const hospitalGoal=6,hospitalFall=800,hospitalSlot=1000,hospitalRounds=12;
export function createHospital(seed=1){
 let h;const random=()=>{let x=h.rng;x^=x<<13;x^=x>>>17;x^=x<<5;return h.rng=(x>>>0)||1};
 function init(s){h={rng:(s>>>0)||1,elapsed:0,phase:0,score:0,slot:0,lane:0,basket:1,resolved:false,claimed:false}}
 init(seed);
 const cloud=()=>h.slot%4===3;
 function start(){const s=random();init(s);h.phase=1;h.lane=random()%3}
 function select(lane){if(h.phase===1&&Number.isInteger(lane)&&lane>=0&&lane<3)h.basket=lane}
 function advance(delta){
  if(h.phase!==1)return h;
  if(!Number.isFinite(delta)||delta<0)throw Error('Invalid hospital time');
  h.elapsed+=Math.min(Math.floor(delta),hospitalRounds*hospitalSlot-h.elapsed);
  while(h.phase===1){const age=h.elapsed-h.slot*hospitalSlot;
   if(!h.resolved&&age>=hospitalFall){h.resolved=true;if(h.lane===h.basket){if(cloud())h.score=Math.max(0,h.score-1);else if(++h.score>=hospitalGoal){h.phase=2;break}}}
   if(age<hospitalSlot)break;
   if(++h.slot>=hospitalRounds){h.phase=3;break}
   h.resolved=false;h.lane=random()%3;
  }return h;
 }
 // Commit the complete proposed roster before consuming the round. A quota
 // failure leaves the same random outcomes available for a safe retry.
 function claim(apply){
  if(h.phase!==2||h.claimed)throw Error('请先接满爱心');
  const previous=h.rng,targets=Array.from({length:5},()=>[0,0,60,0,0]);
  try{const result=apply(targets);h.claimed=true;return result}catch(e){h.rng=previous;throw e}
 }
 return {get state(){return h},start,select,advance,cloud,claim};
}
