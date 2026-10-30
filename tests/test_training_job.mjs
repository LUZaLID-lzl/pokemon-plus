import assert from 'node:assert/strict';
import fs from 'node:fs';
import {createTrainingJob,trainingDurationMs,validTrainingSave} from '../web/training-job.mjs';
import {createEvolutionDemo} from '../web/evolution-choice.js';
const manifest=JSON.parse(fs.readFileSync(new URL('../web/manifest.json',import.meta.url)));
const expected=fs.readFileSync(new URL('../build/training-job-trace.jsonl',import.meta.url),'utf8').trim().split('\n').map(JSON.parse),trace=[];
for(const initialLevel of [1,12])for(let scene=0;scene<6;scene++)for(let count=1;count<=3;count++){
 let pets=[1,2,3].map((species,i)=>({id:String(i),art:manifest.pets.find(p=>p.species===species&&p.stage===0).id,level:initialLevel,xp:0,ivs:[15,15,15,15,15,15],nature:0,health:90,food:90,clean:90,mood:90,energy:100})),saved;
 const commit=(snapshot,reward)=>{saved=structuredClone(snapshot);for(const row of reward.xp)for(const id of row.ids){const p=pets.find(p=>p.id===id);let total=p.xp+row.xp;while(p.level<60&&total>=20*p.level){total-=20*p.level;p.level++}p.xp=p.level===60?0:total;Object.assign(p,row.changes)}};
 let t=createTrainingJob({manifest,getPets:()=>pets,commit,seed:123,now:()=>0});t.start(scene,pets.slice(0,count).map(p=>p.id));
 for(let ms=0;ms<trainingDurationMs;ms+=60000){t.advance(60000);if(ms%600000===0)t=createTrainingJob({manifest,getPets:()=>pets,commit,saved,now:()=>0})}
 assert.equal(t.job.status,2);assert.equal(t.busy('0'),false);assert.equal(t.job.lootChecks,Math.floor(t.job.elapsedMs/60000));
 trace.push([scene,count,t.state.rounds,t.state.totalXp,t.job.coins,t.job.items,pets[0].level,pets[0].xp,t.snapshot().battle.rng,t.job.lootRng]);
 const receipt=t.snapshot();t.advance(trainingDurationMs);assert.deepEqual(t.snapshot(),receipt);
}
fs.writeFileSync(new URL('../build/training-job-web-trace.json',import.meta.url),JSON.stringify(trace));assert.deepEqual(trace,expected);
const store=new Map();let fail=false;const storage={getItem:k=>store.get(k)||null,setItem(k,v){if(fail)throw Error('quota');store.set(k,v)}};
const demo=createEvolutionDemo({storage,roster:manifest.pets,chains:manifest.evolution.chains}),pet=demo.add(1,12);
demo.trainingCommit(null,{xp:[{ids:[pet.id],xp:230}]});
const t=createTrainingJob({manifest,getPets:()=>demo.state.pets,commit:(s,r)=>demo.trainingCommit(s,r),seed:123,now:()=>0});t.start(0,[pet.id]);
assert(demo.trainingBusy(pet.id));demo.setActive([pet.id]);assert.throws(()=>demo.release(pet.id));
const before=t.snapshot(),persistent=structuredClone(demo.state);fail=true;assert.throws(()=>t.advance(60000),/quota/);assert.deepEqual(t.snapshot(),before);assert.deepEqual(demo.state,persistent);
fail=false;t.advance(60000);assert.equal(t.job.elapsedMs,60000);assert.deepEqual(demo.state.training,t.snapshot());
const reloaded=createEvolutionDemo({storage,roster:manifest.pets,chains:manifest.evolution.chains});assert(reloaded.trainingBusy(pet.id));
console.log('PASS: 36 C/Web one-hour jobs match exactly (battles/XP/coins/item weights/levels/RNG), saved resume, no duplicate completion, atomic quota rollback/retry and busy individuals');

