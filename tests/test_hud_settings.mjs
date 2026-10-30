import assert from 'node:assert/strict';
import {createHudSettings} from '../web/hud-settings.js';
const values=new Map();let fail=false;
const storage={getItem:k=>values.get(k),setItem(k,v){if(fail)throw Error('storage failed');values.set(k,v)}};
let s=createHudSettings(storage);assert.deepEqual(s.state,{showTime:true,showCoins:true,showBattery:true,trainingAutoSupply:false});
s.toggle('showTime');assert.deepEqual(s.state,{showTime:false,showCoins:true,showBattery:true,trainingAutoSupply:false});
s=createHudSettings(storage);assert.deepEqual(s.state,{showTime:false,showCoins:true,showBattery:true,trainingAutoSupply:false});
s.toggle('showCoins');assert.deepEqual(createHudSettings(storage).state,{showTime:false,showCoins:false,showBattery:true,trainingAutoSupply:false});
s.toggle('showBattery');assert.equal(createHudSettings(storage).state.showBattery,false);
fail=true;assert.throws(()=>s.toggle('showTime'));assert.equal(s.state.showTime,false);
assert.throws(()=>s.toggle('unknown'));fail=false;
values.set('pokemon-plus.web-display-v1','invalid');assert.deepEqual(createHudSettings(storage).state,{showTime:true,showCoins:true,showBattery:true,trainingAutoSupply:false});
values.set('pokemon-plus.web-display-v1',JSON.stringify({showTime:false,showCoins:'invalid'}));assert.deepEqual(createHudSettings(storage).state,{showTime:false,showCoins:true,showBattery:true,trainingAutoSupply:false});
assert.deepEqual(createHudSettings({getItem(){throw Error('denied')}}).state,{showTime:true,showCoins:true,showBattery:true,trainingAutoSupply:false});
console.log('PASS: time/coins/battery default on, independent persistence/reload, invalid fallback and failed-write rollback');

s=createHudSettings(storage);s.toggle('trainingAutoSupply');assert.equal(createHudSettings(storage).state.trainingAutoSupply,true);fail=true;assert.throws(()=>s.toggle('trainingAutoSupply'));assert.equal(s.state.trainingAutoSupply,true);fail=false;
