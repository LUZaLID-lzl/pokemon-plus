import {readFileSync} from 'node:fs';import {webcrypto} from 'node:crypto';import assert from 'node:assert/strict';globalThis.crypto??=webcrypto;
const {createEvolutionDemo}=await import('../web/evolution-choice.js');
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url)));const {chains,pendingChains}=JSON.parse(readFileSync(new URL('../assets/evolution.json',import.meta.url)));
let raw=null,fail=false;const storage={getItem(){return raw},setItem(k,v){if(fail)throw Error('quota');raw=v}};let demo=createEvolutionDemo({storage,roster,chains});
for(const [art,next,level,cost]of [[115,116,32,0],[117,118,33,0],[119,120,0,200]]){
 const p=demo.add(art),other=demo.add(art),offer=demo.offer(p.id);assert.equal(offer.next.id,next);assert.equal(offer.cost,cost);assert.equal(offer.step.level??0,level);
 if(cost){const before=raw;assert.throws(()=>demo.choose(p.id,false,199),/金币不足/);assert.equal(raw,before);assert.equal(offer.step.item,'水之石')}
 const before=raw;fail=true;assert.throws(()=>demo.choose(p.id,false,cost),/quota/);fail=false;assert.equal(raw,before);assert.equal(demo.state.pets.find(v=>v.id===p.id).art,art);
 assert.equal(demo.choose(p.id,false,cost).art,next);assert.equal(demo.state.coins,0);assert.equal(demo.offer(other.id).next.id,next);
 demo.choose(other.id,true,0);demo=createEvolutionDemo({storage,roster,chains});assert(demo.state.heart);assert.throws(()=>demo.offer(other.id),/永久/);assert.equal(demo.state.coins,0);const keep=raw;assert.throws(()=>demo.choose(other.id,false,200),/永久/);assert.equal(raw,keep);
}
for(const art of [116,118,120,121,122,123,124,125])assert.throws(()=>demo.offer(demo.add(art).id),/没有可用/);
assert(!pendingChains.some(c=>c.nationalIds.some(d=>d<=126)));
console.log('PASS:117-126 Web three evolution gates,water-stone200,atomic failed write,zero-coin permanent individual refusal/save/gift and five independent forms');
