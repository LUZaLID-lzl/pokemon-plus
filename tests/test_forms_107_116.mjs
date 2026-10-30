import {readFileSync} from 'node:fs';import {webcrypto} from 'node:crypto';import assert from 'node:assert/strict';
globalThis.crypto??=webcrypto;
const {createEvolutionDemo}=await import('../web/evolution-choice.js');
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url)));const {chains,pendingChains}=JSON.parse(readFileSync(new URL('../assets/evolution.json',import.meta.url)));
let raw=null,fail=false;const storage={getItem(){return raw},setItem(k,v){if(fail)throw Error('quota');raw=v}};let demo=createEvolutionDemo({storage,roster,chains});
for(const [art,next,level]of [[108,109,35],[110,111,40]]){
 const p=demo.add(art),other=demo.add(art),offer=demo.offer(p.id);assert.equal(offer.next.id,next);assert.equal(offer.cost,0);assert.equal(offer.step.level,level);
 const before=raw;fail=true;assert.throws(()=>demo.choose(p.id,false,0),/quota/);fail=false;assert.equal(raw,before);assert.equal(demo.state.pets.find(v=>v.id===p.id).art,art);
 assert.equal(demo.choose(p.id,false,0).art,next);assert.equal(demo.state.coins,0);assert.equal(demo.offer(other.id).next.id,next);
 demo.choose(other.id,true,0);demo=createEvolutionDemo({storage,roster,chains});assert(demo.state.heart);assert.throws(()=>demo.offer(other.id),/永久/);assert.equal(demo.state.coins,0);
 const retained=raw;assert.throws(()=>demo.choose(other.id,false,0),/永久/);assert.equal(raw,retained);
}
for(const art of [106,107,109,111,112,113,114])assert.throws(()=>demo.offer(demo.add(art).id),/没有可用/);
if(!roster.some(p=>p.nationalId===117)){assert.equal(pendingChains.length,1);assert.deepEqual(pendingChains[0].nationalIds,[116,117]);}else assert.equal(demo.offer(demo.add(115).id).next.nationalId,117);
console.log('PASS:107-116 two Web level rules,atomic write failure,zero-coin permanent refusal/gift/reload/isolation,six independent forms and117 missing-resource guard');
