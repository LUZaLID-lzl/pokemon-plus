#include "game.h"
#include "hatch_fixture.h"
#include "legacy_game_v1.h"
#include "legacy_game_v2.h"
#include "legacy_game_v3.h"
#include "pedometer.h"
#include <math.h>
#include <limits.h>
#include "moves.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static void init(void) {
  pp_init(&g, 123);
  pp_tick(&g, 0, 20261022);
  assert(pp_valid(&g));
}
static void hatch(void) {
  g.coins += 200;
  assert(pp_buy(&g, 5) == PP_OK);
  assert(pp_use(&g, 5, 0) == PP_OK);
  assert(pp_shake(&g) == PP_OK);
  pp_tick(&g, 86400, g.day);
}
static unsigned earned_xp(const pp_growth *p) { return 10u*p->level*(p->level-1)+p->xp; }
static void test_walk(void) {
  init(); assert(pp_choose_starter(&g,1)==PP_OK);
  g.count=2; g.pets[1]=g.pets[0];
  unsigned coins=g.coins;
  pp_record_steps(&g,999);assert(g.coins==coins&&!g.steps_xp);
  pp_record_steps(&g,1);assert(g.steps_coins==30&&g.steps_xp==20);
  pp_record_steps(&g,4000);assert(g.steps_coins==150&&g.steps_xp==100);assert(earned_xp(&g.growth[0])==100&&earned_xp(&g.growth[1])==100);
  pp_game restored=g;uint32_t checksum=pp_checksum(&g);pp_record_steps(&restored,0);assert(pp_checksum(&restored)==checksum);pp_record_steps(&restored,5000);assert(restored.steps_coins==200);
  unsigned xp=restored.steps_xp;pp_record_steps(&restored,10000);assert(restored.steps_coins==200&&restored.steps_xp==xp);
  pp_tick(&restored,0,20261023);assert(!restored.steps_today&&restored.total_steps==20000);pp_record_steps(&restored,1000);assert(restored.steps_coins==30);
  restored.coins=UINT32_MAX-2;pp_record_steps(&restored,1000);assert(restored.coins==UINT32_MAX&&pp_valid(&restored));restored.steps_rewarded=1000;assert(!pp_valid(&restored));
  pp_pedometer detector={0}; unsigned n=0;
  for(unsigned ms=0;ms<60000;ms+=50) n+=pp_pedometer_sample(&detector,ms,0,0,9.80665f+.1f*sinf(ms*.02f));
  assert(!n);
  for(unsigned ms=60000;ms<70000;ms+=50) n+=pp_pedometer_sample(&detector,ms,0,0,9.80665f+2.8f*sinf(ms*.01256637f));
  assert(n>=17 && n<=21);
  detector=(pp_pedometer){0}; n=0;
  for(unsigned ms=0;ms<30000;ms+=50) n+=pp_pedometer_sample(&detector,ms,9.80665f*sinf(ms*.003f),0,9.80665f*cosf(ms*.003f));
  assert(!n);
  detector=(pp_pedometer){0}; n=0;
  for(unsigned ms=0;ms<30000;ms+=50) n+=pp_pedometer_sample(&detector,ms,0,0,9.80665f+4*sinf(ms*.0376991f));
  assert(!n);
  assert(!pp_pedometer_sample(&detector,50000,NAN,0,9.8f));
  puts("PASS: stationary/rotation/fast-shake rejection, walking cadence, atomic milestone rewards, tiers, all-pet XP, daily reset and overflow");
}
static void test_v2_and_eggs(void) {
  init(); pp_choose_starter(&g,4); g.coins=1234; g.growth[0].level=16; g.growth[0].stage=1;
  g.inventory[5]=1; assert(pp_use(&g,5,0)==PP_OK); g.egg_end=g.seconds+86400;
  pp_game_v2 old={0}; memcpy(&old,&g,sizeof(old)); old.version=2;
  uint32_t hash=2166136261u;
  for(unsigned i=0;i<sizeof(old);i++) hash=(hash^((uint8_t*)&old)[i])*16777619u;
  pp_game migrated; assert(pp_import_v2(&migrated,&old,sizeof(old),hash));
  assert(pp_valid(&migrated) && migrated.coins==1234 && migrated.growth[0].stage==1 && migrated.pets[0].species==2);
  assert(migrated.egg_end-migrated.seconds==14400 && !migrated.total_steps);
  assert(!pp_import_v2(&migrated,&old,sizeof(old),hash+1));
  unsigned min=UINT_MAX,max=0;
  for(unsigned seed=1;seed<=300;seed++) {
    pp_init(&g,seed); g.inventory[5]=1; assert(pp_use(&g,5,0)==PP_OK);
    int ex,ey;pp_egg_position(&g,&ex,&ey);
    assert(ex==128 && ey==150);
    pp_game saved_egg=g;int sx,sy;pp_egg_position(&saved_egg,&sx,&sy);assert(ex==sx && ey==sy);
    unsigned duration=(unsigned)(g.egg_end-g.seconds);
    assert(duration>=7200 && duration<=14400 && duration%60==0);
    if(duration<min) min=duration; if(duration>max) max=duration;
    assert(pp_shake(&g)==PP_OK); pp_tick(&g,duration-1,20261022); assert(g.count==1);
    pp_tick(&g,1,20261022); assert(g.count==2 && !g.egg_end);
  }
  assert(max>min);
  puts("PASS: v2 migration preserves pets/save state, caps existing eggs at 4h; randomized 2–4h eggs never hatch early");
}
static void test_shop_and_random_drops(void) {
  init();pp_choose_starter(&g,1);g.coins=149;
  uint32_t before=pp_checksum(&g);
  assert(pp_buy(&g,PP_HATCH_BOOST)==PP_NO_COINS && pp_checksum(&g)==before);
  g.coins=150;assert(pp_buy(&g,PP_HATCH_BOOST)==PP_OK);
  assert(g.coins==0 && g.count==1 && !g.egg_end && g.hatch_boosts==1);
  before=pp_checksum(&g);assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_NO_EGG && pp_checksum(&g)==before);
  g.inventory[5]=1;assert(pp_use(&g,5,0)==PP_OK); /* no shake needed */
  uint64_t seconds=g.seconds;uint32_t coins=g.coins;
  assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK);
  assert(g.count==2 && !g.egg_end && !g.hatch_boosts && g.seconds==seconds && g.coins==coins && pp_valid(&g));
  before=pp_checksum(&g);assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_EMPTY && pp_checksum(&g)==before);
  g.hatch_boosts=UINT16_MAX;g.coins=500;before=pp_checksum(&g);
  assert(pp_buy(&g,PP_HATCH_BOOST)==PP_FULL && pp_checksum(&g)==before);
  init();pp_choose_starter(&g,1);g.coins=10000;
  for(unsigned i=1;i<PP_PETS+PP_RESERVES;i++)assert(pp_test_hatch(&g)==PP_OK);
  assert(g.count==5 && g.reserve_count==11);
  assert(pp_buy(&g,PP_HATCH_BOOST)==PP_OK); /* Tool purchase doesn't need a free pet slot. */
  g.egg_end=g.seconds+7200;before=pp_checksum(&g);
  assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK && pp_warehouse_count(&g)==12 && !g.egg_end);
  pp_dispose(&g);
  init(); pp_choose_starter(&g,1); g.inventory[0]=101;
  bool spots[PP_DROP_SPOTS]={0};unsigned used=0;
  for(unsigned i=0;i<100;i++) {
    g.pets[0].food=0;
    assert(pp_use(&g,0,0)==PP_OK && g.pets[0].food==0);
    int x,y;pp_drop_position(&g,&x,&y);
    assert(x>=70 && x<=270 && y>=145 && y<=211);
    spots[g.drop_active-2]=true;
    pp_game saved=g; int sx,sy;pp_drop_position(&saved,&sx,&sy);
    assert(sx==x && sy==y && pp_valid(&saved));
    before=pp_checksum(&g);assert(pp_use(&g,0,0)==PP_BUSY && pp_checksum(&g)==before);
    pp_tick(&g,3,g.day);int tx,ty;pp_drop_position(&g,&tx,&ty);
    assert(tx==x && ty==y && g.pets[0].food==0);
    pp_tick(&g,1,g.day);assert(!g.drop_active && g.pets[0].food==40);
    pp_tick(&g,1,g.day);assert(g.pets[0].food==40 && pp_valid(&g));
  }
  for(unsigned i=0;i<PP_DROP_SPOTS;i++) used+=spots[i];assert(used>=20);
  g.drop_active=1;g.drop_item=0;g.drop_target=0;
  int x,y;pp_drop_position(&g,&x,&y);assert(x==164 && y==234 && pp_valid(&g));
  g.drop_active=PP_DROP_SPOTS+2;assert(!pp_valid(&g));
  puts("PASS: 120-coin stocked hatch boost, no-egg/full atomic guards, unshaken egg completion and reserve capacity; randomized safe drops, saved positions and single-use effects");
}
static void test_missing_species_and_pokedex(void) {
  for(unsigned mode=0;mode<2;mode++) for(unsigned seed=1;seed<=100;seed++) {
    pp_init(&g,seed);pp_choose_starter(&g,1+(seed%3)*3);
    bool seen[PP_SPECIES]={0}; seen[g.pets[0].species]=true;
    for(unsigned n=1;n<PP_PETS;n++) {
      if(mode) {g.coins=330;assert(pp_test_hatch(&g)==PP_OK);}
      else {g.inventory[5]=1;assert(pp_use(&g,5,0)==PP_OK);assert(pp_shake(&g)==PP_OK);pp_tick(&g,(unsigned)(g.egg_end-g.seconds),20261023);}
      unsigned species=g.pets[n].species;
      if(n<PP_SPECIES) assert(!seen[species]);
      seen[species]=true;assert(g.count==n+1 && pp_valid(&g));
    }
    unsigned seen_count=0;for(unsigned i=0;i<PP_SPECIES;i++) seen_count+=seen[i];assert(seen_count==PP_PETS);
  }
  init(); assert(pp_dex_count(&g)==0);
  pp_choose_starter(&g,7);assert(pp_dex_count(&g)==1);
  unsigned order[PP_DEX_COUNT];pp_dex_order(&g,order);
  for(unsigned i=0;i<PP_DEX_COUNT;i++) {
    assert(pp_pokedex[order[i]].national_id==i+1);
    assert(pp_dex_owned(&g,order[i])==(i==6));
  }
  g.growth[0].level=16;assert(pp_evolve(&g,0)==PP_OK);assert(pp_dex_count(&g)==2);
  g.growth[0].level=36;assert(pp_evolve(&g,0)==PP_OK);assert(pp_dex_count(&g)==3);
  pp_dex_order(&g,order);
  for(unsigned i=0;i<PP_DEX_COUNT;i++) {
    assert(pp_pokedex[order[i]].national_id==i+1);
    assert(pp_dex_owned(&g,order[i])==(i>=6 && i<=8));
  }
  pp_game restored=g;assert(pp_dex_count(&restored)==3 && pp_valid(&restored));
  assert(!pp_dex_owned(&g,PP_DEX_COUNT));
  g.count=4;
  for(unsigned i=0;i<4;i++){g.pets[i].species=i;g.growth[i].stage=i?2:0;g.growth[i].level=36;}
  assert(pp_dex_count(&g)==10);pp_dex_order(&g,order);
  for(unsigned i=0;i<PP_DEX_COUNT;i++) assert(pp_pokedex[order[i]].national_id==i+1);
  for(unsigned family=4;family<7;family++) {
    init();g.pets[0].species=family;
    unsigned levels[2]={family==6?18:7,family==6?36:10};
    unsigned base=10+(family-4)*3;
    assert(pp_art_id(&g,0)==base && pp_dex_count(&g)==1);
    for(unsigned stage=0;stage<2;stage++){
      assert(pp_evolution_level(&g,0)==levels[stage]);
      g.growth[0].level=levels[stage]-1;assert(pp_evolve(&g,0)==PP_NOT_READY);
      g.growth[0].level++;g.traveler=0;assert(pp_evolve(&g,0)==PP_BUSY);g.traveler=-1;
      assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==base+stage+1 && pp_valid(&g));
      assert(pp_dex_count(&g)==stage+2);
      pp_game saved=g;assert(pp_valid(&saved)&&pp_art_id(&saved,0)==base+stage+1);
      saved.growth[0].level--;assert(!pp_valid(&saved));
    }
    assert(pp_evolve(&g,0)==PP_INVALID);
  }
  puts("PASS: both egg types unlock all missing base species before duplicates; dex national order regardless of ownership, evolution history, complete roster and save restoration");
}

