import {trainingReward,balance} from './balance-rules.mjs?v=trainingxp2-variety1';
import {trainingScenes,trainingTypeChart} from './training-catalog.mjs?v=variety1';
export {trainingScenes};
export const trainingApproachMs=1800;
const types=['ELECTRIC','GRASS','FIRE','WATER','NORMAL','BUG','POISON','FLYING','GROUND','ROCK','FAIRY','PSYCHIC','FIGHTING','ICE','STEEL','GHOST','DARK','DRAGON'];
const labels=['电','草','火','水','一般','虫','毒','飞行','地面','岩石','妖精','超能','格斗','冰','钢','幽灵','恶','龙'];
export function trainingStat(p,form,stat){
 const l=p.level,rank=form.species===67?Number(!!form.stage):form.stage,axis=[0,0,1,3,4,2];
 let value=(stat===0?64+6*l+6*rank:stat===1||stat===3?22+3*l+4*rank:stat===5?20+2*l:16+2*l+3*rank)+Math.floor((p.ivs?.[stat]??15)*l/60);
 const up=Math.floor((p.nature??0)/5),down=(p.nature??0)%5;if(stat&&up!==down){if(axis[stat]===up)value=Math.floor(value*110/100);else if(axis[stat]===down)value=Math.floor(value*90/100)}return value;
}
export function createTraining({manifest,getPets,grant,seed=1}){
 let rng=seed>>>0||1,battleRng=1,speedRemainder=0;
 const state={phase:'select',scene:0,party:[],foes:[],ally:0,enemy:0,elapsed:0,checks:0,round:0,rounds:0,xp:0,totalXp:0,claimed:false,win:false,story:'',fx:null,resultTime:0,approachTime:0,alertTime:0};
 function next(){let x=rng;x^=x<<13;x^=x>>>17;x^=x<<5;rng=x>>>0||1;return rng}
 const roll=n=>next()%n;
 function actor(p){const form=manifest.pets[p.art];return {...p,ivs:[...(p.ivs??[15,15,15,15,15,15])],form,maxHp:Math.floor(trainingStat(p,form,0)*((p.health??100)>=80?1:(p.health??100)>=60?.9:.8)),hp:Math.floor(trainingStat(p,form,0)*((p.health??100)>=80?1:(p.health??100)>=60?.9:.8))}}
 function refresh(){state.party=state.party.map(p=>actor(getPets().find(x=>x.id===p.id)||p));battleRng=(next()|1)>>>0}
 function encounter(){
  refresh();const s=trainingScenes[state.scene],recent=state.rounds?state.foes.map(p=>p.form.nationalId):[],pool=s.pool.map((_,i)=>i).filter(i=>!recent.includes(s.pool[i]));state.foes=[];
  for(let i=0;i<state.party.length;i++){const j=i+roll(pool.length-i);[pool[i],pool[j]]=[pool[j],pool[i]];const form=manifest.pets.find(p=>p.nationalId===s.pool[pool[i]]);state.foes.push(actor({art:form.id,level:(()=>{const levels=state.party.map(p=>p.level).sort((a,b)=>a-b),ref=levels[Math.floor(levels.length/2)],spread=ref<=5?1:2,lo=Math.min(s.maxLevel,Math.max(s.minLevel,ref-spread)),hi=Math.max(lo,Math.min(s.maxLevel,ref<=5?ref:ref<=10?ref+1:ref+2));return lo+roll(hi-lo+1)})(),ivs:[15,15,15,15,15,15],nature:0}))}
  const ref=state.party.map(p=>p.level).sort((a,b)=>a-b)[Math.floor(state.party.length/2)],difficulty=balance.training.noviceScale[ref<=5?0:ref<=10?1:2];for(const foe of state.foes){foe.difficulty=difficulty;foe.hp=foe.maxHp=Math.floor(foe.maxHp*difficulty/100)}
  state.ally=state.enemy=state.round=0;state.elapsed=0;state.approachTime=0;state.phase='approach';state.claimed=false;state.fx=null;state.story='野生伙伴出现了！';
 }
 function move(p,slot){const m=manifest.skills.find(x=>x.familyId===p.form.species&&x.slot===slot);if(!m)throw Error('技能资源缺失');return {...m,power:m.formPowers.find(x=>x.nationalId===p.form.nationalId)?.power??m.power}}
 function damage(a,d,slot,enemy){const m=move(a,slot),type=types[labels.indexOf(m.type==='超能力'?'超能':m.type)],special=['FIRE','WATER','GRASS','ELECTRIC','ICE','PSYCHIC','DRAGON','DARK'].includes(type);let factor=100;
  for(const t of manifest.petTypes[d.form.nationalId]??[])factor=Math.floor(factor*(trainingTypeChart[type]?.[t]??100)/100);
  factor=!factor?0:factor<=25?35:factor<=50?65:factor>=400?200:factor>=200?150:100;
  const attack=enemy?Math.floor(Math.floor((trainingStat(a,a.form,1)+trainingStat(a,a.form,3))/2)*(a.difficulty??100)/100):trainingStat(a,a.form,special?3:1);
  const defense=enemy?trainingStat(d,d.form,special?4:2):Math.floor(Math.floor((trainingStat(d,d.form,2)+trainingStat(d,d.form,4))/2)*(d.difficulty??100)/100);
  let x=battleRng;x^=x<<13;x^=x>>>17;x^=x<<5;battleRng=x>>>0;
  let hit=Math.floor((Math.floor((12+a.level)*attack/(3*defense))+4)*m.power*factor*(90+battleRng%11)/550000);
  return {hit:factor&&m.power&&!hit?1:hit,move:m,immune:factor===0&&m.power>0};
 }
 function finish(win){state.phase='result';state.resultTime=0;state.win=win;state.xp=0;state.story=win?'历练获胜':'历练结束';}

 function turn(){
  const a=state.party[state.ally],d=state.foes[state.enemy];if(!a.hp||!d.hp){if(!a.hp)state.ally++;if(!d.hp)state.enemy++;state.story='新的伙伴上场！';state.fx=null;return}
  if(state.round>=100){finish(false);return}const slot=state.round++%4,hit=damage(a,d,slot,false);d.hp=Math.max(0,d.hp-hit.hit);state.fx={side:0,type:hit.move.type,hit:hit.hit,immune:hit.immune,at:0};state.story=`${a.nickname||a.form.name}使出了${hit.move.name}`;
  if(!d.hp){if(state.enemy+1===state.foes.length)finish(true);return}
  const reply=damage(d,a,slot,true);a.hp=Math.max(0,a.hp-reply.hit);state.fx.reply={type:reply.move.type,hit:reply.hit,immune:reply.immune};if(!a.hp&&state.ally+1===state.party.length)finish(false);
 }
 return {state,get rng(){return rng},
  snapshot(){const strip=team=>team.map(({form,...p})=>p);return structuredClone({rng,battleRng,speedRemainder,state:{...state,party:strip(state.party),foes:strip(state.foes)}})},
  restore(saved){rng=saved.rng>>>0||1;battleRng=saved.battleRng>>>0||1;speedRemainder=saved.speedRemainder||0;for(const key of Object.keys(state))delete state[key];Object.assign(state,structuredClone(saved.state));state.party=state.party.map(p=>({...p,form:manifest.pets[p.art]}));state.foes=state.foes.map(p=>({...p,form:manifest.pets[p.art]}))},
  start(scene,ids){if(!trainingScenes[scene]||!Array.isArray(ids)||ids.length<1||ids.length>3||new Set(ids).size!==ids.length)throw Error('请选择1至3位伙伴');
   const pets=getPets();if(ids.some(id=>!pets.some(p=>p.id===id&&!p.sleeping&&p.health>=30&&p.food>=20&&p.energy>=40)))throw Error('需健康30、饱腹20、精力40');
   speedRemainder=0;Object.assign(state,{phase:'search',scene,foes:[],ally:0,enemy:0,round:0,fx:null,resultTime:0,story:'',win:false,party:ids.map(id=>actor(pets.find(p=>p.id===id))),elapsed:0,checks:0,rounds:0,totalXp:0,xp:0,claimed:false,alertTime:0,approachTime:0});refresh();
  },
  advance(ms){ms=Math.max(0,Math.min(250,Math.floor(ms)));if(state.phase==='search'){state.elapsed+=ms;const check=Math.floor(state.elapsed/1000);if(check>state.checks){state.checks=check;if(check>=5&&(roll(100)<25||check>=16)){encounter()}}}
   else if(state.phase==='approach'){state.approachTime+=ms;if(state.approachTime>=trainingApproachMs){state.phase='alert';state.alertTime=0}}
   else if(state.phase==='alert'){state.alertTime+=ms;if(state.alertTime>=2000){state.phase='fight';state.elapsed=0;speedRemainder=0}}
   else if(state.phase==='fight'){const scaled=ms*3+speedRemainder;speedRemainder=scaled%2;state.elapsed+=Math.floor(scaled/2);if(state.fx)state.fx.at+=ms;if(state.elapsed>=2000){state.elapsed-=2000;turn()}}
   else if(state.phase==='result'){state.resultTime+=ms;if(state.fx)state.fx.at+=ms;}
  },
  claim(){if(state.phase!=='result')return false;if(state.claimed)return true;
   const enemy=Math.floor(state.foes.reduce((n,p)=>n+p.level,0)/state.foes.length),defeated=state.foes.filter(p=>!p.hp).length;state.xp=0;state.earnedBy=[];
   for(const actor of state.party){const pet=getPets().find(p=>p.id===actor.id)||actor,result=trainingReward(pet,actor.level,enemy,defeated,state.foes.length,state.win,!actor.hp);grant([pet.id],result.xp,result.changes);state.xp+=result.xp;state.earnedBy.push({id:pet.id,xp:result.xp});}
   state.story=(state.win?'历练获胜':'历练结束')+' · 本场经验 +'+state.xp;state.claimed=true;state.totalXp+=state.xp;state.rounds++;return true},
  resume(){if(state.phase==='result'&&state.claimed){state.phase='search';state.elapsed=state.checks=0;state.fx=null}},
  close(){state.phase='select';state.fx=null}
 };
}
