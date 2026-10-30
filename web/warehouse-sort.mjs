import {battleArtTypes} from './battle-types.mjs?v=petinfotypes1';
export const warehouseSortLabels=['原序','等级','属性'];
const ranks=[3,4,1,2,0,11,7,9,8,12,17,10,6,5,16,13,15,14];
export function warehouseSortKey(level,art,mode){
 const descending=60-Math.min(60,level);
 if(mode!==2||!battleArtTypes[art])return descending;
 const [first,second]=battleArtTypes[art];
 return (ranks[first]*19+(second===undefined?0:1+ranks[second]))*61+descending;
}
export function sortWarehouse(pets,active,mode){
 if(!mode)return pets;
 const ids=new Set(active),front=pets.filter(p=>ids.has(p.id));
 const stored=pets.map((pet,slot)=>({pet,slot})).filter(x=>!ids.has(x.pet.id));
 stored.sort((a,b)=>warehouseSortKey(a.pet.level,a.pet.art,mode)-warehouseSortKey(b.pet.level,b.pet.art,mode)||a.slot-b.slot);
 return [...front,...stored.map(x=>x.pet)];
}
