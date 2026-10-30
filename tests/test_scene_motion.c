#include "scene_motion.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint16_t base[PP_SCENE_PIXELS],out[PP_SCENE_PIXELS],copy[PP_SCENE_PIXELS];
int main(int argc,char **argv) {
  for(unsigned i=0;i<PP_SCENE_PIXELS;i++)base[i]=(uint16_t)(i*97u+0x20a3);
  if(argc>1) {
    unsigned scene,time;assert(sscanf(argv[1],"%u:%u",&scene,&time)==2);
    pp_scene_motion_render(scene,time,base,out);
    assert(fwrite(out,sizeof(out),1,stdout)==1);return 0;
  }
  memcpy(copy,base,sizeof(base));
  for(unsigned scene=0;scene<13;scene++) {
    unsigned changes=0;
    const uint32_t times[]={0,33,66,99,120,960,2400,4800,9600,59999,UINT32_MAX};
    for(unsigned t=0;t<sizeof(times)/sizeof(times[0]);t++) {
      pp_scene_motion_render(scene,times[t],base,out);
      for(int y=0;y<120;y++)for(int x=0;x<120;x++) {
        if(out[y*120+x]!=base[y*120+x]) {assert(pp_scene_motion_safe(x,y));changes++;}
      }
    }
    assert(changes);assert(!memcmp(base,copy,sizeof(base)));
  }
  pp_scene_motion_render(99,1000,base,out);assert(!memcmp(base,out,sizeof(base)));
  puts("Scenery:13 scenes visible, stable HUD/navigation/rim, immutable source and rollover: PASS");
}
