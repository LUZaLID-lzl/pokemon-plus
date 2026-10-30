#include "game.h"
#include "moves.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static void init(unsigned species,unsigned level) {
 pp_init(&g,42);pp_choose_starter(&g,1);g.pets[0].species=species;g.growth[0].level=level;g.coins=500;assert(pp_valid(&g));
}
int main(void) {
 const unsigned family[]={18,19,20,21,22},gate[]={24,31,26,28,33},base[]={45,47,49,51,53},dest[]={46,48,50,52,54};
 assert(PP_SPECIES>=28 && PP_DEX_COUNT>=66 && PP_EVOLUTIONS>=38 && PP_MOVE_COUNT>=112);
 for(unsigned i=0;i<5;i++) {
  init(family[i],gate[i]-1);assert(pp_art_id(&g,0)==base[i] && !pp_evolution_stone(&g,0));
  uint32_t before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
  g.growth[0].level++;g.traveler=0;before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
  assert(pp_evolution_offer(&g,0)==PP_OK && pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==dest[i] && pp_valid(&g));
  unsigned owned=0;for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].species==family[i]){assert(pp_dex_owned(&g,d));owned++;}assert(owned==2);
  before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  pp_game saved=g;assert(pp_valid(&saved) && pp_art_id(&saved,0)==dest[i]);
  g.growth[0].level=gate[i]-1;assert(!pp_valid(&g));
  init(family[i],gate[i]);g.coins=0;
  assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART) && pp_art_id(&g,0)==base[i]);
  saved=g;assert(pp_valid(&saved) && pp_evolution_locked(&saved,0));before=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
  assert(pp_decline_evolution(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
 }
 init(29,PP_MAX_LEVEL);g.growth[0].stage=1;uint32_t before=pp_checksum(&g);
 assert(pp_art_id(&g,0)==72 && pp_evolve(&g,0)==PP_INVALID && pp_checksum(&g)==before);
 g.growth[0].stage=2;assert(!pp_valid(&g));
 for(unsigned s=19;s<24;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[76+slot*5+s-19]);
 assert(pp_move_at(19,2)->type==PP_PSYCHIC && pp_move_at(22,3)->type==PP_PSYCHIC && pp_move_at(23,3)->type==PP_FIGHTING);
 /* Sample independent saves across the current family pool; the historical
  * fixed512 seeds no longer cover every family as the roster grows. */
 bool seen[PP_SPECIES]={0};
 for(unsigned seed=1;seed<=PP_SPECIES*128u;seed++) {
  pp_init(&g,seed);pp_choose_starter(&g,1);g.coins=10000;
  assert(pp_test_hatch(&g)==PP_OK);unsigned s=g.pets[1].species;assert(s!=1 && s<PP_SPECIES && !g.growth[1].stage && pp_valid(&g));seen[s]=true;
 }
 for(unsigned s=0;s<PP_SPECIES;s++)if(s!=1)assert(seen[s]==pp_test_ordinary_species(s));
 puts("PASS: five new level gates, busy/final/save/history guards, permanent refusal and Pure Heart; final-form guard; 20 stable moves and 31-family missing-first hatch pool within 16 persisted slots");
}
