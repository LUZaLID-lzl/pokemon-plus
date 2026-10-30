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
 assert(PP_SPECIES>=31 && PP_DEX_COUNT>=76 && PP_EVOLUTIONS>=45 && PP_MOVE_COUNT>=124);
 const unsigned families[]={27,28,29,30},gates[]={28,21,30,25},base[]={65,68,71,73},dest[]={66,69,72,74};
 for(unsigned i=0;i<4;i++) {
  init(families[i],0,gates[i]-1);assert(pp_art_id(&g,0)==base[i] && !pp_evolution_stone(&g,0));
  uint32_t before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
  g.growth[0].level++;g.traveler=0;before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
  assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==dest[i] && pp_valid(&g));
  pp_game saved=g;assert(pp_valid(&saved));g.growth[0].level=gates[i]-1;assert(!pp_valid(&g));
  init(families[i],0,gates[i]);g.coins=0;assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));
  saved=g;assert(pp_valid(&saved) && pp_evolution_locked(&saved,0));before=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
 }
 const unsigned itemFamily[]={27,28,30},levels[]={28,21,25},itemBase[]={66,69,74},itemDest[]={67,70,75};
 const char *items[]={"联系之绳","叶之石","联系之绳"};
 for(unsigned i=0;i<3;i++) {
  init(itemFamily[i],1,levels[i]);assert(!strcmp(pp_evolution_stone(&g,0),items[i]) && pp_art_id(&g,0)==itemBase[i]);
  uint32_t before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  g.coins=199;before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_NO_COINS && pp_checksum(&g)==before);
  g.coins=200;g.battling=true;g.battle_pet=0;before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.battling=false;
  assert(pp_evolve_stone(&g,0)==PP_OK && !g.coins && pp_art_id(&g,0)==itemDest[i] && pp_valid(&g));
  before=pp_checksum(&g);assert(pp_evolution_offer(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].species==itemFamily[i])assert(pp_dex_owned(&g,d));
  g.growth[0].level=levels[i]-1;assert(!pp_valid(&g));g.growth[0].level=levels[i];g.growth[0].stage=3;assert(!pp_valid(&g));
  init(itemFamily[i],1,levels[i]);g.coins=0;assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));
  pp_game saved=g;assert(pp_valid(&saved) && pp_evolution_locked(&saved,0));before=pp_checksum(&g);
  assert(pp_evolve_stone(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
 }
 init(29,1,30);uint32_t before=pp_checksum(&g);assert(pp_art_id(&g,0)==72 && pp_evolution_offer(&g,0)==PP_INVALID && pp_checksum(&g)==before);
 for(unsigned s=28;s<31;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[112+slot*3+s-28]);
 assert(pp_move_at(28,3)->type==PP_GRASS && pp_move_at(29,3)->type==PP_WATER && pp_move_at(30,3)->type==PP_GROUND);
 bool seen[PP_SPECIES]={0};for(unsigned seed=1;seed<=1024;seed++){pp_init(&g,seed);pp_choose_starter(&g,1);g.coins=10000;assert(pp_test_hatch(&g)==PP_OK);unsigned s=g.pets[1].species;assert(s!=1 && s<PP_SPECIES && !g.growth[1].stage && pp_valid(&g));seen[s]=true;}
 for(unsigned s=0;s<PP_SPECIES;s++)if(s!=1)assert(seen[s]==pp_test_ordinary_species(s));
 puts("PASS: 067-076 four level gates/three item transitions, previous level requirements, atomic busy/cost/final/save guards, permanent refusal/Pure Heart, 12 stable moves and 31-family hatch sampling");
}
