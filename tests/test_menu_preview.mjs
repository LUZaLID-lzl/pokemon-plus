import assert from 'node:assert/strict';
import {appMenuPages,appMenuPosition,createSceneMenu} from '../web/menu-preview.mjs';
import {balance} from '../web/balance-config.mjs';
assert.deepEqual(appMenuPages.map(p=>p.length),[9,5]);assert.equal(new Set(appMenuPages.flat()).size,14);
for(const [page,entries] of appMenuPages.entries())for(const [i] of entries.entries()){
 const {x,y,size}=appMenuPosition(page,i);assert.equal(size,64);for(const px of [x,x+size-1])for(const py of [y,y+size-1])assert.ok((px-180)**2+(py-180)**2<=168**2);
}
for(let n=1;n<=151;n++){
 const menu=createSceneMenu(13,{coins:0,getProsperity:()=>n});
 for(let scene=0;scene<13;scene++){menu.move(scene-menu.selected);assert.equal(menu.required,balance.sceneRequirements[scene]);const expected=n<menu.required?'not-ready':menu.current===scene?'active':'ok';assert.equal(menu.apply(),expected);assert.equal(menu.coins,0);}
}
const old=createSceneMenu(13,{ownedScenes:[0,12],getProsperity:()=>1});old.move(-1);assert.equal(old.state,'owned');assert.equal(old.apply(),'ok');assert.throws(()=>old.enter(13));assert.throws(()=>createSceneMenu(0));
console.log('PASS: APP circular safety, 151 prosperity counts × 13 thresholds, zero-coin unlock and retained ownership');
