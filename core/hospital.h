#pragma once
#include "game.h"
#include <string.h>

enum { PP_HOSPITAL_READY, PP_HOSPITAL_PLAY, PP_HOSPITAL_WON, PP_HOSPITAL_MISSED };
#define PP_HOSPITAL_GOAL 6u
#define PP_HOSPITAL_FALL_MS 800u
#define PP_HOSPITAL_SLOT_MS 1000u
#define PP_HOSPITAL_ROUNDS 12u
typedef struct {
  uint32_t rng, elapsed;
  unsigned phase, score, slot, lane, basket, treated, mask;
  bool resolved, claimed;
  uint8_t restored[PP_PETS][5];
} pp_hospital;
static inline uint32_t pp_hospital_random(pp_hospital *h) {
  uint32_t x=h->rng;x^=x<<13;x^=x>>17;x^=x<<5;return h->rng=x?x:1;
}
static inline bool pp_hospital_cloud(const pp_hospital *h){return h->slot%4==3;}
static inline void pp_hospital_init(pp_hospital *h,uint32_t seed) {
  memset(h,0,sizeof(*h));h->rng=seed?seed:1;h->basket=1;
}
static inline void pp_hospital_start(pp_hospital *h) {
  uint32_t seed=pp_hospital_random(h);pp_hospital_init(h,seed);
  h->phase=PP_HOSPITAL_PLAY;h->lane=pp_hospital_random(h)%3;
}
static inline void pp_hospital_select(pp_hospital *h,unsigned lane) {
  if(h->phase==PP_HOSPITAL_PLAY && lane<3)h->basket=lane;
}
static inline void pp_hospital_advance(pp_hospital *h,uint32_t delta) {
  if(h->phase!=PP_HOSPITAL_PLAY)return;
  unsigned remaining=PP_HOSPITAL_ROUNDS*PP_HOSPITAL_SLOT_MS-h->elapsed;
  h->elapsed+=delta>remaining?remaining:delta;
  while(h->phase==PP_HOSPITAL_PLAY) {
    unsigned age=h->elapsed-h->slot*PP_HOSPITAL_SLOT_MS;
    if(!h->resolved && age>=PP_HOSPITAL_FALL_MS) {
      h->resolved=true;
      if(h->lane==h->basket) {
        if(pp_hospital_cloud(h)){if(h->score)h->score--;}
        else if(++h->score>=PP_HOSPITAL_GOAL){h->phase=PP_HOSPITAL_WON;break;}
      }
    }
    if(age<PP_HOSPITAL_SLOT_MS)break;
    if(++h->slot>=PP_HOSPITAL_ROUNDS){h->phase=PP_HOSPITAL_MISSED;break;}
    h->resolved=false;h->lane=pp_hospital_random(h)%3;
  }
}
static inline unsigned pp_hospital_patients(const pp_game *g) {
  unsigned n=0;for(unsigned i=0;i<g->count;i++)if(g->traveler!=(int)i&&!pp_training_busy(g,i))n++;return n;
}
static inline pp_result pp_hospital_available(const pp_game *g) {
  if(g->battling || g->drop_active)return PP_BUSY;
  return pp_hospital_patients(g)?PP_OK:PP_NOT_READY;
}
static inline pp_result pp_hospital_claim(pp_hospital *h,pp_game *g) {
  if(h->phase!=PP_HOSPITAL_WON || h->claimed)return PP_NOT_READY;
  pp_result result=pp_hospital_available(g);if(result!=PP_OK)return result;
  for(unsigned i=0;i<g->count;i++)if(g->traveler!=(int)i&&!pp_training_busy(g,i)){
    pp_pet *p=&g->pets[i];if(p->health<60)p->health=60;
    uint8_t values[5]={p->food,p->clean,p->health,p->mood,p->energy};memcpy(h->restored[i],values,5);h->mask|=1u<<i;h->treated++;
  }
  h->claimed=true;g->revision++;return PP_OK;
}
