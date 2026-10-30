#include "scene_fx.h"
#include <stdio.h>
int main(void) {
 const unsigned times[]={0,40,500,1200,2499,5000,10000,17999,18000,24000,42000,44999,45000,46199,59999,61000,78000,179999,4294967000u};
 for(unsigned s=0;s<13;s++)for(unsigned seed=1;seed<=8;seed++)for(unsigned mode=0;mode<2;mode++)for(unsigned t=0;t<sizeof(times)/sizeof(times[0]);t++) {
  pp_scene_particle p[PP_SCENE_FX_MAX];unsigned n=pp_scene_fx_sample(s,seed,times[t],mode,p);
  printf("{\"scene\":%u,\"seed\":%u,\"time\":%u,\"preview\":%u,\"particles\":[",s,seed,times[t],mode);
  for(unsigned i=0;i<n;i++) {
   pp_scene_rect rects[PP_SCENE_FX_PARTS];unsigned parts=pp_scene_fx_parts(&p[i],rects);
   printf("%s{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"color\":\"%06x\",\"opacity\":%u,\"shape\":%u,\"full_screen\":%s,\"parts\":[",i?",":"",p[i].x,p[i].y,p[i].w,p[i].h,p[i].color,p[i].opacity,p[i].shape,p[i].full_screen?"true":"false");
   for(unsigned j=0;j<parts;j++)printf("%s{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"opacity\":%u}",j?",":"",rects[j].x,rects[j].y,rects[j].w,rects[j].h,rects[j].opacity);
   printf("]}");
  }
  puts("]}");
 }
}
