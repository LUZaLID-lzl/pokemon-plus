import {trainingAmbientEvents,trainingCareEvents} from './training-catalog.mjs?v=variety1';
export function trainingDiscovery(job,scene,items){
 if(job.elapsedMs<60000)return '';
 if(job.lootChecks>=5&&job.elapsedMs%300000<12000){
  if(job.lastItem)return `发现 ${items[job.lastItem-1].name} ×1`;
  if(job.lastCoins)return `发现 金币 +${job.lastCoins}`;
  const chance=job.lootRng%100;if(chance>=60)return trainingCareEvents[Math.floor((chance-60)/10)];
 }
 return trainingAmbientEvents[scene][(Math.floor(job.elapsedMs/60000)-1+scene)%6];
}