// Checkpoint time is processed time, so a second reload cannot lose offline catch-up.
const anchored=createTrainingJob({manifest,getPets:()=>demo.state.pets,commit:()=>{},seed:123,now:()=>100000});anchored.start(0,[pet.id]);anchored.advance(60000);assert.equal(anchored.job.wallAt,160000);anchored.checkpoint();assert.equal(anchored.job.wallAt,160000);
for(const corrupt of [s=>s.job.items=null,s=>s.battle.state.scene=99,s=>s.battle.state.party[0].art=999,s=>s.battle.state.phase='bad',s=>s.battle.state.party[0].hp=-1,s=>s.job.elapsedMs=NaN]){const bad=t.snapshot();corrupt(bad);assert(!validTrainingSave(bad,manifest.pets,demo.state.pets));const clean=createTrainingJob({manifest,getPets:()=>demo.state.pets,commit:()=>{},saved:bad});assert.equal(clean.job.status,0)}
const full=createTrainingJob({manifest,getPets:()=>demo.state.pets,getInventory:()=>({coins:0xffffffff,items:Array(9).fill(65535)}),commit:()=>{},seed:123,now:()=>0});full.start(0,[pet.id]);full.advance(trainingDurationMs);assert.equal(full.job.coins,0);assert(full.job.items.every(n=>n===0));
// Each found item is real persistent stock, and occupied individuals cannot use it.
t.advance(trainingDurationMs);demo.trainingCommit(demo.state.training,{xp:[{ids:[pet.id],xp:0,changes:{food:50,clean:50,health:50,mood:50,energy:50}}]});demo.trainingCommit(demo.state.training,{items:[1,1,0,1,1,1,1,1,1]});const stock=[...demo.state.trainingItems];for(let i=0;i<8;i++)if(i!==2)demo.useTrainingItem(i,i<5?pet.id:undefined);assert.deepEqual(demo.state.trainingItems.slice(0,8),stock.slice(0,8).map((n,i)=>i===2?0:n-1));const used=createEvolutionDemo({storage,roster:manifest.pets,chains:manifest.evolution.chains});assert.deepEqual(used.state.trainingItems,demo.state.trainingItems);
console.log('PASS: processed-time checkpoints, invalid saves rejected, full inventory/coins, all found consumables persist and consume once');

let result;
for(const varied of [false,true]){let pets=[{id:'q',art:1,level:12,xp:0,health:90,food:90,clean:90,mood:90,energy:100,nature:0,ivs:[15,15,15,15,15,15]}],saved;const commit=(s,r)=>{saved=s;for(const row of r.xp){let p=pets[0],total=p.xp+row.xp;while(p.level<60&&total>=20*p.level){total-=20*p.level;p.level++}p.xp=p.level===60?0:total;Object.assign(p,row.changes)}};let t=createTrainingJob({manifest,getPets:()=>pets,commit,seed:123,now:()=>0});t.start(0,['q']);const spans=[17,51,233,1000,59999];let i=0;while(t.job.status===1){t.advance(varied?spans[i++%5]:60000);if(varied&&i%101===0){t.checkpoint();t=createTrainingJob({manifest,getPets:()=>pets,commit,saved,now:()=>0})}}if(!varied)result=t.snapshot();else assert.deepEqual(t.snapshot(),result)}
console.log('PASS: varied foreground/background tick sizes and reloads preserve identical one-hour results');

// Completion receipts persist the original/end levels and acknowledge each individual once.
t.advance(trainingDurationMs);assert(t.pendingGrowth);assert.deepEqual(t.job.startLevels,[12]);assert(t.job.endLevels[0]>12);assert(demo.trainingGrowthPending(pet.id));
assert.throws(()=>t.start(0,[pet.id]),/升级提示/);
const receiptBefore=t.snapshot(),storeBefore=structuredClone(demo.state);fail=true;assert.throws(()=>t.ackNotice('growth',0),/quota/);assert.deepEqual(t.snapshot(),receiptBefore);assert.deepEqual(demo.state,storeBefore);fail=false;
t.ackNotice('growth',0);assert(t.pendingGrowth);assert(t.job.growthSeen&1);t.ackNotice('growth',0);assert.equal(t.job.growthSeen,1);
const resumed=createTrainingJob({manifest,getPets:()=>demo.state.pets,commit:(s,r)=>demo.trainingCommit(s,r),saved:demo.state.training,now:()=>0});assert(resumed.pendingGrowth);assert.equal(resumed.job.growthSeen,1);resumed.ackNotice('evolution',0);assert(!resumed.pendingGrowth&&!demo.trainingGrowthPending(pet.id));
assert.throws(()=>resumed.ackNotice('growth',3),/无效/);assert.throws(()=>resumed.ackNotice('bad',0),/无效/);
const again=createTrainingJob({manifest,getPets:()=>demo.state.pets,commit:(s,r)=>demo.trainingCommit(s,r),saved:demo.state.training,now:()=>0});assert(!again.pendingGrowth);assert.equal(again.job.evolutionSeen,1);
const cappedPets=[{id:'cap',art:1,level:60,xp:0,health:90,food:90,clean:90,mood:90,energy:100,nature:0,ivs:[15,15,15,15,15,15]}];const capped=createTrainingJob({manifest,getPets:()=>cappedPets,commit:()=>{},seed:123,now:()=>0});capped.start(0,['cap']);capped.advance(trainingDurationMs);assert(!capped.pendingGrowth);assert.deepEqual(capped.job.startLevels,[60]);assert.deepEqual(capped.job.endLevels,[60]);
console.log('PASS: completion growth receipts, cap/no-growth, queued notices survive reload, acknowledgements persist once, failed writes roll back and next job is protected');

