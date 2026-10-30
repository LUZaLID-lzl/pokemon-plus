// One energy point is 3600 saved units; identical to core/pet_energy.h.
export const careSeconds=600,sleepBelow=20,wakeEnergy=60,sleepRecovery=25;
export function advanceEnergy(state,seconds,{moon=false}={}){
 if(!Number.isSafeInteger(seconds)||seconds<0)throw Error('无效时间');
 const next={...state,seconds:(state.seconds||0)+seconds,remainder:((state.remainder||0)+seconds)%600};
 let fraction=state.energyFraction||0;if(next.energy<20)next.sleeping=true;if(next.energy>=60)next.sleeping=false;
 while(seconds){let rate=next.sleeping?25:10;if(moon)rate=Math.floor(rate*120/100);let take=seconds;
  if(next.sleeping)take=Math.min(take,Math.ceil(((60-next.energy)*3600-fraction)/rate));
  const total=fraction+take*rate;next.energy=Math.min(100,next.energy+Math.floor(total/3600));fraction=total%3600;seconds-=take;if(next.energy>=60)next.sleeping=false;
 }
 next.energyFraction=fraction;return next;
}
export function energyBiscuit(state,{temperance=false}={}){
 const next={...state,energy:Math.min(100,state.energy+(temperance?28:25))};if(next.energy>=60)next.sleeping=false;return next;
}
