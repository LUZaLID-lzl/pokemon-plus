#include "game.h"
#include "hatch_fixture.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static void init(unsigned s,unsigned level){pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);g.pets[0].species=s;g.growth[0].level=level;assert(pp_valid(&g));}
static void refusal(unsigned s,unsigned level){
 init(s,level);g.coins=0;g.count=2;g.pets[1]=g.pets[0];g.growth[1]=g.growth[0];
 assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));
 assert(pp_evolution_locked(&g,0) && !pp_evolution_locked(&g,1));
 pp_game saved=g;uint32_t sum=pp_checksum(&g);assert(pp_valid(&saved));g=saved;
 assert(pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==sum);
 assert(pp_evolve_stone(&g,0)==(s==44?PP_EVOLUTION_LOCKED:PP_INVALID) && pp_checksum(&g)==sum);
 assert(pp_decline_evolution(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==sum);
 assert(pp_decline_evolution(&g,1)==PP_OK && pp_collection_owned(&g,5)==1);
 assert(pp_collected(&g)==1 && !g.coins && pp_valid(&g));
}
int main(void){
 assert(PP_SPECIES>=47 && PP_DEX_COUNT>=106 && PP_EVOLUTIONS>=59 && PP_MOVE_COUNT>=188);
 const unsigned species[]={41,42,43,45},levels[]={26,28,30,28},bases[]={95,97,99,103},dest[]={96,98,100,104};
 for(unsigned i=0;i<4;i++){
  init(species[i],levels[i]-1);assert(pp_art_id(&g,0)==bases[i]);uint32_t before=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
  g.growth[0].level++;g.traveler=0;before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
  assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==dest[i] && pp_valid(&g));
  assert(pp_dex_owned(&g,bases[i]) && pp_dex_owned(&g,dest[i]));pp_game saved=g;assert(pp_valid(&saved));
  assert(pp_evolution_offer(&g,0)==PP_INVALID);g.growth[0].level=levels[i]-1;assert(!pp_valid(&g));
  refusal(species[i],levels[i]);
 }
 init(44,1);assert(!strcmp(pp_evolution_stone(&g,0),"叶之石"));g.coins=199;uint32_t before=pp_checksum(&g);
 assert(pp_evolve_stone(&g,0)==PP_NO_COINS && pp_checksum(&g)==before);
 g.coins=200;g.battling=true;g.battle_pet=0;before=pp_checksum(&g);
 assert(pp_evolve_stone(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.battling=false;
 assert(pp_evolve_stone(&g,0)==PP_OK && !g.coins && pp_art_id(&g,0)==102 && pp_valid(&g));
 assert(pp_dex_owned(&g,101) && pp_dex_owned(&g,102));refusal(44,1);
 init(46,40);before=pp_checksum(&g);assert(pp_art_id(&g,0)==105 && pp_evolution_offer(&g,0)==PP_INVALID && pp_checksum(&g)==before);
 g.growth[0].stage=1;assert(!pp_valid(&g));
 for(unsigned s=42;s<47;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[168+slot*5+s-42]);
 assert(pp_move_at(42,2)->type==PP_WATER && pp_move_at(43,3)->type==PP_ELECTRIC && pp_move_at(46,0)->type==PP_FIGHTING);
 bool seen[PP_SPECIES]={0};for(unsigned seed=1;seed<=8192;seed++){pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=10000;assert(pp_test_hatch(&g)==PP_OK);unsigned s=g.pets[1].species;assert(s!=1&&s<PP_SPECIES&&!g.growth[1].stage&&pp_valid(&g));seen[s]=true;}
 for(unsigned s=0;s<PP_SPECIES;s++)if(s!=1)assert(seen[s]==pp_test_ordinary_species(s));
 puts("PASS:097-106 four level boundaries,200-coin leaf stone,busy/save/dex,permanent individual refusal/unique PureHeart,106 final,20 moves and47-family hatch");
}
