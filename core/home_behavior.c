#include "home_behavior.h"
#include "scene_layout.h"
#include "home_navigation.h"
#include <string.h>
#include <stdlib.h>
/* Match the six shared pose identifiers, without depending on generated art. */
enum {IDLE=0,WALK=4,HAPPY=8};
static int reflect(int value,int low,int high){
 int width=high-low,phase=(value-low)%(width*2);if(phase<0)phase+=width*2;
 return low+(phase>width?width*2-phase:phase);
}
unsigned pp_home_patrol(uint32_t elapsed,unsigned slot,bool tired,int origin_x,int origin_y,
 bool facing,int *x,int *y,bool *left){
 static const int rx[]={0,62,88,24,0},ry[]={0,30,68,54,0};
 unsigned cycle=elapsed%(tired?24000u:20000u);
 *x=origin_x;*y=origin_y;*left=facing;
 if(cycle>=14400)return IDLE;
 unsigned segment=cycle/3600,t=cycle%3600;int sign=slot%2?-1:1;
 int raw_x=origin_x+sign*(rx[segment]+(rx[segment+1]-rx[segment])*(int)t/3600);
 int raw_y=origin_y+ry[segment]+(ry[segment+1]-ry[segment])*(int)t/3600;
 *x=reflect(raw_x,48,272);*y=reflect(raw_y,115,208);
 int direction=sign*(rx[segment+1]-rx[segment]);
 *left=reflect(raw_x+(direction<0?-1:1),48,272)<*x;
 return WALK;
}
static uint32_t roll(pp_home_behavior_state *s){s->rng=s->rng*1664525u+1013904223u;return s->rng;}
unsigned pp_home_scene_patrol(unsigned scene,uint32_t elapsed,unsigned slot,bool tired,
 int origin_x,int origin_y,bool facing,int *x,int *y,bool *left){
 if(scene==12){pp_lake_patrol(elapsed,x,y,left);return WALK;}
 pp_home_ground_project(scene,&origin_x,&origin_y);
 unsigned cycle=elapsed%(tired?24000u:20000u);
 *x=origin_x;*y=origin_y;*left=facing;
 if(cycle>=14400)return IDLE;
 int points[PP_HOME_ROUTE_POINTS][2]={{origin_x,origin_y}};unsigned count=1,length=0;
 for(unsigned segment=0;segment<4;segment++){
  int ax,ay,bx,by,part[6][2];bool direction;
  pp_home_patrol(segment*3600,slot,tired,origin_x,origin_y,facing,&ax,&ay,&direction);
  pp_home_patrol((segment+1)*3600,slot,tired,origin_x,origin_y,facing,&bx,&by,&direction);
  unsigned n=pp_home_nest_route(scene,ax,ay,bx,by,part);
  for(unsigned j=1;j<n&&count<PP_HOME_ROUTE_POINTS;j++){
   length+=abs(part[j][0]-points[count-1][0])+abs(part[j][1]-points[count-1][1]);
   points[count][0]=part[j][0];points[count++][1]=part[j][1];
  }
 }
 pp_home_path_sample(scene,points,count,length,cycle,14400,x,y,left);
 return WALK;
}
static int clamp(int x,int a,int b){return x<a?a:x>b?b:x;}
static bool due(uint32_t now,uint32_t when){return (int32_t)(now-when)>=0;}
static void safe(const pp_home_behavior_state *s,int *x,int *y){
 *x=clamp(*x,s->scene==12?40:48,272);*y=clamp(*y,115,s->scene==12?214:208);
 if(s->scene==12){
  if(*x+24>=86 && *x+24<=274 && *y>132 && *y<214)*y=214;
  bool left;unsigned phase=pp_lake_nearest_phase(*x,*y);pp_lake_patrol(phase,x,y,&left);
 }
 pp_home_ground_project(s->scene,x,y);
}
static unsigned approach(const pp_home_behavior_state *s,int x,int y,int tx,int ty){
 int points[PP_HOME_ROUTE_POINTS][2];unsigned distance;pp_home_route(s->scene,x,y,tx,ty,points,&distance);
 unsigned ms=distance*35;return ms<1000?1000:ms;
}
void pp_home_behavior_init(pp_home_behavior_state *s,uint32_t seed,uint32_t now,unsigned scene){
 memset(s,0,sizeof(*s));s->rng=seed;s->scene=scene;
 s->next_solo=now+20000+roll(s)%20001;s->next_pair=now+60000+roll(s)%60001;
}
void pp_home_behavior_cancel(pp_home_behavior_state *s,unsigned p){
 if(p>=PP_HOME_ACTORS)return;
 unsigned kind=s->pets[p].kind,b=s->pets[p].partner;
 s->pets[p].kind=0;
 if(kind>=PP_HOME_GREET&&kind<=PP_HOME_REST&&b<PP_HOME_ACTORS)s->pets[b].kind=0;
}
void pp_home_behavior_clear(pp_home_behavior_state *s){memset(s->pets,0,sizeof(s->pets));}
static void begin(pp_home_behavior_state *s,uint32_t now,unsigned kind,unsigned p,unsigned b,
 int tx,int ty,const pp_home_actor *in,unsigned span){
 pp_home_activity *a=&s->pets[p];
 *a=(pp_home_activity){.kind=kind,.partner=b,.x=in[p].x,.y=in[p].y,.tx=tx,.ty=ty,
 .left=in[p].left,.start=now,.approach=span};
 unsigned tail=kind==PP_HOME_EXPLORE?2600:kind==PP_HOME_STROLL?4400:kind==PP_HOME_REST?5200:kind==PP_HOME_GREET?2800:1800;
 a->duration=kind==PP_HOME_PAT?2000:kind==PP_HOME_HOP?1800:span+tail;
}
bool pp_home_behavior_request(pp_home_behavior_state *s,uint32_t now,unsigned kind,unsigned a,unsigned b,
 int x,int y,const pp_home_actor *in,unsigned count){
 if(kind<PP_HOME_PAT||kind>PP_HOME_WELCOME||a>=count||a>=PP_HOME_ACTORS||!in[a].available)return false;
 bool pair=kind>=PP_HOME_GREET&&kind<=PP_HOME_REST;
 if(pair&&(b>=count||b>=PP_HOME_ACTORS||a==b||!in[b].available))return false;
 if(pair)for(unsigned p=0;p<PP_HOME_ACTORS;p++)if(s->pets[p].kind>=PP_HOME_GREET&&s->pets[p].kind<=PP_HOME_REST)return false;
 pp_home_behavior_cancel(s,a);if(pair)pp_home_behavior_cancel(s,b);
 if(kind==PP_HOME_PAT||kind==PP_HOME_HOP){x=in[a].x;y=in[a].y;}
 else if(pair){x=clamp((in[a].x+in[b].x)/2,76,216)-26;y=(in[a].y+in[b].y)/2;}
 if(kind!=PP_HOME_PAT&&kind!=PP_HOME_HOP)safe(s,&x,&y);
 int bx=x+54,by=y+(kind==PP_HOME_STROLL?8:0);safe(s,&bx,&by);
 unsigned span=approach(s,in[a].x,in[a].y,x,y);
 if(pair){unsigned t=approach(s,in[b].x,in[b].y,bx,by);if(t>span)span=t;}
 begin(s,now,kind,a,pair?b:PP_HOME_ACTORS,x,y,in,span);
 if(pair)begin(s,now,kind,b,a,bx,by,in,span);
 return true;
}
int pp_home_behavior_call(pp_home_behavior_state *s,uint32_t now,int x,int y,const pp_home_actor *in,unsigned count){
 if(x<62||x>298||y<135||y>248||(s->called&&now-s->last_call<900))return -1;
 int tx=x-24,ty=y-24;safe(s,&tx,&ty);
 for(unsigned p=0;p<count&&p<PP_HOME_ACTORS;p++)if((in[p].x-tx)*(in[p].x-tx)+(in[p].y-ty)*(in[p].y-ty)<34*34)return -1;
 unsigned chosen=PP_HOME_ACTORS;int best=1000000;
 for(unsigned p=0;p<count&&p<PP_HOME_ACTORS;p++)if(in[p].available&&in[p].energy>=20){
  int d=(in[p].x-tx)*(in[p].x-tx)+(in[p].y-ty)*(in[p].y-ty);
  if(d<best){best=d;chosen=p;}
 }
 if(chosen==PP_HOME_ACTORS)return -1;
 pp_home_behavior_request(s,now,PP_HOME_CALL,chosen,0,tx,ty,in,count);
 s->called=true;s->last_call=now;return (int)chosen;
}
static void move(const pp_home_behavior_state *s,const pp_home_activity *a,unsigned age,int *x,int *y,bool *left){
 *left=a->left;
 if(a->kind==PP_HOME_PAT||a->kind==PP_HOME_HOP){*x=a->x;*y=a->y;return;}
 pp_home_route_sample(s->scene,a->x,a->y,a->tx,a->ty,age,a->approach,x,y,left);
}
static void schedule(pp_home_behavior_state *s,uint32_t now,const pp_home_actor *in,unsigned count){
 if(due(now,s->next_solo)){
  s->next_solo=now+20000+roll(s)%20001;
  unsigned start=roll(s)%PP_HOME_ACTORS;
  for(unsigned n=0;n<count;n++){unsigned p=(start+n)%count;
   if(!in[p].available||s->pets[p].kind||in[p].energy<30)continue;
   unsigned kind=in[p].mood>=60&&roll(s)%3==0?PP_HOME_HOP:PP_HOME_EXPLORE;
   int x=in[p].x+(int)(roll(s)%101)-50,y=in[p].y+(int)(roll(s)%55)-27;safe(s,&x,&y);
   bool free=true;for(unsigned j=0;j<count;j++)if(j!=p&&abs(in[j].x-x)<38&&abs(in[j].y-y)<28)free=false;
   if(free)pp_home_behavior_request(s,now,kind,p,0,x,y,in,count);
   break;
  }
 }
 if(due(now,s->next_pair)){
  s->next_pair=now+60000+roll(s)%60001;
  unsigned a=PP_HOME_ACTORS,b=PP_HOME_ACTORS;int best=140*140;
  for(unsigned i=0;i<count;i++)for(unsigned j=i+1;j<count;j++){
   if(!in[i].available||!in[j].available||in[i].energy<30||in[j].energy<30||s->pets[i].kind||s->pets[j].kind)continue;
   int d=(in[i].x-in[j].x)*(in[i].x-in[j].x)+(in[i].y-in[j].y)*(in[i].y-in[j].y);
   if(d<best){best=d;a=i;b=j;}
  }
  if(a<PP_HOME_ACTORS)pp_home_behavior_request(s,now,PP_HOME_GREET+roll(s)%3,a,b,0,0,in,count);
 }
}
void pp_home_behavior_tick(pp_home_behavior_state *s,uint32_t now,const pp_home_actor *in,unsigned count,
 bool automatic,pp_home_output out[PP_HOME_ACTORS]){
 if(count>PP_HOME_ACTORS)count=PP_HOME_ACTORS;
 memset(out,0,sizeof(*out)*PP_HOME_ACTORS);
 for(unsigned p=0;p<PP_HOME_ACTORS;p++)if(s->pets[p].kind&&(p>=count||!in[p].available)){
  unsigned b=s->pets[p].partner;out[p].release=true;
  if(b<count)out[b].release=true;
  pp_home_behavior_cancel(s,p);
 }
 if(automatic&&count)schedule(s,now,in,count);
 else {if(due(now,s->next_solo))s->next_solo=now+20000;if(due(now,s->next_pair))s->next_pair=now+60000;}
 for(unsigned p=0;p<count;p++){
  pp_home_activity *a=&s->pets[p];pp_home_output *o=&out[p];
  if(o->release){o->x=in[p].x;o->y=in[p].y;o->left=in[p].left;}
  if(!a->kind)continue;
  unsigned age=now-a->start;bool stationary=a->kind==PP_HOME_PAT||a->kind==PP_HOME_HOP;
  o->active=true;o->kind=a->kind;o->left=a->left;o->pose=IDLE;
  move(s,a,stationary?0:age,&o->x,&o->y,&o->left);
  if(!stationary&&age<a->approach){o->pose=(o->x!=a->tx||o->y!=a->ty)?WALK:IDLE;}
  else{
   unsigned t=stationary?age:age-a->approach;
   if(a->kind==PP_HOME_PAT||a->kind==PP_HOME_HOP||a->kind==PP_HOME_CALL||a->kind==PP_HOME_WELCOME){
    o->pose=HAPPY;o->left=o->x>156;o->symbol=a->kind==PP_HOME_PAT?1:2;
    unsigned phase=t%650;if(t<1300)o->jump=(int)(4u*4u*phase*(650-phase)/(650u*650u));
   }else if(a->kind==PP_HOME_EXPLORE){o->left=t<800?a->left:t<1700?!a->left:a->left;o->symbol=t<1600?3:0;}
   else if(a->kind==PP_HOME_GREET){o->left=a->tx>s->pets[a->partner].tx;unsigned offset=p<a->partner?0:1000;
    if(t>=offset&&t<offset+1000){o->pose=HAPPY;o->symbol=2;}}
   else if(a->kind==PP_HOME_REST){o->left=a->tx>s->pets[a->partner].tx;}
   else if(a->kind==PP_HOME_STROLL){
    int anchor=a->tx<s->pets[a->partner].tx?a->tx:s->pets[a->partner].tx;
    int direction=anchor<150?1:-1;unsigned delay=p<a->partner?800:1200;
    if(t>=delay){unsigned walk=t-delay;if(walk>2800)walk=2800;
     pp_home_route_sample(s->scene,a->tx,a->ty,a->tx+direction*54,a->ty,walk,2800,&o->x,&o->y,&o->left);
     o->pose=walk<2800?WALK:IDLE;}
   }
  }
  if(age>=a->duration){o->active=false;o->release=true;o->jump=o->symbol=0;o->pose=IDLE;a->kind=0;}
 }
}
