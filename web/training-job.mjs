import {balance} from './balance-config.mjs?v=autosupply1-trainingxp2-shoplottery1';
import {createTraining} from './training.mjs?v=trainingxp2-immunity1-variety1';
import {trainingSupply} from './training-supply.mjs?v=autosupply1';
export const trainingDurationMs=3600000;
const legacyTrainingDurationMs=10800000;
export function validTrainingSave(s,roster,pets){
 const int=(v,max=0xffffffff)=>Number.isSafeInteger(v)&&v>=0&&v<=max;
 const actor=p=>p&&int(p.art,roster.length-1)&&int(p.level,60)&&p.level>0&&int(p.nature,24)&&Array.isArray(p.ivs)&&p.ivs.length===6&&p.ivs.every(v=>int(v,31))&&int(p.maxHp,32767)&&p.maxHp>0&&int(p.hp,p.maxHp);
 const j=s?.job,b=s?.battle,h=b?.state;
 if(s?.format!==1||!j||!b||!h||![1,2].includes(j.status)||!int(j.elapsedMs,legacyTrainingDurationMs)||!int(j.pendingMs,39)||j.elapsedMs%40||j.elapsedMs+j.pendingMs>legacyTrainingDurationMs||!int(j.walkMs,j.elapsedMs)||j.ambientMs!==j.elapsedMs||!int(j.wallAt,Number.MAX_SAFE_INTEGER)||!int(j.lootRng)||!j.lootRng||j.lootChecks!==Math.floor(j.elapsedMs/60000)||!int(j.coins)||!Array.isArray(j.startLevels)||j.startLevels.length!==h.party?.length||!j.startLevels.every(v=>int(v,60)&&v>0)||!Array.isArray(j.endLevels)||j.endLevels.length!==j.startLevels.length||!j.endLevels.every((v,i)=>int(v,60)&&v>=j.startLevels[i])||!int(j.growthSeen,7)||!int(j.evolutionSeen,7)||!Array.isArray(j.items)||j.items.length!==9||!j.items.every(v=>int(v,180)))return false;
 if(!int(b.rng)||!b.rng||!int(b.battleRng)||!b.battleRng||!int(b.speedRemainder,1)||!int(h.scene,5)||!['select','search','approach','alert','fight','result'].includes(h.phase)||!Array.isArray(h.party)||h.party.length<1||h.party.length>3||!h.party.every(actor)||new Set(h.party.map(p=>p.id)).size!==h.party.length)return false;
 if(j.status===1&&(h.phase==='select'||j.elapsedMs===legacyTrainingDurationMs||!h.party.every(p=>pets.some(x=>x.id===p.id))))return false;
 if(j.status===2&&(h.phase!=='select'||j.pendingMs))return false;
 if(j.status===2&&j.startLevels.some((level,i)=>j.endLevels[i]>level&&(!(j.growthSeen&(1<<i))||!(j.evolutionSeen&(1<<i))))&&!h.party.every(p=>pets.some(x=>x.id===p.id)))return false;
 if(!Array.isArray(h.foes)||h.foes.length>3||!h.foes.every(actor)||!int(h.ally,h.party.length-1)||!int(h.enemy,Math.max(0,h.foes.length-1))||!int(h.round,100)||!int(h.rounds)||!int(h.totalXp)||!int(h.xp)||!int(h.elapsed,16000)||!int(h.checks,16)||!int(h.resultTime,3449)||!int(h.approachTime,2049)||!int(h.alertTime,2249)||typeof h.claimed!=='boolean')return false;
 return !['approach','alert','fight','result'].includes(h.phase)||h.foes.length===h.party.length;
}
export function createTrainingJob({manifest,getPets,commit,saved,getInventory=()=>({coins:0,items:Array(9).fill(0)}),getSupplyInventory=getInventory,autoSupply=()=>false,temperance=()=>false,getActiveIds=()=>getPets().map(p=>p.id),seed=1,now=()=>Date.now()}){
 let rewards=[],lastSaved=0;
 const virtualPets=()=>getPets().map(p=>{let total=p.xp||0,level=p.level;for(const row of rewards)if(row.ids.includes(p.id)){total+=row.xp;p={...p,...row.changes}};while(level<60&&total>=20*level){total-=20*level;level++}return {...p,level,xp:level===60?0:total}});
 const training=createTraining({manifest,getPets:virtualPets,grant:(ids,xp,changes)=>rewards.push({ids,xp,changes}),seed});
 let job={status:0,elapsedMs:0,pendingMs:0,walkMs:0,ambientMs:0,lootRng:1,lootChecks:0,coins:0,items:Array(9).fill(0),startLevels:[],endLevels:[],growthSeen:0,evolutionSeen:0,wallAt:now()};
 const snapshot=()=>({format:1,job:structuredClone(job),battle:training.snapshot()});
 const restore=s=>{job=structuredClone(s.job);training.restore(s.battle)};
 if(validTrainingSave(saved,manifest.pets,getPets())){restore(saved);if(job.status===1&&!job.rulesVersion){job.status=2;job.pendingMs=0;job.endLevels=training.state.party.map(p=>getPets().find(x=>x.id===p.id)?.level||p.level);training.close()}lastSaved=job.elapsedMs}
 const next=()=>{let x=job.lootRng;x^=x<<13;x^=x>>>17;x^=x<<5;return job.lootRng=x>>>0||1};
 function checkpoint(extra={coins:0,items:Array(9).fill(0)}){commit(snapshot(),{xp:rewards, ...extra});rewards=[];lastSaved=job.elapsedMs}
 function careEvent(kind){
  const fields=['energy','food','clean','mood'],field=fields[kind],pets=virtualPets();let target,deficit=0;
  for(const actor of training.state.party){const p=pets.find(p=>p.id===actor.id);if(p&&100-p[field]>deficit){target=p;deficit=100-p[field]}}
  if(target)rewards.push({ids:[target.id],xp:0,changes:{[field]:Math.min(100,target[field]+2)}});
 }
 function discover(extra){
  const chance=next()%100;job.lastItem=0;job.lastCoins=0;
  if(chance<50){const coins=Math.min(5+next()%4,0xffffffff-getInventory().coins-extra.coins);job.coins+=coins;job.lastCoins=coins;extra.coins+=coins}
  else if(chance<60){
   const weights=balance.training.itemWeights;let roll=next()%weights.reduce((a,b)=>a+b,0),item=0;
   while(item+1<weights.length&&roll>=weights[item]){roll-=weights[item];item++}
   if((getInventory().items[item]||0)+extra.items[item]<65535){job.items[item]++;extra.items[item]++;job.lastItem=item+1;}
  }else careEvent(Math.floor((chance-60)/10));
 }
 function finishJob(){if(training.state.phase==='result'&&!training.state.claimed)training.claim();job.pendingMs=0;job.status=2;job.endLevels=training.state.party.map(p=>virtualPets().find(x=>x.id===p.id).level);training.close()}
 function supply(extra){
  for(const actor of training.state.party){
   const p=virtualPets().find(p=>p.id===actor.id);if(!p)continue;
   const stock=getSupplyInventory().items.map((v,i)=>(v||0)+extra.items[i]);
   const {changes,used}=trainingSupply(p,stock,temperance());
   if(used.some(Boolean)){rewards.push({ids:[p.id],xp:0,changes});used.forEach((v,i)=>extra.items[i]-=v)}
  }
 }
 const canContinue=()=>training.state.party.every(p=>{const actual=virtualPets().find(x=>x.id===p.id);return actual&&!actual.sleeping&&actual.energy>=20&&actual.food>=20&&actual.health>=30});
 const pending=()=>job.status===2&&job.startLevels.some((level,i)=>job.endLevels[i]>level&&(!(job.growthSeen&(1<<i))||!(job.evolutionSeen&(1<<i))));
 return {get pendingGrowth(){return pending()},ackNotice(kind,index){if(job.status!==2||!['growth','evolution'].includes(kind)||!Number.isInteger(index)||index<0||index>=training.state.party.length)throw Error('升级提示无效');const before=snapshot();try{job[kind+'Seen']|=1<<index;checkpoint()}catch(e){restore(before);throw e}},get state(){return training.state},get job(){return job},snapshot,checkpoint,
  finish(){if(job.status!==1)return;const before=snapshot();try{finishJob();checkpoint()}catch(e){restore(before);rewards=[];throw e}},
  busy:id=>job.status===1&&training.state.party.some(p=>p.id===id),
  start(scene,ids){const before=snapshot();try{if(job.status===1)throw Error('伙伴正在历练');if(pending())throw Error('请先查看历练升级提示');if(ids.some(id=>!getActiveIds().includes(id)))throw Error('仅限庭院内的伙伴');training.start(scene,ids);job={rulesVersion:1,status:1,elapsedMs:0,pendingMs:0,walkMs:0,ambientMs:0,lootRng:(training.rng^0x9e3779b9)>>>0||1,lootChecks:0,coins:0,items:Array(9).fill(0),startLevels:[],endLevels:[],growthSeen:0,evolutionSeen:0,wallAt:now()};job.startLevels=training.state.party.map(p=>p.level);job.endLevels=[...job.startLevels];checkpoint()}catch(e){restore(before);throw e}},
  advance(ms){
   if(job.status!==1)return;
   const before=snapshot(),extra={coins:0,items:Array(9).fill(0)};
   try{
    ms=Math.min(Number.isFinite(ms)?Math.max(0,Math.floor(ms)):0,Math.max(0,trainingDurationMs-job.elapsedMs-job.pendingMs));job.wallAt+=ms;ms+=job.pendingMs;job.pendingMs=ms%40;ms-=job.pendingMs;const fractionalPending=job.pendingMs;
    while(ms&&job.status===1){const step=Math.min(40,ms),was=training.state.phase;
     if(autoSupply()&&was==='search'){supply(extra);if(!canContinue()){finishJob();break}}
     ms-=step;
     if(['search','approach'].includes(was))job.walkMs+=step;
     job.ambientMs+=step;job.elapsedMs+=step;training.advance(step);
     if(training.state.phase==='result'){if(!training.state.claimed){training.claim();training.state.resultTime+=step}if(training.state.resultTime>=3200){if(autoSupply())supply(extra);if(canContinue())training.resume();else finishJob()}}
     if(Math.floor(job.elapsedMs/60000)>job.lootChecks){job.lootChecks++;if(job.lootChecks%5===0)discover(extra);if(job.lootChecks%15===0)for(const p of virtualPets().filter(p=>training.state.party.some(x=>x.id===p.id))){if(p.clean<40)rewards.push({ids:[p.id],xp:0,changes:{health:Math.max(0,p.health-(p.clean<20?2:1))}})}}
    }
    if(job.status===2)job.wallAt-=ms+fractionalPending;
    if(job.elapsedMs>=trainingDurationMs)finishJob();
    if(rewards.length||extra.coins||extra.items.some(Boolean)||job.status===2||job.elapsedMs-lastSaved>=5000)checkpoint(extra);
   }catch(e){restore(before);rewards=[];throw e}
  }
 };
}