// Existing v16 running checkpoints beyond the new limit settle without replaying rewards.
for(const phase of ['search','approach','alert','fight','result']){
 const pets=[{id:'legacy',art:1,level:12,xp:0,health:90,food:90,clean:90,mood:90,energy:100,nature:0,ivs:[15,15,15,15,15,15]}];
 let commits=0,fail=false;const commit=(s,r)=>{if(fail)throw Error('quota');commits++;assert.deepEqual(r.xp,[]);assert.equal(r.coins,0);assert(r.items.every(n=>n===0))};
 const original=createTrainingJob({manifest,getPets:()=>pets,commit:()=>{},seed:123,now:()=>0});original.start(0,['legacy']);
 for(let i=0;original.state.phase!==phase&&i<10000;i++)original.advance(40);
 assert.equal(original.state.phase,phase);const saved=original.snapshot();Object.assign(saved.job,{elapsedMs:5400000,ambientMs:5400000,lootChecks:90,pendingMs:17});
 assert(validTrainingSave(saved,manifest.pets,pets));const job=createTrainingJob({manifest,getPets:()=>pets,commit,saved,now:()=>0});
 fail=true;assert.throws(()=>job.advance(0),/quota/);assert.deepEqual(job.snapshot(),saved);fail=false;job.advance(0);
 assert.equal(job.job.status,2);assert.equal(job.job.elapsedMs,5400000);assert.equal(job.job.pendingMs,0);assert(!job.busy('legacy'));assert.equal(commits,1);
 const receipt=job.snapshot();job.advance(trainingDurationMs);assert.deepEqual(job.snapshot(),receipt);assert.equal(commits,1);
 Object.assign(receipt.job,{elapsedMs:10800000,ambientMs:10800000,lootChecks:180,items:[100,0,0,0,0,0,0,0,0]});assert(validTrainingSave(receipt,manifest.pets,pets));
 const completed=createTrainingJob({manifest,getPets:()=>pets,commit,saved:receipt,now:()=>0});assert.deepEqual(completed.snapshot(),receipt);completed.advance(trainingDurationMs);assert.equal(commits,1);
 receipt.job.elapsedMs+=40;assert(!validTrainingSave(receipt,manifest.pets,pets));
}
assert.equal(trainingDurationMs,3600000);
console.log('PASS: old running checkpoints in every phase settle at once, failed writes roll back, old three-hour receipts survive, no duplicate reward');

for(const phase of ['search','approach','alert','fight','result','earned']){
 let pets=[{id:'quit',art:1,level:12,xp:0,health:90,food:90,clean:90,mood:90,energy:100,nature:0,ivs:[15,15,15,15,15,15]}],saved,fail=false,coins=0,items=Array(9).fill(0);
 const commit=(s,r)=>{if(fail)throw Error('quota');saved=structuredClone(s);coins+=r.coins;items=items.map((v,i)=>v+r.items[i]);for(const row of r.xp){const p=pets[0];let total=p.xp+row.xp;while(p.level<60&&total>=20*p.level){total-=20*p.level;p.level++}p.xp=p.level===60?0:total;Object.assign(p,row.changes)}};
 const job=createTrainingJob({manifest,getPets:()=>pets,commit,seed:123,now:()=>0});job.start(0,['quit']);
 if(phase==='earned')job.advance(600000);else {for(let i=0;job.state.phase!==phase&&i<10000;i++)job.advance(40);assert.equal(job.state.phase,phase)}
 job.advance(17);assert.equal(job.job.pendingMs,17);const before=job.snapshot(),stock={coins,items:[...items],pets:structuredClone(pets)};
 fail=true;assert.throws(()=>job.finish(),/quota/);assert.deepEqual(job.snapshot(),before);assert(job.busy('quit'));assert.deepEqual({coins,items,pets},stock);fail=false;
 job.finish();assert.equal(job.job.status,2);assert(!job.busy('quit'));assert.equal(job.job.pendingMs,0);assert.equal(job.job.elapsedMs,before.job.elapsedMs);assert.deepEqual({coins,items,pets},stock);assert.equal(job.state.rounds,before.battle.state.rounds);assert.equal(job.job.lootChecks,before.job.lootChecks);
 assert(validTrainingSave(saved,manifest.pets,pets));const receipt=job.snapshot();job.finish();job.advance(trainingDurationMs);assert.deepEqual(job.snapshot(),receipt);assert.deepEqual({coins,items,pets},stock);
 const again=createTrainingJob({manifest,getPets:()=>pets,commit,saved,now:()=>0});assert.deepEqual(again.snapshot(),receipt);
 if(phase==='earned'&&again.pendingGrowth){assert.throws(()=>again.start(0,['quit']),/升级提示/);again.ackNotice('growth',0);again.ackNotice('evolution',0);assert(!again.pendingGrowth)}
}
console.log('PASS: immediate/approach/alert/fight/result and earned-reward exits, atomic failed-exit retry, no unfinished battle XP, unchanged earned loot, reload and growth queue');
