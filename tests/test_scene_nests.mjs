import assert from 'node:assert/strict';
import fs from 'node:fs';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {eggPosition} from '../web/hatch-preview.js';
const config=JSON.parse(fs.readFileSync(new URL('../assets/scene-nests.json',import.meta.url)));
const manifest=JSON.parse(fs.readFileSync(new URL('../web/manifest.json',import.meta.url)));
const rows=execFileSync(fileURLToPath(new URL('build/test_scene_nests',import.meta.url)),['--samples'],{encoding:'utf8'}).trim().split('\n');
assert.equal(rows.length,13);
for(const row of rows){
 const [scene,x,y]=row.split(' ').map(Number);
 assert.deepEqual(eggPosition(manifest.sceneNests,scene),{x,y});
 assert.deepEqual(config.scenes[scene].position,manifest.sceneNests.scenes[scene].position);
}
console.log('PASS: all13 actual C egg anchors match Web and the single scene art config');
