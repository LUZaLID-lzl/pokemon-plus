#include "game.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
static pp_game g;
static unsigned progress(unsigned i){return 10u*g.growth[i].level*(g.growth[i].level-1)+g.growth[i].xp;}
static unsigned expected(unsigned lv,unsigned xp){unsigned total=10*lv*(lv-1)+xp;return total>35400?35400:total;}

static const unsigned team[]={0,1,2};
static void init(unsigned seed,unsigned level,unsigned species){
 pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.count=4;g.gym.badges=3;g.hatch_boosts=2;
 for(unsigned i=0;i<4;i++){g.pets[i]=g.pets[0];g.pets[i].species=species;g.pets[i].energy=100;g.growth[i]=(pp_growth){.level=level};}assert(pp_valid(&g));
}
static void finish(void){for(unsigned i=0;i<600&&g.battling;i++){pp_tick(&g,3,0);assert(pp_valid(&g));}assert(!g.battling);}
int main(void){
 init(1,28,10);g.gym.badges=1;uint32_t before=pp_checksum(&g);
 assert(pp_gym_unlocked(&g,1)&&!pp_gym_unlocked(&g,2));
 assert(pp_gym_start_at(&g,2,team)==PP_NOT_READY&&pp_checksum(&g)==before);
 assert(pp_gym_start_at(&g,PP_GYM_COUNT,team)==PP_INVALID&&pp_checksum(&g)==before);
 g.gym.badges=4;assert(!pp_valid(&g));g.gym.badges=6;assert(!pp_valid(&g));g.gym.badges=8;assert(!pp_valid(&g));g.gym.badges=3;
 assert(pp_gym_start_at(&g,2,team)==PP_OK&&g.gym.mode==3&&pp_valid(&g));
 assert(g.enemy_max_hp==236&&g.gym.enemy_max[1]==254&&g.gym.enemy_max[2]==280);
 assert(pp_gym_current(&g)->team[0].art==99&&pp_gym_current(&g)->team[1].art==0&&pp_gym_current(&g)->team[2].art==25);
 assert(pp_battle_move(&g,1,0)->type==PP_ELECTRIC&&pp_battle_move(&g,1,1)->type==PP_NORMAL);
 assert(pp_gym_effectiveness(&g,true,0)==0&&pp_gym_damage(&g,true,0,0)==0&&pp_gym_damage(&g,true,1,0)>0);
 for(unsigned i=0;i<4;i++)if(pp_battle_move(&g,0,i)->type==PP_GROUND)assert(pp_gym_effectiveness(&g,false,i)==150);
 pp_game saved=g;g.gym.badges=1;assert(!pp_valid(&g));g=saved;
 pp_tick(&g,3,0);saved=g;before=pp_checksum(&g);g=saved;assert(pp_valid(&g)&&pp_checksum(&g)==before);
 finish();assert(g.gym.result==1&&g.gym.badges==7&&g.gym.reward_coins==600&&g.gym.first_clear&&g.hatch_boosts==2);
 assert(g.gym.reward_xp==((pp_battle_xp(&g.pets[0],28,27)*3)/1000000+6*28)&&pp_collectibles[g.gym.reward_item].quality>=2&&g.gym.reward_item!=PP_PURE_HEART&&pp_has(&g,g.gym.reward_item));
 assert(!g.growth[3].xp);for(unsigned i=0;i<3;i++)assert(progress(i)==expected(28,g.gym.reward_xp));
 unsigned coins=g.coins;pp_tick(&g,0,0);assert(g.coins==coins);pp_gym_leave(&g);assert(g.gym.badges==7);
 for(unsigned i=0;i<3;i++)g.pets[i].energy=100;
 assert(pp_gym_start_at(&g,2,team)==PP_OK);finish();assert(g.gym.result==1&&g.gym.reward_coins==30&&!g.gym.first_clear&&g.gym.wins==2);
 for(unsigned gym=0;gym<2;gym++){for(unsigned i=0;i<3;i++)g.pets[i].energy=100;assert(pp_gym_start_at(&g,gym,team)==PP_OK);finish();assert(g.gym.badges==7&&!g.gym.first_clear&&g.gym.reward_coins==(gym?0:20));}
 init(2,1,3);assert(pp_gym_start_at(&g,2,team)==PP_OK);assert(pp_gym_effectiveness(&g,true,0)==150);finish();assert(g.gym.result==2&&g.gym.badges==3&&!g.gym.reward_coins&&g.gym.reward_xp==0&&g.gym.entered==7);
 for(unsigned i=0;i<3;i++)assert(progress(i)==0);
 init(3,24,6);assert(pp_gym_start_at(&g,2,team)==PP_OK);assert(pp_gym_effectiveness(&g,true,0)==150);
 init(4,24,1);assert(pp_gym_start_at(&g,2,team)==PP_OK);assert(pp_gym_effectiveness(&g,true,0)==65);
 init(5,24,0);assert(pp_gym_start_at(&g,2,team)==PP_OK);assert(pp_gym_effectiveness(&g,true,0)==65&&pp_gym_effectiveness(&g,false,0)==65);
 for(unsigned sp=0;sp<PP_SPECIES;sp++){init(200+sp,PP_MAX_LEVEL,sp);assert(pp_gym_start_at(&g,2,team)==PP_OK);finish();for(unsigned i=0;i<3;i++)assert(g.growth[i].level==PP_MAX_LEVEL&&!g.growth[i].xp);}
 for(unsigned level=24;level<=28;level+=2){
  unsigned ground=0,grass=0,normal=0,water=0;
  for(unsigned seed=1;seed<=100;seed++){
   init(seed,level,10);assert(pp_gym_start_at(&g,2,team)==PP_OK);finish();ground+=g.gym.result==1;
   init(seed,level,1);assert(pp_gym_start_at(&g,2,team)==PP_OK);finish();grass+=g.gym.result==1;
   init(seed,level,7);assert(pp_gym_start_at(&g,2,team)==PP_OK);finish();normal+=g.gym.result==1;
   init(seed,level,3);assert(pp_gym_start_at(&g,2,team)==PP_OK);finish();water+=g.gym.result==1;
  }
  printf("Vermilion Lv%u /100 wins: ground=%u grass=%u normal=%u water=%u\n",level,ground,grass,normal,water);
  assert(ground>=90&&water<=20);
 }
 puts("PASS: third-gym gate/no mutation, sequential badge validation, real electric/ground types, restore, independent rematches, participation XP, loss, rarity,60 cap,all families");
}
