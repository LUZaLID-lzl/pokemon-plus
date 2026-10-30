// Browser collection sandbox: independent of device NVS and evolution gift storage.
export function collectionItems(catalog,quality=6){return [...catalog.items.slice(583),...catalog.items.slice(0,583)].filter(x=>quality===6||x.quality===quality)}
// Previewing a cleared gym does not grant a badge. Supply actual progress only.
export function ownsAchievement(id,{badges=0,ownedForms=new Set(),roster=[]}={}) {
 if(id===584)return badges===255;
 if(id===585)return roster.length===151&&roster.every(p=>ownedForms.has(p.id));
 return false;
}
export function createCollectionDemo({storage,random=()=>crypto.getRandomValues(new Uint32Array(1))[0]}){
 const key='pokemon-plus.web-travel-collection-v1';let ids=[];
 try{const saved=JSON.parse(storage.getItem(key)||'[]');if(Array.isArray(saved))ids=[...new Set(saved.filter(id=>Number.isInteger(id)&&id>=0&&id<583))]}catch{}
 return {has:id=>ids.includes(id),get ids(){return [...ids]},claim(){
  const missing=Array.from({length:583},(_,id)=>id).filter(id=>!ids.includes(id));
  if(!missing.length)return {id:null,complete:true};
  // Every unowned travel item has the same chance; never select the special gift.
  const limit=Math.floor(4294967296/missing.length)*missing.length;let roll;do{roll=random()}while(roll>=limit);
  const id=missing[roll%missing.length],next=[...ids,id];
  storage.setItem(key,JSON.stringify(next));ids=next;return {id,complete:next.length===583};
 }};
}
