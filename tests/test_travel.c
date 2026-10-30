#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void) {
  pp_game g;
  for(unsigned scene=0;scene<PP_TRAVEL_DESTINATIONS;scene++) {
    pp_init(&g,42);pp_tick(&g,0,20261023);g.count=2;g.pets[1]=g.pets[0];
    g.pets[1].species=1;g.scene=scene;g.scenes|=1u<<scene;
    /* Exercise every independent destination, regardless of courtyard. */
    g.travel_end=PP_TRAVEL_SECONDS;
    while(pp_travel_destination(&g)!=scene) {g.seconds++;g.travel_end++;}
    g.travel_end=0;
    assert(!pp_travel_describe(&g).active);
    assert(pp_travel(&g,0)==PP_OK);
    unsigned destination=pp_travel_destination(&g);assert(destination==scene);
    pp_game restored=g;g.scene=(scene+1)%PP_SCENES;
    assert(pp_travel_destination(&g)==destination && pp_travel_destination(&restored)==destination);
    uint32_t before=pp_checksum(&g);
    pp_travel_status trip=pp_travel_describe(&g);
    assert(trip.active && !trip.ready && trip.walking && trip.remaining==PP_TRAVEL_SECONDS);
    assert(!trip.event && trip.pending_coins==50 && pp_checksum(&g)==before);
    const char *first=trip.activity;
    pp_tick(&g,300,g.day);trip=pp_travel_describe(&g);
    assert(!trip.walking && strcmp(first,trip.activity) && !trip.event);
    uint32_t coins=g.coins;
    pp_tick(&g,1500,g.day);trip=pp_travel_describe(&g);
    assert(trip.events==1 && g.travel_reward==5 && trip.pending_coins==55);
    assert(trip.event && strstr(g.log[3],trip.event) && g.coins==coins);
    const char *event=trip.event;
    pp_tick(&g,1800,g.day);trip=pp_travel_describe(&g);
    assert(trip.events==2 && g.travel_reward==10 && strcmp(event,trip.event));
    before=pp_checksum(&g);for(unsigned i=0;i<100;i++) (void)pp_travel_describe(&g);
    assert(pp_checksum(&g)==before);pp_tick(&g,0,g.day);assert(g.travel_reward==10);
    pp_tick(&g,7200,g.day);trip=pp_travel_describe(&g);
    assert(trip.ready && !trip.walking && !trip.remaining && trip.events==6 && trip.pending_coins==80);
    pp_tick(&g,3600,g.day);assert(g.travel_reward==30);
    pp_travel_reward reward;
    assert(pp_claim_travel(&g,&reward)==PP_OK && g.coins==coins+80 && pp_collected(&g)==1);
    assert(reward.pet==0 && reward.destination==destination && reward.item<PP_TRAVEL_COLLECTION);
    assert(reward.coins==80 && reward.xp==4 && reward.before.level==1 && reward.after.level==1);
    assert(pp_has(&g,reward.item));
    assert(g.growth[0].level==1 && g.growth[0].xp==4);
    coins=g.coins;assert(pp_claim(&g)==PP_INVALID && g.coins==coins);
    assert(!pp_travel_describe(&g).active);
  }
  pp_init(&g,42);pp_tick(&g,0,20261023);g.count=2;g.pets[1]=g.pets[0];
  for(unsigned id=0;id<PP_TRAVEL_COLLECTION;id++)g.collected[id/8]|=1u<<(id%8);
  g.tarot_card=3;g.tarot_day=g.tarot_effect_day=g.day;
  assert(pp_travel(&g,0)==PP_OK);pp_tick(&g,PP_TRAVEL_SECONDS,g.day);
  unsigned coins=g.coins,rng=g.rng;
  assert(pp_claim(&g)==PP_OK && g.coins==coins+96);
  assert(g.growth[0].level==1 && g.growth[0].xp==4 && g.rng==rng);
  assert(pp_collected(&g)==583 && !pp_has(&g,PP_PURE_HEART) && strstr(g.log[3],"旅行藏品已集齐"));
  puts("PASS: completed travel catalog preserves World tarot coin bonus, 120XP and separate special gift");
  /* Old pending amounts, reload, cap truncation and failure leave no extra payout. */
  for(unsigned level=59;level<=60;level++) {
    pp_init(&g,42);g.count=2;g.pets[1]=g.pets[0];g.growth[0]=(pp_growth){.level=level,.xp=level==59?1170:0};
    assert(pp_travel(&g,0)==PP_OK);pp_travel_reward reward={.coins=777};
    assert(pp_claim_travel(&g,&reward)==PP_NOT_READY && reward.coins==777);
    pp_tick(&g,PP_TRAVEL_SECONDS-1,g.day);
    assert(pp_travel_describe(&g).remaining==1 && !pp_travel_describe(&g).ready);
    uint32_t before=pp_checksum(&g);
    assert(pp_claim_travel(&g,&reward)==PP_NOT_READY && reward.coins==777 && pp_checksum(&g)==before);
    unsigned bytes=pp_save_size(&g);void *blob=malloc(bytes);assert(blob);
    int64_t wall=0;assert(pp_save_write(&g,blob,bytes,123456));
    assert(pp_save_read(&g,blob,bytes,&wall) && wall==123456);
    assert(pp_travel_describe(&g).remaining==1 && !pp_travel_describe(&g).ready);
    pp_tick(&g,1,g.day);g.travel_reward=30;
    assert(pp_travel_describe(&g).ready && !pp_travel_describe(&g).remaining);
    unsigned coins=g.coins;
    assert(pp_claim_travel(&g,&reward)==PP_OK && reward.coins==80 && g.coins==coins+80);
    assert(reward.xp==(level==59?10u:0u) && reward.after.level==60 && !reward.after.xp);
    assert(pp_save_write(&g,blob,bytes,123457));
    assert(pp_save_read(&g,blob,bytes,&wall) && wall==123457);free(blob);
    uint32_t checksum=pp_checksum(&g);
    assert(pp_claim_travel(&g,&reward)==PP_INVALID && pp_checksum(&g)==checksum);
  }
  unsigned hits[PP_TRAVEL_DESTINATIONS]={0};
  for(unsigned trip=0;trip<10000;trip++) {
    pp_init(&g,42);g.travel_end=(uint64_t)(trip+1)*PP_TRAVEL_SECONDS;
    uint32_t before=pp_checksum(&g);unsigned destination=pp_travel_destination(&g);
    assert(destination<PP_TRAVEL_DESTINATIONS && pp_checksum(&g)==before);hits[destination]++;
  }
  for(unsigned i=0;i<PP_TRAVEL_DESTINATIONS;i++)assert(hits[i]>850 && hits[i]<1150);
  puts("PASS: ten travel destinations, stable across reload/courtyard changes, varied trips, read-only changing activities, truthful events/rewards, atomic single claim");
}
