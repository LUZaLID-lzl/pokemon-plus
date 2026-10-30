#include "home_navigation.h"
#include "home_behavior.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){
 if(argc>1&&!strcmp(argv[1],"--patrol-samples")){
  for(unsigned scene=0;scene<13;scene++)for(unsigned slot=0;slot<5;slot++)for(unsigned seed=0;seed<4;seed++)
   for(unsigned t=0;t<24000;t+=330){int x,y;bool left=false;unsigned pose=pp_home_scene_patrol(scene,t,slot,seed%2,48+seed*71,115+seed*29,false,&x,&y,&left);
    printf("%u %u %u %u %d %d %u %u\n",scene,slot,seed,t,x,y,left,pose);
   }
  return 0;
 }
 bool samples=argc>1&&!strcmp(argv[1],"--samples");
 for(unsigned scene=0;scene<13;scene++)for(unsigned seed=0;seed<128;seed++){
  int x=40+(int)(seed*71%233),y=115+(int)(seed*37%100);
  int tx=40+(int)(seed*131%233),ty=115+(int)(seed*67%100);
  pp_home_ground_project(scene,&x,&y);pp_home_ground_project(scene,&tx,&ty);
  assert(!pp_home_nest_blocked(scene,x,y)&&!pp_home_nest_blocked(scene,tx,ty));
  int points[PP_HOME_ROUTE_POINTS][2];unsigned length,n=pp_home_route(scene,x,y,tx,ty,points,&length);
  assert(n>=2&&n<=PP_HOME_ROUTE_POINTS);
  assert(points[n-1][0]==tx&&points[n-1][1]==ty);
  for(unsigned p=1;p<n;p++)assert(pp_home_segment_clear(scene,points[p-1][0],points[p-1][1],points[p][0],points[p][1]));
  for(unsigned t=0;t<=6000;t+=40){int ox,oy;bool left=false;pp_home_route_sample(scene,x,y,tx,ty,t,6000,&ox,&oy,&left);
   if(pp_home_nest_blocked(scene,ox,oy)){fprintf(stderr,"collision scene%u seed%u t%u at%d,%d\n",scene,seed,t,ox,oy);return 1;}
   assert(pp_home_ground_valid(scene,ox,oy));
   if(samples&&t%240==0)printf("%u %u %u %d %d %u\n",scene,seed,t,ox,oy,left);
  }
 }
 if(samples)return 0;
 for(unsigned scene=0;scene<13;scene++)for(unsigned slot=0;slot<5;slot++)for(unsigned seed=0;seed<24;seed++){
  int x=48+(int)(seed*71%225),y=115+(int)(seed*37%94);pp_home_ground_project(scene,&x,&y);
  int last_x=x,last_y=y;unsigned stalled=0;
  for(unsigned t=0;t<48000;t+=33){int ox,oy;bool left=false;unsigned pose=pp_home_scene_patrol(scene,t,slot,seed%2,x,y,false,&ox,&oy,&left);
   assert(!pp_home_nest_blocked(scene,ox,oy));
   if(t&&scene!=12)assert(abs(ox-last_x)<=6&&abs(oy-last_y)<=6);
   stalled=pose==4&&ox==last_x&&oy==last_y?stalled+33:0;
   if(stalled>500){fprintf(stderr,"stalled scene%u slot%u seed%u at%d,%d t%u\n",scene,slot,seed,ox,oy,t);return 1;}
   last_x=ox;last_y=oy;
  }
 }
 puts("PASS: 13 scenes, 1664 detours, 251264 route samples, patrol collision/continuity/liveness");
}
