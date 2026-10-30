// Session-only resource preview. Device inventory and NVS remain independent.
export function eggPosition(nests,scene){
 const n=nests.scenes[scene]||nests.scenes[0];
 return {x:n.position[0]+nests.eggOffset[0],y:n.position[1]+nests.eggOffset[1]};
}
export function createHatchPreview(nests,now=()=>performance.now()){
 let boosts=0,egg=null;
 return {
  get boosts(){return boosts},get egg(){return egg},
  position(scene){return eggPosition(nests,scene)},
  remaining(){return egg?Math.max(0,Math.ceil((egg.endsAt-now())/1000)):0},
  notice(){const seconds=this.remaining();return seconds?`距离孵化还有\n${[Math.floor(seconds/3600),Math.floor(seconds/60)%60,seconds%60].map(n=>String(n).padStart(2,'0')).join(':')}`:'孵化时间已到'},
  buy(coins){if(coins<120)throw Error('金币不足');if(boosts>=65535)throw Error('背包已满');boosts++;return coins-120},
  place(count){if(egg)throw Error('场上已经有蛋了');egg={active:true,endsAt:now()+7200000};return egg},
  use(count,found=false){if(!boosts&&!found)throw Error('背包里没有孵化药剂');if(!egg)throw Error('场上还没有宠物蛋');if(!found)boosts--;egg=null}
 };
}
