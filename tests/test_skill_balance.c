#include "game.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static void fresh(unsigned seed,unsigned species){pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.pets[0].species=species;assert(pp_valid(&g));}
static void finish(void){for(unsigned n=0;n<300 && g.battling;n++){pp_tick(&g,2,0);assert(pp_valid(&g));}assert(!g.battling);}
static void numeric(void){
 assert(PP_MAX_LEVEL==60 && PP_SPECIES==79 && PP_DEX_COUNT==151 && PP_MOVE_COUNT==316);
 for(unsigned dex=1;dex<=151;dex++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_form_power(dex,slot)<200);
 assert(!pp_move_form_power(0,0) && !pp_move_form_power(152,0) && !pp_move_form_power(1,4));
 fresh(9,1);g.count=3;for(unsigned i=1;i<3;i++){g.pets[i]=g.pets[0];g.growth[i]=g.growth[0];}g.growth[0].level=60;
 unsigned team[]={0,1,2};assert(pp_gym_start(&g,team)==PP_OK);
 unsigned previous=0;for(unsigned stage=0;stage<3;stage++){g.growth[0].stage=stage;unsigned hit=pp_gym_damage(&g,false,0,10);assert(hit>previous);previous=hit;}
 g.growth[0].stage=0;assert(pp_gym_damage(&g,false,3,10)>pp_gym_damage(&g,false,0,10));
 g.pets[0].species=67;unsigned powers[4];for(unsigned stage=0;stage<4;stage++){g.growth[0].stage=stage;powers[stage]=pp_battle_move_power(&g,0,1);}assert(powers[0]<powers[1] && powers[1]<powers[2] && powers[2]<powers[3]);
 // Status skill emits no fabricated damage, but still advances a turn.
 fresh(7,4);assert(pp_move_at(4,1)->power==0);assert(pp_battle(&g,0)==PP_OK);g.battle_round=1;int hp=g.enemy_hp;pp_tick(&g,2,0);assert(g.enemy_hp==hp && g.battle_round==2);
 // New forms resolve to dedicated art and their own four live moves.
 for(unsigned d=137;d<=151;d++){const pp_dex_entry *e=&pp_pokedex[d-1];fresh(d,e->species);g.growth[0]=(pp_growth){.level=60,.stage=e->stage};assert(pp_art_id(&g,0)==e->art && pp_valid(&g));assert(pp_battle(&g,0)==PP_OK);for(unsigned s=0;s<4;s++)assert(pp_battle_move_power(&g,0,s)==pp_move_form_power(d,s));finish();}
}
static void training(void){
 unsigned max_fights=0;
 for(unsigned seed=1;seed<=30;seed++){
  fresh(seed,76);unsigned fights=0,wins=0;while(g.growth[0].level<60 && fights<6000){if(g.pets[0].energy<3||g.pets[0].sleeping)pp_tick(&g,28800,0);assert(pp_battle(&g,0)==PP_OK);finish();wins+=g.battle_hp>0;fights++;while(pp_evolution_offer(&g,0)==PP_OK)assert(pp_evolve(&g,0)==PP_OK);}
  assert(g.growth[0].level==60 && g.growth[0].stage==2 && !g.growth[0].xp && pp_art_id(&g,0)==148 && wins*10>=fights*9);
  if(fights>max_fights)max_fights=fights;
  pp_game saved=g;uint32_t before=pp_checksum(&g);pp_gain_xp(&g,0,999999);assert(pp_checksum(&g)==before);g=saved;
 }
 printf("PASS:30 seeds, Lv1 Dratini to Lv60 Dragonite using only actual challenge XP, <=%u fights, >=90%% wins,30/55 evolution and cap persistence\n",max_fights);
}
static void progression(void){
 unsigned bosses[]={12,20,28,36,44,50,55,60};
 for(unsigned i=0;i<8;i++){assert(pp_gyms[i].team[2].level==bosses[i] && pp_gyms[i].win_xp>pp_gyms[i].loss_xp);if(i)assert(pp_gyms[i].win_xp>pp_gyms[i-1].win_xp);}
 fresh(5,76);g.growth[0].level=54;g.growth[0].stage=1;pp_gain_xp(&g,0,pp_xp_needed(54)-1);assert(pp_evolution_offer(&g,0)==PP_NOT_READY);pp_gain_xp(&g,0,1);assert(g.growth[0].level==55 && pp_evolve(&g,0)==PP_OK);
 fresh(5,76);g.growth[0].level=55;assert(pp_decline_evolution(&g,0)==PP_OK);pp_gain_xp(&g,0,100000);assert(g.growth[0].level==60 && pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_has(&g,PP_PURE_HEART));
 fresh(5,1);g.count=2;g.pets[1]=g.pets[0];g.growth[1]=(pp_growth){.level=55};assert(pp_travel(&g,1)==PP_OK && pp_travel_xp(&g)==220);pp_tick(&g,PP_TRAVEL_SECONDS,0);assert(pp_claim(&g)==PP_OK && g.growth[1].level==55 && g.growth[1].xp==220);unsigned xp=g.growth[1].xp;assert(pp_claim(&g)==PP_INVALID && g.growth[1].xp==xp);
 fresh(5,1);g.growth[0].level=59;pp_record_steps(&g,10000);unsigned step_xp=g.growth[0].xp;pp_record_steps(&g,0);assert(g.growth[0].xp==step_xp && step_xp>0);
 // Older v9 gym states remain valid and resume without losing the saved party.
 unsigned old_hp[8][3]={{124,142,164},{160,174,196},{200,216,240},{258,276,296},{310,334,356},{318,334,356},{340,360,386},{340,360,390}};
 for(unsigned gym=0;gym<8;gym++){
  fresh(7,3);g.count=3;for(unsigned i=1;i<3;i++){g.pets[i]=g.pets[0];g.growth[i]=g.growth[0];}g.gym.badges=(1u<<gym)-1;unsigned team[]={0,1,2};assert(pp_gym_start_at(&g,gym,team)==PP_OK);
  for(unsigned i=0;i<3;i++)g.gym.enemy_max[i]=g.gym.enemy_hp[i]=old_hp[gym][i];g.enemy_hp=g.enemy_max_hp=old_hp[gym][0];assert(pp_valid(&g));pp_game saved=g;g=saved;pp_tick(&g,3,0);assert(pp_valid(&g) && g.count==3 && g.gym.badges==((1u<<gym)-1));
 }
 puts("PASS:151 form powers, real evolved damage, branch strengths, status turns, all new runtime skills, eight gym tiers, high-level travel/walking and permanent refusal");
}
int main(void){numeric();training();progression();}
