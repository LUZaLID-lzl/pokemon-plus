#include "hospital.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static pp_game g,copy;
static void win(pp_hospital *h){
 pp_hospital_start(h);
 while(h->phase==PP_HOSPITAL_PLAY){pp_hospital_select(h,pp_hospital_cloud(h)?(h->lane+1)%3:h->lane);pp_hospital_advance(h,PP_HOSPITAL_SLOT_MS);}
 assert(h->phase==PP_HOSPITAL_WON&&h->score==6);
}
static void trace(void){
 for(unsigned seed=1;seed<=25;seed++){
  pp_hospital h;pp_hospital_init(&h,seed);pp_hospital_start(&h);
  const unsigned steps[]={0,17,33,1001,511,200,400,1300};
  for(unsigned n=0;n<100&&h.phase==PP_HOSPITAL_PLAY;n++){
   pp_hospital_select(&h,(n+seed)%3);pp_hospital_advance(&h,steps[n%8]);
   printf("[%u,%u,%u,%u,%u,%u,%u,%u,%u]\n",seed,h.elapsed,h.phase,h.score,h.slot,h.lane,h.basket,h.rng,h.resolved);
  }
  win(&h);pp_game *roster=calloc(1,sizeof(*roster));assert(roster);roster->count=5;roster->traveler=1;
  assert(pp_hospital_claim(&h,roster)==PP_OK);
  printf("[%u,%u",seed,h.rng);for(unsigned i=0;i<5;i++)for(unsigned j=0;j<5;j++)printf(",%u",h.restored[i][j]);puts("]");free(roster);
 }
}
int main(int argc,char **argv){
 (void)argv;if(argc>1){trace();return 0;}
 pp_init(&g,42);g.coins=5000;assert(pp_choose_starter(&g,1)==PP_OK);for(unsigned i=0;i<6;i++)assert(pp_test_hatch(&g)==PP_OK);
 pp_hospital h;pp_hospital_init(&h,42);assert(pp_hospital_claim(&h,&g)==PP_NOT_READY);pp_hospital_start(&h);
 pp_hospital_select(&h,9);assert(h.basket==1);pp_hospital_select(&h,h.lane);pp_hospital_advance(&h,PP_HOSPITAL_FALL_MS-1);assert(!h.score);pp_hospital_advance(&h,1);assert(h.score==1);pp_hospital_advance(&h,PP_HOSPITAL_SLOT_MS-PP_HOSPITAL_FALL_MS-1);assert(h.score==1);pp_hospital_advance(&h,1);assert(h.slot==1);
 pp_hospital_init(&h,9);pp_hospital_start(&h);while(h.phase==PP_HOSPITAL_PLAY){pp_hospital_select(&h,pp_hospital_cloud(&h)?h.lane:(h.lane+1)%3);pp_hospital_advance(&h,PP_HOSPITAL_SLOT_MS);assert(!h.score);}assert(h.phase==PP_HOSPITAL_MISSED);assert(pp_hospital_claim(&h,&g)==PP_NOT_READY);
 pp_hospital_start(&h);pp_hospital_advance(&h,UINT32_MAX);assert(h.phase!=PP_HOSPITAL_PLAY&&h.elapsed==PP_HOSPITAL_ROUNDS*PP_HOSPITAL_SLOT_MS);
 unsigned seen=0;for(unsigned seed=1;seed<=100;seed++){
  for(unsigned i=0;i<g.count;i++){g.pets[i].food=1;g.pets[i].clean=2;g.pets[i].health=3;g.pets[i].mood=4;g.pets[i].energy=5;g.pets[i].sleeping=1;}
  g.pets[0].food=99;g.traveler=1;pp_pet traveler=g.pets[1];pp_stored_pet stored;assert(pp_warehouse_get(&g,0,&stored));
  pp_hospital_init(&h,seed);win(&h);unsigned rev=g.revision,coins=g.coins,rng=g.rng;
  g.battling=1;uint32_t hrng=h.rng;assert(pp_hospital_claim(&h,&g)==PP_BUSY&&!h.claimed&&h.rng==hrng);g.battling=0;
  assert(pp_hospital_claim(&h,&g)==PP_OK&&h.treated==g.count-1&&g.revision==rev+1&&g.coins==coins&&g.rng==rng);
  assert(!memcmp(&traveler,&g.pets[1],sizeof(traveler)));pp_stored_pet check;assert(pp_warehouse_get(&g,0,&check)&&!memcmp(&stored,&check,sizeof(stored)));
  for(unsigned i=0;i<g.count;i++)if(i!=1){uint8_t values[]={g.pets[i].food,g.pets[i].clean,g.pets[i].health,g.pets[i].mood,g.pets[i].energy};for(unsigned j=0;j<5;j++){if(i==0&&!j){assert(values[j]==99);continue;}assert(values[j]==(j==2?60:j+1));}assert(g.pets[i].sleeping);seen|=1;}

  assert(pp_hospital_claim(&h,&g)==PP_NOT_READY&&g.revision==rev+1);
 }
 assert(seen==1);g.traveler=-1;unsigned size=pp_save_size(&g);void *data=malloc(size);assert(data&&pp_valid(&g)&&pp_save_write(&g,data,size,1790000000));assert(pp_save_read(&copy,data,size,NULL));free(data);for(unsigned i=0;i<g.count;i++)assert(!memcmp(&g.pets[i],&copy.pets[i],sizeof(pp_pet)));
 g.count=1;g.traveler=0;pp_hospital_init(&h,1);win(&h);assert(pp_hospital_claim(&h,&g)==PP_NOT_READY&&!h.claimed);
 pp_dispose(&g);pp_dispose(&copy);puts("PASS: hospital real-time catch/cloud/miss, bounded time, 80-90 variety, no downgrade, exclusions, claim once, save v15");
}
