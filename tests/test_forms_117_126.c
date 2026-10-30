#include "game.h"
#include "hatch_fixture.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
static pp_game g;
static void init(unsigned species,unsigned level){pp_init(&g,126);assert(pp_choose_starter(&g,1)==PP_OK);g.pets[0].species=species;g.growth[0].level=level;assert(pp_valid(&g));}
int main(void){
 assert(PP_SPECIES>=62 && PP_DEX_COUNT>=126 && PP_EVOLUTIONS>=64 && PP_MOVE_COUNT>=248);
 const unsigned species[]={54,55},levels[]={32,33},bases[]={115,117},dest[]={116,118};
 for(unsigned k=0;k<2;k++){
  init(species[k],levels[k]-1);assert(pp_art_id(&g,0)==bases[k]);uint32_t before=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);pp_gain_xp(&g,0,20*(levels[k]-1));assert(g.growth[0].level==levels[k]);
  g.traveler=0;before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
  assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==dest[k] && pp_valid(&g));assert(pp_dex_owned(&g,bases[k]) && pp_dex_owned(&g,dest[k]));pp_game saved=g;g=saved;assert(pp_valid(&g));
  init(species[k],levels[k]);g.coins=0;g.count=2;g.pets[1]=g.pets[0];g.growth[1]=g.growth[0];assert(pp_decline_evolution(&g,0)==PP_OK && pp_evolution_locked(&g,0) && !pp_evolution_locked(&g,1));before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);assert(pp_evolve(&g,1)==PP_OK && !g.coins && pp_collected(&g)==1 && pp_valid(&g));
 }
 init(56,1);assert(pp_art_id(&g,0)==119 && !pp_evolution_level(&g,0));assert(pp_evolution_stone(&g,0) && pp_evolution_offer(&g,0)==PP_OK);
 g.coins=199;uint32_t before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_NO_COINS && pp_checksum(&g)==before);
 g.coins=200;assert(pp_evolve_stone(&g,0)==PP_OK && !g.coins && pp_art_id(&g,0)==120 && pp_valid(&g));assert(pp_dex_owned(&g,119) && pp_dex_owned(&g,120));before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_INVALID && pp_checksum(&g)==before);
 init(56,1);g.coins=0;g.count=2;g.pets[1]=g.pets[0];g.growth[1]=g.growth[0];assert(pp_decline_evolution(&g,0)==PP_OK && pp_collected(&g)==1);pp_game saved=g;g=saved;before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);assert(pp_decline_evolution(&g,1)==PP_OK && pp_collected(&g)==1 && !g.coins && pp_valid(&g));
 for(unsigned s=57;s<62;s++){init(s,PP_MAX_LEVEL);before=pp_checksum(&g);assert(pp_evolution_offer(&g,0)==PP_INVALID && pp_checksum(&g)==before);g.growth[0].stage=1;assert(!pp_valid(&g));}
 for(unsigned s=55;s<62;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[220+slot*7+s-55]);
 assert(pp_move_at(56,0)->type==PP_WATER && pp_move_at(57,3)->type==PP_PSYCHIC && pp_move_at(58,3)->type==PP_BUG && pp_move_at(59,3)->type==PP_ICE && pp_move_at(60,3)->type==PP_ELECTRIC && pp_move_at(61,3)->type==PP_FIRE);
 bool seen[PP_SPECIES]={0};for(unsigned seed=1;seed<=8192;seed++){pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=10000;assert(pp_test_hatch(&g)==PP_OK);unsigned s=g.pets[1].species;assert(s!=1 && s<PP_SPECIES && !g.growth[1].stage && pp_valid(&g));seen[s]=true;}for(unsigned s=0;s<PP_SPECIES;s++)if(s!=1)assert(seen[s]==pp_test_ordinary_species(s));
 puts("PASS:117-126 reachable32/33 evolution,water-stone200 boundary/zero-coin refusal/individual save lock/unique gift/five finals/28 moves/62-family hatch");
}
