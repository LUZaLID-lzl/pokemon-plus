// Shared map catalog; pet forms inherit their base family's local pool.
export function habitatPets(catalog,roster,scene,{eggs=false}={}) {
 const region=catalog.scenes.find(s=>s.scene===scene);
 if(!region)return eggs?roster.filter(p=>!p.stage&&![144,145,146,150,151].includes(p.nationalId)):[];
 const families=new Set(roster.filter(p=>region.baseNationalIds.includes(p.nationalId)).map(p=>p.species));
 return roster.filter(p=>families.has(p.species)&&(!eggs||!p.stage));
}
export function hatchPool(catalog,roster,scene,pets=[]) {
 const local=habitatPets(catalog,roster,scene,{eggs:true});
 const owned=new Set(pets.map(p=>p.species));
 const missing=local.filter(p=>!owned.has(p.species));
 return missing.length?missing:local;
}
export function encounterPool(catalog,roster,scene) {
 return habitatPets(catalog,roster,scene).filter(p=>!p.stage);
}
