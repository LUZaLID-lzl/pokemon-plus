#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Draw-only samples relative to the32px egg; fixed storage, no game RNG. */
#define PP_EGG_GLOW_DOTS 18
#define PP_EGG_GLOW_WIDTH 84
#define PP_EGG_GLOW_HEIGHT 64
#define PP_EGG_GLOW_OFFSET_X 26
#define PP_EGG_GLOW_OFFSET_Y 44
typedef struct {int x,y,size;unsigned opacity;uint32_t color;} pp_egg_glow_dot;
static inline unsigned pp_egg_glow_sample(uint32_t elapsed,bool legendary,
                                          pp_egg_glow_dot dots[PP_EGG_GLOW_DOTS]) {
  static const uint32_t warm[]={0xffedbd,0xffdc83,0xfff5d9};
  static const uint32_t rare[]={0xe5d2ff,0xc3eaff,0xf5eaff};
  unsigned count=0;
  for(unsigned i=0;i<PP_EGG_GLOW_DOTS;i++) {
    unsigned age=(elapsed%2400u+(i*137u)%2400u)%2400u;
    if(age>=1800u)continue;
    unsigned opacity=age<240u?age*210u/240u:(1800u-age)*210u/1560u;
    if(!opacity)continue;
    int step=(int)(age/120u),lane=(int)(i*7u%9u)-4;
    /* Separate grid points fan out only above the upper shell. */
    int x=15+lane*3+(lane*step/10)*3;
    dots[count++]=(pp_egg_glow_dot){x,9-(step+(int)(i%3))*3,
      step<8?3:2,opacity,(legendary?rare:warm)[i%3]};
  }
  return count;
}
