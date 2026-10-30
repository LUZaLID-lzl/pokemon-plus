import {readFileSync} from 'node:fs';import {webcrypto} from 'node:crypto';import assert from 'node:assert/strict';
globalThis.crypto??=webcrypto;
const {createEvolutionDemo}=await import('../web/evolution-choice.js');
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url)));const {chains}=JSON.parse(readFileSync(new URL('../assets/evolution.json',import.meta.url)));
let raw=null,fail=false;const storage={getItem(){return raw},setItem(k,v){if(fail)throw Error('quota');raw=v}};let demo=createEvolutionDemo({storage,roster,chains});
for(const [art,next,level,cost,item]of [[85,86,34,0,null],[87,88,38,0,null],[89,90,0,200,'水之石'],[91,92,25,0,null],[92,93,0,200,'联系之绳']]){
 const p=demo.add(art);assert.equal(demo.offer(p.id).next.id,next);assert.equal(demo.offer(p.id).cost,cost);assert.equal(demo.offer(p.id).step.level,level);if(item)assert.equal(demo.offer(p.id).step.item,item);
 const before=raw;if(cost){assert.throws(()=>demo.choose(p.id,false,199),/金币不足/);assert.equal(raw,before);}
 fail=true;assert.throws(()=>demo.choose(p.id,false,200),/quota/);fail=false;assert.equal(raw,before);assert.equal(demo.choose(p.id,false,cost).art,next);assert.equal(demo.state.coins,0);
 const other=demo.add(art);demo.choose(other.id,true,0);demo=createEvolutionDemo({storage,roster,chains});assert(demo.state.heart);assert.throws(()=>demo.offer(other.id),/永久/);assert.equal(demo.state.coins,0);
}
for(const art of [86,88,90,93,94,96])assert.throws(()=>demo.offer(demo.add(art).id),/没有可用/);
console.log('PASS:087-096 five Web gates,level/item prices,atomic failure,zero-coins refusal/gift/reload,final and097 guards');
