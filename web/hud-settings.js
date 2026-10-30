// Browser-only display and training preferences, independent of device NVS/game saves.
export function createHudSettings(storage) {
 const key='pokemon-plus.web-display-v1';let state={showTime:true,showCoins:true,showBattery:true,trainingAutoSupply:false};
 try{const x=JSON.parse(storage.getItem(key));if(x&&typeof x==='object')for(const k of Object.keys(state))if(typeof x[k]==='boolean')state[k]=x[k]}catch{}
 return {get state(){return {...state}},toggle(field){if(!Object.hasOwn(state,field))throw Error('Unknown display preference');const next={...state,[field]:!state[field]};storage.setItem(key,JSON.stringify(next));state=next;return state[field]}};
}