static void test_forms_019_027(void) {
  const unsigned families[]={7,8,9};const unsigned levels[]={20,20,22};const unsigned arts[]={20,22,24};
  for(unsigned i=0;i<3;i++) {
    init();g.pets[0].species=families[i];g.growth[0].level=levels[i]-1;
    assert(pp_evolution_level(&g,0)==levels[i] && pp_evolve(&g,0)==PP_NOT_READY);
    g.growth[0].level++;g.traveler=0;uint32_t sum=pp_checksum(&g);
    assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==sum);g.traveler=-1;
    assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==arts[i] && pp_dex_count(&g)==2 && pp_valid(&g));
    assert(!pp_evolution_level(&g,0) && pp_evolve(&g,0)==PP_INVALID);
    pp_game saved=g;assert(pp_valid(&saved)&&pp_art_id(&saved,0)==arts[i]);
    saved.growth[0].stage=2;assert(!pp_valid(&saved));
  }
  init();g.pets[0].species=10;g.growth[0].level=PP_MAX_LEVEL;
  assert(pp_art_id(&g,0)==26 && pp_evolution_level(&g,0)==22 && pp_evolve(&g,0)==PP_OK);
  assert(pp_valid(&g) && pp_art_id(&g,0)==27 && pp_evolve(&g,0)==PP_INVALID);
  init();assert(pp_evolve_thunder_stone(&g,0)==PP_INVALID); /* unchosen default */
  g.count=2;g.pets[1].species=1;g.coins=199;uint32_t sum=pp_checksum(&g);
  assert(pp_evolve_thunder_stone(&g,0)==PP_NO_COINS && pp_checksum(&g)==sum);
  g.coins=500;g.growth[0].level=PP_MAX_LEVEL;assert(pp_evolve(&g,0)==PP_INVALID && !pp_evolution_level(&g,0));
  g.traveler=0;sum=pp_checksum(&g);assert(pp_evolve_thunder_stone(&g,0)==PP_BUSY && pp_checksum(&g)==sum);g.traveler=-1;
  g.growth[0].level=1;uint16_t stock[PP_ITEMS];memcpy(stock,g.inventory,sizeof(stock));
  assert(pp_evolve_thunder_stone(&g,1)==PP_INVALID);
  assert(pp_evolve_thunder_stone(&g,0)==PP_OK && g.coins==300 && pp_art_id(&g,0)==25 && pp_valid(&g));
  assert(!strcmp(pp_form_name(&g,0),"雷丘") && !memcmp(stock,g.inventory,sizeof(stock)));
  sum=pp_checksum(&g);assert(pp_evolve_thunder_stone(&g,0)==PP_INVALID && pp_checksum(&g)==sum);
  pp_game saved=g;assert(pp_valid(&saved)&&pp_art_id(&saved,0)==25);saved.growth[0].stage=2;assert(!pp_valid(&saved));
  puts("PASS: 019/021/023 single-stage gates and final guards; 025 stone-only atomic cost, busy/no-coins/repeat guards, save restoration; 027 to 028 final guard");
}

