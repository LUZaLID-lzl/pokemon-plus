import assert from 'node:assert/strict';
import {shop,shopPrize,shopReelSlot,shopReelView,shopReelAdvance} from '../web/shop.mjs';
import {createEvolutionDemo} from '../web/evolution-choice.js';
import fs from 'node:fs';
const manifest=JSON.parse(fs.readFileSync(new URL('../web/manifest.json',import.meta.url)));
const data=new Map();let fail=false;
const storage={getItem:k=>data.get(k),setItem:(k,v)=>{if(fail)throw Error('quota');data.set(k,v)}};
const make=()=>createEvolutionDemo({storage,roster:manifest.pets,chains:manifest.evolution.chains,now:()=>0});
let demo=make();const counts=Array(9).fill(0);
for(let roll=0;roll<100;roll++)counts[shopPrize(roll)]++;
assert.deepEqual(counts,shop.weights);
for(const item of shop.grid.filter(i=>i!==null)){
 const before=demo.state;demo.shopBuy(item,1000);assert.equal(demo.state.coins,1000-manifest.items[item].price);
 assert.equal(item===8?demo.state.treeEssences:demo.state.trainingItems[item],(item===8?before.treeEssences:before.trainingItems[item])+1);
}
for(const item of shop.grid.filter(i=>i!==null)){
 const roll=shop.weights.slice(0,item).reduce((a,b)=>a+b,0),before=demo.state;
 assert.equal(demo.shopDraw(30,()=>roll),item);assert.equal(demo.state.coins,0);
 assert.equal(item===8?demo.state.treeEssences:demo.state.trainingItems[item],(item===8?before.treeEssences:before.trainingItems[item])+1);
 assert.deepEqual(make().state,demo.state);
}
let before=JSON.stringify(demo.state);assert.throws(()=>demo.shopDraw(29),/金币不足/);assert.equal(JSON.stringify(demo.state),before);
assert.throws(()=>demo.shopBuy(2,1000),/下架/);assert.equal(JSON.stringify(demo.state),before);
fail=true;assert.throws(()=>demo.shopDraw(1000,()=>0),/quota/);assert.equal(JSON.stringify(demo.state),before);fail=false;
let saved=structuredClone(demo.state);saved.trainingItems[2]=3;saved.coins=100;storage.setItem('pokemon-plus.web-companions-v1',JSON.stringify(saved));demo=make();assert.equal(demo.state.coins,175);assert.equal(demo.state.trainingItems[2],0);assert.equal(make().state.coins,175);
saved=structuredClone(demo.state);saved.trainingItems[0]=65535;storage.setItem('pokemon-plus.web-companions-v1',JSON.stringify(saved));demo=make();before=JSON.stringify(demo.state);assert.throws(()=>demo.shopDraw(1000,()=>99),/背包已满/);assert.equal(JSON.stringify(demo.state),before);
const trace=fs.readFileSync(new URL('../build/shop-reel-trace.log',import.meta.url),'utf8').split('\n').filter(l=>/^\d+( \d+){6}$/.test(l));
for(const line of trace){const [item,time,delta,next,slot,visible,done]=line.split(' ').map(Number);assert.equal(shopReelAdvance(item,time,delta),next);const view=shopReelView(item,next);assert.equal(view.slot,slot);assert.equal(+view.visible,visible);assert.equal(+view.done,done)}
for(const item of shop.grid.filter(i=>i!==null))for(const cadence of [33,66,132,250,1200]){
 let elapsed=0,view=shopReelView(item,0),steps=0,flashes=0,mask=1,frames=0;
 const target=shop.ring.findIndex(s=>shop.grid[s]===item);
 while(!view.done){
  const next=shopReelAdvance(item,elapsed,cadence),after=shopReelView(item,next);
  if(after.slot!==view.slot){assert.equal(after.slot,shop.ring[(shop.ring.indexOf(view.slot)+1)%8]);steps++}
  if(view.visible&&!after.visible){flashes++;assert.equal(after.slot,shop.ring[target]);assert.equal(steps,24+target)}
  mask|=1<<shop.ring.indexOf(after.slot);view=after;elapsed=next;assert.ok(++frames<2000);
 }
 assert.equal(mask,255);assert.equal(steps,24+target);assert.equal(flashes,3);assert.equal(shop.grid[view.slot],item);assert.ok(view.visible);
}
console.log(`PASS: original shop probability/purchase/save/refund/rollback checks; ${trace.length} C/Web boundary/visibility/done/advance samples; every adjacent cell and three final flashes at five slow render cadences`);
