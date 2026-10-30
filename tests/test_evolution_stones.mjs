import {readFileSync} from 'node:fs';
import {webcrypto} from 'node:crypto';
import assert from 'node:assert/strict';
globalThis.crypto ??= webcrypto;
const {createEvolutionDemo}=await import('../web/evolution-choice.js');
const plan=JSON.parse(readFileSync(new URL('../assets/pokemon/plan-037-046.json',import.meta.url)));
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url)));
for(const p of plan.items)roster[p.artId]={...p,id:p.artId};
const types={'火之石':'fire-stone','月之石':'moon-stone','叶之石':'leaf-stone'};
const chains=plan.evolution.map(c=>({...c,steps:c.steps.map(s=>({...s,trigger:s.item?types[s.item]:'level'}))}));
let raw=null,fail=false;
const storage={getItem(){return raw},setItem(k,v){if(fail)throw Error('quota');raw=v}};
let demo=createEvolutionDemo({storage,roster,chains});
for(const [art,name]of [[36,'火之石'],[38,'月之石'],[43,'叶之石']]){
 const p=demo.add(art);assert.equal(demo.offer(p.id).step.item,name);assert.equal(demo.offer(p.id).cost,200);
 const before=raw;assert.throws(()=>demo.choose(p.id,false,199),/金币不足/);assert.equal(raw,before);
 fail=true;assert.throws(()=>demo.choose(p.id,false,200),/quota/);fail=false;assert.equal(raw,before);
 assert.equal(demo.choose(p.id,false,200).art,art+1);assert.equal(demo.state.coins,0);
 assert.throws(()=>demo.offer(p.id),/没有可用/);
 const other=demo.add(art);demo.choose(other.id,true,0);demo=createEvolutionDemo({storage,roster,chains});
 assert.equal(demo.state.heart,true);assert.throws(()=>demo.offer(other.id),/永久/);
}
const plant=demo.add(42);assert.equal(demo.offer(plant.id).cost,0);
demo.choose(plant.id,false,500);assert.equal(demo.offer(plant.id).step.item,'叶之石');
demo.choose(plant.id,false,500);assert.equal(demo.state.coins,300);
assert.throws(()=>demo.offer(demo.add(65).id),/没有可用/);
console.log('PASS: fire/moon/leaf stage-specific stone types/prices, atomic payment and failed-storage rollback, level-to-leaf progression, per-pet refusal/reload and Machop future-form guard');
