import assert from 'node:assert/strict';
import fs from 'node:fs';
import {habitatPets,hatchPool,encounterPool} from '../web/habitats.js';
const read=file=>JSON.parse(fs.readFileSync(new URL(file,import.meta.url)));
const data=read('../assets/pet-habitats.json'),roster=read('../assets/pet-roster.json');
assert.deepEqual(read('../web/manifest.json').habitats,data);
assert.deepEqual(read('../sdcard/pokemon-plus/manifest.json').habitats,data);
const masks=fs.readFileSync(new URL('../core/pet_habitats.h',import.meta.url),'utf8').match(/0x[0-9a-f]+/g).map(x=>parseInt(x,16));
const all=new Set();
for(const region of data.scenes){
 const eggs=habitatPets(data,roster,region.scene,{eggs:true});
 assert.deepEqual(eggs.map(p=>p.nationalId).sort((a,b)=>a-b),region.baseNationalIds);
 for(const p of roster)assert.equal(habitatPets(data,roster,region.scene).some(x=>x.id===p.id),!!(masks[p.species]&(1<<region.scene)));
 for(const p of eggs)all.add(p.species);
 const missing=hatchPool(data,roster,region.scene,[eggs[0]]);assert(!missing.some(p=>p.species===eggs[0].species));
 assert.deepEqual(hatchPool(data,roster,region.scene,eggs),eggs);
}
assert.equal(all.size,74);assert.deepEqual(habitatPets(data,roster,12),[]);
assert.equal(habitatPets(data,roster,12,{eggs:true}).length,74);
assert.deepEqual(encounterPool(data,roster,12),[]);
assert.equal(encounterPool(data,roster,2).length,data.scenes[2].baseNationalIds.length);
assert.notDeepEqual(data.scenes[2].baseNationalIds,data.scenes[6].baseNationalIds);
console.log('PASS: shared C/Web regional catalog, every evolved family, local missing-first/fallback eggs and map 13 rules');
