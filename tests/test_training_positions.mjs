import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {trainingPosition} from '../web/training-scenery.mjs';
const samples=execFileSync(fileURLToPath(new URL('build/training_position_samples',import.meta.url)),{encoding:'utf8',maxBuffer:10*1024*1024}).trim().split('\n').map(JSON.parse);
const previous=new Map(),bounds=new Map();
for(const [s,n,t,i,x,y,left] of samples){
 assert.deepEqual(trainingPosition(s,i,n,t),{x,y,left:!!left});
 const key=[s,n,i].join(','),last=previous.get(key),b=bounds.get(key)||{min:x,max:x,left:false,right:false};
 if(last){assert.ok(Math.abs(x-last.x)+Math.abs(y-last.y)<=3,'continuous walking');if(left===last.left)assert.ok(left?x<=last.x:x>=last.x,'faces movement direction')}
 previous.set(key,{x,y,left});b.min=Math.min(b.min,x);b.max=Math.max(b.max,x);b[left?'left':'right']=true;bounds.set(key,b);
 for(const dx of [x+2,x+45])for(const dy of [y+45,y+60])assert.ok((dx-180)**2+(dy-180)**2<=168**2,'underfoot label inside circular safe area');
 assert.ok(y-24>=84,'warning below heading');
 if(i){const p=trainingPosition(s,i-1,n,t);assert.ok(Math.abs(x-p.x)>=44||Math.abs(y-p.y)>=16,'no overlapping level labels')}
}
for(const b of bounds.values()){assert.equal(b.max,b.min,'party stays in travel formation');assert.ok(!b.left&&b.right)}
console.log(`PASS: ${samples.length} exact C/Web positions, fixed horizontal formation for all six scenes and 1–3 partners, right-facing walking, safe underfoot labels/warnings and no label overlaps`);
