#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pp_game fresh(unsigned seed,unsigned scene){pp_game g;pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.scene=scene;g.scenes|=1u<<scene;return g;}
static void roundtrip(pp_game *g){
 pp_tick(g,0,g->day);unsigned size=pp_save_size(g);void *data=malloc(size);assert(data);
 assert(pp_save_write(g,data,size,123));pp_game saved={0};int64_t wall=0;
 assert(pp_save_read(&saved,data,size,&wall)&&wall==123&&pp_checksum(g)==pp_checksum(&saved));
 pp_dispose(&saved);free(data);
}
int main(void){
 bool families[PP_SPECIES]={0};unsigned min=700,max=400;uint64_t sum=0;
 for(unsigned scene=0;scene<12;scene++){
  bool seen[PP_SPECIES]={0};
  for(unsigned seed=1;seed<=512;seed++){
   pp_game g=fresh(seed,scene);pp_wild_scene(&g,true);unsigned target=pp_wild_steps_remaining(&g);
   assert(target>=400&&target<=700);if(target<min)min=target;if(target>max)max=target;sum+=target;
   pp_record_steps(&g,target-1);pp_wild_scene(&g,true);assert(!g.wild.phase&&pp_wild_steps_remaining(&g)==1);
   pp_record_steps(&g,1);pp_wild_scene(&g,false);assert(!g.wild.phase);
   pp_wild_scene(&g,true);assert(g.wild.phase==PP_WILD_PENDING&&pp_valid(&g));
   assert(pp_habitat_has(scene,pp_pokedex[g.wild.dex].species));
   assert(!pp_wild_opened(&g)&&pp_wild_alert_remaining(&g)==180);
   unsigned level=g.growth[0].level;assert(g.wild.level<=level);
   pp_tick(&g,59,0);assert(g.wild.phase==PP_WILD_PENDING&&!pp_wild_opened(&g));
   assert(pp_wild_reveal(&g)==PP_OK&&pp_wild_opened(&g));pp_wild_dismiss(&g);
   assert(pp_wild_steps_remaining(&g)>=400);
   /* The same purchased ordinary egg, naturally or with a boost, stays local. */
   g.inventory[5]=1;g.hatch_boosts=1;assert(pp_use(&g,5,0)==PP_OK);
   if(seed&1)assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK);
   else {assert(pp_shake(&g)==PP_OK);pp_tick(&g,(unsigned)(g.egg_end-g.seconds),0);}
   assert(g.count==2&&!g.growth[1].stage&&pp_habitat_has(scene,g.pets[1].species));
   assert(g.pets[1].species!=1);seen[g.pets[1].species]=families[g.pets[1].species]=true;
   pp_dispose(&g);
  }
  for(unsigned s=0;s<PP_SPECIES;s++)if(s!=1)assert(seen[s]==pp_habitat_has(scene,s));
 }
 assert(min<=402&&max>=698&&sum/6144>500&&sum/6144<600);
 for(unsigned d=0;d<PP_DEX_COUNT;d++)if(!pp_pokedex[d].stage&&pp_pokedex[d].species!=1)
  assert(families[pp_pokedex[d].species]==!pp_is_legendary(pp_pokedex[d].national_id));
 /* Exhausting a local pool permits local duplicates, never global outsiders. */
 pp_game local=fresh(123,0);bool owned[PP_SPECIES]={0};owned[1]=true;
 for(unsigned i=0;i<20;i++){
  local.inventory[5]=local.hatch_boosts=1;assert(pp_use(&local,5,0)==PP_OK&&pp_use(&local,PP_HATCH_BOOST,0)==PP_OK);
  pp_stored_pet p;if(local.count==5&&pp_warehouse_count(&local)){assert(pp_warehouse_get(&local,pp_warehouse_count(&local)-1,&p));}
  else {p.pet=local.pets[local.count-1];}
  assert(pp_habitat_has(0,p.pet.species));owned[p.pet.species]=true;
 }
 for(unsigned s=0;s<PP_SPECIES;s++)assert(owned[s]==pp_habitat_has(0,s));pp_dispose(&local);
 /* Pool selection uses the hatch location rather than the egg placement map. */
 for(unsigned boost=0;boost<2;boost++){
  pp_game egg=fresh(9,0);egg.inventory[5]=egg.hatch_boosts=1;assert(pp_use(&egg,5,0)==PP_OK);
  egg.scenes|=4;assert(pp_scene(&egg,2)==PP_OK);
  if(boost)assert(pp_use(&egg,PP_HATCH_BOOST,0)==PP_OK);
  else {assert(pp_shake(&egg)==PP_OK);pp_tick(&egg,(unsigned)(egg.egg_end-egg.seconds),0);}
  assert(egg.count==2&&pp_habitat_has(2,egg.pets[1].species));pp_dispose(&egg);
 }
 /* Menus, date rollover, restart and map changes cannot reroll progress. */
 pp_game g=fresh(42,0);pp_wild_scene(&g,true);unsigned target=pp_wild_steps_remaining(&g);
 pp_record_steps(&g,100);assert(pp_wild_steps_remaining(&g)==target-100);roundtrip(&g);
 pp_tick(&g,0,20261026);pp_tick(&g,0,20261027);assert(pp_wild_steps_remaining(&g)==target-100);
 g.scenes=8191;assert(pp_scene(&g,12)==PP_OK);pp_record_steps(&g,10000);pp_wild_scene(&g,true);
 assert(!g.wild.phase&&pp_wild_steps_remaining(&g)==target-100);
 assert(pp_scene(&g,2)==PP_OK);pp_record_steps(&g,2500);pp_wild_scene(&g,true);roundtrip(&g);
 assert(pp_wild_alert_remaining(&g));uint32_t rng=g.rng;pp_record_steps(&g,10000);assert(g.wild_queue==3);(void)rng;
 assert(pp_wild_reveal(&g)==PP_OK);roundtrip(&g);pp_tick(&g,120,0);assert(pp_wild_opened(&g));pp_wild_dismiss(&g);
 assert(pp_wild_steps_remaining(&g)>=400);pp_wild_scene(&g,true);assert(!g.wild.phase);
 pp_dispose(&g);puts("PASS: all local habitat pools, 400-700 steps, exact forms, eggs, exclusion and queue persistence");
}
