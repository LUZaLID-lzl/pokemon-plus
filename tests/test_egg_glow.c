#include "egg_glow.h"
#include "scene_nests.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {
  pp_egg_glow_dot dots[PP_EGG_GLOW_DOTS],again[PP_EGG_GLOW_DOTS];
  for(unsigned rare=0;rare<2;rare++)for(unsigned t=0;t<2400;t+=37) {
    unsigned count=pp_egg_glow_sample(t,rare,dots);
    assert(count>=10 && count<=PP_EGG_GLOW_DOTS);
    assert(count==pp_egg_glow_sample(t+2400,rare,again));
    for(unsigned i=0;i<count;i++) {
      const pp_egg_glow_dot *d=&dots[i],*a=&again[i];
      assert(d->x==a->x && d->y==a->y && d->size==a->size && d->opacity==a->opacity && d->color==a->color);
      assert(d->size==2 || d->size==3);
      assert(d->opacity>0 && d->opacity<=210);
      assert(d->y+d->size<=12); /* Never encircle the lower shell or nest. */
      assert(d->x+PP_EGG_GLOW_OFFSET_X>=0 && d->x+PP_EGG_GLOW_OFFSET_X+d->size<=PP_EGG_GLOW_WIDTH);
      assert(d->y+PP_EGG_GLOW_OFFSET_Y>=0 && d->y+PP_EGG_GLOW_OFFSET_Y+d->size<=PP_EGG_GLOW_HEIGHT);
      for(unsigned scene=0;scene<PP_SCENE_NEST_COUNT;scene++)for(int wobble=-3;wobble<=3;wobble+=6) {
        int ex,ey;pp_scene_egg_position(scene,&ex,&ey);
        for(int x=0;x<d->size;x++)for(int y=0;y<d->size;y++) {
          int px=ex+wobble+d->x+x,py=ey+d->y+y;
          assert((px-180)*(px-180)+(py-180)*(py-180)<168*168);
        }
      }
    }
  }
  if(argc>1 && !strcmp(argv[1],"--samples")) {
    const uint32_t times[]={0,119,120,239,240,600,1439,1799,1800,2399,2400,UINT32_MAX};
    for(unsigned rare=0;rare<2;rare++)for(unsigned t=0;t<sizeof(times)/sizeof(times[0]);t++) {
      unsigned count=pp_egg_glow_sample(times[t],rare,dots);
      printf("%u %u",rare,times[t]);
      for(unsigned i=0;i<count;i++)printf(" %d,%d,%d,%u,%u",dots[i].x,dots[i].y,dots[i].size,dots[i].opacity,dots[i].color);
      puts("");
    }
  } else puts("PASS: egg dots stay above upper shell, bounded opacity/canvas, seamless2400ms loop and13-scene circular safety including wobble");
}
