import {balance} from './balance-config.mjs?v=variety1';
// In-memory firmware menu preview. Device saves and browser rewards are untouched.
export const appMenuPages = [
 ['travel','training','battle','warehouse','scenes','fishing','steps','tarot','hospital'],
 ['badges','legend','settings','pokedex','collection']
];
export function appMenuPosition(page,index){
 const row=Math.floor(index/3),cols=page&&row===1?2:3;
 return {x:page?(360-cols*84+20)/2+(index%3)*84:74+(index%3)*74,y:page?104+row*84:68+row*72,size:64};
}
export const appMenuLabels = {training:'历练',travel:'旅行',tarot:'塔罗牌',battle:'道馆',warehouse:'仓库',scenes:'场景',legend:'神兽',steps:'计步器',collection:'收藏品',pokedex:'宠物图鉴',badges:'徽章',settings:'设置',fishing:'钓鱼',hospital:'宠物医院'};
export function createSceneMenu(count,{coins=3000,ownedScenes=[0],getProsperity=()=>1}={}){
 if(!Number.isInteger(count)||count<1)throw Error('Invalid scene count');
 const thresholds=balance.sceneRequirements,owned=new Set([0,...ownedScenes]);let selected=0,current=0;
 const unlock=()=>{for(let i=0;i<count;i++)if(getProsperity()>=(thresholds[i]??151))owned.add(i)};
 const valid=n=>{if(!Number.isInteger(n)||n<0||n>=count)throw Error('Invalid scene');return n};
 return {get selected(){return selected},get current(){return current},get coins(){return coins},get prosperity(){return getProsperity()},get required(){return thresholds[selected]??151},get state(){unlock();return selected===current?'active':owned.has(selected)?'owned':'locked'},get rewardLocked(){unlock();return !owned.has(selected)},get price(){return null},get file(){return `menu-scene-${selected}-${this.state}.png`},enter(n){selected=current=valid(n);owned.add(n);unlock()},move(delta){selected=((selected+delta)%count+count)%count},apply(){unlock();if(!owned.has(selected))return 'not-ready';if(selected===current)return 'active';current=selected;return 'ok'}};
}
