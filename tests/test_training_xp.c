#include "training.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed pre-round inputs: expected micro-XP includes only the NEW reward,
 * before saved fractional carry. No care/gap scaling is inferred from code. */
static const struct {unsigned level,enemy,food,mood,xp,micro;} cases[]={
 {20,20,50,50,0,14000000}, {20,20,90,90,0,15400000},
 {20,20,30,30,0,10080000}, {20,9,50,50,0,1200000},
 {20,11,50,50,0,4000000}, {20,60,50,50,0,14000000},
 {59,60,50,50,1179,38000000}, {60,60,50,50,0,38000000},
 {1,1,50,50,0,2000000}
};
int main(int argc,char **argv){
 unsigned tested=0;
 for(unsigned c=0;c<sizeof(cases)/sizeof(cases[0]);c++)for(unsigned count=1;count<=3;count++)for(unsigned outcome=0;outcome<=count+1;outcome++){
  bool win=outcome==count+1;unsigned defeated=win?count:outcome;
  pp_game g={0};pp_init(&g,123);assert(pp_choose_starter(&g,1)==PP_OK);g.count=3;
  for(unsigned p=0;p<3;p++){
   g.pets[p]=(pp_pet){.species=p+1,.health=90,.food=cases[c].food,.clean=90,.mood=cases[c].mood,.energy=100};
   g.growth[p]=(pp_growth){.level=cases[c].level,.xp=cases[c].xp};
   g.pet_balance[p].xp_fraction=350000+p*100000;
  }
  pp_training t;pp_training_init(&t,456);unsigned picks[]={0,1,2};assert(pp_training_start(&t,&g,0,picks,count)==PP_OK);
  t.phase=PP_TRAINING_RESULT;t.battle.gym.result=win?1:2;
  for(unsigned p=0;p<count;p++){t.foes[p].level=cases[c].enemy;t.battle.gym.enemy_hp[p]=p<defeated?0:1;t.battle.gym.hp[p]=win?1:0;}
  assert(pp_training_claim(&t,&g)==PP_OK);unsigned total=0;
  for(unsigned p=0;p<count;p++){
   unsigned micro=win?cases[c].micro:cases[c].micro*defeated/(5*count);
   micro+=350000+p*100000;
   unsigned before=10*cases[c].level*(cases[c].level-1)+cases[c].xp,expected=micro/1000000;
   if(expected>35400-before)expected=35400-before;
   unsigned actual=10*g.growth[p].level*(g.growth[p].level-1)+g.growth[p].xp-before;
   assert(actual==expected);assert(g.pet_balance[p].xp_fraction==(g.growth[p].level==60?0:micro%1000000));
   assert(g.pets[p].energy==99&&g.pets[p].health==(win?90:89)&&g.pets[p].food==cases[c].food);total+=actual;
  }
  assert(t.last_xp==total&&t.total_xp==total&&t.rounds==1);
  uint32_t checksum=pp_checksum(&g);assert(pp_training_claim(&t,&g)==PP_OK&&pp_checksum(&g)==checksum);
  unsigned bytes=pp_save_size(&g);void *blob=malloc(bytes);assert(blob&&pp_save_write(&g,blob,bytes,123));
  pp_game copy={0};assert(pp_save_read(&copy,blob,bytes,NULL));free(blob);
  assert(!memcmp(copy.growth,g.growth,sizeof(g.growth))&&!memcmp(copy.pet_balance,g.pet_balance,sizeof(g.pet_balance)));
  if(argc>1&&!strcmp(argv[1],"--trace")){
   printf("[%u,%u,%u,%u,%u,[",c,count,win,defeated,total);
   for(unsigned p=0;p<count;p++)printf("%s[%u,%u,%u,%u,%u]",p?",":"",g.growth[p].level,g.growth[p].xp,g.pet_balance[p].xp_fraction,g.pets[p].energy,g.pets[p].health);
   puts("]]");
  }
  pp_training_close(&t);pp_dispose(&copy);pp_dispose(&g);tested++;
 }
 if(argc==1)printf("PASS: %u actual training claims: 2x wins/partial losses, every member, care/gaps, legacy fractional carry, Lv60 cap, once-only costs/rewards and save reload\n",tested);
 return 0;
}
