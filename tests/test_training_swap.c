#include "training.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void restore(pp_training *t,pp_game *g){unsigned n=pp_save_size(g);void*b=malloc(n);assert(b&&pp_save_write(g,b,n,123));pp_training_close(t);assert(pp_save_read(g,b,n,NULL));free(b);assert(pp_training_restore(t,g));}
int main(void){
 const unsigned phases[]={PP_TRAINING_SEARCH,PP_TRAINING_APPROACH,PP_TRAINING_ALERT,PP_TRAINING_FIGHT,PP_TRAINING_RESULT};
 for(unsigned n=0;n<5;n++){
  pp_training_record expected={0};pp_growth expected_growth[3];pp_pet_balance expected_balance[3];
  for(unsigned swapped=0;swapped<2;swapped++){
  pp_game g={0};pp_init(&g,123);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=100000;
  while(g.count+pp_warehouse_count(&g)<18)assert(pp_test_hatch(&g)==PP_OK);
  for(unsigned i=0;i<g.count;i++){g.pets[i].health=g.pets[i].food=g.pets[i].clean=g.pets[i].mood=90;g.pets[i].energy=100;g.growth[i].level=12;}
  assert(pp_travel(&g,3)==PP_OK);pp_training t;pp_training_init(&t,123);unsigned picks[]={0,1,2};assert(pp_training_job_start(&t,&g,0,picks,3)==PP_OK);
  for(unsigned i=0;t.phase!=phases[n]&&i<100000;i++)pp_training_job_advance(&t,&g,40);
  assert(t.phase==phases[n]&&g.training.status==1);
  pp_identity identities[5];memcpy(identities,g.identity,sizeof identities);uint64_t end=g.travel_end;pp_stored_pet incoming,outgoing;assert(pp_warehouse_get(&g,12,&incoming));
  if(swapped){
  for(unsigned i=0;i<4;i++){uint32_t h=pp_checksum(&g);assert(pp_replace_pet(&g,i,12)==PP_BUSY&&pp_checksum(&g)==h);}
  assert(pp_fishing_partner_select(&g,12)==PP_OK);uint32_t h=pp_checksum(&g);assert(pp_replace_pet(&g,4,12)==PP_BUSY&&h==pp_checksum(&g));pp_fishing_partner_clear(&g);
  assert(pp_reserve_pet(&g,4)==PP_BUSY);
  assert(pp_replace_pet(&g,4,12)==PP_OK);assert(!memcmp(&g.identity[4],&incoming.identity,sizeof(pp_identity)));assert(pp_warehouse_get(&g,12,&outgoing)&&!memcmp(&outgoing.identity,&identities[4],sizeof(pp_identity)));
  assert(!memcmp(g.identity,identities,4*sizeof(pp_identity))&&g.traveler==3&&g.travel_end==end&&pp_valid(&g));
  }
  restore(&t,&g);
  while(g.training.status==1)pp_training_job_advance(&t,&g,60000);
  if(!swapped){expected=g.training;memcpy(expected_growth,g.growth,sizeof expected_growth);memcpy(expected_balance,g.pet_balance,sizeof expected_balance);}
  else {assert(!memcmp(&g.training,&expected,sizeof(g.training)));for(unsigned i=0;i<3;i++){assert(!memcmp(&g.growth[i],&expected_growth[i],sizeof(pp_growth)));assert(!memcmp(&g.pet_balance[i],&expected_balance[i],sizeof(pp_pet_balance)));assert(!memcmp(&g.identity[i],&identities[i],sizeof(pp_identity)));}
  assert(!memcmp(&g.growth[4],&incoming.growth,sizeof(pp_growth))&&g.traveler==3&&g.travel_end==end);assert(pp_warehouse_get(&g,12,&outgoing)&&!memcmp(&outgoing.identity,&identities[4],sizeof(pp_identity)));}
  assert(pp_valid(&g));pp_training_close(&t);pp_dispose(&g);
  }
 }
 puts("PASS: five training phases: three training + one traveling + idle fixed-slot swap, protected participants/fishing/traveler, identity/receipt/XP parity after save/reload and full hour");return 0;
}
