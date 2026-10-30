#ifndef PP_PET_ENERGY_H
#define PP_PET_ENERGY_H
#include <stdbool.h>
#include <stdint.h>
#define PP_CARE_SECONDS 600u
#define PP_SLEEP_BELOW 20u
#define PP_WAKE_ENERGY 60u
#define PP_SLEEP_RECOVERY 25u
static inline bool pp_energy_wake(uint8_t energy,uint8_t *sleeping){
 if(*sleeping&&energy>=PP_WAKE_ENERGY){*sleeping=0;return true;}return false;
}
/* Integer seconds and a saved 1/3600-point remainder make catch-up independent
 * of call size, including the instant a sleeping individual wakes. */
static inline void pp_energy_restore(uint8_t *energy,uint8_t *sleeping,uint16_t *fraction,uint32_t seconds,bool moon){
 if(*energy<PP_SLEEP_BELOW)*sleeping=1;
 pp_energy_wake(*energy,sleeping);
 while(seconds){
  unsigned rate=*sleeping?25:10;if(moon)rate=rate*120/100;
  unsigned take=seconds;
  if(*sleeping){unsigned need=((PP_WAKE_ENERGY-*energy)*3600u-*fraction+rate-1)/rate;if(take>need)take=need;}
  unsigned total=*fraction+take*rate,value=*energy+total/3600;
  *energy=value>100?100:value;*fraction=total%3600;seconds-=take;
  pp_energy_wake(*energy,sleeping);
 }
}
/* Compatibility for callers using complete ten-minute intervals. */
static inline void pp_energy_advance(uint8_t *energy,uint8_t *sleeping,uint32_t ticks,uint64_t first_tick,bool sun,bool moon){
 (void)first_tick;(void)sun;uint16_t fraction=0;pp_energy_restore(energy,sleeping,&fraction,ticks*600u,moon);
}
#endif
