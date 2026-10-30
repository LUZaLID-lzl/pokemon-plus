import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {groundProject,nestBlocked,routeSample} from '../web/home-navigation.mjs';
import {homeScenePatrol} from '../web/home-behavior.mjs';
const rows=execFileSync(fileURLToPath(new URL('./build/test_home_navigation',import.meta.url)),['--samples'],{encoding:'utf8',maxBuffer:8e6}).trim().split('\n');
let index=0;
for(let scene=0;scene<13;scene++)for(let seed=0;seed<128;seed++){
 const a=groundProject(scene,40+seed*71%233,115+seed*37%100),b=groundProject(scene,40+seed*131%233,115+seed*67%100);
 for(let t=0;t<=6000;t+=240){const p=routeSample(scene,a.x,a.y,b.x,b.y,t,6000);
  assert(!nestBlocked(scene,p.x,p.y));assert.equal([scene,seed,t,p.x,p.y,+p.left].join(' '),rows[index++]);
 }
}
assert.equal(index,rows.length);console.log(`PASS: ${index} exact C/Web obstacle-route samples`);
const patrol=execFileSync(fileURLToPath(new URL('./build/test_home_navigation',import.meta.url)),['--patrol-samples'],{encoding:'utf8',maxBuffer:8e6}).trim().split('\n');
index=0;
for(let scene=0;scene<13;scene++)for(let slot=0;slot<5;slot++)for(let seed=0;seed<4;seed++)for(let t=0;t<24000;t+=330){
 const p=homeScenePatrol(scene,t,slot,!!(seed%2),48+seed*71,115+seed*29,false);
 assert.equal([scene,slot,seed,t,p.x,p.y,+p.left,p.pose].join(' '),patrol[index++]);
}
assert.equal(index,patrol.length);console.log(`PASS: ${index} exact C/Web scene patrol samples`);
