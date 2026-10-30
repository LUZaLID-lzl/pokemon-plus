#include "training.h"
#include "hatch_fixture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned experience(pp_growth p){return 10u*p.level*(p.level-1)+p.xp;}
static void finish(pp_training *t,pp_game *g){for(unsigned n=0;n<4000&&t->phase!=PP_TRAINING_RESULT;n++)pp_training_advance(t,g,250);assert(t->phase==PP_TRAINING_RESULT);}
int main(int argc,char **argv){
 for(unsigned scene=0;scene<6;scene++){assert(pp_training_scenes[scene].min_level==scene*10+1);assert(pp_training_scenes[scene].max_level==(scene+1)*10);}
 pp_game g;pp_init(&g,1234);g.coins=100000;assert(pp_choose_starter(&g,1)==PP_OK);
 for(unsigned n=0;n<20;n++)assert(pp_test_hatch(&g)==PP_OK);
 assert(g.overflow_count>0);
 for(unsigned i=0;i<g.count;i++){g.growth[i].level=30;g.pets[i].health=90;g.pets[i].food=g.pets[i].clean=g.pets[i].mood=90;g.pets[i].energy=100;g.pets[i].sleeping=0;}
 for(unsigned i=0;i<g.reserve_count;i++){g.reserve_growth[i].level=30;g.reserves[i].sleeping=0;}
 for(unsigned i=0;i<g.overflow_count;i++){g.overflow[i].growth.level=30;g.overflow[i].pet.sleeping=0;}
 pp_training t;pp_training_init(&t,77);unsigned picks[]={0,1,2};
 /* Enemy must exist before touch; 1799ms is still approaching. */
 assert(pp_training_start(&t,&g,0,picks,3)==PP_OK);
 while(t.phase==PP_TRAINING_SEARCH)pp_training_advance(&t,&g,250);
 assert(t.phase==PP_TRAINING_APPROACH&&!t.battle.battling);
 uint32_t prepared_rng=t.rng;unsigned seen=t.foes[0].art,revision_before=g.revision;
 for(unsigned k=0;k<7;k++)pp_training_advance(&t,&g,250);
 pp_training_advance(&t,&g,49);assert(t.clock_ms==1799&&t.phase==PP_TRAINING_APPROACH);
 pp_training_advance(&t,&g,1);assert(t.phase==PP_TRAINING_ALERT&&t.clock_ms==0);
 assert(t.foes[0].art==seen&&t.rng==prepared_rng&&g.revision==revision_before);
 for(unsigned k=0;k<8;k++)pp_training_advance(&t,&g,250);
 assert(t.phase==PP_TRAINING_FIGHT&&t.foes[0].art==seen&&t.rng==prepared_rng);
 /* Same real-time round boundary as gyms, despite faster presentation. */
 for(unsigned k=0;k<5;k++)pp_training_advance(&t,&g,250);
 pp_training_advance(&t,&g,83);assert(t.battle.battle_round==0);
 pp_training_advance(&t,&g,1);assert(t.battle.battle_round==1);
 pp_training_close(&t);
 uint32_t rng=g.rng,coins=g.coins;uint64_t badges=g.gym.badges;unsigned count=g.count,stored=pp_warehouse_count(&g);
 for(unsigned scene=0;scene<6;scene++)for(unsigned size=1;size<=3;size++){
  pp_stored_pet before[3];for(unsigned i=0;i<size;i++)assert(pp_training_pet(&g,picks[i],&before[i]));
  assert(pp_training_start(&t,&g,scene,picks,size)==PP_OK);unsigned limit=0;
  while((t.phase==PP_TRAINING_SEARCH||t.phase==PP_TRAINING_APPROACH)&&limit++<90)pp_training_advance(&t,&g,250);
  assert(t.phase==PP_TRAINING_ALERT&&t.clock_ms==0&&!t.battle.battling);
  uint32_t alert_rng=t.rng;unsigned revision=g.revision;
  for(unsigned n=0;n<7;n++)pp_training_advance(&t,&g,250);
  pp_training_advance(&t,&g,249);assert(t.phase==PP_TRAINING_ALERT&&t.clock_ms==1999&&t.rng==alert_rng&&g.revision==revision&&!t.battle.battling);
  assert(pp_training_claim(&t,&g)==PP_NOT_READY);
  pp_training_advance(&t,&g,1);
  assert(t.phase==PP_TRAINING_FIGHT&&t.count==size&&pp_battle_team_count(&t.battle)==size);
  for(unsigned i=0;i<size;i++){
   const pp_gym_enemy *f=&t.foes[i];assert(f->level>=pp_training_scenes[scene].min_level&&f->level<=pp_training_scenes[scene].max_level);
   unsigned match=0;for(unsigned j=0;j<PP_DEX_COUNT;j++)if(pp_pokedex[j].art==f->art)for(unsigned k=0;k<pp_training_pool_count(scene);k++)match|=pp_pokedex[j].national_id==pp_training_scenes[scene].dex[k];assert(match);
   for(unsigned j=0;j<i;j++)assert(t.foes[j].art!=f->art);
  }
  assert(pp_training_claim(&t,&g)==PP_NOT_READY);finish(&t,&g);assert(pp_training_claim(&t,&g)==PP_OK);
  unsigned totalXp=0;unsigned rev=g.revision;assert(pp_training_claim(&t,&g)==PP_OK&&g.revision==rev);
  for(unsigned i=0;i<size;i++){pp_stored_pet after;assert(pp_training_pet(&g,picks[i],&after));totalXp+=experience(after.growth)-experience(before[i].growth);assert(!memcmp(&before[i].identity,&after.identity,sizeof(pp_identity)));assert(after.pet.energy==before[i].pet.energy-1);}
  assert(totalXp==t.last_xp);
  if(argc>1&&!strcmp(argv[1],"--trace"))printf("[%u,%u,%u,%u,%u,%u]\n",scene,size,t.foes[0].level,t.foes[0].art,t.battle.gym.result,t.last_xp);
  pp_training_resume(&t);assert((t.phase==PP_TRAINING_SEARCH||t.phase==PP_TRAINING_APPROACH));finish(&t,&g);assert(pp_training_claim(&t,&g)==PP_OK&&t.rounds==2);
  pp_training_close(&t);assert(!pp_battle_encounter(&t.battle));
 }
 assert(g.rng==rng&&g.coins==coins&&g.gym.badges==badges&&g.count==count&&pp_warehouse_count(&g)==stored);
 assert(pp_training_start(&t,&g,6,picks,1)==PP_INVALID);assert(pp_training_start(&t,&g,0,picks,0)==PP_INVALID);assert(pp_training_start(&t,&g,0,picks,4)==PP_INVALID);
 unsigned dupe[]={0,0};assert(pp_training_start(&t,&g,0,dupe,2)==PP_INVALID);g.traveler=0;assert(pp_training_start(&t,&g,0,picks,1)==PP_BUSY);g.traveler=-1;
 g.pets[0].sleeping=1;assert(pp_training_start(&t,&g,0,picks,1)==PP_NOT_READY);g.pets[0].sleeping=0;
 assert(pp_training_start(&t,&g,0,picks,1)==PP_OK);unsigned xp=experience(g.growth[0]);while((t.phase==PP_TRAINING_SEARCH||t.phase==PP_TRAINING_APPROACH))pp_training_advance(&t,&g,250);assert(t.phase==PP_TRAINING_ALERT);pp_training_close(&t);pp_training_advance(&t,&g,250);assert(t.phase==PP_TRAINING_SELECT);assert(experience(g.growth[0])==xp);
 g.growth[0].level=PP_MAX_LEVEL;g.growth[0].xp=0;assert(pp_roster_gain_xp(&g,false,0,900)==PP_OK&&g.growth[0].xp==0&&g.growth[0].level==PP_MAX_LEVEL);
 unsigned bytes=pp_save_size(&g);void *data=malloc(bytes);assert(data&&pp_save_write(&g,data,bytes,0));pp_game loaded={0};assert(pp_save_read(&loaded,data,bytes,NULL));assert(loaded.growth[1].level==g.growth[1].level);pp_stored_pet a,b;assert(pp_warehouse_get(&g,PP_RESERVES,&a)&&pp_warehouse_get(&loaded,PP_RESERVES,&b)&&a.growth.level==b.growth.level&&a.growth.xp==b.growth.xp);
 free(data);pp_dispose(&loaded);pp_dispose(&g);puts("PASS: six training pools x 1-3 partners, finite encounters, repeated battles, all-member equal XP once including dynamic warehouse, no identity/care/currency/badges mutation, invalid/busy/sleep/cancel/max-level and v15 persistence");return 0;
}
