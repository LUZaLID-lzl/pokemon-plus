#include "game.h"
#include "legacy_game_v7.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static unsigned progress(unsigned i){return 10u*g.growth[i].level*(g.growth[i].level-1)+g.growth[i].xp;}
static unsigned expected(unsigned lv,unsigned xp){unsigned total=10*lv*(lv-1)+xp;return total>35400?35400:total;}

static const unsigned team[]={0,1,2};
static uint32_t hash(const void *data,unsigned n){uint32_t h=2166136261u;const uint8_t *p=data;for(unsigned i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
static void init(unsigned seed,unsigned level,unsigned species){
 pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.count=4;g.gym.badges=1;
 for(unsigned i=0;i<4;i++){g.pets[i]=g.pets[0];g.pets[i].species=species;g.pets[i].energy=100;g.growth[i]=(pp_growth){.level=level};}assert(pp_valid(&g));
}
static void finish(void){for(unsigned i=0;i<600&&g.battling;i++){pp_tick(&g,3,0);assert(pp_valid(&g));}assert(!g.battling);}
int main(void){
 init(1,20,1);g.gym.badges=0;uint32_t before=pp_checksum(&g);
 assert(!pp_gym_unlocked(&g,1)&&pp_gym_unlocked(&g,0));
 assert(pp_gym_start_at(&g,1,team)==PP_NOT_READY&&pp_checksum(&g)==before);
 assert(pp_gym_start_at(&g,PP_GYM_COUNT,team)==PP_INVALID&&pp_checksum(&g)==before);
 g.gym.badges=1;g.hatch_boosts=3;assert(pp_gym_start_at(&g,1,team)==PP_OK&&g.gym.mode==2&&pp_valid(&g));
 assert(g.enemy_max_hp==180&&g.gym.enemy_max[1]==194&&g.gym.enemy_max[2]==218);
 assert(pp_gym_current(&g)->team[2].art==120);assert(pp_battle_move(&g,1,0)->type==PP_WATER&&pp_battle_move(&g,1,3)->type==PP_PSYCHIC);
 assert(pp_gym_effectiveness(&g,false,0)==150&&pp_gym_effectiveness(&g,true,0)==65&&pp_gym_effectiveness(&g,true,3)==150);
 pp_game saved=g;g.gym.badges=0;assert(!pp_valid(&g));g=saved;
 pp_tick(&g,3,0);saved=g;before=pp_checksum(&g);g=saved;assert(pp_valid(&g)&&pp_checksum(&g)==before);
 finish();assert(g.gym.result==1&&g.gym.badges==3&&g.gym.reward_coins==450&&g.gym.first_clear&&g.hatch_boosts==3);
 assert(g.gym.reward_xp==((pp_battle_xp(&g.pets[0],20,19)*3)/1000000+6*20)&&pp_collectibles[g.gym.reward_item].quality>=2&&g.gym.reward_item!=PP_PURE_HEART&&pp_has(&g,g.gym.reward_item));
 assert(!g.growth[3].xp);for(unsigned i=0;i<3;i++)assert(progress(i)==expected(20,g.gym.reward_xp));
 unsigned coins=g.coins;pp_tick(&g,0,0);assert(g.coins==coins);saved=g;assert(pp_valid(&saved));g=saved;
 for(unsigned i=0;i<3;i++)g.pets[i].energy=100;
 assert(pp_gym_start_at(&g,1,team)==PP_OK);finish();assert(g.gym.result==1&&g.gym.reward_coins==25&&!g.gym.first_clear&&g.gym.wins==2);
 for(unsigned i=0;i<3;i++)g.pets[i].energy=100;
 assert(pp_gym_start(&g,team)==PP_OK);finish();assert(g.gym.result==1&&g.gym.badges==3&&g.gym.reward_coins==20&&!g.gym.first_clear);
 init(2,1,2);assert(pp_gym_start_at(&g,1,team)==PP_OK);finish();assert(g.gym.result==2&&g.gym.badges==1&&!g.gym.reward_coins&&g.gym.reward_xp==0&&g.gym.entered==7);
 for(unsigned i=0;i<3;i++)assert(progress(i)==0);
 // Fixed-seed natural combat, including all integrated families at level cap.
 for(unsigned sp=0;sp<PP_SPECIES;sp++){init(200+sp,PP_MAX_LEVEL,sp);assert(pp_gym_start_at(&g,1,team)==PP_OK);finish();for(unsigned i=0;i<3;i++)assert(g.growth[i].level==PP_MAX_LEVEL&&!g.growth[i].xp);}
 for(unsigned level=18;level<=20;level+=2){
  unsigned grass=0,electric=0,normal=0,fire=0;
  for(unsigned seed=1;seed<=100;seed++){
   init(seed,level,1);assert(pp_gym_start_at(&g,1,team)==PP_OK);finish();grass+=g.gym.result==1;
   init(seed,level,0);assert(pp_gym_start_at(&g,1,team)==PP_OK);finish();electric+=g.gym.result==1;
   init(seed,level,7);assert(pp_gym_start_at(&g,1,team)==PP_OK);finish();normal+=g.gym.result==1;
   init(seed,level,2);assert(pp_gym_start_at(&g,1,team)==PP_OK);finish();fire+=g.gym.result==1;
  }
  printf("Cerulean Lv%u /100 wins: grass=%u electric=%u normal=%u fire=%u\n",level,grass,electric,normal,fire);
  if(level==20)assert(grass>=90&&electric>=90&&fire<=20&&normal<grass);
  else assert(!normal && !fire);
  if(level==18)assert(normal==0);
 }
 // An authentic first-gym v7 result imports without losing the unlock or rewards.
 init(22,18,1);assert(pp_gym_start(&g,team)==PP_OK);finish();pp_game_v7 old;memcpy(&old,&g,sizeof(old));old.version=7;
 assert(pp_import_v7(&g,&old,sizeof(old),hash(&old,sizeof(old)))&&pp_valid(&g)&&g.gym.badges==1&&!g.hatch_boosts);
 before=pp_checksum(&g);assert(!pp_import_v7(&g,&old,sizeof(old),0)&&pp_checksum(&g)==before);
 for(unsigned i=0;i<3;i++)g.pets[i].energy=100;
 assert(pp_gym_start_at(&g,1,team)==PP_OK&&pp_valid(&g));
 puts("PASS: second-gym gate/no mutation, enemy roster/types, restore, independent first/rematch, participation XP, loss, rarity,60 cap,all families,v7 migration");
}
