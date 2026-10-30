#include "home_behavior.h"
#include "home_navigation.h"
#include <stdio.h>
int main(void){
 for(unsigned scene=0;scene<13;scene++)for(unsigned kind=0;kind<=8;kind++){
 pp_home_behavior_state s;pp_home_output out[5];
 pp_home_actor in[5]={{80,145,80,80,true,false},{170,165,80,80,true,true},{225,195,80,80,true,true},{110,200,80,80,true,false},{205,120,80,80,true,false}};
 for(unsigned p=0;p<5;p++)pp_home_ground_project(scene,&in[p].x,&in[p].y);
 pp_home_behavior_init(&s,42,0,scene);if(kind)pp_home_behavior_request(&s,0,kind,0,1,225,198,in,5);
 unsigned limit=kind?22000:240000;
 for(unsigned t=0;t<limit;t+=100){
  if(t==12000){in[0].available=false;in[1].available=false;}if(t==15000)in[0].available=in[1].available=true;
  pp_home_behavior_tick(&s,t,in,5,!kind,out);
  for(unsigned p=0;p<5;p++){pp_home_output *o=&out[p];printf("%u %u %u %u %u %u %d %d %d %u %u %u %u\n",scene,kind,t,p,o->active,o->release,o->x,o->y,o->jump,o->left,o->pose,o->symbol,o->kind);
   if(o->active||o->release){in[p].x=o->x;in[p].y=o->y;in[p].left=o->left;}}
 }
 }
 for(unsigned slot=0;slot<5;slot++)for(int x0=48;x0<=272;x0+=112)for(int y0=115;y0<=208;y0+=(y0==115?45:48))for(unsigned tired=0;tired<2;tired++){
  bool left=false;
  for(unsigned t=0;t<48000;t+=33){int x,y;unsigned pose=pp_home_patrol(t,slot,tired,x0,y0,left,&x,&y,&left);
   printf("P %u %d %d %u %u %d %d %u %u\n",slot,x0,y0,tired,t,x,y,left,pose);
  }
 }

}
