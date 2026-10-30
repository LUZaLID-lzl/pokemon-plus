import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {eggGlow} from '../web/egg-glow.mjs';
const rows=execFileSync(fileURLToPath(new URL('build/test_egg_glow',import.meta.url)),['--samples'],{encoding:'utf8'}).trim().split('\n');
for(const row of rows){
 const [rare,time,...pixels]=row.split(' ');
 const actual=eggGlow(Number(time),!!Number(rare)).map(d=>[d.x,d.y,d.size,d.opacity,d.color].join(','));
 assert.deepEqual(actual,pixels);
}
console.log('PASS: Web egg dots match firmware at24 ordinary/legendary samples, including phase boundaries and clock rollover');
