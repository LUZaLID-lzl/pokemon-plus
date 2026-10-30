import assert from 'node:assert/strict';
import fs from 'node:fs';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
import {createTraining,trainingScenes} from '../web/training.mjs';
import {createEvolutionDemo} from '../web/evolution-choice.js';
const manifest=JSON.parse(fs.readFileSync(new URL('../web/manifest.json',import.meta.url)));
// Approach and warning preserve the visible foe without awarding XP or drawing RNG.
{
 const pets=[{id:'alert-test',art:0,level:12,health:90,food:90,clean:90,mood:90,energy:100}],grants=[];
 const t=createTraining({manifest,getPets:()=>pets,grant:(...args)=>grants.push(args),seed:77});
 t.start(0,['alert-test']);while(t.state.phase==='search')t.advance(250);
 assert.equal(t.state.phase,'approach');const seen=t.state.foes[0].art,preparedRng=t.rng;
 for(let i=0;i<7;i++)t.advance(250);t.advance(49);assert.equal(t.state.phase,'approach');assert.equal(t.state.approachTime,1799);
 t.advance(1);assert.equal(t.state.phase,'alert');assert.equal(t.state.foes[0].art,seen);assert.equal(t.rng,preparedRng);
 assert.equal(t.state.phase,'alert');assert.equal(t.state.alertTime,0);const rng=t.rng;
 for(let i=0;i<7;i++)t.advance(250);t.advance(249);
 assert.equal(t.state.phase,'alert');assert.equal(t.state.alertTime,1999);assert.equal(t.rng,rng);assert.equal(t.state.foes.length,1);assert.equal(t.claim(),false);assert.equal(grants.length,0);
 t.advance(1);assert.equal(t.state.phase,'fight');assert.equal(t.state.foes.length,1);
 for(let i=0;i<5;i++)t.advance(250);t.advance(83);assert.equal(t.state.round,0);t.advance(1);assert.equal(t.state.round,1);
 t.close();t.start(0,['alert-test']);while(['search','approach'].includes(t.state.phase))t.advance(250);t.close();t.advance(250);assert.equal(t.state.phase,'select');assert.equal(grants.length,0);
}
const trace=[];
for(let scene=0;scene<6;scene++)for(let count=1;count<=3;count++){
 let pets=[1,2,3].map((species,i)=>({id:String(i),art:manifest.pets.find(p=>p.species===species&&p.stage===0).id,level:12,ivs:[15,15,15,15,15,15],nature:0,health:90,food:90,clean:90,mood:90,energy:100})),grants=[];
 const t=createTraining({manifest,getPets:()=>pets,grant:(ids,xp)=>grants.push({ids,xp}),seed:123});t.start(scene,pets.slice(0,count).map(p=>p.id));
 for(let tick=0;tick<4000;tick++){t.advance(250);const h=t.state;if(h.phase==='result'){t.claim();const a=h.party[h.ally],d=h.foes[h.enemy];trace.push([scene,count,tick,t.rng,h.foes[0].art,h.foes[0].level,h.ally,h.enemy,h.round,a.hp,d.hp,h.win?1:2,h.xp]);assert.equal(grants.length,count);const fxAge=h.fx?.at;t.advance(250);if(fxAge!==undefined)assert.equal(h.fx.at,fxAge+250,'final impact keeps advancing after settlement');t.claim();assert.equal(grants.length,count);assert.equal(grants[0].ids.length,1);t.resume();assert.equal(h.phase,'search');break}}
 t.close();assert.equal(t.state.phase,'select');
}
assert.equal(trace.length,18);
const native=execFileSync(fileURLToPath(new URL('./build/training_trace',import.meta.url)),{encoding:'utf8'}).trim().split('\n').map(x=>JSON.parse(x));
assert.deepEqual(trace,native);
const data=new Map();let fail=false;const storage={getItem:k=>data.get(k)||null,setItem(k,v){if(fail)throw Error('disk full');data.set(k,v)}};
const demo=createEvolutionDemo({storage,roster:manifest.pets,chains:manifest.evolution.chains});const ids=[demo.add(1,12).id,demo.add(2,12).id,demo.add(3,12).id];demo.setActive([ids[0]]);
const before=demo.state;fail=true;assert.throws(()=>demo.trainingReward(ids,100));assert.equal(demo.state,before);fail=false;demo.trainingReward(ids,100);for(const id of ids)assert.equal(demo.state.pets.find(p=>p.id===id).xp,100);assert.deepEqual(demo.state.active,[ids[0]]);
const restored=createEvolutionDemo({storage,roster:manifest.pets,chains:manifest.evolution.chains});assert.equal(restored.state.pets[2].xp,100);assert.throws(()=>demo.trainingReward([ids[0],ids[0]],20));
console.log('PASS: all 18 Web encounters match C exactly (RNG/roster/levels/turns/HP/outcome/XP), 1-3 partners, single grant, resume/exit, mixed roster persistence and atomic write failure');
