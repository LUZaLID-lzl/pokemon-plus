import {readFileSync} from 'node:fs';import {webcrypto} from 'node:crypto';import assert from 'node:assert/strict';globalThis.crypto??=webcrypto;
const {createEvolutionDemo}=await import('../web/evolution-choice.js');
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url)));
const {chains,pendingChains}=JSON.parse(readFileSync(new URL('../assets/evolution.json',import.meta.url)));
let raw=null;const storage={getItem(){return raw},setItem(k,v){raw=v}};
let demo=createEvolutionDemo({storage,roster,chains});
for(const dex of [127,128,129]){
 const form=roster.find(p=>p.nationalId===dex);assert(form && form.stage===0);
 const p=demo.add(form.id),before=raw;
 if(dex===129 && roster.some(p=>p.nationalId===130)){
  const other=demo.add(form.id),offer=demo.offer(p.id);assert.equal(offer.next.nationalId,130);assert.equal(offer.step.level,20);assert.equal(offer.cost,0);
  assert.equal(demo.choose(p.id,false,0).art,129);demo.choose(other.id,true,0);
  demo=createEvolutionDemo({storage,roster,chains});assert.throws(()=>demo.offer(other.id),/永久/);assert(demo.state.heart);
  assert.throws(()=>demo.offer(p.id),/没有可用/);continue;
 }
 assert.throws(()=>demo.offer(p.id),/没有可用/);
 assert.throws(()=>demo.choose(p.id,false),/没有可用/);
 assert.throws(()=>demo.choose(p.id,true),/没有可用/);
 assert.equal(raw,before);assert.equal(demo.state.heart,false);
 demo=createEvolutionDemo({storage,roster,chains});assert(demo.state.pets.some(x=>x.id===p.id && x.art===form.id));
}
if(!roster.some(p=>p.nationalId===130))assert(pendingChains.some(c=>c.level===20 && c.nationalIds.join(',')==='129,130'));
else assert(!pendingChains.some(c=>c.nationalIds.includes(130)));
console.log('PASS:127+ Web save/reload/final guards;130 level and permanent individual refusal/unique gift when integrated');
