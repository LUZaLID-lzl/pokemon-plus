#include "game.h"
#include "legacy_game_v11.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(((pp_game*)0)->collected)==73,"Preserve v12 collection bits");
_Static_assert(offsetof(pp_game,pets)==offsetof(pp_game_v11,pets),"Preserve frozen save prefix");
static pp_game fresh(void) {pp_game g;pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);return g;}
static void win(pp_game *g) {
  for(unsigned i=0;i<100&&g->battling;i++) {
    if(g->enemy_hp>0)g->enemy_hp=g->gym.enemy_hp[g->gym.enemy]=1;
    pp_tick(g,PP_BATTLE_TURN_SECONDS,0);assert(pp_valid(g));
  }
  assert(!g->battling && g->gym.result==1);
}
int main(void) {
  pp_game g=fresh();
  for(unsigned badges=0;badges<256;badges++){g.gym.badges=badges;assert(pp_has(&g,PP_GYM_CROWN)==(badges==255));}
  g=fresh();g.gym.badges=127;g.count=3;
  for(unsigned i=0;i<3;i++){g.pets[i]=g.pets[0];g.growth[i]=(pp_growth){.level=60,.stage=2};}
  const unsigned team[]={0,1,2};assert(!pp_has(&g,PP_GYM_CROWN));
  assert(pp_gym_start_at(&g,7,team)==PP_OK);win(&g);
  assert(g.gym.badges==255 && pp_has(&g,PP_GYM_CROWN) && pp_collection_owned(&g,5)==1);
  for(unsigned i=0;i<3;i++)g.pets[i].energy=100;
  assert(pp_gym_start_at(&g,7,team)==PP_OK);win(&g);
  assert(!g.gym.first_clear && pp_collection_owned(&g,5)==1);
  g=fresh();g.gym.badges=255;g.count=PP_PETS;
  for(unsigned i=1;i<g.count;i++)g.pets[i]=g.pets[0];
  memset(g.dex_history,255,sizeof(g.dex_history));
  g.dex_history[(PP_DEX_COUNT-1)/8]&=~(1u<<((PP_DEX_COUNT-1)%8));
  assert(pp_dex_count(&g)==150 && !pp_has(&g,PP_DEX_TOME));
  g.legend.eggs[4]=1;assert(pp_legend_egg_use(&g,4)==PP_OK);g.hatch_boosts=1;
  assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK);
  assert(g.reserve_count==1 && pp_dex_count(&g)==151 && pp_has(&g,PP_DEX_TOME));
  assert(pp_collection_owned(&g,5)==2 && !pp_has(&g,PP_PURE_HEART));
  assert(pp_release(&g,0)==PP_OK && !g.reserve_count && pp_has(&g,PP_DEX_TOME));
  /* Completed v12 progress already grants both: no new flag, draw or write. */
  pp_tick(&g,0,g.day);uint32_t checksum=pp_checksum(&g),rng=g.rng;
  for(unsigned i=0;i<100;i++)assert(pp_has(&g,584)&&pp_has(&g,585)&&!pp_has(&g,586));
  assert(pp_checksum(&g)==checksum && g.rng==rng && pp_valid(&g));
  unsigned size=pp_save_size(&g);void *blob=malloc(size);assert(blob);
  assert(pp_save_write(&g,blob,size,123456));pp_game loaded={0};int64_t wall=0;
  assert(pp_save_read(&loaded,blob,size,&wall) && wall==123456 && loaded.version==PP_VERSION);
  assert(pp_has(&loaded,584)&&pp_has(&loaded,585)&&pp_checksum(&loaded)==checksum);
  for(unsigned i=0;i<PP_TRAVEL_COLLECTION;i++)loaded.collected[i/8]|=1u<<(i%8);
  unsigned coins=loaded.coins;loaded.count=2;loaded.traveler=0;loaded.travel_end=loaded.seconds;
  assert(pp_claim(&loaded)==PP_OK && loaded.coins==coins+50+loaded.travel_events*5);
  assert(pp_collection_owned(&loaded,5)==2 && !pp_has(&loaded,PP_PURE_HEART));
  free(blob);pp_dispose(&g);pp_dispose(&loaded);
  puts("PASS: eighth first clear only, once-only rematch, 150/151 boundary, last legendary hatch, released history, completed v12 save, no RNG/read mutation and travel exclusion");
}
