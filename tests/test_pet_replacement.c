#include "game.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static pp_game g;
static void setup(void) {
  pp_dispose(&g);pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=100000;
  while(pp_warehouse_count(&g)<2)assert(pp_test_hatch(&g)==PP_OK);
}
static void swap(unsigned slot) {
  pp_stored_pet incoming,outgoing;assert(pp_warehouse_get(&g,0,&incoming));
  pp_identity original=g.identity[slot];
  assert(pp_replace_pet(&g,slot,0)==PP_OK && pp_valid(&g));
  assert(!memcmp(&g.identity[slot],&incoming.identity,sizeof(original)));
  assert(pp_warehouse_get(&g,0,&outgoing));
  assert(!memcmp(&outgoing.identity,&original,sizeof(original)));
}
static void blocked(unsigned slot) {
  uint32_t before=pp_checksum(&g);
  assert(pp_replace_pet(&g,slot,0)==PP_BUSY && pp_checksum(&g)==before);
}
int main(void) {
  setup();assert(pp_travel(&g,0)==PP_OK);
  pp_identity traveler=g.identity[0];uint64_t end=g.travel_end;unsigned reward=g.travel_reward;
  blocked(0);swap(1);
  assert(g.traveler==0 && g.travel_end==end && g.travel_reward==reward && !memcmp(&g.identity[0],&traveler,sizeof(traveler)));
  setup();g.inventory[0]=1;assert(pp_use(&g,0,0)==PP_OK);
  pp_pet eater=g.pets[0];end=g.drop_end;unsigned active=g.drop_active;
  blocked(0);swap(1);
  assert(g.drop_target==0 && g.drop_end==end && g.drop_active==active && !memcmp(&g.pets[0],&eater,sizeof(eater)));
  setup();assert(pp_battle(&g,0)==PP_OK);blocked(0);blocked(1);
  setup();const unsigned team[]={0,1,2};assert(pp_gym_start(&g,team)==PP_OK);blocked(0);blocked(4);
  /* Pending encounters and completed receipts have no active fight. Keep the
   * receipt/capture transaction intact while permitting unrelated UI actions. */
  setup();g.wild=(pp_wild_state){.phase=PP_WILD_PENDING,.dex=0,.level=1,.escape_at=g.seconds+15};
  pp_wild_state pending=g.wild;swap(1);assert(!memcmp(&pending,&g.wild,sizeof(pending)));
  assert(pp_wild_start(&g,0)==PP_OK);blocked(1);g.enemy_hp=1;pp_tick(&g,3,0);
  assert(g.wild.phase==PP_WILD_WON && g.wild.outcome==PP_WILD_CAPTURE_READY);
  pp_wild_state receipt=g.wild;unsigned coins=g.coins,owned=pp_warehouse_count(&g);
  swap(1);assert(g.coins==coins && !memcmp(&receipt,&g.wild,sizeof(receipt)));
  g.balls=1;assert(pp_wild_capture(&g)==PP_OK && pp_warehouse_count(&g)==owned+1 && !g.balls);
  swap(1);assert(g.wild.outcome==PP_WILD_CAUGHT && pp_wild_capture(&g)==PP_INVALID);
  setup();g.wild=(pp_wild_state){.phase=PP_WILD_LOST,.dex=0,.level=1};
  receipt=g.wild;swap(1);assert(!memcmp(&receipt,&g.wild,sizeof(receipt)));
  /* Adding a new courtyard pet also preserves another pet's trip index. */
  setup();assert(pp_reserve_pet(&g,4)==PP_OK);assert(pp_travel(&g,0)==PP_OK);
  assert(pp_replace_pet(&g,g.count,0)==PP_OK && g.count==5 && g.traveler==0 && pp_valid(&g));
  pp_dispose(&g);
  puts("PASS: replace idle pet during another trip/item use; atomic busy target/combat; pending/won/lost receipts preserved; once-only capture; add slot during travel");
}
