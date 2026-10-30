import {readFileSync} from 'node:fs';import {webcrypto} from 'node:crypto';import assert from 'node:assert/strict';
globalThis.crypto??=webcrypto;
const {createEvolutionDemo}=await import('../web/evolution-choice.js');
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url)));const {chains}=JSON.parse(readFileSync(new URL('../assets/evolution.json',import.meta.url)));
let raw=null,fail=false;const storage={getItem(){return raw},setItem(k,v){if(fail)throw Error('quota');raw=v}};let demo=createEvolutionDemo({storage,roster,chains});
for(const [art,next,level,cost,item]of [[95,96,26,0,null],[97,98,28,0,null],[99,100,30,0,null],[101,102,0,200,'叶之石'],[103,104,28,0,null]]){
 const p=demo.add(art),other=demo.add(art);const offer=demo.offer(p.id);assert.equal(offer.next.id,next);assert.equal(offer.cost,cost);assert.equal(offer.step.level,level);if(item)assert.equal(offer.step.item,item);
 const before=raw;if(cost){assert.throws(()=>demo.choose(p.id,false,199),/金币不足/);assert.equal(raw,before);}
 fail=true;assert.throws(()=>demo.choose(p.id,false,200),/quota/);fail=false;assert.equal(raw,before);assert.equal(demo.state.pets.find(v=>v.id===p.id).art,art);
 assert.equal(demo.choose(p.id,false,cost).art,next);assert.equal(demo.state.coins,0);assert.equal(demo.offer(other.id).next.id,next);
 demo.choose(other.id,true,0);demo=createEvolutionDemo({storage,roster,chains});assert(demo.state.heart);assert.throws(()=>demo.offer(other.id),/永久/);assert.equal(demo.state.coins,0);assert.equal(demo.state.pets.filter(v=>v.id===other.id).length,1);
 const retained=raw;assert.throws(()=>demo.choose(other.id,false,200),/永久/);assert.equal(raw,retained);
}
for(const art of [96,98,100,102,104,105])assert.throws(()=>demo.offer(demo.add(art).id),/没有可用/);
console.log('PASS:097-106 five Web evolution gates,atomic writes/199-200 coins,individual permanent refusal/unique gift/reload and six final forms');
