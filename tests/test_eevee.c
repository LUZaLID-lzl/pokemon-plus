#include "game.h"
#include "moves.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static void init(void){pp_init(&g,136);assert(pp_choose_starter(&g,1)==PP_OK);g.pets[0].species=67;g.coins=500;assert(pp_valid(&g));}
int main(void){
 assert(PP_SPECIES>=68 && PP_DEX_COUNT>=136);
 for(unsigned branch=1;branch<=3;branch++){
  init();uint32_t before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_INVALID && pp_evolve_stone(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  assert(pp_evolve_branch(&g,0,0)==PP_INVALID && pp_evolve_branch(&g,0,4)==PP_INVALID && pp_checksum(&g)==before);
  g.coins=199;before=pp_checksum(&g);assert(pp_evolve_branch(&g,0,branch)==PP_NO_COINS && pp_checksum(&g)==before);
  g.coins=200;g.traveler=0;before=pp_checksum(&g);assert(pp_evolve_branch(&g,0,branch)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
  g.battling=1;g.battle_pet=0;before=pp_checksum(&g);assert(pp_evolve_branch(&g,0,branch)==PP_BUSY && pp_checksum(&g)==before);g.battling=0;
  assert(pp_evolve_branch(&g,0,branch)==PP_OK && !g.coins && g.growth[0].stage==branch && pp_art_id(&g,0)==132+branch && pp_valid(&g));
  assert(pp_gym_hp(&g,0)==64+6*g.growth[0].level+6);
  assert(!strcmp(pp_form_name(&g,0),branch==1?"水伊布":branch==2?"雷伊布":"火伊布"));
  assert(pp_dex_owned(&g,132));for(unsigned d=1;d<=3;d++)assert(pp_dex_owned(&g,132+d)==(d==branch));
  before=pp_checksum(&g);assert(pp_evolve_branch(&g,0,1)==PP_INVALID && pp_decline_evolution(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  pp_game save=g;pp_init(&g,1);g=save;assert(pp_valid(&g) && pp_art_id(&g,0)==132+branch);
  g.count=2;g.pets[1]=g.pets[0];g.growth[1]=g.growth[0];g.pets[0].species=1;g.growth[0]=(pp_growth){.level=1};
  assert(pp_reserve_pet(&g,1)==PP_OK && pp_valid(&g));for(unsigned d=1;d<=3;d++)assert(pp_dex_owned(&g,132+d)==(d==branch));
  assert(pp_replace_pet(&g,0,0)==PP_OK && pp_art_id(&g,0)==132+branch && pp_valid(&g));
 }
 init();g.count=2;g.pets[1]=g.pets[0];g.growth[1]=g.growth[0];g.coins=0;assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));uint32_t before=pp_checksum(&g);
 for(unsigned b=1;b<=3;b++)assert(pp_evolve_branch(&g,0,b)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
 assert(pp_decline_evolution(&g,1)==PP_OK && pp_collected(&g)==1 && !g.coins && pp_valid(&g));
 assert(pp_reserve_pet(&g,1)==PP_OK && pp_replace_pet(&g,0,0)==PP_OK && pp_evolution_locked(&g,0) && pp_valid(&g));
 init();g.pets[0].species=1;g.growth[0].stage=3;assert(!pp_valid(&g));init();g.growth[0].stage=4;assert(!pp_valid(&g));
 for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(67,slot)==&pp_moves[268+slot]);
 bool hatched=false;for(unsigned seed=1;seed<=8192 && !hatched;seed++){pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=10000;assert(pp_test_hatch(&g)==PP_OK);hatched=g.pets[1].species==67;assert(pp_valid(&g));}assert(hatched);
 puts("PASS:Eevee explicit exclusive branches/200 coin boundary/busy/final/save/stage3/bench/dex ownership/permanent individual refusal/unique gift/hatch/move ID stability");
}
