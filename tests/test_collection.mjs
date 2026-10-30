import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {collectionItems,createCollectionDemo,ownsAchievement} from '../web/collection.js';
const catalog=JSON.parse(readFileSync(new URL('../assets/collection-catalog.json',import.meta.url)));
const quotas=[350,146,58,23,6,3];
for(let q=0;q<6;q++){const list=collectionItems(catalog,q);assert.equal(list.length,quotas[q]);assert(list.every(x=>x.quality===q))}
assert.equal(collectionItems(catalog)[0].id,583);
assert.deepEqual(collectionItems(catalog,5).map(x=>x.id),[583,584,585]);
assert.equal(new Set(collectionItems(catalog).map(x=>x.id)).size,586);
assert(!ownsAchievement(584,{badges:127}));assert(ownsAchievement(584,{badges:255}));
assert(!ownsAchievement(584)); // A displayed preview never grants a reward.
const roster=Array.from({length:151},(_,id)=>({id}));
const forms=new Set(roster.map(p=>p.id));
assert(ownsAchievement(585,{roster,ownedForms:forms}));forms.delete(150);
assert(!ownsAchievement(585,{roster,ownedForms:forms}));
assert(!ownsAchievement(585,{roster:[],ownedForms:new Set()}));
const storage={value:'[1,1,-1,583,999,"2"]',writes:0,getItem(){return this.value},setItem(k,v){this.writes++;this.value=v}};
let demo=createCollectionDemo({storage,random:()=>0});assert.deepEqual(demo.ids,[1]);
for(let i=0;i<582;i++){
 const before=new Set(demo.ids),result=demo.claim();assert(!before.has(result.id));assert.equal(demo.ids.length,before.size+1);assert.equal(result.complete,i===581);
}
assert.equal(demo.ids.length,583);assert(!demo.has(583));
const writes=storage.writes;demo=createCollectionDemo({storage,random:()=>{throw Error('empty pool must not use RNG')}});
assert.equal(demo.ids.length,583);assert.deepEqual(demo.claim(),{id:null,complete:true});assert.equal(storage.writes,writes);
for(const missing of [0,42,582]){
 const near={getItem(){return JSON.stringify(Array.from({length:583},(_,i)=>i).filter(i=>i!==missing))},setItem(){}};
 demo=createCollectionDemo({storage:near,random:()=>4294967295});assert.deepEqual(demo.claim(),{id:missing,complete:true});
}
const fail={getItem(){return null},setItem(){throw Error('quota')}};demo=createCollectionDemo({storage:fail,random:()=>42});
assert.throws(()=>demo.claim(),/quota/);assert.equal(demo.ids.length,0);
let calls=0;demo=createCollectionDemo({storage:{getItem(){return null},setItem(){}},random:()=>++calls===1?4294967295:7});assert.equal(demo.claim().id,7);assert.equal(calls,2);
console.log('PASS: browser missing-only collection, last item, completed pool no draw/write, special exclusion, reload, rejection and storage rollback');
