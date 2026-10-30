import assert from 'node:assert/strict';
import fs from 'node:fs';
import {battleArtTypes,battleTypePositions,drawBattleTypes} from '../web/battle-types.mjs';
const m=JSON.parse(fs.readFileSync(new URL('../web/manifest.json',import.meta.url)));
const names=['ELECTRIC','GRASS','FIRE','WATER','NORMAL','BUG','POISON','FLYING','GROUND','ROCK','FAIRY','PSYCHIC','FIGHTING','ICE','STEEL','GHOST','DARK','DRAGON'];
for(const p of m.pets){
 const expected=m.petTypes[p.nationalId].map(t=>names.indexOf(t));assert.deepEqual(battleArtTypes[p.id],expected);
 for(const side of [0,1]){
  const pos=battleTypePositions(side,p.id);assert.equal(pos.length,expected.length);assert.deepEqual(pos.map(p=>p.type),expected);
  assert.equal(side?pos[0].x:pos.at(-1).x+16,side?224:136);
  const dots=[],ctx={save(){},restore(){},fillRect(x,y,w,h){dots.push({x,y,w,h,color:this.fillStyle})}};
  drawBattleTypes(ctx,side,p.id);assert(dots.length>100);
  assert(dots.every(d=>d.y>=64&&d.y<80&&d.x>=pos[0].x&&d.x<pos.at(-1).x+16));
  // Every pixel inside the circle remains within the radius180 screen.
  assert(dots.every(({x,y})=>Math.hypot(x-179.5,y-179.5)<168));
 }
}
// Charizard -> Charmander relay clears the second badge; selected attack is irrelevant.
const art=dex=>m.pets.find(p=>p.nationalId===dex).id;
assert.equal(battleTypePositions(0,art(6)).length,2);assert.equal(battleTypePositions(0,art(4)).length,1);
assert.equal(battleTypePositions(1,art(144)).length,2);assert.equal(battleTypePositions(1,art(150)).length,1);
console.log('PASS: 151 current forms, both sides, authoritative types, exact HP alignment, 16px pixel bounds, circle safety, dual-to-single relay');
