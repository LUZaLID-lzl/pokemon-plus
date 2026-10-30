import assert from 'node:assert/strict';
import {createFishing,pickFish,fishWeight,fishWeightTotal} from '../web/fishing-preview.mjs';
import {fishCatalog} from '../web/fish-catalog.js';
assert.equal(fishCatalog.length,27);
for(let q=0;q<3;q++){const counts=fishCatalog.map(()=>0),total=fishWeightTotal(q);for(let n=0;n<total;n++)counts[pickFish(n,q)]++;assert.deepEqual(counts,fishCatalog.map((_,i)=>fishWeight(i,q)))}
for(let offset=0;offset<=25;offset++){
 let caught=0;const f=createFishing(42+offset,{onCatch:()=>{caught++}});const now=0xfffff000;f.press(now);const s=f.state,bite=(now+s.waitMs)>>>0;
 assert.ok(s.waitMs>=6000&&s.waitMs<=10000);assert.equal(f.press((bite-1)>>>0),0);
 const reward=f.press((bite+(s.target+offset)*10)>>>0);assert.equal(f.state.caught,offset<=18);assert.equal(caught,+(offset<=18));assert.equal(reward,offset<=18?Math.max(2,Math.floor(fishCatalog[f.state.fish].coins/6))+f.state.quality:0);
 f.press((bite+1000)>>>0);assert.equal(f.state.phase,1);f.tick((f.state.started+f.state.waitMs+6000)>>>0);assert.equal(f.state.phase,3);assert(!f.state.caught);
}
const f=createFishing();f.press(100);f.screen(false,200);f.tick(100000);assert.equal(f.state.phase,1);f.screen(true,100000);const s=f.state;assert(f.press(s.started+s.waitMs+s.target*10)>0);
let fail=true;const atomic=createFishing(42,{onCatch:()=>{if(fail)throw Error('quota');return 0}});atomic.press(0);const a=atomic.state,t=a.waitMs+a.target*10;assert.throws(()=>atomic.press(t),/quota/);assert(!atomic.state.caught&&atomic.state.phase===2);fail=false;assert.equal(atomic.press(t),0);assert(atomic.state.caught);
console.log('PASS:27 fish exact odds, precision bonus, rollover/pause, once-only callbacks, storage failure retry and capped coin catch');
