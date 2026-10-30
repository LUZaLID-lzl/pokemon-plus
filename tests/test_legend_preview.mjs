import {readFileSync} from 'node:fs';
import assert from 'node:assert/strict';
import {createLegendPreview,isLegend,legendIds} from '../web/legend-preview.mjs';
const p=createLegendPreview();
assert.equal(p.file(),'legend-locked.png');
for(const phase of ['cleared','party','battle','won','lost','bag','egg','hatched'])assert.throws(()=>p.show(phase));
assert.throws(()=>p.select(5));assert.throws(()=>p.select(-1));
p.toggle();assert.equal(p.file(),'legend-list.png');
for(let i=0;i<5;i++){
 p.select(i);assert.equal(p.file(),`legend-${i}-detail.png`);
 for(const phase of ['party','battle','won','lost','egg','hatched']){p.show(phase);assert.match(p.file(),new RegExp(`legend-${i}-${phase}`))}
 p.show('battle');assert.equal(p.file(true),`legend-${i}-battle.png`);
}
p.show('cleared');assert.equal(p.file(),'legend-cleared.png');
p.show('bag');assert.equal(p.file(),'legend-bag.png');p.toggle();assert.equal(p.phase,'list');assert.equal(p.unlocked,false);
for(let id=1;id<=151;id++)assert.equal(isLegend(id),legendIds.includes(id));
assert.equal(isLegend(59),false);assert.equal(isLegend(149),false);
console.log('PASS: legendary screenshot scenario gate, all targets, exact hatch mapping, reduced motion and ordinary-pool classification');

const manifest=JSON.parse(readFileSync(new URL("../web/manifest.json",import.meta.url)));
assert.deepEqual(manifest.legendaryChallenges.pets.map(p=>p.nationalId),legendIds);
assert.equal(manifest.egg.ordinaryFamilies,74);
assert.equal(manifest.pets.filter(p=>p.acquisition==="legendary-egg").length,5);
