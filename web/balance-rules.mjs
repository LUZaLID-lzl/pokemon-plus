import {balance} from './balance-config.mjs?v=trainingxp2';
export {balance};
export function battleScaledXp(p,level,enemy){
 const effective=Math.min(enemy,level+3),base=1+Math.floor(balance.xpTenths*effective/10),gap=level>enemy+10?20:level>enemy+5?50:100;
 const food=(p.food??80)>=80?105:(p.food??80)<40?80:100,mood=(p.mood??80)>=80?105:(p.mood??80)<40?90:100;
 return base*gap*Math.min(11000,food*mood);
}
export function trainingReward(p,level,enemy,defeated,count,win,fainted){
 let scaled=battleScaledXp(p,level,enemy)*balance.training.xpMultiplier;if(!win)scaled=Math.floor(scaled*defeated/(5*count));scaled+=p.xpFraction||0;
 const battles=(p.trainingBattles||0)+1,energy=Math.max(0,p.energy-1);
 const xp=Math.min(Math.floor(scaled/1000000),35400-(10*level*(level-1)+(p.xp||0)));
 return {xp,changes:{xpFraction:xp>=35400-(10*level*(level-1)+(p.xp||0))?0:scaled%1000000,trainingBattles:battles%20,energy,sleeping:energy<20||!!p.sleeping,food:Math.max(0,p.food-(battles%10===0?1:0)),clean:Math.max(0,p.clean-(battles===20?1:0)),health:Math.max(0,p.health-(fainted?1:0))}};
}
