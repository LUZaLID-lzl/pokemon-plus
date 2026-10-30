import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {lakePatrol,lakeApproach,lakeDrop,lakeInFront} from '../web/sacred-lake.js';
const samples=execFileSync(fileURLToPath(new URL('build/test_sacred_lake',import.meta.url)),['--samples'],{encoding:'utf8',maxBuffer:4*1024*1024}).trim().split('\n');
const blocked=(x,y)=>x>=86&&x<=274&&y>=204&&y<=253;
let patrol=0,approach=0;
for(const row of samples){
 const [kind,...s]=row.split(' ');const a=s.map(Number);
 if(kind==='P'){
  const [t,x,y,left,front]=a,p=lakePatrol(t);
  assert.deepEqual(p,{x,y,left:!!left},`C/Web patrol at ${t}`);
  assert.equal(lakeInFront(y),!!front);assert.ok(!blocked(x+24,y+42));patrol++;
 }else{
  const [x,y,dx,dy,t,px,py]=a;
  assert.deepEqual(lakeApproach(x,y,dx,dy,t),{x:px,y:py},`C/Web approach at ${row}`);
  assert.ok(!blocked(px+24,py+42));approach++;
 }
}
for(let spot=0;spot<24;spot++)for(const egg of [false,true]){
 const p=lakeDrop(spot,egg);assert.ok(!blocked(p.x+16,p.y+(egg?28:24)));
}
assert.equal(lakeInFront(211),false);assert.equal(lakeInFront(212),true);
for(const t of [0xffffffff,0xffffff00]){const a=lakePatrol(t),b=lakePatrol(t>>>0);assert.deepEqual(a,b)}
console.log(`PASS: C/Web ${patrol} patrol positions/depth and ${approach} item routes match; expanded roots avoided, 48 drop/egg positions safe`);
