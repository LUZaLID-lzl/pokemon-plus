#include "game.h"
#include "hatch_fixture.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
static pp_game g;
int main(void){
 assert(PP_SPECIES>=65 && PP_DEX_COUNT>=129 && PP_MOVE_COUNT>=260);
 for(unsigned s=62;s<65;s++){
  pp_init(&g,129);assert(pp_choose_starter(&g,1)==PP_OK);
  g.pets[0].species=s;g.growth[0].level=PP_MAX_LEVEL;assert(pp_valid(&g));
  assert(pp_art_id(&g,0)==s+64);
#if PP_DEX_COUNT >= 130
  if(s==64){
   g.growth[0].level=19;uint32_t before=pp_checksum(&g);
   assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
   g.growth[0].level=20;g.traveler=0;before=pp_checksum(&g);
   assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
   g.count=2;g.pets[1]=g.pets[0];g.growth[1]=g.growth[0];g.coins=0;
   assert(pp_decline_evolution(&g,1)==PP_OK && pp_evolution_locked(&g,1) && !pp_evolution_locked(&g,0));
   assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==129 && pp_valid(&g));
   assert(pp_evolution_offer(&g,1)==PP_EVOLUTION_LOCKED && pp_collected(&g)==1);
   pp_game saved=g;g=saved;assert(pp_valid(&g) && pp_art_id(&g,0)==129);
   assert(pp_evolution_offer(&g,0)==PP_INVALID);
   continue;
  }
#endif
  uint32_t before=pp_checksum(&g);
  assert(pp_evolution_offer(&g,0)==PP_INVALID);
  assert(pp_evolve(&g,0)==PP_INVALID && pp_evolve_stone(&g,0)==PP_INVALID);
  assert(pp_decline_evolution(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  assert(!pp_evolution_stone(&g,0) && !pp_evolution_level(&g,0));
  assert(!pp_collected(&g));
  for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[(s<67?248+slot*5+s-62:268+slot)]);
  pp_game saved=g;g=saved;assert(pp_valid(&g) && pp_art_id(&g,0)==s+64);
  assert(pp_reserve_pet(&g,0)==PP_NEED_TWO); /* Last active pet cannot disappear. */
  g.count=2;g.pets[1]=g.pets[0];g.growth[1]=g.growth[0];
  assert(pp_reserve_pet(&g,1)==PP_OK && g.reserve_count==1 && pp_valid(&g));
  assert(pp_replace_pet(&g,0,0)==PP_OK && pp_art_id(&g,0)==s+64 && pp_valid(&g));
  g.growth[0].stage=1;assert(!pp_valid(&g));
 }
 bool seen[PP_SPECIES]={0};
 for(unsigned seed=1;seed<=8192;seed++){
  pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=10000;
  assert(pp_test_hatch(&g)==PP_OK);seen[g.pets[1].species]=true;
  assert(!g.growth[1].stage && pp_valid(&g));
 }
 assert(seen[62] && seen[63] && seen[64]);
 for(unsigned s=62;s<68;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[(s<67?248+slot*5+s-62:268+slot)]);
 puts("PASS:127+ actual art IDs, saves/bench/hatch/new moves;130 level/busy/permanent individual refusal/gift guards when integrated, otherwise no missing-art evolution");
}
