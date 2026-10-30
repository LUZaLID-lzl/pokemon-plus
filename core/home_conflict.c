#include "home_conflict.h"
#include "moves.h"
#include <string.h>
static uint32_t roll(pp_home_conflict *s){s->rng=s->rng*1664525u+1013904223u;return s->rng;}
static void schedule(pp_home_conflict *s,uint32_t now){
 s->next_check=now+PP_HOME_CONFLICT_MIN_MS+roll(s)%(PP_HOME_CONFLICT_MAX_MS-PP_HOME_CONFLICT_MIN_MS+1);
}
bool pp_nature_feisty(unsigned n){return n==3||n==4||n==8||n==11||n==24;}
void pp_home_conflict_init(pp_home_conflict *s,uint32_t seed,uint32_t now){
 memset(s,0,sizeof(*s));s->rng=seed;schedule(s,now);
}
void pp_home_conflict_pause(pp_home_conflict *s,uint32_t now){
 if(!s->suspended)schedule(s,now);
 s->suspended=true;s->active=false;
}
static bool available(const pp_game *g,unsigned p,unsigned mask){
 return p<g->count && (mask&(1u<<p)) && !g->pets[p].sleeping &&
  g->traveler!=(int)p && g->pets[p].health && g->pets[p].energy>=30;
}
static bool valid_pair(const pp_home_conflict *s,const pp_game *g,unsigned mask){
 return s->attacker!=s->target && available(g,s->attacker,mask) && available(g,s->target,mask) &&
  pp_nature_feisty(g->identity[s->attacker].nature) && g->pets[s->attacker].health>=30 &&
  (s->hit || g->pets[s->target].health>1);
}
bool pp_home_conflict_start(pp_home_conflict *s,const pp_game *g,uint32_t now,unsigned a,unsigned b,unsigned slot){
 pp_home_conflict pair={.attacker=a,.target=b};
 if(s->active || g->battling || g->drop_active || g->wild.phase || g->gift.pending ||
    !valid_pair(&pair,g,(1u<<PP_PETS)-1) || slot>=PP_MOVE_SLOTS || !pp_move_at(g->pets[a].species,slot)->power)return false;
 schedule(s,now);s->attacker=a;s->target=b;s->slot=slot;
 s->type=pp_move_at(g->pets[a].species,slot)->type;s->started=now;
 s->active=true;s->hit=false;s->suspended=false;s->health_loss=s->mood_loss=0;
 return true;
}
pp_conflict_event pp_home_conflict_tick(pp_home_conflict *s,pp_game *g,uint32_t now,
 bool enabled,unsigned mask){
 if(!enabled || g->battling || g->drop_active || g->wild.phase || g->gift.pending){
  bool active=s->active;pp_home_conflict_pause(s,now);return active?PP_CONFLICT_CANCEL:PP_CONFLICT_NONE;
 }
 if(s->suspended){s->suspended=false;schedule(s,now);}
 if(s->active){
  if(!valid_pair(s,g,mask)){s->active=false;return PP_CONFLICT_CANCEL;}
  uint32_t age=now-s->started;
  /* Missed/hidden animation must never produce a delayed invisible hit. */
  if(age>=PP_HOME_CONFLICT_DURATION_MS){s->active=false;return PP_CONFLICT_END;}
  if(!s->hit && age>=PP_HOME_CONFLICT_IMPACT_MS){
   pp_pet *p=&g->pets[s->target];
   s->health_loss=p->health>PP_HOME_CONFLICT_HEALTH?PP_HOME_CONFLICT_HEALTH:p->health-1;
   s->mood_loss=p->mood>PP_HOME_CONFLICT_MOOD?PP_HOME_CONFLICT_MOOD:p->mood;
   p->health-=s->health_loss;p->mood-=s->mood_loss;s->hit=true;g->revision++;
   return PP_CONFLICT_HIT;
  }
  return PP_CONFLICT_NONE;
 }
 if((int32_t)(now-s->next_check)<0)return PP_CONFLICT_NONE;
 schedule(s,now);
 if(roll(s)%100>=PP_HOME_CONFLICT_CHANCE)return PP_CONFLICT_NONE;
 unsigned pairs[PP_PETS*(PP_PETS-1)][2],count=0;
 for(unsigned a=0;a<g->count;a++)for(unsigned b=0;b<g->count;b++){
  pp_home_conflict candidate={.attacker=a,.target=b};
  if(valid_pair(&candidate,g,mask)){pairs[count][0]=a;pairs[count++][1]=b;}
 }
 if(!count)return PP_CONFLICT_NONE;
 unsigned chosen=roll(s)%count;s->attacker=pairs[chosen][0];s->target=pairs[chosen][1];
 unsigned slots[PP_MOVE_SLOTS],moves=0;
 for(unsigned i=0;i<PP_MOVE_SLOTS;i++)if(pp_move_at(g->pets[s->attacker].species,i)->power)slots[moves++]=i;
 if(!moves)return PP_CONFLICT_NONE;
 unsigned slot=slots[roll(s)%moves];
 return pp_home_conflict_start(s,g,now,s->attacker,s->target,slot)?PP_CONFLICT_BEGIN:PP_CONFLICT_NONE;
}
unsigned pp_home_conflict_sample(const pp_home_conflict *s,uint32_t now,
 int sx,int sy,int tx,int ty,pp_scene_particle out[PP_HOME_CONFLICT_PARTICLES]){
 if(!s->active)return 0;
 unsigned age=now-s->started;if(age<200||age>=1150)return 0;
 static const unsigned palette[]={0xffdf69,0x8fe68b,0xff9a63,0x6edaff,0xe6d2c1,
  0xb8d961,0xc78de0,0xb4e8ed,0xd6b685,0xbcb397,0xf4a8c5,0xd695ed,
  0xe6a083,0xa8e5f3,0xcbd8e2,0x9f87dc,0x796b99,0x9296ff};
 unsigned tint=palette[s->type<18?s->type:PP_NORMAL];
 unsigned t=age>PP_HOME_CONFLICT_IMPACT_MS?PP_HOME_CONFLICT_IMPACT_MS:age;
 int x=sx+(tx-sx)*(int)(t-200)/(int)(PP_HOME_CONFLICT_IMPACT_MS-200);
 int y=sy+(ty-sy)*(int)(t-200)/(int)(PP_HOME_CONFLICT_IMPACT_MS-200);
 static const int direction[8][2]={{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
 for(unsigned i=0;i<PP_HOME_CONFLICT_PARTICLES;i++){
  int dx,dy,w=3,h=3;
  if(age>=PP_HOME_CONFLICT_IMPACT_MS){int radius=5+(int)(age-PP_HOME_CONFLICT_IMPACT_MS)/35+(int)(i/8)*4;
   dx=direction[i%8][0]*radius;dy=direction[i%8][1]*radius;
  }else{dx=(int)(i%4)*5-8;dy=(int)(i/4)*4-6+(int)((age/60+i)%3)*2;
   if(s->type==PP_ELECTRIC){dx+=(i%2?3:-3);h=6;}
   if(s->type==PP_GRASS||s->type==PP_BUG){w=6;h=3;}
   if(s->type==PP_FIRE)dy-=(int)(i%3)*3;
  }
  out[i]=(pp_scene_particle){.x=x+dx,.y=y+dy,.w=w,.h=h,.color=i%4?tint:0xfff2d5,
   .opacity=age<700?230:230*(1150-age)/450,.shape=PP_FX_DOT};
 }
 return PP_HOME_CONFLICT_PARTICLES;
}
