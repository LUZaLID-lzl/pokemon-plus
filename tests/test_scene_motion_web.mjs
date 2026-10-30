import assert from 'node:assert/strict';
import fs from 'node:fs';
import {execFileSync} from 'node:child_process';
const root=new URL('../',import.meta.url);
const {renderSceneMotion}=await import(new URL('web/scene-motion.js',root));
const config=JSON.parse(fs.readFileSync(new URL('web/scene-motion.json',root)));
const base=Uint16Array.from({length:14400},(_,i)=>(i*97+0x20a3)&65535),out=new Uint16Array(14400);
let count=0;
for(let scene=0;scene<config.scenes.length;scene++)for(const t of [0,33,66,99,120,960,2400,4800,9600,59999,4294967295]){
 renderSceneMotion(config.scenes[scene],t,base,out);
 const binary=execFileSync(new URL('tests/build/test_scene_motion',root).pathname,[`${scene}:${t}`]);
 assert.equal(Buffer.compare(binary,Buffer.from(out.buffer)),0,`scene${scene} at${t}`);count++;
}
console.log(`Scenery C/Web: ${count} complete14400px RGB565 frames match: PASS`);
const {renderFishingScenery}=await import(new URL('web/fishing-scenery.js',root));
const original=new Uint16Array(base),seen=new Set();
for(const t of [...Array.from({length:100},(_,i)=>i*33),4294967295]){
 renderFishingScenery(t,base,out);
 const binary=execFileSync(new URL('tests/build/test_scene_motion',root).pathname,[`13:${t}`]);
 assert.equal(Buffer.compare(binary,Buffer.from(out.buffer)),0,`fishing at ${t}`);
 for(let y=0;y<120;y++)for(let x=0;x<120;x++)if(out[y*120+x]!==base[y*120+x]){
  assert((x*2-119)**2+(y*2-119)**2<114*114,'stable circular rim');
  seen.add(y<54?'canopy':y<78?'ripples and fish':'deep fish and leaves');
 }
 assert.deepEqual(base,original,'immutable pond source');
}
assert.equal(seen.size,3);
console.log('PASS: pond canopy/ripple/fading fish/leaves, 101 complete C/Web frames, rollover, immutable source and stable circular rim');
