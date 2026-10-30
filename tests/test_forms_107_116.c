#include "game.h"
#include "hatch_fixture.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
static pp_game g;
static void init(unsigned species,unsigned level){pp_init(&g,116);assert(pp_choose_starter(&g,1)==PP_OK);g.pets[0].species=species;g.growth[0].level=level;assert(pp_valid(&g));}
int main(void){
 assert(PP_SPECIES>=55 && PP_DEX_COUNT>=116 && PP_EVOLUTIONS>=61 && PP_MOVE_COUNT>=220);
 assert(PP_MAX_LEVEL==60);
 init(50,40);pp_gain_xp(&g,0,800+820);assert(g.growth[0].level==42 && g.growth[0].xp==0 && pp_valid(&g));
 assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==111 && pp_valid(&g));
 init(50,39);pp_gain_xp(&g,0,100000);assert(g.growth[0].level==PP_MAX_LEVEL && g.growth[0].xp==0 && pp_valid(&g));
 const unsigned species[]={49,50},levels[]={35,40},bases[]={108,110},dest[]={109,111};
 for(unsigned k=0;k<2;k++){
  init(species[k],levels[k]-1);assert(pp_art_id(&g,0)==bases[k]);uint32_t sum=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==sum);
  g.growth[0].level++;g.traveler=0;sum=pp_checksum(&g);assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==sum);g.traveler=-1;
  assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==dest[k] && pp_valid(&g));
  assert(pp_dex_owned(&g,bases[k]) && pp_dex_owned(&g,dest[k]));pp_game saved=g;assert(pp_valid(&saved));
  g.growth[0].level=levels[k]-1;assert(!pp_valid(&g));
  init(species[k],levels[k]);g.coins=0;g.count=2;g.pets[1]=g.pets[0];g.growth[1]=g.growth[0];
  assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));
  assert(pp_evolution_locked(&g,0) && !pp_evolution_locked(&g,1));saved=g;g=saved;sum=pp_checksum(&g);
  assert(pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==sum);
  assert(pp_evolve_stone(&g,0)==PP_INVALID && pp_checksum(&g)==sum);
  assert(pp_decline_evolution(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==sum);
  assert(pp_decline_evolution(&g,1)==PP_OK && pp_collected(&g)==1 && !g.coins && pp_valid(&g));
 }
 const unsigned finals[]={47,48,51,52,53};
 for(unsigned i=0;i<sizeof(finals)/sizeof(finals[0]);i++){
  init(finals[i],40);uint32_t sum=pp_checksum(&g);assert(pp_evolution_offer(&g,0)==PP_INVALID && pp_checksum(&g)==sum);g.growth[0].stage=1;assert(!pp_valid(&g));
 }
 for(unsigned s=47;s<55;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[188+slot*8+s-47]);
 assert(pp_move_at(47,0)->type==PP_FIGHTING && pp_move_at(48,0)->type==PP_GHOST && pp_move_at(49,3)->type==PP_POISON && pp_move_at(50,3)->type==PP_GROUND && pp_move_at(54,3)->type==PP_WATER);
 bool seen[PP_SPECIES]={0};
 for(unsigned seed=1;seed<=8192;seed++){pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=10000;assert(pp_test_hatch(&g)==PP_OK);unsigned s=g.pets[1].species;assert(s!=1 && s<PP_SPECIES && !g.growth[1].stage && pp_valid(&g));seen[s]=true;}
 for(unsigned s=0;s<PP_SPECIES;s++)if(s!=1)assert(seen[s]==pp_test_ordinary_species(s));
 puts("PASS:107-116 Lv35/40 boundaries,busy/dex/save,permanent individual refusal/unique PureHeart,six final and117 guards,32 moves,55-family hatching");
}
