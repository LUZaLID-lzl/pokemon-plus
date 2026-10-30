import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {trainingReward,balance} from '../web/balance-rules.mjs';
assert.equal(balance.training.xpMultiplier,2);
const cases=[
 [20,20,50,50,0],[20,20,90,90,0],[20,20,30,30,0],
 [20,9,50,50,0],[20,11,50,50,0],[20,60,50,50,0],
 [59,60,50,50,1179],[60,60,50,50,0],[1,1,50,50,0]
],rows=[];
for(const [c,[level,enemy,food,mood,xp]] of cases.entries())for(let count=1;count<=3;count++)for(let outcome=0;outcome<=count+1;outcome++){
 const win=outcome===count+1,defeated=win?count:outcome,party=[];let total=0;
 for(let p=0;p<count;p++){
  const old={level,xp,food,mood,health:90,clean:90,energy:100,xpFraction:350000+p*100000};
  const reward=trainingReward(old,level,enemy,defeated,count,win,!win);let after=level,progress=xp+reward.xp;
  while(after<60&&progress>=20*after){progress-=20*after;after++}
  party.push([after,after===60?0:progress,reward.changes.xpFraction,reward.changes.energy,reward.changes.health]);total+=reward.xp;
 }
 rows.push([c,count,Number(win),defeated,total,party]);
}
const native=execFileSync(fileURLToPath(new URL('./build/test_training_xp',import.meta.url)),['--trace'],{encoding:'utf8'}).trim().split('\n').map(JSON.parse);
assert.deepEqual(rows,native);
console.log(`PASS: ${rows.length} training XP cases exactly match native claims, including post-level-up fractional cap and previously saved carry`);
