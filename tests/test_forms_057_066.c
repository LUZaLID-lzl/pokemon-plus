#include "game.h"
#include "hatch_fixture.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static void init(unsigned species,unsigned stage,unsigned level) {
 pp_init(&g,42);pp_choose_starter(&g,1);g.pets[0].species=species;g.growth[0].stage=stage;g.growth[0].level=level;g.coins=500;assert(pp_valid(&g));
}
int main(void) {
 assert(PP_SPECIES>=28 && PP_DEX_COUNT>=66 && PP_EVOLUTIONS>=38 && PP_MOVE_COUNT>=112);
 const unsigned families[]={23,25,26},gates[]={28,25,16},base[]={55,59,62},dest[]={56,60,63};
 for(unsigned i=0;i<3;i++) {
  init(families[i],0,gates[i]-1);assert(pp_art_id(&g,0)==base[i] && !pp_evolution_stone(&g,0));
  uint32_t before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
  g.growth[0].level++;g.traveler=0;before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
  assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==dest[i] && pp_valid(&g));
  pp_game saved=g;assert(pp_valid(&saved));g.growth[0].level=gates[i]-1;assert(!pp_valid(&g));
  init(families[i],0,gates[i]);g.coins=0;assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));
  saved=g;assert(pp_valid(&saved) && pp_evolution_locked(&saved,0));before=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
 }
 const unsigned stoneFamily[]={24,25,26},stage[]={0,1,1},level[]={1,25,16},stoneBase[]={57,60,63},stoneDest[]={58,61,64};
 const char *item[]={"火之石","水之石","联系之绳"};
 for(unsigned i=0;i<3;i++) {
  init(stoneFamily[i],stage[i],level[i]);assert(!strcmp(pp_evolution_stone(&g,0),item[i]) && pp_art_id(&g,0)==stoneBase[i]);
  uint32_t before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  g.coins=199;before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_NO_COINS && pp_checksum(&g)==before);
  g.coins=200;g.battling=true;g.battle_pet=0;before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.battling=false;
  assert(pp_evolve_stone(&g,0)==PP_OK && !g.coins && pp_art_id(&g,0)==stoneDest[i] && pp_valid(&g));
  before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].species==stoneFamily[i])assert(pp_dex_owned(&g,d));
  init(stoneFamily[i],stage[i],level[i]);g.coins=0;assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));
  before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
  assert(pp_decline_evolution(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
 }
 init(29,1,PP_MAX_LEVEL);uint32_t before=pp_checksum(&g);assert(pp_art_id(&g,0)==72 && pp_evolution_offer(&g,0)==PP_INVALID && pp_checksum(&g)==before);g.growth[0].stage=2;assert(!pp_valid(&g));
 for(unsigned s=24;s<28;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[96+slot*4+s-24]);
 assert(pp_move_at(24,3)->type==PP_FIRE && pp_move_at(25,3)->type==PP_WATER && pp_move_at(26,3)->type==PP_PSYCHIC && pp_move_at(27,3)->type==PP_FIGHTING);
 bool seen[PP_SPECIES]={0};for(unsigned seed=1;seed<=1024;seed++){pp_init(&g,seed);pp_choose_starter(&g,1);g.coins=10000;assert(pp_test_hatch(&g)==PP_OK);unsigned s=g.pets[1].species;assert(s!=1 && s<PP_SPECIES && !g.growth[1].stage && pp_valid(&g));seen[s]=true;}
 for(unsigned s=0;s<PP_SPECIES;s++)if(s!=1)assert(seen[s]==pp_test_ordinary_species(s));
 puts("PASS: 057-066 level/item transitions, atomic costs/busy/final/save guards, permanent refusal/Pure Heart, final-form guard, 16 stable moves and 31-family missing-first hatch sampling");
}
