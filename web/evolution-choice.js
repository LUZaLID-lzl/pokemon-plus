import {shop,shopEnabled,shopPrize} from './shop.mjs?v=shoplottery1';
import {advanceEnergy} from './pet-energy.mjs?v=variety1';
import {balance} from './balance-config.mjs?v=autosupply1-shoplottery1';
import {validTrainingSave} from './training-job.mjs?v=autosupply1-shoplottery1';
import {fishCatalog} from './fish-catalog.js?v=fishingdex1';
// Browser demonstration state. Individual IDs remain stable across reloads.
export function evolutionStep(chain,stage) {
 if(chain?.steps?.[stage])return chain.steps[stage];
 const trigger=chain?.trigger?.endsWith('-stone')?chain.trigger:'level';
 return {trigger,level:chain?.levels?.[stage]??0,item:chain?.item||(trigger==='thunder-stone'?'雷之石':trigger==='moon-stone'?'月之石':null),price:trigger==='level'?0:(chain?.price??200)};
}
export function createEvolutionDemo({storage,roster,chains,now=()=>Date.now()}) {
 const key='pokemon-plus.web-companions-v1';
 let state={pets:[],coins:240,heart:false};
 try{const s=JSON.parse(storage.getItem(key));if(s&&Array.isArray(s.pets)&&Number.isSafeInteger(s.coins)&&s.coins>=0&&typeof s.heart==='boolean'&&s.pets.every(p=>p&&typeof p.id==='string'&&roster[p.art]&&typeof p.locked==='boolean')&&new Set(s.pets.map(p=>p.id)).size===s.pets.length&&(!s.pets.some(p=>p.locked)||s.heart))state=s}catch{}
 const natureNames=['勤奋','怕寂寞','勇敢','固执','顽皮','大胆','坦率','悠闲','淘气','乐天','胆小','急躁','认真','爽朗','天真','内敛','慢吞吞','冷静','害羞','马虎','温和','温顺','自大','慎重','浮躁'];
 const roll=n=>crypto.getRandomValues(new Uint32Array(1))[0]%n;
 const normalized=p=>({...p,...Object.fromEntries([["food",80],["clean",85],["energy",85]].map(([key,fallback])=>[key,Number.isInteger(p[key])&&p[key]>=0&&p[key]<=100?p[key]:fallback])),health:Number.isInteger(p.health)&&p.health>=0&&p.health<=100?p.health:100,mood:Number.isInteger(p.mood)&&p.mood>=0&&p.mood<=100?p.mood:80,level:Number.isInteger(p.level)&&p.level>=1&&p.level<=60?p.level:1,nature:Number.isInteger(p.nature)&&p.nature>=0&&p.nature<25?p.nature:roll(25),ivs:Array.isArray(p.ivs)&&p.ivs.length===6&&p.ivs.every(v=>Number.isInteger(v)&&v>=0&&v<=31)?p.ivs:Array.from({length:6},()=>roll(32)),nickname:typeof p.nickname==='string'?[...p.nickname].slice(0,8).join(''):''});
 state={...state,patCoinsToday:Math.min(25,Math.max(0,state.patCoinsToday||0)),fishToday:Math.max(0,state.fishToday||0),rewardDay:state.rewardDay||0,dexHistory:Array.isArray(state.dexHistory)?state.dexHistory.filter(n=>Number.isInteger(n)&&n>0&&n<=151):[],treeEssences:Number.isInteger(state.treeEssences)&&state.treeEssences>=0&&state.treeEssences<=65535?state.treeEssences:0,patDay:Number.isInteger(state.patDay)&&state.patDay>=0?state.patDay:0,pets:state.pets.map(p=>({...normalized(p),xpFraction:Number.isInteger(p.xpFraction)&&p.xpFraction>=0&&p.xpFraction<1000000?p.xpFraction:0,trainingBattles:Number.isInteger(p.trainingBattles)&&p.trainingBattles>=0&&p.trainingBattles<20?p.trainingBattles:0,patsToday:p.patsToday===undefined?0:Number.isInteger(p.patsToday)&&p.patsToday>=0&&p.patsToday<=20?p.patsToday:20,xp:Number.isInteger(p.xp)&&p.xp>=0&&p.xp<20*p.level&&p.level<60?p.xp:0}))};
 if(state.training&&!validTrainingSave(state.training,roster,state.pets))state.training=null;
 state.trainingItems=Array.from({length:9},(_,i)=>Number.isInteger(state.trainingItems?.[i])&&state.trainingItems[i]>=0&&state.trainingItems[i]<=65535?state.trainingItems[i]:0);
 state.active=Array.isArray(state.active)&&state.active.length<=5&&new Set(state.active).size===state.active.length&&state.active.every(id=>state.pets.some(p=>p.id===id))?state.active:state.pets.slice(0,5).map(p=>p.id);
 state.fishingPartner=typeof state.fishingPartner==='string'&&state.pets.some(p=>p.id===state.fishingPartner)&&!state.active.includes(state.fishingPartner)?state.fishingPartner:null;
 state.fishCaught=fishCatalog.map((_,i)=>Number.isInteger(state.fishCaught?.[i])&&state.fishCaught[i]>=0&&state.fishCaught[i]<=0xffffffff?state.fishCaught[i]:0);
 function knownDex(pets){const ids=[];for(const p of pets){const form=roster[p.art];if(!form)continue;ids.push(form.nationalId);if(form.stage){const chain=chains.find(c=>c.species===form.species);if(chain?.branches){ids.push(chain.nationalIds[0])}else if(chain){ids.push(...chain.nationalIds.slice(0,chain.nationalIds.indexOf(form.nationalId)+1))}}}return ids}
 state.dexHistory=[...new Set([...state.dexHistory,...knownDex(state.pets)])];
 state.careWall=Number.isSafeInteger(state.careWall)?state.careWall:now();state.careSeconds=state.careSeconds||0;
 function commit(next){next={...next,dexHistory:[...new Set([...(state.dexHistory||[]),...knownDex(next.pets)])]};storage.setItem(key,JSON.stringify(next));state=next;return state}
 if(state.training?.job?.items?.[2]||state.training?.job?.lastItem===3){const training=structuredClone(state.training);training.job.items[2]=0;if(training.job.lastItem===3)training.job.lastItem=0;commit({...state,training});}
 // Retire the frozen potion slot once; failed writes keep the old state intact.
 if(state.trainingItems[2]){const count=state.trainingItems[2],trainingItems=[...state.trainingItems];trainingItems[2]=0;commit({...state,trainingItems,coins:Math.min(0xffffffff,state.coins+count*balance.prices[2])});}
 function shopTransaction(item,coins,cost){
  if(!shopEnabled(item))throw Error('商品已下架');
  if(!Number.isSafeInteger(coins)||coins<cost)throw Error('金币不足');
  const stock=item===8?state.treeEssences:state.trainingItems[item];if(stock>=65535)throw Error('背包已满');
  const trainingItems=[...state.trainingItems];if(item!==8)trainingItems[item]++;
  commit({...state,coins:coins-cost,trainingItems,treeEssences:state.treeEssences+(item===8?1:0)});return state.coins;
 }
 function shopBuy(item,coins=state.coins){return shopTransaction(item,coins,balance.prices[item]);}
 function shopDraw(coins=state.coins,random=()=>crypto.getRandomValues(new Uint32Array(1))[0]){
  if(coins<shop.drawPrice)throw Error('金币不足');
  if(shop.weights.some((w,i)=>w&&(i===8?state.treeEssences:state.trainingItems[i])>=65535))throw Error('背包已满');
  const item=shopPrize(random()%100);shopTransaction(item,coins,shop.drawPrice);return item;
 }
 function add(art,level=1){if(!roster[art])throw Error('未知伙伴');const pet={...normalized({id:crypto.randomUUID(),art,locked:false,level}),patsToday:0,xp:0};commit({...state,pets:[...state.pets,pet],active:state.active.length<5?[...state.active,pet.id]:state.active});return pet}
 function choices(id){if(trainingBusy(id))throw Error("伙伴正在历练");
  const pet=state.pets.find(p=>p.id===id);if(!pet)throw Error('请选择伙伴');if(pet.locked)throw Error('这只伙伴已选择永久不进化');
  const form=roster[pet.art],chain=chains.find(c=>c.species===form.species),i=chain?.nationalIds.indexOf(form.nationalId);
  const steps=chain?.branches?chain.branches.filter(b=>b.fromNationalId===form.nationalId):chain&&i>=0&&i<chain.nationalIds.length-1?[{...evolutionStep(chain,i),toNationalId:chain.nationalIds[i+1]}]:[];
  if(!steps.length)throw Error('当前没有可用进化');
  return steps.map(step=>{const next=roster.find(p=>p.nationalId===step.toNationalId);if(!next)throw Error('形态尚未接入');return {pet,next,step,cost:step.trigger==='level'?0:step.price}});
 }
 function offer(id,target){const all=choices(id);if(target!==undefined){const picked=all.find(x=>x.next.nationalId===target);if(!picked)throw Error('进化分支无效');return picked}if(all.length!==1)throw Error('请选择进化分支');return all[0]}
 function choose(id,decline,coins=state.coins,target){
  const {pet,next,cost}=decline?choices(id)[0]:offer(id,target);
  if(!Number.isSafeInteger(coins)||coins<0)throw Error('金币数据无效');if(!decline&&coins<cost)throw Error('金币不足');
  const updated={...pet,art:decline?pet.art:next.id,locked:decline};commit({...state,coins:coins-(decline?0:cost),heart:state.heart||decline,pets:state.pets.map(p=>p.id===id?updated:p)});return updated;
 }
 // A monotonic calendar day prevents rollback/reload/swap from refilling quotas.
 function pat(ids,day,xp=1,coins=state.coins){
  if(!Array.isArray(ids)||!Number.isInteger(day)||day<0||![1,2].includes(xp)||!Number.isSafeInteger(coins)||coins<0)throw Error('互动数据无效');
  const requested=new Set(ids),reset=state.patDay>0&&day>state.patDay,rewarded=[];
  const pets=state.pets.map(p=>{
   const used=reset?0:p.patsToday;
   if(!requested.has(p.id)||trainingBusy(p.id)||used>=20)return reset?{...p,patsToday:0}:p;
   rewarded.push(p.id);let level=p.level,total=p.xp+(used<5?xp:0);
   while(level<60&&total>=20*level){total-=20*level;level++}
   return {...p,mood:Math.min(100,p.mood+4),patsToday:used+1,level,xp:level===60?0:total};
  });
  if(!rewarded.length){if(!state.patDay&&day>0)commit({...state,patDay:day});return rewarded;}
  commit({...state,patDay:Math.max(day,state.patDay),pets,patCoinsToday:Math.min(25,(reset?0:state.patCoinsToday)+pets.filter(p=>rewarded.includes(p.id)&&p.patsToday<=5).length),coins:Math.min(0xffffffff,coins+Math.min(25-(reset?0:state.patCoinsToday),pets.filter(p=>rewarded.includes(p.id)&&p.patsToday<=5).length))});
  return rewarded;
 }
 function rename(id,name){if(typeof name!=='string'||!/^[a-z]{0,8}$/.test(name)||!state.pets.some(p=>p.id===id))throw Error('仅限8个小写字母');commit({...state,pets:state.pets.map(p=>p.id===id?{...p,nickname:name}:p)});}
 function warehouse(){return [...state.active.map(id=>state.pets.find(p=>p.id===id)),...state.pets.filter(p=>!state.active.includes(p.id))];}
 function release(id){if(trainingGrowthPending(id))throw Error("请先查看历练升级提示");if(trainingBusy(id))throw Error("伙伴正在历练");if(id===state.fishingPartner)throw Error('请先更换陪钓伙伴');if(state.active.includes(id))throw Error('请先收入仓库');if(!state.pets.some(p=>p.id===id))throw Error('未知伙伴');commit({...state,pets:state.pets.filter(p=>p.id!==id)});}
 function setActive(ids){if(state.training?.job?.status===1&&(ids?.length!==state.active.length||state.active.some((id,i)=>trainingBusy(id)&&ids?.[i]!==id)))throw Error("历练中只能替换空闲伙伴");if(state.fishingPartner&&ids?.includes(state.fishingPartner))throw Error('请先更换陪钓伙伴');if(!Array.isArray(ids)||ids.length>5||!ids.length||new Set(ids).size!==ids.length||!ids.every(id=>state.pets.some(p=>p.id===id)))throw Error('庭院需要1至5只伙伴');commit({...state,active:[...ids]});}
 function homeHit(id,health,mood){
  const p=state.pets.find(p=>p.id===id);
  if(!p||trainingBusy(id)||!state.active.includes(id)||!Number.isInteger(health)||health<0||health>3||!Number.isInteger(mood)||mood<0||mood>5)throw Error('互动数据无效');
  commit({...state,pets:state.pets.map(p=>p.id===id?{...p,health:Math.max(1,p.health-health),mood:Math.max(0,p.mood-mood)}:p)});
 }
 function buyEssence(coins=state.coins){
  if(!Number.isSafeInteger(coins)||coins<250)throw Error('金币不足');
  if(state.treeEssences>=65535)throw Error('背包已满');
  commit({...state,coins:coins-250,treeEssences:state.treeEssences+1});return state.coins;
 }
 function useEssence(ids=state.active){
  if(!state.treeEssences)throw Error('道具数量不足');
  const sum=p=>p.food+p.clean+p.health+p.mood+p.energy;
  const target=state.pets.filter(p=>ids.includes(p.id)&&state.active.includes(p.id)&&!trainingBusy(p.id)).sort((a,b)=>sum(a)-sum(b)||state.active.indexOf(a.id)-state.active.indexOf(b.id))[0];
  if(!target)throw Error('宠物正忙着呢');if(sum(target)===500)throw Error('状态已满');
  const updated={...target,food:100,clean:100,health:100,mood:100,energy:100,sleeping:false};
  commit({...state,treeEssences:state.treeEssences-1,pets:state.pets.map(p=>p.id===target.id?updated:p)});return updated;
 }
 function selectFishingPartner(id){if(trainingBusy(id))throw Error('伙伴正在历练');if(id!==null&&(!state.pets.some(p=>p.id===id)||state.active.includes(id)))throw Error('请选择仓库伙伴');if(id!==state.fishingPartner)commit({...state,fishingPartner:id});}
 function catchFish(id,reward,coins=state.coins){
  if(!fishCatalog[id]||!Number.isInteger(reward)||reward<0||!Number.isSafeInteger(coins)||coins<0||coins>0xffffffff)throw Error('钓鱼数据无效');
  const date=new Date(now()),today=date.getFullYear()*10000+(date.getMonth()+1)*100+date.getDate(),fresh=today>state.rewardDay,used=fresh?0:state.fishToday;
  const amount=used>=15?Math.max(1,Math.floor(reward/5)):reward,granted=Math.min(0xffffffff-coins,amount),fishCaught=[...state.fishCaught];fishCaught[id]=Math.min(0xffffffff,fishCaught[id]+1);
  commit({...state,fishCaught,coins:coins+granted,rewardDay:Math.max(today,state.rewardDay),fishToday:Math.min(65535,used+1)});return granted;
 }
 function advanceCare(){
  const elapsed=Math.floor((now()-state.careWall)/1000);if(elapsed<=0)return;
  const dt=Math.min(28800,elapsed),start=state.careSeconds,end=start+dt;
  const foodTicks=Math.floor(end/3600)-Math.floor(start/3600),otherTicks=Math.floor(end/7200)-Math.floor(start/7200);
  const pets=state.pets.map(p=>{if(trainingBusy(p.id)||p.id===state.traveler)return p;const rest=advanceEnergy(p,dt),active=state.active.includes(p.id);
   return {...p,energy:rest.energy,sleeping:rest.sleeping,energyFraction:rest.energyFraction,...(active?{food:Math.max(Math.min(20,p.food),p.food-foodTicks),clean:Math.max(Math.min(20,p.clean),p.clean-otherTicks),mood:Math.max(Math.min(30,p.mood),p.mood-otherTicks)}:{})};
  });commit({...state,pets,careWall:state.careWall+elapsed*1000,careSeconds:end});
 }
 function hospitalTreat(id){const p=state.pets.find(p=>p.id===id);if(!p||!state.active.includes(id)||trainingBusy(id)||id===state.traveler)throw Error('伙伴忙碌中');if(p.health===100)throw Error('健康已满');if(state.coins<20)throw Error('金币不足');commit({...state,coins:state.coins-20,pets:state.pets.map(p=>p.id===id?{...p,health:100}:p)});}
 function hospitalRecover(targets){
  if(!Array.isArray(targets)||targets.length!==5||targets.some(row=>!Array.isArray(row)||row.length!==5||row.some(n=>!Number.isInteger(n)||n<0||n>100)))throw Error('护理数值无效');
  const eligible=state.active.filter(id=>id!==state.traveler&&!trainingBusy(id));if(!eligible.length)throw Error('暂时没有可护理的伙伴');
  const fields=['food','clean','health','mood','energy'],treated=[];
  const pets=state.pets.map(p=>{const slot=state.active.indexOf(p.id);if(slot<0||!eligible.includes(p.id))return p;
   const updated={...p,health:Math.max(p.health,60)};treated.push(updated);return updated});
  commit({...state,pets});return treated;
 }
 function useTrainingItem(item,petId){
  if(!Number.isInteger(item)||item<0||item>7||!state.trainingItems[item])throw Error('道具数量不足');
  let pets=state.pets;
  if(item<5){const p=pets.find(p=>p.id===petId);if(!p||!state.active.includes(petId)||trainingBusy(petId))throw Error('宠物正忙着呢');
   const values=balance.care[item],fields=['food','clean','health','mood','energy'];
   if(p[['food','clean','health','mood','energy'][item]]===100)throw Error('状态已满');
   pets=pets.map(p=>p.id!==petId?p:{...p,...Object.fromEntries(fields.map((key,i)=>[key,Math.min(100,p[key]+values[i])])),...(item===4&&p.energy+values[4]>=60?{sleeping:false}:{})});
  }
  if(!shopEnabled(item))throw Error("商品已下架");
  const trainingItems=[...state.trainingItems];trainingItems[item]--;commit({...state,pets,trainingItems});
 }
 function trainingCommit(training,reward){
  for(let i=0;i<9;i++){const delta=reward.items?.[i]||0,stock=i===8?state.treeEssences:state.trainingItems[i];if(!Number.isSafeInteger(delta)||stock+delta<0)throw Error('道具数量不足');}
  const pets=state.pets.map(p=>{let total=p.xp,level=p.level;for(const row of reward.xp||[])if(row.ids.includes(p.id)){total+=row.xp;p={...p,...row.changes}}while(level<60&&total>=20*level){total-=20*level;level++}return {...p,level,xp:level===60?0:total}});
  const stock=Array.from({length:9},(_,i)=>i===2?0:Math.max(0,Math.min(65535,(state.trainingItems?.[i]||0)+(reward.items?.[i]||0))));
  commit({...state,pets,training,trainingItems:stock,coins:Math.min(0xffffffff,(reward.baseCoins??state.coins)+(reward.coins||0)),treeEssences:Math.min(65535,state.treeEssences+(reward.items?.[8]||0))});
 }
 function trainingGrowthPending(id){const t=state.training;if(t?.job?.status!==2)return false;return t.battle.state.party.some((p,i)=>(id===undefined||p.id===id)&&t.job.endLevels[i]>t.job.startLevels[i]&&(!(t.job.growthSeen&(1<<i))||!(t.job.evolutionSeen&(1<<i))))}
 function trainingBusy(id){return state.training?.job?.status===1&&state.training.battle?.state?.party?.some(p=>p.id===id)}
 function trainingReward(ids,xp){
  if(!Array.isArray(ids)||!ids.length||ids.length>3||new Set(ids).size!==ids.length||!Number.isInteger(xp)||xp<0||xp>1000||ids.some(id=>!state.pets.some(p=>p.id===id)))throw Error('历练数据无效');
  const pets=state.pets.map(p=>{if(!ids.includes(p.id)||p.level>=60)return p;let level=p.level,total=p.xp+xp;while(level<60&&total>=20*level){total-=20*level;level++}return {...p,level,xp:level===60?0:total}});
  commit({...state,pets});
 }
 return {get state(){return state},shopBuy,shopDraw,trainingReward,trainingCommit,trainingBusy,trainingGrowthPending,advanceCare,useTrainingItem,hospitalRecover,hospitalTreat,selectFishingPartner,catchFish,buyEssence,useEssence,homeHit,add,choices,offer,choose,pat,rename,release,setActive,warehouse,natureNames};
}
