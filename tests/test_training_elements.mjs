import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {trainingBattleElements} from '../web/training-scenery.mjs';
const samples=execFileSync(fileURLToPath(new URL('build/training_elements_samples',import.meta.url)),{encoding:'utf8'}).trim().split('\n').map(JSON.parse);
let active=0,rest=0;
for(const [scene,time,expected] of samples){
 const actual=[];trainingBattleElements(scene,time,(_, ...rect)=>actual.push(rect));assert.deepEqual(actual,expected);
 assert.ok(actual.length<=40,'bounded drawing work');
 for(const [x,y,w,h,,alpha] of actual){assert.ok(alpha>=0&&alpha<=240);assert.ok(y>=108&&y+h<=228,'inside travel strip');for(const dx of [x,x+w])for(const dy of [y,y+h])assert.ok((dx-180)**2+(dy-180)**2<=168**2,'round-screen safe')}
 if(actual.length)active++;else rest++;
}
assert.ok(active>0&&rest>active*2,'brief intermittent moments, not permanent overlays');
console.log(`PASS: ${samples.length} C/Web axe/dust/spark samples, intermittent cycle, <=40 rectangles and safe travel-strip bounds`);
