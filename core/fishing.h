#pragma once
#include "game.h"
#include <limits.h>
#include <string.h>

typedef enum { PP_FISH_READY, PP_FISH_WAIT, PP_FISH_REEL, PP_FISH_RESULT } pp_fish_phase;
typedef struct {
  pp_fish_phase phase;
  uint32_t rng, started, wait_ms;
  unsigned target, reward, fish, needle, quality;
  bool caught;
} pp_fishing;
#define PP_FISH_REEL_MS 6000u
static inline unsigned pp_fishing_weight(unsigned id,unsigned quality){
  static const unsigned bonus[3][4]={{100,100,100,100},{100,120,150,175},{100,150,250,400}};
  return id<PP_FISH_COUNT?pp_fish_catalog[id].weight*bonus[quality>2?2:quality][pp_fish_catalog[id].rarity]:0;
}
static inline unsigned pp_fishing_weight_total(unsigned quality){unsigned sum=0;for(unsigned i=0;i<PP_FISH_COUNT;i++)sum+=pp_fishing_weight(i,quality);return sum;}
static inline unsigned pp_fishing_pick(unsigned roll,unsigned quality){
  roll%=pp_fishing_weight_total(quality);
  for(unsigned i=0;i<PP_FISH_COUNT;i++){unsigned weight=pp_fishing_weight(i,quality);if(roll<weight)return i;roll-=weight;}
  return PP_FISH_COUNT-1;
}
static inline void pp_fishing_init(pp_fishing *f,uint32_t seed){memset(f,0,sizeof(*f));f->rng=seed?seed:1;}
static inline uint32_t pp_fishing_random(pp_fishing *f){uint32_t x=f->rng;x^=x<<13;x^=x>>17;x^=x<<5;return f->rng=x?x:1;}
static inline unsigned pp_fishing_needle(uint32_t elapsed){unsigned p=elapsed%2000u;return p<=1000?p/10:(2000-p)/10;}
static inline void pp_fishing_tick(pp_fishing *f,uint32_t now){
  uint32_t age=now-f->started;
  if(f->phase==PP_FISH_WAIT && age>=f->wait_ms){f->phase=PP_FISH_REEL;f->started+=f->wait_ms;age=now-f->started;}
  if(f->phase==PP_FISH_REEL){f->needle=pp_fishing_needle(age);if(age>=PP_FISH_REEL_MS){f->phase=PP_FISH_RESULT;f->reward=0;f->caught=false;}}
}
static inline void pp_fishing_press(pp_fishing *f,pp_game *g,uint32_t now){
  pp_fishing_tick(f,now);
  if(f->phase==PP_FISH_READY || f->phase==PP_FISH_RESULT){
    f->phase=PP_FISH_WAIT;f->started=now;f->reward=0;f->fish=0;f->needle=0;f->caught=false;f->quality=0;
    f->wait_ms=6000u+pp_fishing_random(f)%4001u;f->target=30u+pp_fishing_random(f)%41u;return;
  }
  if(f->phase!=PP_FISH_REEL)return;
  unsigned distance=f->needle>f->target?f->needle-f->target:f->target-f->needle;
  f->phase=PP_FISH_RESULT;f->caught=distance<=18;
  if(!f->caught)return;
  f->quality=distance<=4?2:distance<=10?1:0;
  unsigned total=pp_fishing_weight_total(f->quality),roll;
  /* Rejection sampling avoids modulo bias, same algorithm in the Web preview. */
  uint32_t limit=UINT32_MAX-UINT32_MAX%total;
  do{roll=pp_fishing_random(f);}while(roll>=limit);
  f->fish=pp_fishing_pick(roll,f->quality);
  unsigned base=pp_fish_catalog[f->fish].coins/6;if(base<2)base=2;unsigned award=base+f->quality;if(g->fish_today>=15){award/=5;if(!award)award=1;}if(g->fish_today<UINT16_MAX)g->fish_today++;
  f->reward=UINT32_MAX-g->coins<award?UINT32_MAX-g->coins:award;
  g->coins+=f->reward;if(g->fish_caught[f->fish]<UINT32_MAX)g->fish_caught[f->fish]++;
  g->revision++;
}