static uint32_t raw_hash(const void *data, unsigned bytes) {
  const uint8_t *p=data;uint32_t h=2166136261u;
  for(unsigned i=0;i<bytes;i++) h=(h^p[i])*16777619u;
  return h;
}
static void test_evolution_choice(void) {
  init();pp_choose_starter(&g,1);
  uint32_t before=pp_checksum(&g);
  assert(pp_decline_evolution(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
  g.growth[0].level=16;g.traveler=0;before=pp_checksum(&g);
  assert(pp_decline_evolution(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
  g.battling=1;g.battle_pet=0;before=pp_checksum(&g);
  assert(pp_decline_evolution(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.battling=0;
  g.drop_active=1;g.drop_target=0;before=pp_checksum(&g);
  assert(pp_decline_evolution(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.drop_active=0;
  unsigned coins=g.coins,art=pp_art_id(&g,0);
  assert(pp_decline_evolution(&g,0)==PP_OK && pp_evolution_locked(&g,0));
  assert(g.coins==coins && pp_art_id(&g,0)==art && g.growth[0].stage==0);
  assert(pp_has(&g,PP_PURE_HEART) && pp_collected(&g)==1 && pp_valid(&g));
  before=pp_checksum(&g);
  assert(pp_decline_evolution(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
  assert(pp_evolve(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
  pp_gain_xp(&g,0,UINT32_MAX);
  pp_game restored;memcpy(&restored,&g,sizeof(g));
  assert(pp_valid(&restored) && pp_checksum(&restored)==pp_checksum(&g));
  assert(restored.growth[0].level==PP_MAX_LEVEL && !pp_evolution_level(&restored,0));
  assert(pp_evolve(&restored,0)==PP_EVOLUTION_LOCKED);
  /* Same species, different individual; lock persists through stage two. */
  g.count=2;g.pets[1]=g.pets[0];g.growth[1].level=16;
  assert(!pp_evolution_locked(&g,1) && pp_evolve(&g,1)==PP_OK);
  g.growth[1].level=32;assert(pp_decline_evolution(&g,1)==PP_OK);
  assert(g.growth[1].stage==1 && pp_collected(&g)==1 && pp_valid(&g));
  assert(pp_evolve(&g,1)==PP_EVOLUTION_LOCKED);
  init();g.count=2;g.pets[1].species=1;g.coins=0;
  assert(pp_decline_evolution(&g,0)==PP_OK && g.coins==0 && pp_art_id(&g,0)==0);
  g.coins=500;before=pp_checksum(&g);
  assert(pp_evolve_thunder_stone(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before);
  init();g.pets[0].species=10;g.growth[0].stage=1;g.growth[0].level=PP_MAX_LEVEL;before=pp_checksum(&g);
  assert(pp_decline_evolution(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  init();pp_choose_starter(&g,4);g.growth[0].level=36;g.growth[0].stage=2;before=pp_checksum(&g);
  assert(pp_decline_evolution(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  /* Frozen v3 migration preserves actual state, unlocks no unearned gift. */
  g.total_steps=900;g.steps_today=800;g.steps_rewarded=8;g.coins=987;
  g.collected[2]=0x55;pp_game_v3 old;memcpy(&old,&g,sizeof(old));old.version=3;
  uint32_t hash=raw_hash(&old,sizeof(old));
  assert(pp_import_v3(&restored,&old,sizeof(old),hash) && pp_valid(&restored));
  assert(restored.version==PP_VERSION && restored.coins==987 && restored.total_steps==900);
  assert(restored.growth[0].stage==2 && restored.collected[2]==0x55);
  assert(!restored.evolution_declined && !pp_has(&restored,PP_PURE_HEART));
  before=pp_checksum(&restored);
  assert(!pp_import_v3(&restored,&old,sizeof(old)-1,hash));
  assert(!pp_import_v3(&restored,&old,sizeof(old),hash+1) && pp_checksum(&restored)==before);
  restored.evolution_declined=1ULL<<4;assert(!pp_valid(&restored));
  /* Travel drops never award the exclusive gift. */
  init();pp_choose_starter(&g,1);g.count=2;g.pets[1].species=2;
  for(unsigned i=0;i<1000;i++) {g.traveler=0;g.travel_end=g.seconds;assert(pp_claim(&g)==PP_OK);}
  assert(!pp_has(&g,PP_PURE_HEART));
  puts("PASS: permanent per-pet evolution refusal, atomic unique Pure Heart, unchanged coins/art, busy/early/final guards, level-cap and stone bypass prevention, v4 save restoration and frozen v3 migration");
}

static void test_collection(void) {
  static const unsigned quotas[]={350,146,58,23,6,3};
  bool seen[PP_COLLECTION]={0};unsigned all=0;
  init();
  for(unsigned q=0;q<PP_QUALITY_COUNT;q++) {
    assert(pp_collection_total(q)==quotas[q]);
    for(unsigned index=0;index<quotas[q];index++) {
      unsigned id=pp_collection_id(q,index);
      assert(id<PP_COLLECTION && !seen[id] && pp_collectibles[id].quality==q);
      assert(pp_collectibles[id].name[0] && (id>=PP_TRAVEL_COLLECTION || pp_collectibles[id].scene<PP_SCENES));
      seen[id]=true;all++;if(id<PP_STORED_COLLECTION)g.collected[id/8]|=1u<<(id%8);
    }
    assert(pp_collection_id(q,quotas[q])==PP_COLLECTION);
    assert(pp_collection_owned(&g,q)==(q==5?1:quotas[q]));
  }
  assert(all==PP_COLLECTION && pp_collected(&g)==584 && pp_collection_owned(&g,PP_QUALITY_ALL)==584);
  assert(pp_collection_id(PP_QUALITY_ALL,0)==PP_PURE_HEART);
  for(unsigned i=0;i<PP_SPECIAL_COLLECTION;i++)assert(pp_collection_id(PP_QUALITY_ALL,i)==PP_TRAVEL_COLLECTION+i);
  for(unsigned i=0;i<PP_TRAVEL_COLLECTION;i++)assert(pp_collection_id(PP_QUALITY_ALL,i+PP_SPECIAL_COLLECTION)==i);
  assert(pp_collection_total(7)==0 && pp_collection_id(7,0)==PP_COLLECTION && pp_collection_owned(&g,7)==0);
  /* Frozen v4 bitset roundtrip; drawing/filtering must never grant new bits. */
  pp_game saved=g;assert(pp_valid(&saved));uint32_t checksum=pp_checksum(&saved);
  for(unsigned q=0;q<7;q++)for(unsigned i=0;i<pp_collection_total(q);i++)(void)pp_collection_id(q,i);
  assert(pp_checksum(&saved)==checksum);
  /* Every claim adds one missing travel item, preserving the separate gift. */
  memset(g.collected,0,sizeof(g.collected));
  g.collected[PP_PURE_HEART/8]|=1u<<(PP_PURE_HEART%8);
  g.count=2;g.pets[1]=g.pets[0];
  for(unsigned i=0;i<PP_TRAVEL_COLLECTION;i++) {
    g.traveler=0;g.travel_end=g.seconds;
    assert(pp_claim(&g)==PP_OK && pp_collected(&g)==i+2);
    assert(strstr(g.log[3],"新收藏") && pp_has(&g,PP_PURE_HEART));
  }
  /* A single missing item must be returned, whatever its quality. */
  for(unsigned q=0;q<5;q++) {
    unsigned id=pp_collection_id(q,0);g.collected[id/8]&=~(1u<<(id%8));
    g.traveler=0;g.travel_end=g.seconds;assert(pp_claim(&g)==PP_OK);
    assert(pp_has(&g,id) && pp_collected(&g)==584);
  }
  /* A missing special gift must not block travel-pool completion. */
  g.collected[PP_PURE_HEART/8]&=~(1u<<(PP_PURE_HEART%8));
  for(unsigned i=0;i<3;i++) {
    g.growth[0]=(pp_growth){.level=1};g.traveler=0;g.travel_end=g.seconds;
    g.travel_xp_snapshot=4;g.travel_reward=i*5;g.travel_events=i;uint32_t coins=g.coins,rng=g.rng;
    uint8_t collected[sizeof(g.collected)];memcpy(collected,g.collected,sizeof(collected));
    assert(pp_claim(&g)==PP_OK && g.coins==coins+50+i*5);
    assert(earned_xp(&g.growth[0])==4 && g.rng==rng && !memcmp(collected,g.collected,sizeof(collected)));
    assert(strstr(g.log[3],"旅行藏品已集齐") && !pp_has(&g,PP_PURE_HEART));
    coins=g.coins;assert(pp_claim(&g)==PP_INVALID && g.coins==coins);
  }
  puts("PASS: stable collection qualities/IDs, 583 claims without repeats, last missing item for every quality, full pool coins/XP only, special exclusion and no duplicate claim");
}
int main(void) {
  test_collection();
  test_evolution_choice();
  test_forms_019_027();
  test_missing_species_and_pokedex();
  test_shop_and_random_drops();
  test_walk();
  test_v2_and_eggs();
  const unsigned starters[] = {1, 4, 7};
  for (unsigned i = 0; i < 3; i++) {
    init();
    uint32_t before = pp_checksum(&g);
    assert(pp_choose_starter(&g, 25) == PP_INVALID && pp_checksum(&g) == before);
    assert(pp_needs_starter(&g));
    assert(pp_choose_starter(&g, starters[i]) == PP_OK);
    assert(g.pets[0].species == i + 1 && g.growth[0].level == 1 && g.count == 1);
    assert(pp_valid(&g) && !pp_needs_starter(&g) && g.coins == 240);
    pp_game rebooted = g;
    assert(!pp_needs_starter(&rebooted));
    assert(pp_choose_starter(&g, starters[(i + 1) % 3]) == PP_INVALID);
  }
  puts("PASS: 001/004/007 starter selection, invalid IDs, no reroll and saved-state persistence");
  init();
  uint32_t c = g.coins;
  assert(pp_pat(&g, 0) == PP_OK);
  assert(g.coins == c + 1 && g.pets[0].mood == 82);
  assert(pp_pat(&g, 5) == PP_INVALID);
  assert(pp_buy(&g, 9) == PP_INVALID);
  g.coins = 0;
  assert(pp_buy(&g, 0) == PP_NO_COINS);
  init();
  g.pets[0].food = 10;
  assert(pp_use(&g, 0, 0) == PP_OK);
  assert(g.inventory[0] == 2 && g.pets[0].food == 10);
  pp_tick(&g, 3, g.day);
  assert(g.pets[0].food == 10);
  pp_tick(&g, 1, g.day);
  assert(g.pets[0].food == 50 && !g.drop_active);
  assert(pp_use(&g, 0, 9) == PP_INVALID);
  init();
  assert(pp_travel(&g, 0) == PP_NEED_TWO);
  hatch();
  assert(g.count == 2 && g.pets[1].species != 0);
  assert(pp_travel(&g, 1) == PP_OK);
  assert(pp_pat(&g, 1) == PP_BUSY);
  assert(pp_claim(&g) == PP_NOT_READY);
  pp_tick(&g, 10799, g.day);
  assert(pp_claim(&g) == PP_NOT_READY);
  pp_tick(&g, 1, g.day);
  assert(pp_claim(&g) == PP_OK);
  assert(pp_collected(&g) == 1);
  assert(pp_claim(&g) == PP_INVALID);
  init();
  for (int i = 0; i < 4; i++)
    hatch();
  assert(g.count == 5);
  bool hatch_seen[PP_SPECIES]={0};
  for(unsigned i=0;i<4;i++){assert(!hatch_seen[g.pets[i].species]);hatch_seen[g.pets[i].species]=true;}
  g.inventory[5]++;
  assert(pp_use(&g, 5, 0) == PP_OK);
  assert(g.inventory[5] == 0);
  init();
  g.inventory[5] = 1;
  assert(pp_use(&g, 5, 0) == PP_OK);
  pp_tick(&g, 86400, g.day);
  assert(g.count == 1);
  pp_shake(&g);
  pp_tick(&g, 1, g.day);
  assert(g.count == 2);
  init();
  for (int i = 0; i < 6; i++) {
    assert(pp_dig_start(&g) == PP_OK);
    for (int j = 0; j < 100 && g.digging; j++)
      pp_shake(&g);
    assert(!g.digging);
  }
  assert(pp_dig_start(&g) == PP_DAILY_LIMIT);
  pp_tick(&g, 0, 20261021);
  assert(g.digs_today == 6);
  pp_tick(&g, 0, 20261023);
  assert(pp_dig_start(&g) == PP_OK);
  assert(pp_dig_stop(&g) == PP_OK);
  init();
  assert(pp_tarot(&g) == PP_OK);
  uint32_t card = g.tarot_card;
  assert(pp_tarot(&g) == PP_DAILY_LIMIT && g.tarot_card == card);
  pp_tick(&g, 0, 20261023);
  assert(pp_tarot(&g) == PP_OK);
  pp_tick(&g, 0, 0);
  assert(pp_tarot(&g) == PP_NOT_READY);
  assert(pp_dig_start(&g) == PP_NOT_READY);
  init();
  assert(pp_battle(&g, 0) == PP_OK);
  assert(pp_battle(&g, 0) == PP_BUSY);
  pp_tick(&g, 120, g.day);
  assert(!g.battling);
  assert(g.stage == 1);
  assert(pp_valid(&g));
  for (unsigned species = 0; species < PP_SPECIES; species++) {
    init();
    g.pets[0].species = species;
    assert(pp_battle(&g, 0) == PP_OK);
    pp_tick(&g, 2, g.day);
    assert(strstr(g.log[3], pp_move_at(species,0)->name));
    assert((g.enemy_hp < g.enemy_max_hp)==(pp_battle_move_power(&g,0,0)>0));
    assert((g.battle_hp < g.battle_max_hp)==(pp_battle_move_power(&g,1,0)>0));
    assert(pp_valid(&g));
  }
  init();
  assert(pp_scene(&g, 11) == PP_NOT_READY);assert(pp_choose_starter(&g,1)==PP_OK);for(unsigned d=0;d<100;d++)g.dex_history[d/8]|=1u<<(d%8);
  g.coins = 500;
  assert(pp_scene(&g, 11) == PP_OK);
  assert(g.coins == 500);
  assert(pp_scene(&g, 11) == PP_OK && g.coins == 500);
  assert(pp_scene(&g, PP_SCENES) == PP_INVALID);
  init();
  g.pets[0].energy = 19;
  pp_tick(&g, 600, g.day);
  assert(g.pets[0].sleeping);
  pp_tick(&g, 600 * 10, g.day);
  assert(!g.pets[0].sleeping);
  assert(pp_valid(&g));
  init();
  uint32_t checksum = pp_checksum(&g);
  g.coins++;
  assert(pp_checksum(&g) != checksum);
  g.count = 6;
  assert(!pp_valid(&g));
  init();
  g.traveler = -2;
  assert(!pp_valid(&g));
  init();
  memset(g.log[0], 'x', 96);
  assert(!pp_valid(&g));
  init();
  g.pets[0].species = 2;
  assert(pp_evolve(&g, 0) == PP_NOT_READY);
  g.growth[0].level = 15;
  g.growth[0].xp = 299;
  pp_gain_xp(&g, 0, 1);
  assert(g.growth[0].level == 16 && g.growth[0].xp == 0);
  assert(pp_evolution_level(&g, 0) == 16);
  assert(pp_battle(&g, 0) == PP_OK);
  assert(pp_evolve(&g, 0) == PP_BUSY);
  g.battling = 0;
  assert(pp_evolve(&g, 0) == PP_OK && g.growth[0].stage == 1);
  assert(!strcmp(pp_form_name(&g, 0), "火恐龙"));
  assert(pp_evolve(&g, 0) == PP_NOT_READY);
  g.growth[0].level = 36;
  assert(pp_evolve(&g, 0) == PP_OK && pp_art_id(&g, 0) == 5);
  assert(pp_evolve(&g, 0) == PP_INVALID);
  pp_gain_xp(&g, 0, UINT32_MAX);
  assert(g.growth[0].level == PP_MAX_LEVEL && g.growth[0].xp == 0 && pp_valid(&g));
  assert(pp_evolve(&g, 5) == PP_INVALID);
  for (unsigned species = 0; species < PP_SPECIES; species++) {
    for (unsigned slot = 0; slot < PP_MOVE_SLOTS; slot++)
      assert(pp_move_at(species, slot));
    assert(!pp_move_at(species, 4));
    init();
    g.pets[0].species = species;
    assert(pp_battle(&g, 0) == PP_OK);
    for (unsigned turn = 0; turn < 8; turn++) {
      g.enemy_hp = g.enemy_max_hp = 200;
      g.battle_hp = g.battle_max_hp = 200;
      pp_tick(&g, 2, g.day);
      assert(g.battle_round == turn + 1);
      assert(strstr(g.log[3], pp_move_at(species, turn % 4)->name));
    }
  }
  init();
  pp_game_v1 old;
  memset(&old, 0, sizeof(old));
  assert(offsetof(pp_game, growth) <= sizeof(old));
  memcpy(&old, &g, offsetof(pp_game, growth));
  old.version = 1;
  uint32_t hash = 2166136261u;
  for (unsigned i = 0; i < sizeof(old); i++)
    hash = (hash ^ ((uint8_t *)&old)[i]) * 16777619u;
  pp_game migrated;
  assert(pp_import_v1(&migrated, &old, sizeof(old), hash));
  assert(pp_valid(&migrated) && migrated.coins == g.coins &&
         migrated.growth[0].level == 1);
  uint32_t before = pp_checksum(&migrated);
  assert(!pp_import_v1(&migrated, &old, sizeof(old) - 1, hash));
  assert(!pp_import_v1(&migrated, &old, sizeof(old), hash + 1));
  assert(pp_checksum(&migrated) == before);
  puts("PASS: evolution thresholds, busy/final guards, XP cap, 4-slot "
       "rotation/cap, v1 migration");
  puts("PASS: economy, delayed feeding, hatch uniqueness/capacity, travel "
       "timing/claim, daily limits, tarot, battle, scenes, sleep, corrupt "
       "saves");
  const unsigned family_art[3][3]={{1,6,7},{2,4,5},{3,8,9}};
  const char *forms[3][3]={{"妙蛙种子","妙蛙草","妙蛙花"},{"小火龙","火恐龙","喷火龙"},{"杰尼龟","卡咪龟","水箭龟"}};
  for (unsigned family=1;family<=3;family++) {
    init();g.pets[0].species=family;
    assert(pp_art_id(&g,0)==family_art[family-1][0]);
    assert(pp_evolve(&g,0)==PP_NOT_READY);
    g.growth[0].level=16;
    assert(pp_evolve(&g,0)==PP_OK && pp_valid(&g));
    assert(pp_art_id(&g,0)==family_art[family-1][1]);
    assert(strcmp(pp_form_name(&g,0),forms[family-1][1])==0);
    unsigned final_level=family==1?32:36;
    g.growth[0].level=final_level-1;
    assert(pp_evolve(&g,0)==PP_NOT_READY);
    g.growth[0].level=final_level;assert(pp_evolve(&g,0)==PP_OK);
    assert(pp_valid(&g) && pp_art_id(&g,0)==family_art[family-1][2]);
    assert(strcmp(pp_form_name(&g,0),forms[family-1][2])==0);
    pp_game saved=g;assert(pp_checksum(&saved)==pp_checksum(&g) && pp_valid(&saved));
    assert(pp_evolve(&g,0)==PP_INVALID);
    assert(pp_move_at(family,3)!=NULL);
  }
  puts("PASS: all three starter evolution chains, levels, dedicated art/name, persistence layout and family moves");

}
