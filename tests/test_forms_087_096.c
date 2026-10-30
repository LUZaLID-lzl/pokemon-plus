#include "game.h"
#include "hatch_fixture.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static void init(unsigned s,unsigned stage,unsigned level){pp_init(&g,42);pp_choose_starter(&g,1);g.pets[0].species=s;g.growth[0].stage=stage;g.growth[0].level=level;assert(pp_valid(&g));}
int main(void){
 assert(PP_SPECIES>=42 && PP_DEX_COUNT>=96 && PP_EVOLUTIONS>=54 && PP_MOVE_COUNT>=168);
 const unsigned families[]={36,37,39},gates[]={34,38,25},bases[]={85,87,91},dest[]={86,88,92};
 for(unsigned i=0;i<3;i++){
  init(families[i],0,gates[i]-1);assert(pp_art_id(&g,0)==bases[i]);uint32_t before=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
  g.growth[0].level++;g.traveler=0;before=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
  assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==dest[i] && pp_valid(&g));
  pp_game saved=g;assert(pp_valid(&saved));assert(pp_dex_owned(&g,bases[i]));assert(pp_dex_owned(&g,dest[i]));
  g.growth[0].level=gates[i]-1;assert(!pp_valid(&g));
  init(families[i],0,gates[i]);g.coins=0;assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));before=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
 }
 for(unsigned i=0;i<2;i++){
  init(i?39:38,i?1:0,i?25:1);assert(!strcmp(pp_evolution_stone(&g,0),i?"联系之绳":"水之石"));
  g.coins=199;uint32_t before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_NO_COINS && pp_checksum(&g)==before);
  g.coins=200;g.battling=true;g.battle_pet=0;before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.battling=false;
  assert(pp_evolve_stone(&g,0)==PP_OK && g.coins==0 && pp_art_id(&g,0)==(i?93:90) && pp_valid(&g));
  assert(pp_evolution_offer(&g,0)==PP_INVALID);
  if(i){g.growth[0].level=24;assert(!pp_valid(&g));}
  init(i?39:38,i?1:0,i?25:1);g.coins=0;assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));before=pp_checksum(&g);
  assert(pp_evolve_stone(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
 }
 for(unsigned s=40;s<=40;s++){init(s,0,PP_MAX_LEVEL);uint32_t before=pp_checksum(&g);assert(pp_evolution_offer(&g,0)==PP_INVALID && pp_checksum(&g)==before);g.growth[0].stage=1;assert(!pp_valid(&g));}
 for(unsigned s=37;s<42;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[148+slot*5+s-37]);
 assert(pp_move_at(39,0)->type==PP_GHOST && pp_move_at(38,3)->type==PP_ICE);
 bool seen[PP_SPECIES]={0};for(unsigned seed=1;seed<=4096;seed++){pp_init(&g,seed);pp_choose_starter(&g,1);g.coins=10000;assert(pp_test_hatch(&g)==PP_OK);unsigned s=g.pets[1].species;assert(s!=1&&s<PP_SPECIES&&!g.growth[1].stage&&pp_valid(&g));seen[s]=true;}
 for(unsigned s=0;s<PP_SPECIES;s++)if(s!=1)assert(seen[s]==pp_test_ordinary_species(s));
 puts("PASS:087-096 three level gates,two atomic stones,previous-stage levels,busy/save/dex/permanent refusal/PureHeart,097 guard,20 moves/ghost and42-family hatching");
}
