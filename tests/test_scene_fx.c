#include "scene_fx.h"
#include <assert.h>
#include <stdio.h>
#include "scene_fx_config.h"
int main(void) {
 pp_scene_particle p[PP_SCENE_FX_MAX],q[PP_SCENE_FX_MAX];
 for(unsigned scene=0;scene<13;scene++) {
  unsigned active=0,quiet=0,moving=0,previous=0;int min_x=360,max_x=0,min_y=360,max_y=0;
  for(unsigned seed=1;seed<=16;seed++) {
   assert(!pp_scene_fx_sample(scene,seed,0,false,p));
   for(unsigned t=0;t<180000;t+=733) {
    unsigned n=pp_scene_fx_sample(scene,seed,t,false,p);
    unsigned m=pp_scene_fx_sample(scene,seed,t,false,q);
    assert(n==m&&n<=PP_SCENE_FX_MAX&&n<=pp_scene_fx_capacity(scene));
    if(n)active++;else quiet++;
    for(unsigned i=0;i<n;i++) {
     assert(p[i].x==q[i].x&&p[i].y==q[i].y&&p[i].opacity==q[i].opacity);
     assert(pp_scene_fx_safe(&p[i]) && p[i].opacity<=255);
     if(!p[i].full_screen)assert(!(p[i].x<280 && p[i].x+p[i].w>80 && p[i].y<112 && p[i].y+p[i].h>22));
     if(!p[i].full_screen)assert(!(p[i].x<262 && p[i].x+p[i].w>98 && p[i].y<328 && p[i].y+p[i].h>258));
     pp_scene_rect rects[PP_SCENE_FX_PARTS];unsigned parts=pp_scene_fx_parts(&p[i],rects);
     assert((p[i].shape>=PP_FX_TUMBLE_0 ? parts==0 : parts>=1) && parts<=PP_SCENE_FX_PARTS && parts<=pp_scene_fx_part_limit(scene));
     if(scene==0)assert(p[i].shape==PP_FX_LEAF);
     if(scene==8)assert(p[i].shape==PP_FX_STAR || p[i].shape==PP_FX_COMET);
     if(scene==12)assert(p[i].shape==PP_FX_LEAF || p[i].shape==PP_FX_LAKE_RING);
     if(scene==4)assert(p[i].shape==PP_FX_MAPLE);
     if(scene==6)assert(p[i].shape==PP_FX_STAR || (p[i].shape>=PP_FX_BAT_UP && p[i].shape<=PP_FX_BAT_DOWN));
     for(unsigned j=0;j<parts;j++)assert(rects[j].w>0 && rects[j].h>0 && rects[j].x>=p[i].x && rects[j].y>=p[i].y && rects[j].x+rects[j].w<=p[i].x+p[i].w && rects[j].y+rects[j].h<=p[i].y+p[i].h && rects[j].opacity<=p[i].opacity);
     for(unsigned c=0;c<4;c++) {
      int dx=p[i].x+(c&1?p[i].w:0)-180,dy=p[i].y+(c&2?p[i].h:0)-180;
      int radius=p[i].full_screen?176:172;assert(dx*dx+dy*dy<=radius*radius);
     }
     if(p[i].x<min_x)min_x=p[i].x;if(p[i].x+p[i].w>max_x)max_x=p[i].x+p[i].w;
     if(p[i].y<min_y)min_y=p[i].y;if(p[i].y+p[i].h>max_y)max_y=p[i].y+p[i].h;
     unsigned pos=p[i].x*360u+p[i].y;if(previous&&pos!=previous)moving++;previous=pos;
    }
   }
  }
  assert(active&&quiet&&moving);
  assert(pp_scene_fx_configs[scene].chance==30);
  unsigned triggered=0;
  for(unsigned seed=1;seed<=10000;seed++) {
   if(pp_scene_fx_sample(scene,seed,10000,false,p))triggered++;
   /* No residual particle layer after the longest possible event window. */
   assert(!pp_scene_fx_sample(scene,seed,44999,false,p));
  }
  assert(triggered>2700 && triggered<3300);

  if(scene==0 || scene==1 || scene==3 || scene==4 || scene==5 || scene==7 || scene==10) {
   assert(min_x<50 && max_x>310 && max_y-min_y>130);
  }
  if(scene==8)assert(max_y<=138); /* Stars and meteors remain above the treeline. */
  /* Every overlay, including glints and lake rings, is absent before an event. */
  unsigned quiet_visible=0,quiet_weather=0,burst_weather=0,area=0;
  for(unsigned seed=1;seed<=16;seed++) {
   unsigned n=pp_scene_fx_sample(scene,seed,1200,false,p);
   quiet_visible+=n;
   for(unsigned i=0;i<n;i++)if((scene==5&&p[i].color!=pp_scene_fx_configs[scene].color)||(scene==10&&p[i].shape==PP_FX_RAIN))quiet_weather++;
   n=pp_scene_fx_sample(scene,seed,5000,true,p);
   for(unsigned i=0;i<n;i++) {
    pp_scene_rect rects[PP_SCENE_FX_PARTS];unsigned parts=pp_scene_fx_parts(&p[i],rects);
    for(unsigned j=0;j<parts;j++)area+=rects[j].w*rects[j].h*rects[j].opacity;
    if((scene==5&&p[i].color!=pp_scene_fx_configs[scene].color)||(scene==10&&p[i].shape==PP_FX_RAIN))burst_weather++;
   }
  }
  assert(area>16*80*255);
  assert(!quiet_visible);
  if(scene==8)
  for(unsigned seed=1;seed<=16;seed++) {
   /* A shooting star is intermittent, unlike continuous drifting leaves. */
   unsigned visible=0;
   for(unsigned t=1000;t<31000;t+=100){unsigned n=pp_scene_fx_sample(scene,seed,t,true,p);for(unsigned i=0;i<n;i++)visible+=p[i].shape==PP_FX_COMET;}
   assert(visible>80);
  }
  if(scene==5 || scene==10)assert(!quiet_weather && burst_weather);
  /* Airborne ambience must reach every sector, including the former HUD
   * and dock holes, without increasing the fixed particle pool. */
  if(scene==0 || scene==1 || scene==3 || scene==4 || scene==5 || scene==7 || scene==10) {
   unsigned sectors[3][3]={{0}},hud=0,dock=0;
   int left=360,right=0,top=360,bottom=0;
   for(unsigned seed=1;seed<=16;seed++)for(unsigned t=5000;t<41000;t+=100) {
    unsigned n=pp_scene_fx_sample(scene,seed,t,true,p),falling=0;
    for(unsigned i=0;i<n;i++)if(p[i].full_screen) {
     falling++;assert(p[i].opacity>0);
     sectors[p[i].y/120][p[i].x/120]++;
     if(p[i].x<left)left=p[i].x;if(p[i].x+p[i].w>right)right=p[i].x+p[i].w;
     if(p[i].y<top)top=p[i].y;if(p[i].y+p[i].h>bottom)bottom=p[i].y+p[i].h;
     hud+=p[i].x>=100&&p[i].x<260&&p[i].y<100;
     dock+=p[i].x>=100&&p[i].x<260&&p[i].y>290;
    }
    assert(falling>=10); /* No burst frame collapses into a sparse middle band. */
   }
   for(unsigned y=0;y<3;y++)for(unsigned x=0;x<3;x++)assert(sectors[y][x]>100);
   assert(left<12&&right>348&&top<12&&bottom>348&&hud>100&&dock>100);
  }
  /* Sea anchors include water behind the HUD, not just the old y>112 strip. */
  if(scene==2) {
   int top=360,bottom=0;
   for(unsigned i=0;i<pp_scene_anchor_counts[scene];i++) {
    int y=pp_scene_anchors[scene][i].y;if(y<top)top=y;if(y>bottom)bottom=y;
   }
   assert(top<35&&bottom>115);
  }
  /* Material-bound highlights never move onto arbitrary air/paths/shorelines. */
  if(scene==2 || scene==6)for(unsigned t=0;t<18000;t+=121) {
   unsigned n=pp_scene_fx_sample(scene,7,t,true,p);
   for(unsigned i=0;i<n;i++) {
    if(p[i].shape>=PP_FX_BAT_UP && p[i].shape<=PP_FX_BAT_DOWN)continue;
    bool anchored=false;
    for(unsigned j=0;j<pp_scene_anchor_counts[scene];j++)
     anchored|=p[i].x+1==pp_scene_anchors[scene][j].x && p[i].y+1==pp_scene_anchors[scene][j].y;
    assert(anchored);
   }
  }

 }
 /* Sky clouds have chunky puff silhouettes and three visibly drifting depths. */
 unsigned cloud_layers=0,cloud_motion=0,cloud_left=360,cloud_right=0;
 for(unsigned seed=1;seed<=16;seed++)for(unsigned t=4000;t<20000;t+=1000) {
  unsigned n=pp_scene_fx_sample(11,seed,t,true,p),m=pp_scene_fx_sample(11,seed,t+1000,true,q);
  assert(n<=12 && pp_scene_fx_part_limit(11)==7);
  for(unsigned i=0;i<n;i++) {
   assert(p[i].shape==PP_FX_CLOUD && p[i].h==p[i].w/2 && p[i].y>=94);
   unsigned layer=p[i].w==40?1:p[i].w==60?2:p[i].w==80?4:0;
   assert(layer);cloud_layers|=layer;
   pp_scene_rect rects[PP_SCENE_FX_PARTS];assert(pp_scene_fx_parts(&p[i],rects)==7);
   int grid=p[i].w/20;
   for(unsigned j=0;j<7;j++)assert((rects[j].x-p[i].x)%grid==0 && (rects[j].y-p[i].y)%grid==0 && rects[j].w%grid==0 && rects[j].h%grid==0);
   for(unsigned j=0;j<m;j++)if(p[i].w==q[j].w && p[i].y-q[j].y>=-2 && p[i].y-q[j].y<=2) {
    int dx=q[j].x-p[i].x;
    if(dx>=14 && dx<=34)cloud_motion|=layer;
   }
   if((unsigned)p[i].x<cloud_left)cloud_left=(unsigned)p[i].x;
   if((unsigned)(p[i].x+p[i].w)>cloud_right)cloud_right=(unsigned)(p[i].x+p[i].w);
  }
 }
 assert(cloud_layers==7 && cloud_motion==7 && cloud_left<50 && cloud_right>310);
 /* Desert weeds are eight-frame sprites, present only inside the event. */
 unsigned tumble_poses=0,tumble_left=360,tumble_right=0,tumble_sizes=0,max_weeds=0,directions=0;
 for(unsigned seed=1;seed<=16;seed++) {
  bool triggered=pp_scene_fx_sample(9,seed,10000,false,p)>0;
  unsigned weeds=0;
  for(unsigned t=0;t<45000;t+=100) {
   unsigned n=pp_scene_fx_sample(9,seed,t,false,p),balls=0;
   if(!triggered)assert(!n);
   for(unsigned i=0;i<n;i++)if(p[i].shape>=PP_FX_TUMBLE_0 && p[i].shape<=PP_FX_TUMBLE_7) {
    balls++;weeds++;
    assert(p[i].y>=190 && p[i].y+p[i].h<=248 && p[i].full_screen);
    tumble_poses|=1u<<(p[i].shape-PP_FX_TUMBLE_0);
    tumble_sizes|=p[i].w==32?1:p[i].w==40?2:p[i].w==36?4:0;
    pp_scene_rect unused[PP_SCENE_FX_PARTS];assert(!pp_scene_fx_parts(&p[i],unused));
    if((unsigned)p[i].x<tumble_left)tumble_left=(unsigned)p[i].x;
    if((unsigned)(p[i].x+p[i].w)>tumble_right)tumble_right=(unsigned)(p[i].x+p[i].w);
   }
   assert(balls<=3);if(balls>max_weeds)max_weeds=balls;
  }
  assert(triggered?weeds>80:!weeds);
  /* Fixed slot0 travels in one direction and bounces while crossing the sand. */
  unsigned a=pp_scene_fx_sample(9,seed,3000,true,p),b=pp_scene_fx_sample(9,seed,3300,true,q);
  int first=-1,second=-1;
  for(unsigned i=0;i<a;i++)if(p[i].shape>=PP_FX_TUMBLE_0 && p[i].w==32)first=(int)i;
  for(unsigned i=0;i<b;i++)if(q[i].shape>=PP_FX_TUMBLE_0 && q[i].w==32)second=(int)i;
  assert(first>=0 && second>=0 && p[first].x!=q[second].x && p[first].y!=q[second].y);
  directions|=p[first].x<q[second].x?1:2;
 }
 assert(tumble_poses==255 && tumble_sizes==7 && max_weeds>=2 && directions==3);
 assert(tumble_left<50 && tumble_right>310);
 /* A cave flock belongs to the30% event, never to the always-moving scenery. */
 unsigned poses=0,bat_left=360,bat_right=0,max_flock=0,near=0,far=0;
 for(unsigned seed=1;seed<=16;seed++) {
  bool triggered=pp_scene_fx_sample(6,seed,10000,false,p)>0;
  unsigned bats=0;
  for(unsigned t=0;t<45000;t+=100) {
   unsigned n=pp_scene_fx_sample(6,seed,t,false,p),flock=0;
   if(!triggered)assert(!n);
   for(unsigned i=0;i<n;i++)if(p[i].shape>=PP_FX_BAT_UP && p[i].shape<=PP_FX_BAT_DOWN) {
    poses|=1u<<(p[i].shape-PP_FX_BAT_UP);bats++;flock++;
    assert(p[i].y>=90 && p[i].y+p[i].h<200 && p[i].full_screen);
    near+=p[i].w==23;far+=p[i].w==17;
    if((unsigned)p[i].x<bat_left)bat_left=(unsigned)p[i].x;
    if((unsigned)(p[i].x+p[i].w)>bat_right)bat_right=(unsigned)(p[i].x+p[i].w);
   }
   assert(flock<=8);if(flock>max_flock)max_flock=flock;
  }
  assert(triggered?bats>80:!bats);
 }
 assert(poses==7 && max_flock>=4 && bat_left<50 && bat_right>310 && near && far);
 assert(!pp_scene_fx_sample(13,1,10000,true,p));
 puts("PASS:13 scene recipes, chunky sky clouds/three moving depths, event-only tumbleweeds/bats, anchored glints, circle safety and fixed pools");
}
