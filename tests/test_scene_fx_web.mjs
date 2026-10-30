import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import fs from 'node:fs';
import {fileURLToPath} from 'node:url';
import {sampleSceneEffects,sceneParticleParts,sceneParticleSafe,drawSceneEffects} from '../web/scene-effects.js';
const root=new URL('../',import.meta.url),recipes=JSON.parse(fs.readFileSync(new URL('web/scene-effects.json',root)));
assert.deepEqual(recipes.map(({anchors,...recipe})=>recipe),JSON.parse(fs.readFileSync(new URL('assets/scene-effects.json',root))));
const lines=execFileSync(fileURLToPath(new URL('tests/build/scene_fx_samples',root)),{maxBuffer:32*1024*1024,encoding:'utf8'}).trim().split('\n');
const shapes=new Set();
for(const line of lines){
 const row=JSON.parse(line);
 const actual=sampleSceneEffects(recipes,row.scene,row.seed,row.time,!!row.preview).map(p=>{assert(sceneParticleSafe(p));shapes.add(p.shape);return {...p,parts:sceneParticleParts(p)}});
 assert.deepEqual(actual,row.particles,`scene ${row.scene}, seed ${row.seed}, t ${row.time}, preview ${row.preview}`);
}
assert.deepEqual([...shapes].sort((a,b)=>a-b),[0,1,2,3,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22]);
console.log(`PASS: ${lines.length} C/Web samples match exactly, including twenty-two active shapes, desert rolling/cave bat event gates and uint32 rollover`);

// Bitmap slots must use the native frame, with no rectangle placeholder.
for(let frame=0;frame<8;frame++) {
 const draws=[],ctx={globalAlpha:1,fillRect(){assert.fail('A tumbleweed cannot render as a rectangle')}};
 drawSceneEffects(ctx,[{x:80,y:200,w:40,h:40,shape:15+frame,opacity:240,color:'866037'}],(...args)=>draws.push(args));
 assert.deepEqual(draws,[[`effects/tumble-40-${frame}.png`,80,200,40,40]]);
 assert.equal(ctx.globalAlpha,1);
 assert(fs.existsSync(new URL(`sdcard/pokemon-plus/effects/tumble-40-${frame}.png`,root)));
}
console.log('PASS: all eight tumbleweed frames render through native sprite images, with no rectangle fallback');
