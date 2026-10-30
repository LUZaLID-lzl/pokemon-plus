#include "game.h"
#include "hatch_fixture.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
static pp_game g;
static void init(unsigned species,unsigned stage,unsigned level){pp_init(&g,42);pp_choose_starter(&g,1);g.pets[0].species=species;g.growth[0].stage=stage;g.growth[0].level=level;assert(pp_valid(&g));}
int main(void){
 assert(PP_SPECIES>=37 && PP_DEX_COUNT>=86 && PP_EVOLUTIONS>=49 && PP_MOVE_COUNT>=148);
 const unsigned families[]={31,32,33,35},gates[]={40,37,30,31},base[]={76,78,80,83},dest[]={77,79,81,84};
 for(unsigned i=0;i<4;i++){
  init(families[i],0,gates[i]-1);assert(pp_art_id(&g,0)==base[i] && !pp_evolution_stone(&g,0));uint32_t before=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
  g.growth[0].level++;g.traveler=0;before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
  assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==dest[i] && pp_valid(&g));
  pp_game saved=g;assert(pp_valid(&saved));before=pp_checksum(&g);assert(pp_evolution_offer(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].species==families[i])assert(pp_dex_owned(&g,d));
  g.growth[0].level=gates[i]-1;assert(!pp_valid(&g));g.growth[0].level=gates[i];g.growth[0].stage=2;assert(!pp_valid(&g));
  init(families[i],0,gates[i]);g.coins=0;assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));saved=g;
  assert(pp_valid(&saved)&&pp_evolution_locked(&saved,0));before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
 }
 for(unsigned s=34;s<=40;s+=6){init(s,0,PP_MAX_LEVEL);uint32_t before=pp_checksum(&g);assert(pp_evolution_offer(&g,0)==PP_INVALID && pp_checksum(&g)==before);g.growth[0].stage=1;assert(!pp_valid(&g));}
 for(unsigned s=31;s<37;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[124+slot*6+s-31]);
 assert(pp_move_at(33,2)->type==PP_STEEL && pp_move_at(36,3)->type==PP_ICE);
 bool seen[PP_SPECIES]={0};for(unsigned seed=1;seed<=2048;seed++){pp_init(&g,seed);pp_choose_starter(&g,1);g.coins=10000;assert(pp_test_hatch(&g)==PP_OK);unsigned s=g.pets[1].species;assert(s!=1 && s<PP_SPECIES && !g.growth[1].stage && pp_valid(&g));seen[s]=true;}
 for(unsigned s=0;s<PP_SPECIES;s++)if(s!=1)assert(seen[s]==pp_test_ordinary_species(s));
 puts("PASS:077-086 four level gates/previous levels/busy/save/dex/refusal/PureHeart, final and087 missing-art guards,24 stable moves/ice/steel and37-family hatching");
}
