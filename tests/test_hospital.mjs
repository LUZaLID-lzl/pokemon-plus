import assert from 'node:assert/strict';
import {webcrypto} from 'node:crypto';
import {readFileSync} from 'node:fs';
import {createHospital,hospitalFall,hospitalSlot,hospitalRounds} from '../web/hospital.mjs';
import {createEvolutionDemo} from '../web/evolution-choice.js';
globalThis.crypto??=webcrypto;
function win(h){h.start();while(h.state.phase===1){h.select(h.cloud()?(h.state.lane+1)%3:h.state.lane);h.advance(hospitalSlot)}assert.equal(h.state.phase,2)}
const h=createHospital(42);assert.throws(()=>h.claim(()=>{}));h.start();h.select(9);assert.equal(h.state.basket,1);h.select(h.state.lane);h.advance(hospitalFall-1);assert.equal(h.state.score,0);h.advance(1);assert.equal(h.state.score,1);h.advance(hospitalSlot-hospitalFall-1);assert.equal(h.state.score,1);h.advance(1);assert.equal(h.state.slot,1);assert.throws(()=>h.advance(-1));assert.throws(()=>h.advance(NaN));
h.start();while(h.state.phase===1){h.select(h.cloud()?h.state.lane:(h.state.lane+1)%3);h.advance(hospitalSlot);assert.equal(h.state.score,0)}assert.equal(h.state.phase,3);h.start();h.advance(2**32);assert.equal(h.state.elapsed,hospitalRounds*hospitalSlot);assert.notEqual(h.state.phase,1);
const manifest=JSON.parse(readFileSync(new URL('../web/manifest.json',import.meta.url)));
let raw=null,fail=false,writes=0;const storage={getItem:()=>raw,setItem:(k,v)=>{if(fail)throw Error('quota');raw=v;writes++}};
const open=()=>createEvolutionDemo({storage,roster:manifest.pets,chains:manifest.evolution.chains});let demo=open();for(let i=0;i<7;i++)demo.add(i);
let data=JSON.parse(raw);for(const p of data.pets){for(const key of ['food','clean','health','mood','energy'])p[key]=10;p.sleeping=true}data.pets[0].food=99;data.traveler=data.active[1];raw=JSON.stringify(data);demo=open();
const original=structuredClone(demo.state),active=demo.state.active,seen=new Set();win(h);let saved=raw,rng=h.state.rng;fail=true;assert.throws(()=>h.claim(t=>demo.hospitalRecover(t)),/quota/);assert.equal(raw,saved);assert.equal(h.state.rng,rng);assert(!h.state.claimed);assert.deepEqual(demo.state,original);fail=false;
const beforeWrites=writes,treated=h.claim(t=>demo.hospitalRecover(t));assert.equal(treated.length,4);assert.equal(writes,beforeWrites+1);assert.throws(()=>h.claim(()=>{}));assert.equal(writes,beforeWrites+1);
for(const p of demo.state.pets){const old=original.pets.find(x=>x.id===p.id);if(!active.includes(p.id)||p.id===data.traveler){assert.deepEqual(p,old);continue}assert.equal(p.sleeping,old.sleeping);for(const key of ['food','clean','health','mood','energy']){if(p.id===active[0]&&key==='food'){assert.equal(p[key],99);continue}assert.equal(p[key],key==='health'?60:old[key]);seen.add(p[key])}assert.equal(p.xp,old.xp);assert.equal(p.level,old.level)}assert.equal(demo.state.coins,original.coins);assert.deepEqual(open().state,demo.state);assert(seen.has(60));
for(let seed=1;seed<=100;seed++){const round=createHospital(seed);win(round);round.claim(t=>{t.flat().forEach(n=>{assert(n===0||n===60);seen.add(n)})})}assert(seen.has(60));
console.log('PASS: hospital engine timing, cloud/miss/win, random recovery, no downgrade, warehouse/travel exclusion, atomic storage rollback/retry, duplicate prevention/reload');

// Compare every transition and recovery draw with the compiled C implementation.
const {execFileSync}=await import('node:child_process');
const expected=execFileSync(new URL('./build/test_hospital',import.meta.url).pathname,['trace'],{encoding:'utf8'}).trim().split('\n').map(JSON.parse),actual=[];
for(let seed=1;seed<=25;seed++){
 const h=createHospital(seed);h.start();const steps=[0,17,33,1001,511,200,400,1300];
 for(let n=0;n<100&&h.state.phase===1;n++){h.select((n+seed)%3);h.advance(steps[n%8]);const s=h.state;actual.push([seed,s.elapsed,s.phase,s.score,s.slot,s.lane,s.basket,s.rng,+s.resolved])}
 win(h);let restored;h.claim(targets=>{restored=targets.map((row,i)=>i===1?[0,0,0,0,0]:row)});actual.push([seed,h.state.rng,...restored.flat()]);
}
assert.deepEqual(actual,expected);console.log('PASS: C/Web frame transition and random recovery parity across 25 seeds, irregular timing and skipped traveling slot');
