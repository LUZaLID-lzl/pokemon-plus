import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {talkCatalog,talkKinds,itemTalk,talkContext,pickTalk} from '../web/pet-dialogue.mjs';
const all=talkCatalog.flatMap(t=>t.lines);
assert.equal(all.length,111);assert.equal(new Set(all).size,111);
assert(all.every(line=>[...line].length<=14));
assert.deepEqual([0,1,2,3,4,8].map(i=>talkCatalog[itemTalk(i)].icon),[0,1,2,3,14,8]);
for(let scene=0;scene<13;scene++){
 assert.equal(talkContext(scene,30,30,30,30,30,0),talkKinds.scene0+scene);
 assert.equal(talkContext(scene,80,80,80,80,80,2),talkKinds.idle);
}
assert.equal(talkContext(0,10,10,10,10,10,0),talkKinds.unwell);
const expected=execFileSync(new URL('./build/test_pet_dialogue',import.meta.url).pathname,['trace'],{encoding:'utf8'}).trim().split('\n');
const actual=[];
for(let pet=0;pet<5;pet++){
 const cursors=[];
 for(let topic=0;topic<talkCatalog.length;topic++){
  let previous='';
  for(let n=0;n<1000;n++){
   const line=pickTalk(cursors,topic,pet*7);assert.notEqual(previous,line.text);previous=line.text;
   if(n<12)actual.push(`${pet}\t${topic}\t${line.icon}\t${line.text}`);
  }
 }
}
assert.deepEqual(actual,expected);
console.log('PASS: 111 unique pet lines; care icons, scene/need contexts; 1680 C/Web speech samples and 140000 non-repeating selections');
