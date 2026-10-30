import assert from 'node:assert/strict';
import {createHatchPreview} from '../web/hatch-preview.js';
import fs from 'node:fs';
const nests=JSON.parse(fs.readFileSync(new URL('../assets/scene-nests.json',import.meta.url)));
let now=0;const h=createHatchPreview(nests,()=>now);
assert.throws(()=>h.buy(119),/金币/);assert.equal(h.boosts,0);
assert.equal(h.buy(120),0);assert.equal(h.boosts,1);assert.equal(h.egg,null);
assert.throws(()=>h.use(1),/场上/);assert.equal(h.boosts,1);
h.place(1);const egg=h.egg;assert.throws(()=>h.place(1),/已经/);assert.equal(h.egg,egg);
assert.equal(h.notice(),'距离孵化还有\n02:00:00');
now=1001;assert.equal(h.notice(),'距离孵化还有\n01:59:59');
now=7199999;assert.equal(h.notice(),'距离孵化还有\n00:00:01');
now=7200000;assert.equal(h.notice(),'孵化时间已到');
now+=10000;assert.equal(h.remaining(),0);assert.equal(h.egg,egg);
for(const n of nests.scenes){
 assert.deepEqual(h.position(n.scene),{x:n.position[0]+nests.eggOffset[0],y:n.position[1]+nests.eggOffset[1]});
 assert.equal(h.egg,egg);
}
h.use(5);assert.equal(h.egg,null);assert.equal(h.boosts,0);assert.throws(()=>h.use(1),/没有孵化药剂/);
console.log('PASS: Web hatch preview purchases only stock; absent/full guards and existing egg consumed exactly once');
