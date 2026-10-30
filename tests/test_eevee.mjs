import {readFileSync} from 'node:fs';import {webcrypto} from 'node:crypto';import assert from 'node:assert/strict';globalThis.crypto??=webcrypto;
const {createEvolutionDemo}=await import('../web/evolution-choice.js');
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url))),{chains}=JSON.parse(readFileSync(new URL('../assets/evolution.json',import.meta.url)));
let raw=null,fail=false;const storage={getItem(){return raw},setItem(k,v){if(fail)throw Error('write failed');raw=v}};const open=()=>createEvolutionDemo({storage,roster,chains});let demo=open();
for(const dex of [134,135,136]){
 const pet=demo.add(132);assert.equal(demo.choices(pet.id).length,3);let before=raw;
 assert.throws(()=>demo.choose(pet.id,false,200),/分支/);assert.throws(()=>demo.choose(pet.id,false,200,137),/分支/);assert.throws(()=>demo.choose(pet.id,false,199,dex),/金币/);assert.equal(raw,before);
 fail=true;assert.throws(()=>demo.choose(pet.id,false,200,dex),/write failed/);assert.equal(raw,before);assert.equal(demo.state.pets.find(p=>p.id===pet.id).art,132);fail=false;
 assert.equal(demo.offer(pet.id,dex).cost,200);assert.equal(demo.choose(pet.id,false,200,dex).art,dex-1);assert.equal(demo.state.coins,0);
 demo=open();assert.throws(()=>demo.choices(pet.id),/没有可用/);assert.equal(demo.state.pets.find(p=>p.id===pet.id).art,dex-1);
}
const a=demo.add(132),b=demo.add(132);demo.choose(a.id,true,0);demo=open();assert(demo.state.heart);for(const dex of [134,135,136])assert.throws(()=>demo.choose(a.id,false,200,dex),/永久/);assert.equal(demo.choices(b.id).length,3);demo.choose(b.id,true,0);assert.equal(demo.state.coins,0);
console.log('PASS:Web explicit exclusive Eevee branches/atomic coins/storage-failure rollback/final reload/per-individual refusal/unique heart');
