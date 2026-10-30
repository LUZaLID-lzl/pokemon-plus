#include "game.h"
#include "legacy_game_v6.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static unsigned progress(unsigned i){return 10u*g.growth[i].level*(g.growth[i].level-1)+g.growth[i].xp;}
static unsigned expected(unsigned lv,unsigned xp){unsigned total=10*lv*(lv-1)+xp;return total>35400?35400:total;}

static const unsigned team[3]={0,1,2};
static uint32_t hash(const void *data,unsigned n){uint32_t h=2166136261u;const uint8_t *p=data;for(unsigned i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
static void init(unsigned seed,unsigned level,unsigned species){
 pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.count=4;
 for(unsigned i=0;i<4;i++){g.pets[i]=g.pets[0];g.pets[i].species=species;g.growth[i]=(pp_growth){.level=level};}assert(pp_valid(&g));
}
static void finish(void){for(unsigned i=0;i<500&&g.battling;i++){pp_tick(&g,3,0);assert(pp_valid(&g));}assert(!g.battling);}
int main(void){
 init(1,12,1);g.count=2;uint32_t before=pp_checksum(&g);assert(pp_gym_start(&g,team)==PP_NEED_THREE&&pp_checksum(&g)==before);g.count=4;
 const unsigned repeated[3]={0,0,2},invalid[3]={0,1,5};before=pp_checksum(&g);
 assert(pp_gym_start(&g,repeated)==PP_INVALID&&pp_gym_start(&g,invalid)==PP_INVALID&&pp_checksum(&g)==before);
 g.pets[1].sleeping=1;before=pp_checksum(&g);assert(pp_gym_start(&g,team)==PP_NOT_READY&&pp_checksum(&g)==before);g.pets[1].sleeping=0;
 /* A travelling or item-using partner alone must not lock the other three. */
 g.traveler=3;assert(pp_gym_start(&g,team)==PP_OK&&pp_valid(&g));g.battling=0;g.gym.mode=0;g.traveler=-1;
 const unsigned travelling[3]={0,1,3};g.traveler=3;before=pp_checksum(&g);assert(pp_gym_start(&g,travelling)==PP_BUSY&&pp_checksum(&g)==before);g.traveler=-1;
 g.drop_active=2;g.drop_target=3;assert(pp_gym_start(&g,team)==PP_OK&&pp_valid(&g));g.battling=0;g.gym.mode=0;g.drop_active=0;
 g.drop_active=2;g.drop_target=2;before=pp_checksum(&g);assert(pp_gym_start(&g,team)==PP_BUSY&&pp_checksum(&g)==before);g.drop_active=0;
 assert(pp_gym_start(&g,team)==PP_OK&&pp_valid(&g));before=pp_checksum(&g);assert(pp_gym_start(&g,team)==PP_BUSY&&pp_checksum(&g)==before);
 assert(pp_evolution_offer(&g,1)==PP_BUSY||pp_evolution_offer(&g,1)==PP_NOT_READY);
 pp_game saved=g;assert(pp_checksum(&saved)==pp_checksum(&g));finish();assert(g.gym.result==1&&g.gym.badges==1&&g.gym.reward_coins==300);
 assert(pp_collectibles[g.gym.reward_item].quality>=2&&pp_collectibles[g.gym.reward_item].quality<=4&&pp_has(&g,g.gym.reward_item));
 assert(!g.growth[3].xp);for(unsigned i=0;i<3;i++)assert(progress(i)==expected(12,g.gym.reward_xp));
 before=pp_checksum(&g);pp_tick(&g,0,0);assert(pp_checksum(&g)==before);unsigned coins=g.coins;pp_tick(&g,3600,0);assert(g.coins==coins);
 for(unsigned i=0;i<3;i++)g.pets[i].energy=100;
 assert(pp_gym_start(&g,team)==PP_OK);finish();assert(g.gym.reward_coins==20&&!g.gym.first_clear&&g.gym.wins==2);
 init(2,1,2);assert(pp_gym_start(&g,team)==PP_OK);finish();assert(g.gym.result==2&&!g.gym.badges&&!g.gym.reward_coins&&g.gym.entered==7);for(unsigned i=0;i<3;i++)assert(progress(i)==0);
 // Every actual integrated family receives a bounded valid combat simulation.
 for(unsigned species=0;species<PP_SPECIES;species++){
  init(99+species,PP_MAX_LEVEL,species);assert(pp_gym_start(&g,team)==PP_OK);finish();assert(pp_valid(&g));for(unsigned i=0;i<3;i++)assert(g.growth[i].level==PP_MAX_LEVEL&&!g.growth[i].xp);
 }
 unsigned grass_wins=0,normal_wins=0;
 for(unsigned seed=1;seed<=100;seed++){
  init(seed,12,1);assert(pp_gym_start(&g,team)==PP_OK);finish();grass_wins+=g.gym.result==1;
  init(seed,12,7);assert(pp_gym_start(&g,team)==PP_OK);finish();normal_wins+=g.gym.result==1;
 }
 assert(grass_wins>=90);printf("Gym balance at Lv12: grass %u/100, normal %u/100 wins\n",grass_wins,normal_wins);
 init(13,12,0);g.pets[0].species=0;assert(pp_gym_start(&g,team)==PP_OK);assert(pp_gym_damage(&g,false,0,10)==0);assert(pp_gym_damage(&g,false,1,10)>0);
 g.pets[0].species=6;assert(pp_gym_effectiveness(&g,true,3)==0);g.pets[0].species=39;assert(pp_gym_effectiveness(&g,true,1)==0);g.pets[0].species=3;assert(pp_gym_effectiveness(&g,false,0)==200);
 // A complete v6 blob migrates active+reserve levels while retaining individual choices and badge bits.
 init(25,39,50);g.reserve_count=1;g.reserves[0]=g.pets[0];g.reserve_growth[0]=(pp_growth){.level=40};pp_game_v6 old;memcpy(&old,&g,sizeof(old));old.version=6;old.growth[0]=(pp_growth){.level=100,.stage=1};old.reserve_growth[0]=(pp_growth){.level=99,.xp=100};old.gym.badges=1;old.collected[0]=0x5a;old.evolution_declined=1;old.collected[PP_PURE_HEART/8]|=1u<<(PP_PURE_HEART%8);
 assert(pp_import_v6(&g,&old,sizeof(old),hash(&old,sizeof(old)))&&pp_valid(&g));assert(g.growth[0].level==PP_MAX_LEVEL&&g.growth[0].stage==1&&!g.growth[0].xp&&g.reserve_growth[0].level==PP_MAX_LEVEL&&!g.reserve_growth[0].xp&&g.gym.badges==1&&g.collected[0]==0x5a&&g.evolution_declined==1);
 before=pp_checksum(&g);assert(!pp_import_v6(&g,&old,sizeof(old),0)&&pp_checksum(&g)==before);
 puts("PASS: 3v3 eligibility, relay/defeat, participation XP, first/rematch rewards, rare+ pools, all integrated families, immunities,60 cap,v6 migration,no repeat payout");
}
