import {readFileSync} from 'node:fs';
import {webcrypto} from 'node:crypto';
import assert from 'node:assert/strict';
globalThis.crypto ??= webcrypto;
const {createEvolutionDemo,evolutionStep}=await import('../web/evolution-choice.js');
const plan=JSON.parse(readFileSync(new URL('../assets/pokemon/plan-028-036.json',import.meta.url)));
// Test mixed level/stone stages independently of whether the image batch is published.
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url)));
for(const p of plan.items)roster[p.artId]={...p,id:p.artId};
const chains=plan.evolution.map(c=>({...c,steps:c.steps.map(s=>({...s,trigger:s.item?'moon-stone':'level'}))}));
let raw=null,fail=false;
const storage={getItem(){return raw},setItem(k,v){if(fail)throw Error('quota');raw=v}};
let demo=createEvolutionDemo({storage,roster,chains});
for(const art of [28,31]) {
  const pet=demo.add(art);
  assert.equal(demo.offer(pet.id).cost,0);
  assert.equal(demo.choose(pet.id,false,500).art,art+1);
  assert.equal(demo.state.coins,500);
  assert.equal(demo.offer(pet.id).step.item,'月之石');
  assert.equal(demo.offer(pet.id).cost,200);
  const before=raw;
  assert.throws(()=>demo.choose(pet.id,false,199),/金币不足/);
  assert.equal(raw,before);
  fail=true;assert.throws(()=>demo.choose(pet.id,false,500),/quota/);fail=false;
  assert.equal(raw,before);
  assert.equal(demo.choose(pet.id,false,500).art,art+2);
  assert.equal(demo.state.coins,300);
  assert.throws(()=>demo.offer(pet.id),/没有可用/);
}
for(const art of [29,32,34]) {
  const pet=demo.add(art);
  assert.equal(demo.offer(pet.id).cost,200);
  assert.equal(demo.choose(pet.id,true,0).art,art);
  assert.equal(demo.state.coins,0);assert.equal(demo.state.heart,true);
  demo=createEvolutionDemo({storage,roster,chains});
  assert.throws(()=>demo.offer(pet.id),/永久/);
  const other=demo.add(art);
  assert.equal(demo.choose(other.id,false,500).art,art+1);
}
assert.equal(evolutionStep({trigger:'thunder-stone',price:200},0).item,'雷之石');
console.log('PASS: mixed level/moon-stone stages, per-stage price, all moon branches, final guards, refusal/reload/individual isolation and storage-failure rollback');
