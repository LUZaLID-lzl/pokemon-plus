#include "game.h"
#include "legacy_game_v16.h"
#include "legacy_game_v13.h"
#include "hatch_fixture.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

static pp_game g;
static void setup(void) {
  pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);
  for(unsigned i=1;i<5;i++) {
    g.pets[i]=g.pets[0];g.growth[i]=g.growth[0];g.identity[i]=g.identity[0];
  }
  g.count=5;
  for(unsigned i=0;i<5;i++)g.inventory[i]=pp_item_enabled(i)?4:0;
  g.pets[0].food=2;g.pets[1].clean=3;g.pets[2].health=4;
  g.pets[3].mood=5;g.pets[4].energy=6;
}
static uint32_t hash(const void *data,unsigned bytes){const uint8_t *p=data;uint32_t h=2166136261u;while(bytes--)h=(h^*p++)*16777619u;return h;}
static void essence(void) {
  setup();g.coins=249;uint32_t before=pp_checksum(&g);
  assert(pp_buy(&g,PP_TREE_ESSENCE)==PP_NO_COINS && pp_checksum(&g)==before);
  g.coins=250;assert(pp_buy(&g,PP_TREE_ESSENCE)==PP_OK && !g.coins && g.tree_essences==1);
  g.tree_essences=UINT16_MAX;g.coins=250;before=pp_checksum(&g);
  assert(pp_buy(&g,PP_TREE_ESSENCE)==PP_FULL && pp_checksum(&g)==before);
  g.tree_essences=2;g.pets[3]=(pp_pet){.species=g.pets[3].species,.food=0,.clean=0,.health=0,.mood=0,.energy=0,.sleeping=1};
  assert(pp_use_auto(&g,PP_TREE_ESSENCE)==PP_OK && g.drop_target==3 && g.tree_essences==1 && pp_valid(&g));
  before=pp_checksum(&g);assert(pp_use_auto(&g,PP_TREE_ESSENCE)==PP_BUSY && pp_checksum(&g)==before);
  pp_game restored={0};unsigned n=pp_save_size(&g);void *blob=malloc(n);assert(blob&&pp_save_write(&g,blob,n,1790000000));
  assert(pp_save_read(&restored,blob,n,NULL) && restored.tree_essences==1 && pp_checksum(&restored)==pp_checksum(&g));free(blob);
  pp_tick(&restored,4,0);const pp_pet *p=&restored.pets[3];
  assert(p->food==100&&p->clean==100&&p->health==100&&p->mood==100&&p->energy==100&&!p->sleeping&&!restored.drop_active);
  before=pp_checksum(&restored);pp_tick(&restored,0,0);assert(pp_checksum(&restored)==before);
  pp_dispose(&restored);pp_dispose(&g);
  setup();g.tree_essences=1;g.pets[0].food=30;g.pets[0].health=30;g.pets[0].energy=30;
  assert(pp_travel(&g,0)==PP_OK);g.pets[2].food=g.pets[2].clean=g.pets[2].health=g.pets[2].mood=g.pets[2].energy=2;
  assert(pp_use_auto(&g,PP_TREE_ESSENCE)==PP_OK && g.drop_target==2);pp_tick(&g,4,0);pp_dispose(&g);
  setup();g.tree_essences=0;before=pp_checksum(&g);assert(pp_use_auto(&g,PP_TREE_ESSENCE)==PP_EMPTY&&pp_checksum(&g)==before);
  g.tree_essences=1;assert(pp_battle(&g,0)==PP_OK);before=pp_checksum(&g);assert(pp_use_auto(&g,PP_TREE_ESSENCE)==PP_BUSY&&pp_checksum(&g)==before);pp_dispose(&g);
  /* Real v13 fixed and variable records retain every individual and quota. */
  setup();g.coins=100000;for(unsigned i=0;i<14;i++)assert(pp_test_hatch(&g)==PP_OK);assert(g.overflow_count);
  g.coins=456;g.balls=7;g.hatch_boosts=8;g.pats_today[0]=12;g.overflow[0].pats_today=19;strcpy(g.identity[0].nickname,"sprout");
  unsigned fixed=offsetof(pp_game_v13,overflow);n=16+fixed+g.overflow_count*sizeof(pp_stored_pet_v16);
  uint8_t *old=calloc(1,n);assert(old);memcpy(old+4,&n,4);int64_t wall=1790000000;memcpy(old+8,&wall,8);memcpy(old+16,&g,fixed);
  uint32_t v=13;memcpy(old+20,&v,4);for(unsigned i=0;i<g.overflow_count;i++)memcpy(old+16+fixed+i*sizeof(pp_stored_pet_v16),&g.overflow[i],sizeof(pp_stored_pet_v16));v=hash(old+4,n-4);memcpy(old,&v,4);
  assert(pp_save_read(&restored,old,n,&wall)&&restored.version==PP_VERSION&&!restored.tree_essences&&restored.coins==456&&restored.balls==7&&restored.hatch_boosts==8&&restored.pats_today[0]==12&&restored.overflow[0].pats_today==19&&!strcmp(restored.identity[0].nickname,"sprout"));
  assert(restored.overflow_count==g.overflow_count&&restored.overflow[0].pats_today==g.overflow[0].pats_today);
  before=pp_checksum(&restored);old[n-1]^=1;assert(!pp_save_read(&restored,old,n,NULL)&&pp_checksum(&restored)==before);free(old);pp_dispose(&restored);pp_dispose(&g);
  puts("PASS: essence200, stock bounds, lowest five-stat sum, skip traveler, full recovery/wake once, atomic busy/empty, in-flight save and frozen v13 overflow/quota migration");
}
int main(void) {
  essence();
  for(unsigned item=0;item<5;item++) {
    if(!pp_item_enabled(item))continue;
    setup();assert(pp_use_auto(&g,item)==PP_OK && g.drop_target==item && g.inventory[item]==3);
    uint32_t before=pp_checksum(&g);
    assert(pp_use_auto(&g,item)==PP_BUSY && pp_checksum(&g)==before);
    pp_tick(&g,4,0);assert(!g.drop_active && pp_valid(&g));
    unsigned value=item==0?g.pets[item].food:item==1?g.pets[item].clean:
      item==2?g.pets[item].health:item==3?g.pets[item].mood:g.pets[item].energy;
    assert(value>(item==0?2:item==1?3:item==2?4:item==3?5:6));
    pp_dispose(&g);
  }
  setup();g.pets[0].food=30;g.pets[1].food=10;assert(pp_travel(&g,0)==PP_OK);
  assert(pp_use_auto(&g,0)==PP_OK && g.drop_target==1 && g.traveler==0);
  pp_tick(&g,4,0);g.pets[2].food=1;
  assert(pp_use_auto(&g,0)==PP_OK && g.drop_target==2);pp_dispose(&g);
  setup();g.pets[4].sleeping=1;
  assert(pp_use_auto(&g,4)==PP_OK && g.drop_target==4);pp_dispose(&g);
  setup();assert(pp_battle(&g,0)==PP_OK);uint32_t before=pp_checksum(&g);
  assert(pp_use_auto(&g,1)==PP_BUSY && pp_checksum(&g)==before);pp_dispose(&g);
  setup();g.egg_end=g.seconds+7200;g.hatch_boosts=1;
  assert(pp_use_auto(&g,PP_HATCH_BOOST)==PP_OK && !g.egg_end && pp_warehouse_count(&g)==1);
  pp_dispose(&g);
  puts("PASS: five care stats select lowest, travelling minimum skipped, next item reselects, sleeping recovery, atomic busy/stock and independent egg boost");
}
