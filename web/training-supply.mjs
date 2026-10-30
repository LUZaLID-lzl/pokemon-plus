import {balance} from './balance-config.mjs?v=autosupply1-trainingxp2-shoplottery1';

// Pure care calculation; the caller commits recovery, stock and job together.
export function trainingSupply(pet,inventory,temperance=false){
 const fields=['food','clean','health','mood','energy'],p={...pet},used=Array(9).fill(0);
 const thresholds=balance.training.autoSupplyThresholds;
 const recover=v=>v+(temperance?Math.ceil(v*15/100):0);
 for(let item=0;item<5;item++){
  if(item===balance.shop.retiredItem)continue;
  while((p[fields[item]]<thresholds[item]||(item===4&&p.sleeping))&&used[item]<(inventory[item]||0)){
   used[item]++;
   balance.care[item].forEach((v,i)=>p[fields[i]]=Math.min(100,p[fields[i]]+recover(v)));
   if(item===4&&p.energy>=60)p.sleeping=false;
  }
 }
 if((p.sleeping||fields.some((f,i)=>p[f]<thresholds[i]))&&inventory[8]>0){
  used[8]++;for(const f of fields)p[f]=100;p.sleeping=false;
 }
 const changes=Object.fromEntries([...fields,'sleeping'].filter(f=>p[f]!==pet[f]).map(f=>[f,p[f]]));
 return {changes,used};
}
