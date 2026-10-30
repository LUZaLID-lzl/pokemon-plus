import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {advanceEnergy,energyBiscuit} from '../web/pet-energy.mjs';
const lines=execFileSync(fileURLToPath(new URL('./build/test_sleep',import.meta.url)),['--samples'],{encoding:'utf8',maxBuffer:4*1024*1024}).trim().split('\n');
for(const line of lines){
 const [energy,sleeping,effect,first,ticks,e,s]=line.split(' ').map(Number);
 const before={energy,sleeping:!!sleeping,seconds:(first-1)*600,remainder:0};
 const next=advanceEnergy(before,ticks*600,{sun:effect===1,moon:effect===2});
 assert.equal(next.energy,e,line);assert.equal(next.sleeping,!!s,line);
 assert.equal(before.energy,energy);assert.equal(before.sleeping,!!sleeping);
}
let p={energy:10,sleeping:true};p=advanceEnergy(p,7199);assert.equal(p.sleeping,true);p=advanceEnergy(p,1);assert.equal(p.energy,60);assert.equal(p.sleeping,false);
assert.equal(energyBiscuit({energy:34,sleeping:true}).sleeping,true);assert.equal(energyBiscuit({energy:35,sleeping:true}).sleeping,false);assert.equal(energyBiscuit({energy:32,sleeping:true},{temperance:true}).energy,60);assert.throws(()=>advanceEnergy(p,-1));
console.log(`PASS: ${lines.length} C/Web rest states, sleep transitions, fraction persistence and biscuit wake`);
