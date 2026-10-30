#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static const unsigned team[]={0,1,2};
static void init(void){pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);g.count=5;for(unsigned i=1;i<5;i++){g.pets[i]=g.pets[0];g.pets[i].species=i;g.growth[i]=(pp_growth){.level=10};}assert(pp_valid(&g));}
static void pending(bool opened){g.wild=(pp_wild_state){.phase=PP_WILD_PENDING,.dex=0,.level=1,.scene=0,.next_at=opened?0:PP_WILD_STEP_MARK,.escape_at=opened?UINT64_MAX:g.seconds+60};assert(pp_valid(&g));}
static void won(unsigned outcome){g.wild=(pp_wild_state){.phase=PP_WILD_WON,.dex=0,.level=1,.outcome=outcome,.reward_coins=(outcome==PP_WILD_CAPTURE_READY||outcome==PP_WILD_CAUGHT)?25:105,.reward_xp=(outcome==PP_WILD_CAPTURE_READY||outcome==PP_WILD_CAUGHT)?30:90};assert(pp_valid(&g));}
int main(void){
 init();pending(false);g.pets[0].sleeping=1;uint32_t before=pp_checksum(&g);assert(pp_gym_start(&g,team)==PP_NOT_READY&&pp_checksum(&g)==before);g.pets[0].sleeping=0;
 unsigned coins=g.coins;assert(pp_gym_start(&g,team)==PP_OK&&pp_valid(&g)&&!g.wild.phase&&g.coins==coins);pp_tick(&g,2,0);assert(g.battle_round==1&&pp_valid(&g));
 init();pending(true);before=pp_checksum(&g);assert(pp_gym_start(&g,team)==PP_BUSY&&pp_checksum(&g)==before);pp_wild_dismiss(&g);assert(pp_gym_start(&g,team)==PP_OK&&pp_valid(&g));
 init();won(PP_WILD_CAPTURE_READY);g.balls=1;before=pp_checksum(&g);assert(pp_gym_start(&g,team)==PP_BUSY&&pp_checksum(&g)==before);assert(pp_wild_capture(&g)==PP_OK);coins=g.coins;unsigned owned=pp_warehouse_count(&g),xp=g.growth[0].xp;assert(pp_gym_start(&g,team)==PP_OK&&pp_valid(&g)&&!g.wild.phase&&g.coins==coins&&g.growth[0].xp==xp&&pp_warehouse_count(&g)==owned);
 for(unsigned outcome=PP_WILD_CAUGHT;outcome<=PP_WILD_FULL;outcome++){init();won(outcome);coins=g.coins;assert(pp_gym_start(&g,team)==PP_OK&&pp_valid(&g)&&!g.wild.phase&&g.coins==coins);}
 init();g.wild=(pp_wild_state){.phase=PP_WILD_LOST,.dex=0,.level=1};assert(pp_valid(&g));assert(pp_gym_start(&g,team)==PP_OK&&pp_valid(&g));
 init();pending(false);assert(pp_wild_reveal(&g)==PP_OK&&pp_wild_start(&g,0)==PP_OK);before=pp_checksum(&g);assert(pp_gym_start(&g,team)==PP_BUSY&&pp_checksum(&g)==before);
 init();g.traveler=4;pending(false);assert(pp_gym_start(&g,team)==PP_OK&&g.traveler==4&&pp_valid(&g));
 init();g.traveler=2;pending(false);before=pp_checksum(&g);assert(pp_gym_start(&g,team)==PP_BUSY&&pp_checksum(&g)==before);
 init();g.drop_active=2;g.drop_target=4;pending(false);assert(pp_gym_start(&g,team)==PP_OK&&g.drop_active==2&&g.drop_target==4&&pp_valid(&g));
 puts("PASS: gym clears only untouched notice/settled receipts after valid team; actual combat/opened encounter/capture reward protected; no duplicate reward or unrelated travel/item blockage; real turns advance");
}
