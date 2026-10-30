#include "home_behavior.h"
#include "home_navigation.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static pp_home_actor actors[5]={{80,145,80,80,true,false},{170,165,80,80,true,true},{225,195,80,80,true,true},{110,200,80,80,true,false},{205,120,80,80,true,false}};
int main(void){
 pp_home_behavior_state s;pp_home_output out[5];
 for(unsigned scene=0;scene<13;scene++)for(unsigned kind=1;kind<=8;kind++){
  pp_home_actor in[5];memcpy(in,actors,sizeof(in));pp_home_behavior_init(&s,42,0,scene);
  for(unsigned p=0;p<5;p++)pp_home_ground_project(scene,&in[p].x,&in[p].y);
  assert(pp_home_behavior_request(&s,0,kind,0,1,225,198,in,5));
  bool moved=false,happy=false,symbol=false;unsigned duration=s.pets[0].duration;
  for(unsigned t=0;t<duration+66;t+=33){pp_home_behavior_tick(&s,t,in,5,false,out);
   for(unsigned p=0;p<5;p++)if(out[p].active||out[p].release){
    assert(out[p].x>=40&&out[p].x<=272&&out[p].y>=115&&out[p].y<=214&&out[p].jump>=0&&out[p].jump<=4);
    assert(!pp_home_nest_blocked(scene,out[p].x,out[p].y));
    assert(abs(out[p].x-in[p].x)<=5&&abs(out[p].y-in[p].y)<=5);
    moved|=out[p].x!=in[p].x||out[p].y!=in[p].y;happy|=out[p].pose==8;symbol|=out[p].symbol>0;
    in[p].x=out[p].x;in[p].y=out[p].y;in[p].left=out[p].left;
   }
  }
  assert(!s.pets[0].kind);if(kind!=1&&kind!=4)assert(moved);
  if(kind==1||kind==2||kind==4||kind==5||kind==8)assert(happy&&symbol);
 }
 /* Quantized slow movement must keep its route heading, including zero-pixel frames. */
 pp_home_actor slow[5];memcpy(slow,actors,sizeof(slow));slow[0].x=100;slow[0].y=150;slow[0].left=true;
 pp_home_behavior_init(&s,42,0,0);assert(pp_home_behavior_request(&s,0,PP_HOME_CALL,0,0,225,150,slow,5));
 for(unsigned t=33;t<4000;t+=33){
  pp_home_behavior_tick(&s,t,slow,5,false,out);
  assert(out[0].active&&out[0].pose==4);
  if(out[0].left){fprintf(stderr,"Wrong/flickering heading at %ums, x %d -> %d\n",t,slow[0].x,out[0].x);abort();}
  slow[0].x=out[0].x;slow[0].y=out[0].y;slow[0].left=out[0].left;
 }
 pp_home_behavior_init(&s,42,0,0);assert(pp_home_behavior_request(&s,0,5,0,1,0,0,actors,5));
 assert(!pp_home_behavior_request(&s,0,6,2,3,0,0,actors,5));
 pp_home_actor copy[5];memcpy(copy,actors,sizeof(copy));copy[0].available=false;
 pp_home_behavior_tick(&s,300,copy,5,false,out);assert(!s.pets[0].kind&&!s.pets[1].kind&&out[1].release);
 assert(!pp_home_behavior_request(&s,301,1,0,0,0,0,copy,5));
 for(unsigned i=0;i<5;i++)copy[i].available=false;
 assert(pp_home_behavior_call(&s,1000,260,180,copy,5)<0);
 memcpy(copy,actors,sizeof(copy));assert(pp_home_behavior_call(&s,1000,290,145,copy,5)>=0);
 assert(pp_home_behavior_call(&s,1200,290,145,copy,5)<0);
 assert(pp_home_behavior_call(&s,3000,180,300,copy,5)<0);
 /* Ten simulated minutes, independent scheduling, bounded positions and one pair. */
 for(unsigned scene=0;scene<13;scene++)for(unsigned seed=0;seed<8;seed++){
  memcpy(copy,actors,sizeof(copy));pp_home_behavior_init(&s,seed,0,scene);
  for(unsigned p=0;p<5;p++)pp_home_ground_project(scene,&copy[p].x,&copy[p].y);
  for(unsigned t=0;t<600000;t+=100){pp_home_behavior_tick(&s,t,copy,5,true,out);unsigned pair=0;
   for(unsigned p=0;p<5;p++){pair+=s.pets[p].kind>=5&&s.pets[p].kind<=7;
    if(out[p].active||out[p].release){if(abs(out[p].x-copy[p].x)>12||abs(out[p].y-copy[p].y)>12){fprintf(stderr,"jump scene%u seed%u t%u p%u kind%u %d,%d -> %d,%d\n",scene,seed,t,p,out[p].kind,copy[p].x,copy[p].y,out[p].x,out[p].y);fprintf(stderr,"from %d,%d to %d,%d start%u approach%u duration%u\n",s.pets[p].x,s.pets[p].y,s.pets[p].tx,s.pets[p].ty,s.pets[p].start,s.pets[p].approach,s.pets[p].duration);abort();}copy[p].x=out[p].x;copy[p].y=out[p].y;copy[p].left=out[p].left;}}
   assert(pair==0||pair==2);
  }
 }
 /* Every lawn corner must move while WALK is selected after a rebase. */
 for(unsigned slot=0;slot<5;slot++)for(int x0=48;x0<=272;x0+=112)for(int y0=115;y0<=208;y0+=(y0==115?45:48))for(unsigned tired=0;tired<2;tired++){
  int last_x=x0,last_y=y0;bool left=false;unsigned stalled=0;
  for(unsigned t=0;t<48000;t+=33){int x,y;unsigned pose=pp_home_patrol(t,slot,tired,x0,y0,left,&x,&y,&left);
   assert(x>=48&&x<=272&&y>=115&&y<=208);
   assert(abs(x-last_x)<=2&&abs(y-last_y)<=2);
   stalled=pose==4&&x==last_x&&y==last_y?stalled+33:0;
   assert(stalled<=200);last_x=x;last_y=y;
  }
 }
 /* Petting/hops must not project a stationary pet onto another lake coordinate. */
 memcpy(slow,actors,sizeof(slow));slow[0].x=123;slow[0].y=181;
 for(unsigned kind=1;kind<=4;kind+=3){pp_home_behavior_init(&s,42,0,12);assert(pp_home_behavior_request(&s,0,kind,0,0,0,0,slow,5));pp_home_behavior_tick(&s,0,slow,5,false,out);assert(out[0].x==123&&out[0].y==181);}
 puts("PASS: eight behaviors x13 scenes, smooth paths, safe bounds, unavailable/paired cancellation, call hit gates, 104 ten-minute schedules, stable subpixel heading, bounded patrol liveness and one-pair limit");
 return 0;
}
