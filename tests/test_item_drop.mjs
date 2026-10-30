import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {itemDrop,itemDropDuration} from '../web/item-drop.mjs';
const rows=execFileSync(fileURLToPath(new URL('./build/item_drop_samples',import.meta.url)),{encoding:'utf8'}).trim().split('\n');
for(const row of rows){const [size,spot,time,x,y,opacity]=row.split(' ').map(Number),p=itemDrop(time,70+spot%6*40,145+Math.floor(spot/6)*22,size);assert.deepEqual(p,{x,y,opacity});}
for(const size of [24,32]){
 const ground=180+32-size;
 assert.equal(itemDrop(0,100,180,size).y,96);
 assert.equal(itemDrop(0,100,180,size).opacity,0);
 assert.equal(itemDrop(400,100,180,size).y,ground);
 assert.equal(itemDrop(540,100,180,size).y,ground-12);
 assert.equal(itemDrop(itemDropDuration,100,180,size).y,ground);
 assert.deepEqual(itemDrop(100000,100,180,size),itemDrop(itemDropDuration,100,180,size));
 let prev=96;
 for(let t=0;t<=400;t+=10){const p=itemDrop(t,100,180,size);assert(p.y>=prev&&p.y<=ground);prev=p.y;}
}
console.log(`PASS: ${rows.length} exact C/Web drop positions/opacity; gravity fall, one 12px bounce, 24px care / 32px egg, bounded completion`);
