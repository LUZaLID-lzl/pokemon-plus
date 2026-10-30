import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {battleEntryState,battleEntryPaint,battleEntryDuration} from '../web/battle-entrance.mjs';
const samples=execFileSync(fileURLToPath(new URL('./build/battle_entrance_samples',import.meta.url)),{encoding:'utf8'}).trim().split('\n').map(JSON.parse);
let maximum=0;
for(const [side,time,opacity,white,drop,pixels] of samples){
 const state=battleEntryState(time),render=[];battleEntryPaint(side,time,(_,x,y,w,h,c,a)=>render.push([x,y,w,h,c,a]));
 assert.deepEqual(state,{opacity,white,drop});assert.deepEqual(render,pixels);
 assert.ok(opacity>=0&&opacity<=255&&white>=0&&white<=255&&drop>=0&&drop<=6);
 if(time<360)assert.equal(opacity,0);if(time>=battleEntryDuration){assert.deepEqual(state,{opacity:255,white:0,drop:0});assert.equal(render.length,0)}
 maximum=Math.max(maximum,pixels.length*2);assert.ok(pixels.length*2<=48,'both releases fit the existing fixed FX pool');
 for(const [x,y,w,h,,a] of pixels){assert.ok(w>0&&h>0&&a>=0&&a<=255);for(const px of [x,x+w-1])for(const py of [y+64,y+h+63])assert.ok((px-180)**2+(py-180)**2<=168**2,'release stays inside circular safe area')}
}
console.log(`PASS: ${samples.length} exact native/Web release samples, hidden→white reveal→full color, 1-second finish, circular bounds and ${maximum}/48 maximum simultaneous FX`);
