import {balance} from './balance-config.mjs?v=variety1';
export function highestWildPartner(pets,traveler=-1){
 let best=-1;pets.forEach((p,i)=>{if(p&&i!==traveler&&!p.sleeping&&(p.health??100)>=30&&(p.energy??80)>=3&&(best<0||p.level>pets[best].level))best=i});return best;
}
/* Encounter timing preview; actual combat and grants use the shared C core. */
export function createWildPreview({now=()=>performance.now(),random=Math.random,storage=null,onSpawn=()=>null}={}){
 const cfg=balance.wild,key='pokemon-plus.web-wild-v2',nextSteps=()=>cfg.minSteps+Math.min(cfg.maxSteps-cfg.minSteps,Math.floor(random()*(cfg.maxSteps-cfg.minSteps+1)));
 let s={phase:'idle',steps:nextSteps(),queue:0,visible:cfg.visibleSeconds,activity:0,delay:0,encounter:null},last=now(),remainder=0;
 try{const saved=JSON.parse(storage?.getItem(key));if(saved&&['idle','alert','pending','battle','escaped'].includes(saved.phase)&&['steps','queue','visible','activity','delay'].every(k=>Number.isInteger(saved[k])&&saved[k]>=0)&&saved.steps<=cfg.maxSteps&&saved.queue<=cfg.queue&&saved.visible<=cfg.visibleSeconds&&saved.activity<cfg.activitySeconds&&saved.delay<=cfg.nextDelay)s={...saved,phase:saved.phase==='battle'?'pending':saved.phase}}catch{}
 const mutate=fn=>{const before={...s};try{const result=fn();if(JSON.stringify(s)!==JSON.stringify(before))storage?.setItem(key,JSON.stringify(s));return result}catch(e){s=before;throw e}};
 const charge=()=>{if(s.queue>=cfg.queue)return;s.queue++;s.activity=0;s.steps=nextSteps()};
 const spawn=()=>{if(!s.queue||s.delay||['alert','pending','battle'].includes(s.phase))return false;s.encounter=onSpawn();s.phase='alert';s.visible=cfg.visibleSeconds;return true};
 const dismiss=()=>{if(!['alert','pending','battle'].includes(s.phase))return false;s.phase='escaped';s.queue=Math.max(0,s.queue-1);s.delay=cfg.nextDelay;s.visible=cfg.visibleSeconds;return true};
 return {get phase(){return s.phase},get encounter(){return s.encounter},get queue(){return s.queue},get stepsRemaining(){return s.queue>=cfg.queue?0:s.steps},get remaining(){return s.phase==='alert'?s.visible:0},snapshot:()=>({...s}),
  walk(count,{enabled=true}={}){return mutate(()=>{if(!enabled)return false;let steps=Number.isFinite(count)?Math.max(0,Math.floor(count)):0;while(steps&&s.queue<cfg.queue){const take=Math.min(steps,s.steps);steps-=take;s.steps-=take;if(!s.steps)charge()}return spawn()})},
  tick({active=true,enabled=true}={}){const current=now(),delta=Math.max(0,current-last);last=current;const total=remainder+delta,elapsed=Math.floor(total/1000);remainder=total%1000;return mutate(()=>{if(!active||!enabled||elapsed>5)return false;if(s.phase==='alert'){s.visible=Math.max(0,s.visible-elapsed);if(!s.visible)return dismiss();return false}if(['pending','battle'].includes(s.phase))return false;if(s.queue<cfg.queue){s.activity+=elapsed;if(s.activity>=cfg.activitySeconds)charge()}if(s.delay){s.delay=Math.max(0,s.delay-elapsed);return false}return spawn()})},
  enter(){return mutate(()=>{if(!s.queue)s.queue=1;s.delay=0;s.phase='idle';return spawn()})},
  reveal(){return mutate(()=>{if(s.phase!=='alert'||!s.visible)return false;s.phase='pending';return true})},
  expire(){return s.phase==='escaped'},
  confirm(){return mutate(()=>{if(s.phase!=='pending')return false;s.phase='battle';return true})},
  dismiss(){return mutate(dismiss)}
 };
}
