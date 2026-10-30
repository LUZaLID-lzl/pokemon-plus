#include "training.h"
#include "legacy_game_v15.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
static void roundtrip(pp_training *t,pp_game *g){
 unsigned bytes=pp_save_size(g);void *blob=malloc(bytes);assert(blob&&pp_valid(g)&&pp_save_write(g,blob,bytes,123));
 pp_game copy={0};assert(pp_save_read(&copy,blob,bytes,NULL));free(blob);pp_training_close(t);pp_dispose(g);*g=copy;
 assert(pp_training_restore(t,g));
}
static void init(pp_game *g){
 pp_init(g,123);assert(pp_choose_starter(g,1)==PP_OK);g->count=3;
 for(unsigned i=0;i<3;i++){g->pets[i]=(pp_pet){.species=i+1,.health=90,.food=90,.clean=90,.mood=90,.energy=100};g->growth[i]=(pp_growth){.level=12};g->identity[i]=(pp_identity){0};for(unsigned k=0;k<6;k++)g->identity[i].iv[k]=15;}
}
static void legacy15(void){
 pp_game g={0};init(&g);g.coins=98765;g.tree_essences=7;g.fish_caught[2]=19;g.balls=11;
 unsigned fixed=offsetof(pp_game_v15,overflow),bytes=16+fixed;uint8_t *blob=calloc(1,bytes);assert(blob);
 int64_t wall=123456;memcpy(blob+4,&bytes,4);memcpy(blob+8,&wall,8);memcpy(blob+16,&g,fixed);unsigned version=15;memcpy(blob+20,&version,4);
 uint32_t hash=2166136261u;for(unsigned i=4;i<bytes;i++)hash=(hash^blob[i])*16777619u;memcpy(blob,&hash,4);
 pp_game copy={0};int64_t read_wall=0;assert(pp_save_read(&copy,blob,bytes,&read_wall));assert(copy.version==PP_VERSION&&read_wall==wall&&copy.coins==98765&&copy.tree_essences==7&&copy.fish_caught[2]==19&&copy.balls==11&&!copy.training.status);assert(!memcmp(copy.identity,g.identity,sizeof(g.identity)));free(blob);pp_dispose(&copy);pp_dispose(&g);
}
static void saturation(void){
 pp_game g={0};init(&g);g.growth[0].level=60;g.coins=UINT32_MAX;g.tree_essences=g.hatch_boosts=g.balls=UINT16_MAX;for(unsigned i=0;i<PP_ITEMS;i++)g.inventory[i]=pp_item_enabled(i)?UINT16_MAX:0;
 pp_training t;pp_training_init(&t,123);unsigned pick=0;assert(pp_training_job_start(&t,&g,0,&pick,1)==PP_OK);pp_training_job_advance(&t,&g,PP_TRAINING_DURATION_MS);
 assert(g.training.status==2&&g.training.coins==0&&g.coins==UINT32_MAX&&!pp_training_growth_pending(&g));for(unsigned i=0;i<PP_SHOP_ITEMS;i++)assert(g.training.items[i]==0&&pp_item_count(&g,i)==(pp_item_enabled(i)?UINT16_MAX:0));roundtrip(&t,&g);pp_training_close(&t);pp_dispose(&g);
}
static void quantum_resume(void){
 pp_training_record expected={0};pp_growth growth={0};
 for(unsigned variant=0;variant<2;variant++){
  pp_game g={0};init(&g);pp_training t;pp_training_init(&t,123);unsigned pick=0;assert(pp_training_job_start(&t,&g,0,&pick,1)==PP_OK);
  const unsigned spans[]={17,51,233,1000,59999};unsigned n=0;
  while(g.training.status==1){pp_training_job_advance(&t,&g,variant?spans[n++%5]:60000);if(variant&&n==1){assert(g.training.pending_ms==17);roundtrip(&t,&g);}if(variant&&n%101==0)roundtrip(&t,&g);}
  if(!variant){expected=g.training;growth=g.growth[0];}else{assert(!memcmp(&expected,&g.training,sizeof(expected)));assert(!memcmp(&growth,&g.growth[0],sizeof(growth)));}
  pp_training_close(&t);pp_dispose(&g);
 }
}
static void warehouse_evolution(void){
 for(unsigned mode=0;mode<6;mode++){
  pp_game g={0};init(&g);g.reserve_count=1;g.reserves[0]=g.pets[0];g.reserves[0].species=mode==0?1:mode==1?0:67;
  g.reserve_growth[0]=(pp_growth){.level=40};g.reserve_identity[0]=g.identity[0];g.reserve_identity[0].nature=7;g.reserve_pats_today[0]=13;g.coins=500;
  pp_stored_pet before,after;assert(pp_warehouse_get(&g,0,&before));pp_game preview={0};
  assert(pp_roster_evolution_preview(&g,PP_PETS,&preview)&&pp_evolution_offer(&preview,0)==PP_OK);
  unsigned branch=mode>=2&&mode<=4?mode-1:0;bool decline=mode==5;
  if(mode){g.coins=0;uint32_t checksum=pp_checksum(&g);assert(pp_roster_evolution_choose(&g,PP_PETS,false,branch)==(mode==5?PP_INVALID:PP_NO_COINS));assert(pp_checksum(&g)==checksum);g.coins=500;}
  g.fishing_partner=1;assert(pp_roster_evolution_choose(&g,PP_PETS,decline,branch)==PP_BUSY);g.fishing_partner=0;
  assert(pp_roster_evolution_choose(&g,PP_PETS,decline,branch)==PP_OK&&pp_valid(&g));
  assert(pp_warehouse_get(&g,0,&after)&&g.count==3&&g.reserve_count==1&&after.pats_today==13&&!memcmp(&after.identity,&before.identity,sizeof(before.identity)));
  assert(after.growth.stage==(decline?0:branch?branch:1)&&after.declined==decline&&g.coins==(mode==0||decline?500:300));
  if(decline){assert(pp_has(&g,PP_PURE_HEART));assert(pp_roster_evolution_choose(&g,PP_PETS,true,0)==PP_EVOLUTION_LOCKED);}
  unsigned bytes=pp_save_size(&g);void *blob=malloc(bytes);assert(blob&&pp_save_write(&g,blob,bytes,123));pp_game copy={0};assert(pp_save_read(&copy,blob,bytes,NULL));free(blob);assert(pp_warehouse_get(&copy,0,&after)&&after.declined==decline&&after.pats_today==13);pp_dispose(&copy);pp_dispose(&g);
 }
}
/* v16 keeps its byte layout; old elapsed time and earned receipts must survive. */
static void legacy_duration(void){
 for(unsigned phase=PP_TRAINING_SEARCH;phase<=PP_TRAINING_APPROACH;phase++){
  pp_game g={0};init(&g);pp_training t;pp_training_init(&t,123);unsigned pick=0;
  assert(pp_training_job_start(&t,&g,0,&pick,1)==PP_OK);
  for(unsigned i=0;t.phase!=phase&&i<10000;i++)pp_training_job_advance(&t,&g,40);
  assert(t.phase==phase);
  g.training.elapsed_ms=g.training.ambient_ms=5400000;g.training.loot_checks=90;g.training.pending_ms=17;
  unsigned coins=g.coins,xp=g.growth[0].xp,level=g.growth[0].level,rounds=g.training.rounds;
  roundtrip(&t,&g);assert(g.training.status==2&&g.training.elapsed_ms==5400000&&!g.training.pending_ms);
  assert(!pp_training_busy(&g,0)&&g.coins==coins&&g.growth[0].xp==xp&&g.growth[0].level==level&&g.training.rounds==rounds);
  roundtrip(&t,&g);pp_training_job_advance(&t,&g,UINT32_MAX);assert(g.coins==coins&&g.training.rounds==rounds);
  /* A historical completed three-hour receipt remains readable, including its loot. */
  g.training.elapsed_ms=g.training.ambient_ms=PP_TRAINING_LEGACY_DURATION_MS;g.training.loot_checks=180;
  g.training.items[0]=100;roundtrip(&t,&g);assert(g.training.status==2&&g.training.items[0]==100&&g.training.elapsed_ms==10800000);
  g.training.elapsed_ms+=40;assert(!pp_valid(&g));g.training.elapsed_ms-=40;
  pp_training_close(&t);pp_dispose(&g);
 }
}
static void early_finish(void){
 const unsigned phases[]={PP_TRAINING_SEARCH,PP_TRAINING_APPROACH,PP_TRAINING_ALERT,PP_TRAINING_FIGHT,PP_TRAINING_RESULT};
 for(unsigned n=0;n<6;n++){
  pp_game g={0};init(&g);pp_training t;pp_training_init(&t,123);unsigned pick=0;
  assert(pp_training_job_start(&t,&g,0,&pick,1)==PP_OK);
  if(n<5)for(unsigned i=0;t.phase!=phases[n]&&i<10000;i++)pp_training_job_advance(&t,&g,40);
  else pp_training_job_advance(&t,&g,600000);
  if(n<5)assert(t.phase==phases[n]);
  pp_training_job_advance(&t,&g,17);unsigned elapsed=g.training.elapsed_ms,coins=g.coins,xp=g.growth[0].xp,level=g.growth[0].level,rounds=g.training.rounds,loot=g.training.loot_checks;
  assert(g.training.pending_ms==17);pp_training_job_finish(&t,&g);
  assert(g.training.status==2&&g.training.elapsed_ms==elapsed&&!g.training.pending_ms&&!pp_training_busy(&g,0)&&!t.battle.battling&&pp_valid(&g));
  assert(g.coins==coins&&g.growth[0].xp==xp&&g.growth[0].level==level&&g.training.rounds==rounds&&g.training.loot_checks==loot&&g.training.end_levels[0]==level);
  uint32_t checksum=pp_checksum(&g);pp_training_job_finish(&t,&g);pp_training_job_advance(&t,&g,UINT32_MAX);assert(pp_checksum(&g)==checksum);
  roundtrip(&t,&g);assert(g.training.status==2&&g.training.elapsed_ms==elapsed&&g.coins==coins);
  if(n==5)assert(g.training.rounds>0&&g.training.total_xp>0);
  pp_training_close(&t);pp_dispose(&g);
 }
}
int main(int argc,char **argv){
 legacy15();legacy_duration();early_finish();saturation();quantum_resume();warehouse_evolution();
 for(unsigned initial_level=1;initial_level<=12;initial_level+=11)for(unsigned scene=0;scene<6;scene++)for(unsigned count=1;count<=3;count++){
  pp_game g={0};init(&g);for(unsigned i=0;i<3;i++)g.growth[i].level=initial_level;pp_training t;pp_training_init(&t,123);unsigned picks[]={0,1,2};uint32_t initial=g.coins;
  assert(pp_training_job_start(&t,&g,scene,picks,count)==PP_OK&&pp_valid(&g));
  assert(pp_training_busy(&g,0)&&pp_travel(&g,0)==PP_BUSY&&pp_pat(&g,0)==PP_BUSY);
  assert(pp_training_job_start(&t,&g,scene,picks,count)==PP_BUSY);
  unsigned seen=0;
  for(unsigned ms=0;ms<PP_TRAINING_DURATION_MS&&g.training.status==1;ms+=40){
   pp_training_job_advance(&t,&g,40);
   assert(g.training.elapsed_ms==ms+40);
   if(!(seen&(1u<<t.phase))){seen|=1u<<t.phase;roundtrip(&t,&g);}
   if(ms%600000==0)roundtrip(&t,&g);
  }
  assert(g.training.status==2&&!pp_training_busy(&g,0)&&!t.battle.battling&&g.training.loot_checks==g.training.elapsed_ms/60000);
  assert(g.coins==initial+g.training.coins&&g.training.rounds>0);
  unsigned coins=g.coins,rounds=g.training.rounds;pp_training_job_advance(&t,&g,UINT32_MAX);assert(g.coins==coins&&g.training.rounds==rounds);
  roundtrip(&t,&g);assert(g.training.status==2&&!pp_training_busy(&g,0));
  bool grew=false;for(unsigned i=0;i<count;i++)grew|=g.training.end_levels[i]>g.training.start_levels[i];assert(pp_training_growth_pending(&g)==grew);
  unsigned original_pick=g.training.picks[0];g.training.picks[0]=PP_PETS+pp_warehouse_count(&g);if(grew)assert(!pp_valid(&g));g.training.picks[0]=original_pick;
  for(unsigned i=0;i<count;i++)assert(g.training.start_levels[i]==initial_level&&g.training.end_levels[i]==g.growth[i].level);
  if(grew)assert(pp_training_job_start(&t,&g,scene,picks,count)==PP_BUSY);
  g.training.growth_seen=(1u<<count)-1;roundtrip(&t,&g);assert(pp_training_growth_pending(&g)==grew);
  g.training.evolution_seen=(1u<<count)-1;roundtrip(&t,&g);assert(!pp_training_growth_pending(&g));
  if(argc>1&&!strcmp(argv[1],"--trace")){
   printf("[%u,%u,%u,%u,%u,[",scene,count,g.training.rounds,g.training.total_xp,g.training.coins);
   for(unsigned i=0;i<PP_SHOP_ITEMS;i++)printf("%s%u",i?",":"",g.training.items[i]);
   printf("],%u,%u,%u,%u]\n",g.growth[0].level,g.growth[0].xp,g.training.rng,g.training.loot_rng);
  }
  pp_training_close(&t);pp_dispose(&g);
 }
 if(argc==1)puts("PASS: 36 one-hour jobs, checkpoints in every phase/repeated reload, busy protection, finite expiry, loot and XP once, stable identities and completed receipt");
 return 0;
}
