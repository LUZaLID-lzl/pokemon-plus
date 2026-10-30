#include "training.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned next(unsigned x){x^=x<<13;x^=x>>17;x^=x<<5;return x?x:1;}
static void init(pp_game *g){pp_init(g,1);assert(pp_choose_starter(g,1)==PP_OK);g->pets[0].food=g->pets[0].clean=g->pets[0].mood=70;g->pets[0].energy=50;}
int main(int argc,char **argv){
 bool forms[152]={0};
 for(unsigned scene=0;scene<6;scene++){
  unsigned count=pp_training_pool_count(scene);assert(count>=24&&count<=PP_TRAINING_POOL);bool sampled[152]={0};
  for(unsigned n=0;n<count;n++){unsigned national=pp_training_scenes[scene].dex[n];assert(national&&national<=149&&national!=144&&national!=145&&national!=146&&!sampled[national]);sampled[national]=true;forms[national]=true;}
  memset(sampled,0,sizeof(sampled));
  for(unsigned seed=1;seed<=300;seed++){
   pp_game g={0};init(&g);pp_training t;pp_training_init(&t,seed);unsigned pick=0;assert(pp_training_start(&t,&g,scene,&pick,1)==PP_OK);
   while(t.phase==PP_TRAINING_SEARCH)pp_training_advance(&t,&g,250);
   unsigned old=t.foes[0].art,national=0;for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].art==old)national=pp_pokedex[d].national_id;assert(national);sampled[national]=true;
   t.phase=PP_TRAINING_RESULT;t.awarded=true;t.rounds=1;pp_training_resume(&t);
   while(t.phase==PP_TRAINING_SEARCH)pp_training_advance(&t,&g,250);
   assert(t.foes[0].art!=old);pp_training_close(&t);pp_dispose(&g);
  }
  for(unsigned n=0;n<count;n++)assert(sampled[pp_training_scenes[scene].dex[n]]);
 }
 unsigned total=0;for(unsigned n=1;n<=151;n++)total+=forms[n];assert(total==146);
 for(unsigned kind=0;kind<4;kind++){
  pp_game g={0};init(&g);pp_training t;pp_training_init(&t,1);unsigned pick=0;assert(pp_training_job_start(&t,&g,0,&pick,1)==PP_OK);
  unsigned seed=1;while((next(seed)%100)/10!=6+kind)seed++;
  g.training.loot_rng=seed;g.training.elapsed_ms=g.training.ambient_ms=299960;g.training.loot_checks=4;
  size_t bytes=pp_save_size(&g);void *blob=malloc(bytes);assert(pp_save_write(&g,blob,bytes,0));
  pp_pet before=g.pets[0];unsigned coins=g.coins;pp_training_job_advance(&t,&g,40);
  assert(g.training.loot_checks==5&&g.coins==coins&&!g.training.total_xp);
  uint8_t *v=kind==0?&before.energy:kind==1?&before.food:kind==2?&before.clean:&before.mood;*v+=2;
  assert(!memcmp(&before,&g.pets[0],sizeof(before)));uint32_t checksum=pp_checksum(&g);pp_training_job_advance(&t,&g,0);assert(checksum==pp_checksum(&g));
  pp_game restored={0};int64_t wall;assert(pp_save_read(&restored,blob,bytes,&wall));pp_training replay;pp_training_init(&replay,1);assert(pp_training_restore(&replay,&restored));pp_training_job_advance(&replay,&restored,40);assert(!memcmp(&g.pets[0],&restored.pets[0],sizeof(pp_pet)));free(blob);pp_training_close(&replay);pp_dispose(&restored);pp_training_close(&t);pp_dispose(&g);
 }
 if(argc>1&&!strcmp(argv[1],"--events")){
  for(unsigned scene=0;scene<6;scene++)for(unsigned minute=0;minute<=60;minute++){
   pp_training_record r={.scene=scene,.elapsed_ms=minute*60000,.loot_checks=minute,.loot_rng=60+minute%40};char text[96];pp_training_discovery(&r,text,sizeof(text));puts(text);
  }
 }else puts("PASS: 146 ordinary forms, all regional rosters sampled, no consecutive foe repeat; four care events capped, no XP/coins, exactly-once save/replay");
 return 0;
}
