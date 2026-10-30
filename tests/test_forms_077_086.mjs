import {readFileSync} from 'node:fs';import {webcrypto} from 'node:crypto';import assert from 'node:assert/strict';
globalThis.crypto??=webcrypto;
const {createEvolutionDemo}=await import('../web/evolution-choice.js');
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url)));const {chains}=JSON.parse(readFileSync(new URL('../assets/evolution.json',import.meta.url)));
let raw=null,fail=false;const storage={getItem(){return raw},setItem(k,v){if(fail)throw Error('quota');raw=v}};let demo=createEvolutionDemo({storage,roster,chains});
for(const [art,next,level]of [[76,77,40],[78,79,37],[80,81,30],[83,84,31]]){
 const p=demo.add(art);assert.equal(demo.offer(p.id).next.id,next);assert.equal(demo.offer(p.id).cost,0);assert.equal(demo.offer(p.id).step.level,level);
 const before=raw;fail=true;assert.throws(()=>demo.choose(p.id,false,0),/quota/);fail=false;assert.equal(raw,before);assert.equal(demo.choose(p.id,false,0).art,next);
 const other=demo.add(art);demo.choose(other.id,true,0);demo=createEvolutionDemo({storage,roster,chains});assert(demo.state.heart);assert.throws(()=>demo.offer(other.id),/永久/);
}
for(const art of [77,79,81,82,84,86])assert.throws(()=>demo.offer(demo.add(art).id),/没有可用/);
console.log('PASS:077-086 four Web gates, atomic persistence failure, zero-coins evolution/refusal, gift/reload, final and087 missing-art guards');
