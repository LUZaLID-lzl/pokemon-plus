#include "game.h"
#include "legacy_game_v7.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint32_t hash(const void *data,unsigned n){const uint8_t *p=data;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;}
int main(void){
 pp_game g;pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=10000;
 assert(pp_items[PP_HATCH_BOOST].price==150);
 assert(pp_buy(&g,PP_HATCH_BOOST)==PP_OK && g.count==1 && g.hatch_boosts==1);
 uint32_t before=pp_checksum(&g);assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_NO_EGG && pp_checksum(&g)==before);
 assert(pp_buy(&g,5)==PP_OK && pp_use(&g,5,0)==PP_OK && !g.egg_shakes);
 uint64_t egg_end=g.egg_end;g.battling=1;g.battle_pet=0;g.battle_hp=g.enemy_hp=1;
 before=pp_checksum(&g);assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_BUSY && pp_checksum(&g)==before);g.battling=0;
 /* v7 migration keeps all published bytes, including a live egg and badge. */
 g.gym.badges=1;g.inventory[1]=17;pp_game_v7 old;memcpy(&old,&g,sizeof(old));old.version=7;
 pp_game restored;memset(&restored,0xff,sizeof(restored));
 assert(pp_import_v7(&restored,&old,sizeof(old),hash(&old,sizeof(old))));
 assert(restored.version==PP_VERSION && restored.hatch_boosts==0 && pp_valid(&restored));
 restored.version=7;assert(!memcmp(&restored,&old,sizeof(old)));restored.version=PP_VERSION;
 before=pp_checksum(&restored);assert(!pp_import_v7(&restored,&old,sizeof(old),0) && pp_checksum(&restored)==before);
 assert(!pp_import_v7(&restored,&old,sizeof(old)-1,hash(&old,sizeof(old)-1)) && pp_checksum(&restored)==before);
 assert(pp_buy(&restored,PP_HATCH_BOOST)==PP_OK);
 pp_game loaded=restored;assert(pp_valid(&loaded) && loaded.hatch_boosts==1 && loaded.egg_end==egg_end);
 uint64_t seconds=loaded.seconds;uint32_t coins=loaded.coins;
 assert(pp_use(&loaded,PP_HATCH_BOOST,0)==PP_OK);
 assert(loaded.count==2 && !loaded.egg_end && !loaded.egg_shakes && !loaded.hatch_boosts && loaded.seconds==seconds && loaded.coins==coins && loaded.gym.badges==1 && loaded.inventory[1]==17 && pp_valid(&loaded));
 pp_tick(&loaded,(unsigned)egg_end,20261024);assert(loaded.count==2); /* can't hatch twice at the former deadline */
 /* A filled courtyard routes the hatch to reserves without replacing anyone. */
 assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK);
 while(g.count<5)assert(pp_test_hatch(&g)==PP_OK);
 assert(pp_buy(&g,5)==PP_OK && pp_use(&g,5,0)==PP_OK && pp_buy(&g,PP_HATCH_BOOST)==PP_OK);
 pp_pet pets[5];memcpy(pets,g.pets,sizeof(pets));assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK);
 assert(g.reserve_count==1 && !g.egg_end && !memcmp(pets,g.pets,sizeof(pets)) && pp_valid(&g));
 puts("PASS: 120-coin bag stock, absent/busy guards, unshaken existing egg only, deadline cancellation, reserve routing, complete frozen v7 migration and persistent tool count");
}
