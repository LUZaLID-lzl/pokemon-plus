#include "training.h"
#include <stdio.h>
int main(void){
 for(unsigned scene=0;scene<6;scene++)for(unsigned count=1;count<=3;count++){
  pp_game g={0};g.count=3;g.starter_chosen=1;g.traveler=-1;
  for(unsigned i=0;i<3;i++){g.pets[i]=(pp_pet){.species=i+1,.health=90,.food=90,.clean=90,.mood=90,.energy=100};g.growth[i].level=12;for(unsigned k=0;k<6;k++)g.identity[i].iv[k]=15;}
  pp_training t;pp_training_init(&t,123);unsigned picks[]={0,1,2};pp_training_start(&t,&g,scene,picks,count);
  for(unsigned tick=0;tick<4000;tick++){
   pp_training_advance(&t,&g,250);
   if(t.phase==PP_TRAINING_RESULT){pp_training_claim(&t,&g);printf("[%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u]\n",scene,count,tick,t.rng,t.foes[0].art,t.foes[0].level,t.battle.gym.ally,t.battle.gym.enemy,t.battle.battle_round,t.battle.battle_hp,t.battle.enemy_hp,t.battle.gym.result,t.last_xp);break;}
  }
  pp_training_close(&t);
 }
 return 0;
}
