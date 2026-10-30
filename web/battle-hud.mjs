import {trainingTypeChart} from './training-catalog.mjs?v=variety1';
import {relationKind,paintBattleRelation} from './battle-relation-pixels.mjs?v=relations3';
const typeKeys={'电':'ELECTRIC','草':'GRASS','火':'FIRE','水':'WATER','一般':'NORMAL','虫':'BUG','毒':'POISON','飞行':'FLYING','地面':'GROUND','岩石':'ROCK','妖精':'FAIRY','超能':'PSYCHIC','超能力':'PSYCHIC','格斗':'FIGHTING','冰':'ICE','钢':'STEEL','幽灵':'GHOST','恶':'DARK','龙':'DRAGON'};
// Uses actual current move and both target types, including compact damage curve.
export function battleRelations(manifest,state,ready=true){
 const actors=[state.party[state.ally],state.foes[state.enemy]],fx=state.fx;
 const slot=Math.max(0,state.round-(fx?1:0))%4,elapsed=(fx?.at??-1)*2.25,active=elapsed<900?0:1,age=elapsed-(active?900:0);
 return actors.map((actor,side)=>{
  const form=manifest.pets[actor.art],target=manifest.pets[actors[1-side].art];
  const move=manifest.skills.find(m=>m.familyId===form.species&&m.slot===slot);
  const power=move.formPowers.find(p=>p.nationalId===form.nationalId)?.power??move.power;
  let factor=100;for(const type of manifest.petTypes[target.nationalId]??[])factor=Math.floor(factor*(trainingTypeChart[typeKeys[move.type]]?.[type]??100)/100);
  factor=!factor?0:factor<=25?35:factor<=50?65:factor>=400?200:factor>=200?150:100;
  const casting=ready&&fx&&elapsed>=0&&elapsed<1800&&side===active&&(!side||fx.reply);
  return {factor,kind:relationKind(factor,power),opacity:!power?0:casting?(age>=360&&age<800?255:200):120};
 });
}
export function drawBattleRelations(ctx,manifest,state,ready=true){
 ctx.save();
 const relations=battleRelations(manifest,state,ready);
 relations.forEach(({kind,opacity},side)=>{if(opacity)paintBattleRelation(side,kind,opacity,(x,y,width,color,alpha)=>{ctx.fillStyle=color;ctx.globalAlpha=alpha/255;ctx.fillRect(x+144,y+63,width,1)})});
 ctx.restore();return relations;
}
export function drawBattleHp(ctx,side,hp,maximum){
 const shown=Math.max(0,Math.min(maximum,Math.ceil(hp))),origin=side?196:0;
 for(let row=0;row<8;row++){
  const dy=84+row-179.5,edge=Math.ceil(179.5-Math.sqrt(180*180-dy*dy)),cap=[2,1,0,0,0,0,1,2][row];
  ctx.fillStyle='#263d4d';ctx.fillRect(origin+(side?cap:edge),84+row,164-edge-cap,1);
  const span=maximum>0?Math.floor((164-edge)*shown/maximum):0,width=span-cap;
  if(width>0){ctx.fillStyle=side?'#f1598a':'#35d79c';ctx.fillRect(origin+(side?164-edge-width:edge),84+row,width,1)}
 }
}
export function drawBattleBall(ctx,x,y,alive,current){
 ctx.save();ctx.translate(x,y);ctx.beginPath();ctx.roundRect(0,0,12,12,6);ctx.clip();
 ctx.fillStyle=alive?'#ed7968':'#40515d';ctx.fillRect(1,1,10,4);
 ctx.fillStyle=alive?'#d5e0e5':'#40515d';ctx.fillRect(1,7,10,4);
 ctx.fillStyle=alive?'#ffffff':'#61717c';ctx.beginPath();ctx.roundRect(4,4,4,4,2);ctx.fill();
 ctx.strokeStyle=current&&alive?'#f4d58c':'#182a38';ctx.lineWidth=1;ctx.beginPath();ctx.roundRect(.5,.5,11,11,5.5);ctx.stroke();ctx.restore();
}
