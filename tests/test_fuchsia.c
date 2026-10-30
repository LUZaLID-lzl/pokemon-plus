#include "game.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
static pp_game g;
static unsigned progress(unsigned i){return 10u*g.growth[i].level*(g.growth[i].level-1)+g.growth[i].xp;}
static unsigned expected(unsigned lv,unsigned xp){unsigned total=10*lv*(lv-1)+xp;return total>35400?35400:total;}

static const unsigned team[]={0,1,2};
static void init(unsigned seed,unsigned level,unsigned species){
 pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.count=4;g.gym.badges=15;g.hatch_boosts=2;
 for(unsigned i=0;i<4;i++){g.pets[i]=g.pets[0];g.pets[i].species=species;g.pets[i].energy=100;g.growth[i]=(pp_growth){.level=level};}assert(pp_valid(&g));
}
static void finish(void){for(unsigned i=0;i<600&&g.battling;i++){pp_tick(&g,3,0);assert(pp_valid(&g));}assert(!g.battling);}
int main(void){
 init(1,44,26);g.gym.badges=7;uint32_t before=pp_checksum(&g);
 assert(pp_gym_unlocked(&g,3)&&!pp_gym_unlocked(&g,4));
 assert(pp_gym_start_at(&g,4,team)==PP_NOT_READY&&pp_checksum(&g)==before);
 assert(pp_gym_start_at(&g,PP_GYM_COUNT,team)==PP_INVALID&&pp_checksum(&g)==before);
 for(unsigned badges=0;badges<512;badges++){g.gym.badges=badges;assert(pp_valid(&g)==(badges==0||badges==1||badges==3||badges==7||badges==15||badges==31||badges==63||badges==127||badges==255));}
 g.gym.badges=15;assert(pp_gym_start_at(&g,4,team)==PP_OK&&g.gym.mode==5&&pp_valid(&g));
 assert(g.enemy_max_hp==383&&g.gym.enemy_max[1]==410&&g.gym.enemy_max[2]==435);
 assert(pp_gym_current(&g)->team[0].art==108&&pp_gym_current(&g)->team[1].art==88&&pp_gym_current(&g)->team[2].art==109);
 assert(pp_battle_move(&g,1,0)->type==PP_POISON&&pp_battle_move(&g,1,2)->type==PP_POISON);
 assert(pp_gym_effectiveness(&g,true,0)==100&&pp_gym_effectiveness(&g,false,0)==150);
 pp_game saved=g;g.gym.badges=7;assert(!pp_valid(&g));g=saved;
 pp_tick(&g,3,0);saved=g;before=pp_checksum(&g);g=saved;assert(pp_valid(&g)&&pp_checksum(&g)==before);
 finish();assert(g.gym.result==1&&g.gym.badges==31&&g.gym.reward_coins==900&&g.gym.first_clear&&g.hatch_boosts==2);
 assert(g.gym.reward_xp==((pp_battle_xp(&g.pets[0],44,43)*3)/1000000+6*44)&&pp_collectibles[g.gym.reward_item].quality>=2&&g.gym.reward_item!=PP_PURE_HEART&&pp_has(&g,g.gym.reward_item));
 assert(!g.growth[3].xp);for(unsigned i=0;i<3;i++)assert(progress(i)==expected(44,g.gym.reward_xp));
 unsigned coins=g.coins;pp_tick(&g,0,0);assert(g.coins==coins);pp_gym_leave(&g);assert(g.gym.badges==31);
 for(unsigned i=0;i<3;i++)g.pets[i].energy=100;
 assert(pp_gym_start_at(&g,4,team)==PP_OK);finish();assert(g.gym.result==1&&g.gym.reward_coins==40&&!g.gym.first_clear&&g.gym.wins==2);
 for(unsigned gym=0;gym<4;gym++){for(unsigned i=0;i<3;i++)g.pets[i].energy=100;assert(pp_gym_start_at(&g,gym,team)==PP_OK);finish();assert(g.gym.badges==31&&!g.gym.first_clear&&g.gym.reward_coins==(g.rematches_today==2&&gym==0?20:0));}
 init(2,1,1);assert(pp_gym_start_at(&g,4,team)==PP_OK);assert(pp_gym_effectiveness(&g,true,0)==100);finish();assert(g.gym.result==2&&g.gym.badges==15&&!g.gym.reward_coins&&g.gym.reward_xp==0&&g.gym.entered==7);
 assert(!g.growth[3].xp);for(unsigned i=0;i<3;i++)assert(g.growth[i].level==1&&!g.growth[i].xp);
 /* Actual defender is poison for each member; no Levitate or status system is claimed. */
 const unsigned species[]={26,10,1,4,27,7};
 for(unsigned kind=0;kind<6;kind++){
  init(5,36,species[kind]);assert(pp_gym_start_at(&g,4,team)==PP_OK);
  for(unsigned enemy=0;enemy<3;enemy++){g.gym.enemy=enemy;for(unsigned slot=0;slot<4;slot++){
   pp_move_type type=pp_battle_move(&g,0,slot)->type;
   if(type==PP_PSYCHIC||type==PP_GROUND)assert(pp_gym_effectiveness(&g,false,slot)==150);
   if(type==PP_GRASS||type==PP_BUG||type==PP_FIGHTING||type==PP_POISON)assert(pp_gym_effectiveness(&g,false,slot)==65);
  }}
 }
 init(6,36,10);assert(pp_gym_start_at(&g,4,team)==PP_OK);assert(pp_gym_effectiveness(&g,true,0)==65);
 init(6,36,13);assert(pp_gym_start_at(&g,4,team)==PP_OK);assert(pp_gym_effectiveness(&g,true,0)==150);
 unsigned qualities[5]={0};for(unsigned seed=1;seed<=300;seed++){init(seed,PP_MAX_LEVEL,26);assert(pp_gym_start_at(&g,4,team)==PP_OK);finish();assert(g.gym.result==1);qualities[pp_collectibles[g.gym.reward_item].quality]++;}assert(!qualities[0]&&!qualities[1]&&qualities[2]&&qualities[3]&&qualities[4]);
 for(unsigned sp=0;sp<PP_SPECIES;sp++){init(200+sp,PP_MAX_LEVEL,sp);assert(pp_gym_start_at(&g,4,team)==PP_OK);finish();for(unsigned i=0;i<3;i++)assert(g.growth[i].level==PP_MAX_LEVEL&&!g.growth[i].xp);}
 for(unsigned level=40;level<=46;level+=2){
  unsigned wins[4]={0};const unsigned sp[]={26,10,7,1};
  for(unsigned seed=1;seed<=100;seed++)for(unsigned kind=0;kind<4;kind++){init(seed,level,sp[kind]);assert(pp_gym_start_at(&g,4,team)==PP_OK);finish();wins[kind]+=g.gym.result==1;}
  printf("Fuchsia Lv%u /100 wins: psychic=%u ground=%u normal=%u grass-poison=%u\n",level,wins[0],wins[1],wins[2],wins[3]);
  if(level>=44)assert(wins[1]>=80&&wins[3]<=20);
  if(level>=44)assert(wins[0]>=90);
 }
 puts("PASS: fifth gym gate/no mutation, sequential badges, poison/psychic/ground types, v8 restore, rematches, participation XP, loss, rarity,60 cap,all families");
}
