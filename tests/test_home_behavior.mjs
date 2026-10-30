import assert from 'node:assert/strict';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
import {createHomeBehavior,homePatrol} from '../web/home-behavior.mjs';
import {groundProject} from '../web/home-navigation.mjs';
const native=execFileSync(fileURLToPath(new URL('./build/home_behavior_samples',import.meta.url)),{maxBuffer:24*1024*1024,encoding:'utf8'}).trim().split('\n');
let index=0;
for(let scene=0;scene<13;scene++)for(let kind=0;kind<=8;kind++){
 const s=createHomeBehavior(42,0,scene),input=[[80,145,false],[170,165,true],[225,195,true],[110,200,false],[205,120,false]].map(([x,y,left])=>({x,y,left,mood:80,energy:80,available:true}));
 input.forEach(p=>Object.assign(p,groundProject(scene,p.x,p.y)));
 if(kind)assert(s.request(0,kind,0,1,225,198,input));
 for(let t=0;t<(kind?22000:240000);t+=100){if(t===12000)input[0].available=input[1].available=false;if(t===15000)input[0].available=input[1].available=true;
  const output=s.tick(t,input,!kind);
  output.forEach((o,p)=>{const row=[scene,kind,t,p,+o.active,+o.release,o.x,o.y,o.jump,+o.left,o.pose,o.symbol,o.kind].join(' ');assert.equal(row,native[index++],`scene${scene}/kind${kind}/t${t}/pet${p}`);if(o.active||o.release)Object.assign(input[p],{x:o.x,y:o.y,left:o.left})});
 }
}
for(let slot=0;slot<5;slot++)for(let x0=48;x0<=272;x0+=112)for(let y0=115;y0<=208;y0+=y0===115?45:48)for(let tired=0;tired<2;tired++){
 let left=false;
 for(let t=0;t<48000;t+=33){const p=homePatrol(t,slot,!!tired,x0,y0,left);left=p.left;
  assert.equal(['P',slot,x0,y0,tired,t,p.x,p.y,+left,p.pose].join(' '),native[index++]);
 }
}
assert.equal(index,native.length);console.log(`PASS: ${index} C/Web behavior samples including all13 scenes, eight actions, interruptions and corner patrol liveness`);
