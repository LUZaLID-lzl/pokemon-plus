import {readFileSync} from 'node:fs';
import {webcrypto} from 'node:crypto';
import assert from 'node:assert/strict';
globalThis.crypto ??= webcrypto;
const {createEvolutionDemo}=await import('../web/evolution-choice.js');
const roster=JSON.parse(readFileSync(new URL('../assets/pet-roster.json',import.meta.url)));
const {chains}=JSON.parse(readFileSync(new URL('../assets/evolution.json',import.meta.url)));
let raw=null,fail=false;const storage={getItem(){return raw},setItem(k,v){if(fail)throw Error('quota');raw=v}};
let demo=createEvolutionDemo({storage,roster,chains});
for(const [art,next,item]of [[55,56,null],[57,58,'火之石'],[59,60,null],[60,61,'水之石'],[62,63,null],[63,64,'联系之绳']]){
 const p=demo.add(art),cost=item?200:0;assert.equal(demo.offer(p.id).next.id,next);assert.equal(demo.offer(p.id).cost,cost);if(item)assert.equal(demo.offer(p.id).step.item,item);
 let before=raw;if(cost){assert.throws(()=>demo.choose(p.id,false,199),/金币不足/);assert.equal(raw,before)}
 fail=true;assert.throws(()=>demo.choose(p.id,false,500),/quota/);fail=false;assert.equal(raw,before);
 assert.equal(demo.choose(p.id,false,500).art,next);assert.equal(demo.state.coins,500-cost);
 const other=demo.add(art);demo.choose(other.id,true,0);demo=createEvolutionDemo({storage,roster,chains});assert.equal(demo.state.heart,true);assert.throws(()=>demo.offer(other.id),/永久/);
}
for(const art of [56,58,61,64,72])assert.throws(()=>demo.offer(demo.add(art).id),/没有可用/);
console.log('PASS: six new Web transitions; fire/water/link costs, atomic storage failure, per-individual permanent refusal/reload, final form guards');
