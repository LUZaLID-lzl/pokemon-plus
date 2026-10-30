import assert from 'node:assert/strict';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
import {sortWarehouse,warehouseSortKey} from '../web/warehouse-sort.mjs';
for(const line of execFileSync(fileURLToPath(new URL('./build/test_warehouse_sort',import.meta.url)),['--keys'],{encoding:'utf8'}).trim().split('\n')){
 const [art,level,mode,key]=line.split(' ').map(Number);assert.equal(warehouseSortKey(level,art,mode),key);
}
const pets=Array.from({length:320},(_,i)=>({id:'pet'+i,art:i%151,level:1+i*17%60}));
for(const count of [0,1,3,5]){
 const active=pets.slice(0,count).map(p=>p.id),before=JSON.stringify(pets);
 for(const mode of [0,1,2,0]){
  const out=sortWarehouse(pets,active,mode);assert.deepEqual(out.slice(0,count),pets.slice(0,count));assert.equal(out.length,320);assert.equal(new Set(out).size,320);
  if(!mode)assert.deepEqual(out,pets);
  else for(let i=count+1;i<320;i++){
   const a=out[i-1],b=out[i],ak=warehouseSortKey(a.level,a.art,mode),bk=warehouseSortKey(b.level,b.art,mode);
   assert(ak<bk||ak===bk&&pets.indexOf(a)<pets.indexOf(b));
  }
  assert.equal(JSON.stringify(pets),before);
 }
}
assert.deepEqual(sortWarehouse([],[],2),[]);
console.log('PASS: 18120 native/Web keys, stable level/type/default ordering, scene prefix and no mutation');
