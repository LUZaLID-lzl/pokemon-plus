// Browser-only daily preview; the device uses the shared C core and its own NVS.
export const tarotNames=['太阳 · 活力','星星 · 希望','月亮 · 想象','世界 · 圆满','力量 · 勇气','恋人 · 陪伴','节制 · 平衡','命运 · 惊喜'];
export const tarotEffects=['活动精力消耗 -10%','计步经验 +10%','休息恢复 +20%','旅行金币 +20%','挑战伤害 +10%','摸头心情 +4 · 经验 +2','道具恢复 +15%','每千步金币 +2'];
export function createDailyPreview({storage,sceneCount,roll=()=>crypto.getRandomValues(new Uint32Array(1))[0]}) {
 const key='pokemon-plus.web-daily-v1';let state={day:0,random:false,scene:0,sceneDay:0,tarotDay:0,card:0};
 try{const x=JSON.parse(storage.getItem(key));if(x&&Number.isInteger(x.day)&&x.day>=0&&typeof x.random==='boolean'&&Number.isInteger(x.scene)&&x.scene>=0&&x.scene<sceneCount&&Number.isInteger(x.sceneDay)&&x.sceneDay>=0&&x.sceneDay<=x.day&&Number.isInteger(x.tarotDay)&&x.tarotDay>=0&&x.tarotDay<=x.day&&Number.isInteger(x.card)&&x.card>=0&&x.card<8)state=x}catch{}
 function commit(x){storage.setItem(key,JSON.stringify(x));state=x;return state}
 function advance(day,random=state.random){if(!Number.isInteger(day)||day<state.day)throw Error('请检查本地日期');const x={...state,day,random};if(random&&x.sceneDay<day){if(sceneCount>1)x.scene=(x.scene+1+roll()%(sceneCount-1))%sceneCount;x.sceneDay=day}if(JSON.stringify(x)!==JSON.stringify(state))commit(x);return state}
 function draw(day){advance(day);if(state.tarotDay===day)throw Error('今天已翻牌');return commit({...state,tarotDay:day,card:roll()%8})}
 return {get state(){return state},advance,toggle(day){return advance(day,!state.random)},draw};
}
