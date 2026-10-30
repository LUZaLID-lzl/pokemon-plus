import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {createHomeConflict,feistyNature,sampleHomeConflict} from '../web/home-conflict.mjs';
import {createEvolutionDemo} from '../web/evolution-choice.js';
const fixture=()=>Array.from({length:3},(_,i)=>({nature:i?20:3,health:100,mood:80,energy:85,moves:Array.from({length:4},()=>({type:i,power:28}))}));
for(let i=0;i<25;i++)assert.equal(feistyNature(i),[3,4,8,11,24].includes(i));
let c=createHomeConflict(42),pets=fixture(),hits=0;
assert(c.start(pets,1000,0,1,0));
assert.equal(c.tick(pets,1699,true,7), 'none');
assert.equal(c.tick(pets,1700,true,7,(i,h,m)=>{pets[i].health-=h;pets[i].mood-=m;hits++}), 'hit');
assert.equal(pets[1].health,97);assert.equal(pets[1].mood,75);
for(let t=1700;t<2800;t++)c.tick(pets,t,true,7,()=>hits++);
assert.equal(hits,1);assert.equal(c.tick(pets,2800,true,7),'end');
for(const disable of ['sleep','travel','lowEnergy','touch','hidden','missed']){
 pets=fixture();c=createHomeConflict(42);assert(c.start(pets,0,0,1,0));
 if(disable==='sleep')pets[1].sleeping=true;if(disable==='travel')pets[0].traveling=true;if(disable==='lowEnergy')pets[0].energy=29;
 assert.equal(c.tick(pets,disable==='missed'?2000:700,disable!=='hidden',disable==='touch'?5:7),disable==='missed'?'end':'cancel');
 assert.equal(pets[1].health,100);assert.equal(pets[1].mood,80);
}
let checks=0,attacks=0,fxHash=2166136261;
for(let seed=0;seed<32;seed++){
 pets=fixture();c=createHomeConflict(seed,0xffffffff-50000);let previous=0xffffffff-50000;
 for(let i=0;i<64;i++){
  const now=c.state.nextCheck,span=(now-previous)>>>0;assert(span>=180000&&span<=300000);previous=now;
  const e=c.tick(pets,now,true,7);checks++;
  if(e==='begin'){attacks++;assert.equal(c.state.attacker,0);assert.notEqual(c.state.target,0);assert.equal(c.tick(pets,(now+1800)>>>0,true,7),'end')}
 }
}
for(let type=0;type<18;type++)for(let dir=0;dir<2;dir++)for(let t=0;t<1800;t+=33){
 const particles=sampleHomeConflict({active:true,type,started:0},t,dir?296:64,139,dir?64:296,238);
 for(const p of particles)for(const field of ['x','y','w','h','color','opacity'])fxHash=Math.imul(fxHash^p[field],16777619)>>>0;
}
const result=execFileSync(fileURLToPath(new URL('./build/test_home_conflict',import.meta.url)),{encoding:'utf8'}).match(/SAMPLES (\d+) (\d+) (\d+)/);
assert.deepEqual([checks,attacks,fxHash],result.slice(1).map(Number));
const store=new Map([['pokemon-plus.web-companions-v1',JSON.stringify({pets:[{id:'a',art:0,locked:false,nature:3},{id:'b',art:1,locked:false,nature:20}],active:['a','b'],coins:240,heart:false})]]);
let failed=false;const storage={getItem:k=>store.get(k),setItem(k,v){if(failed)throw Error('quota');store.set(k,v)}};
let demo=createEvolutionDemo({storage,roster:[{id:0},{id:1}],chains:[]});
demo.homeHit('b',3,5);assert.equal(demo.state.pets[1].health,97);assert.equal(demo.state.pets[1].mood,75);
const saved=store.get('pokemon-plus.web-companions-v1');failed=true;assert.throws(()=>demo.homeHit('b',3,5));assert.equal(store.get('pokemon-plus.web-companions-v1'),saved);assert.equal(demo.state.pets[1].health,97);
demo=createEvolutionDemo({storage,roster:[{id:0},{id:1}],chains:[]});assert.equal(demo.state.pets[1].health,97);
console.log('PASS: C/Web exact rare scheduling and18 skill particle parity; impact-only damage, visibility/eligibility guards, saved individual stats and quota rollback');
