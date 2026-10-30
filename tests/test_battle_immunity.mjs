import assert from 'node:assert/strict';
import fs from 'node:fs';
import {createTraining} from '../web/training.mjs';
import {drawBattleRelations,battleRelations} from '../web/battle-hud.mjs';
const manifest=JSON.parse(fs.readFileSync(new URL('../web/manifest.json',import.meta.url)));
const pairs=[[25,50,'电'],[19,92,'一般'],[50,16,'地面'],[56,92,'格斗'],[92,19,'幽灵'],[23,81,'毒'],[147,35,'龙'],[27,16,'地面'],[4,7,'火'],[4,1,'火'],[4,46,'火'],[56,142,'格斗']];
function actor(dex){
 const form=manifest.pets.find(p=>p.nationalId===dex),pets=[{id:String(dex),art:form.id,level:30,ivs:[15,15,15,15,15,15],nature:0,health:100,food:100,clean:100,mood:100,energy:100}];
 const job=createTraining({manifest,getPets:()=>pets,grant:()=>{},seed:123});job.start(0,[pets[0].id]);
 return {job,pet:job.state.party[0]};
}
for(let i=0;i<pairs.length;i++)for(let attacking=0;attacking<2;attacking++){
 const [caster,defender,type]=pairs[i],a=actor(attacking?defender:caster),d=actor(attacking?caster:defender),t=a.job;
 a.pet.hp=a.pet.maxHp=d.pet.hp=d.pet.maxHp=10000;
 Object.assign(t.state,{foes:[d.pet],phase:'fight',ally:0,enemy:0,elapsed:0});
 const form=manifest.pets.find(p=>p.nationalId===caster),move=manifest.skills.find(m=>m.familyId===form.species&&m.type===type&&(m.formPowers.find(p=>p.nationalId===caster)?.power??m.power)>0);
 t.state.round=i===7?1:move.slot;for(let elapsed=0;!t.state.fx&&elapsed<1400;elapsed+=20)t.advance(20);assert(t.state.fx);
 const fx=attacking?t.state.fx.reply:t.state.fx,immune=i<7;
 assert.equal(fx.immune,immune);assert.equal(fx.hit===0,i<8);
 assert.equal((attacking?a.pet:d.pet).hp===10000,i<8);
 // Draw the result of a real resolved turn, rather than fabricating a UI flag.
 for(const [age,visible] of [[0,false],[359,false],[360,true],[600,true],[799,true],[800,false],[900,false]]){
  t.state.fx.at=(attacking*900+age)/2.25;
  const calls=[],ctx={save(){},restore(){},fillText(){throw Error('relation feedback must not render text')},strokeText(){throw Error('no relation text')},fillRect(x,y,w,h){calls.push({x,y,w,h})}};
  const before=t.snapshot(),relations=drawBattleRelations(ctx,manifest,t.state),r=relations[attacking];
  assert.equal(r.kind,i<7?3:i===7||i===11?0:i===8?2:1);
  assert.equal(r.opacity,i===7?0:age<900?(visible?255:200):120);
  assert(calls.every(({x,y})=>x>=144&&x<216&&y>=63&&y<82));
  assert.deepEqual(t.snapshot(),before);
  const gated=battleRelations(manifest,t.state,false)[attacking];assert.equal(gated.opacity,i===7?0:120);

 }
 const snapshot=t.snapshot(),restored=actor(attacking?defender:caster).job;restored.restore(snapshot);
 assert.deepEqual(restored.snapshot(),snapshot);
}
console.log('PASS: 24 actual Web turns, both directional graphic lanes, seven immunities, zero power, resistance/strong/double/cancelled dual-type, impact brightness, entry gating, no text/mutation, save reload');
