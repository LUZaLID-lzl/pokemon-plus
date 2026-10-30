#include "intro.h"
#include <math.h>
static float ramp(unsigned t,unsigned a,unsigned b) {
  if(t<=a)return 0;
  if(t>=b)return 1;
  return (float)(t-a)/(b-a);
}
static int mix(int a,int b,float t) {return (int)lroundf(a+(b-a)*t);}
pp_intro_state pp_intro_sample(uint32_t t) {
  pp_intro_state s={0};
  const int start_x[]={26,282,250},start_y[]={155,143,245};
  const int meet_x[]={95,177,235},meet_y[]={166,166,190};
  const unsigned starts[]={650,1800,2950},ends[]={2350,3450,4500};
  s.finished=t>=PP_INTRO_MS;
  s.fade=(unsigned)(255*(1-ramp(t,0,650)+ramp(t,10600,11200)));
  s.title=(unsigned)(255*ramp(t,350,1000));
  s.caption=(unsigned)(255*ramp(t,8100,8650));
  s.icons=(unsigned)(255*ramp(t,7800,8350));
  for(unsigned i=0;i<3;i++) {
    pp_intro_actor *p=&s.pets[i];
    float entry=ramp(t,starts[i],ends[i]),line=ramp(t,6850,7850);
    p->x=mix(mix(start_x[i],meet_x[i],entry),84+(int)i*72,line);
    p->y=mix(mix(start_y[i],meet_y[i],entry),172,line);
    p->opacity=(unsigned)(255*ramp(t,starts[i],starts[i]+300));
    p->left=i>0 && t<6850;
    p->pose=t<ends[i]|| (t>=6850 && t<7850)?4:0;
    unsigned since=t<6850?starts[i]:6850;
    if(t>=7850+i*240 && t<8900+i*240) {
      p->pose=8;since=7850+i*240;
    }
    if(i==2 && t>=4500 && t<5200) {p->pose=20;since=4500;}
    if(i==1 && t>=5300 && t<6000) {p->pose=8;since=5300;}
    if(i==0 && t>=6000 && t<6700) {p->pose=8;since=6000;}
    p->elapsed=t>since?t-since:0;
    if(i==1 && t>=5200 && t<5650)p->y-=(int)(9*sinf((t-5200)*3.14159265f/450));
  }
  /* Three small greetings: bubbles, warm sparks, then leaves. */
  for(unsigned i=0;i<PP_INTRO_PARTICLES;i++) {
    pp_intro_particle *p=&s.particles[i];
    unsigned group=i/6,n=i%6,begin=4500+group*700+n*70;
    if(t<begin || t>=begin+1150)continue;
    float u=(float)(t-begin)/1150;
    p->type=group==0?2:group==1?1:0;
    p->opacity=(unsigned)(205*sinf(u*3.14159265f));
    p->size=group==0?5+(n%3)*2:3+(n%2)*2;
    p->x=mix(group==0?246:group==1?200:116,group==0?179:group==1?147:207,u)+(int)(4*sinf(u*6+n));
    p->y=mix(group==0?190:group==1?169:168,118+(int)(n%3)*9,u)-(int)(10*sinf(u*3.14159265f));
  }
  return s;
}
