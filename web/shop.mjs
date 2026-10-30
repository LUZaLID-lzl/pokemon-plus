import {balance} from './balance-config.mjs?v=shopreel2';
export const shop=balance.shop;
export const shopEnabled=item=>Number.isInteger(item)&&item>=0&&item<9&&item!==shop.retiredItem;
export function shopPrize(value){
 if(!Number.isInteger(value)||value<0||value>=100)throw Error('抽奖数据无效');
 for(let i=0;i<shop.weights.length;i++){if(value<shop.weights[i])return i;value-=shop.weights[i]}
 throw Error('抽奖数据无效');
}
export function shopReelView(item,elapsed){
 const target=shop.ring.findIndex(slot=>shop.grid[slot]===item);
 if(target<0)throw Error('抽奖数据无效');
 const steps=shop.laps*8+target;let deadline=0;
 for(let step=0;step<steps;step++){
  const remaining=steps-step;
  deadline+=shop.stepMs+(remaining<=shop.slowSteps?(shop.slowSteps-remaining+1)*shop.slowStepMs:0);
  if(elapsed<deadline)return {slot:shop.ring[step%8],next:deadline,visible:true,done:false};
 }
 const age=elapsed-deadline;
 if(age>=shop.blinkMs*shop.blinkCount*2)return {slot:shop.ring[target],next:elapsed,visible:true,done:true};
 const flash=Math.floor(age/shop.blinkMs);
 return {slot:shop.ring[target],next:deadline+(flash+1)*shop.blinkMs,visible:flash%2===0,done:false};
}
export const shopReelSlot=(item,elapsed)=>shopReelView(item,elapsed).slot;
export function shopReelAdvance(item,elapsed,delta){const view=shopReelView(item,elapsed);return view.done?elapsed:elapsed+Math.min(Math.max(0,delta),view.next-elapsed)}
