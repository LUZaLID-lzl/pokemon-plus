#include "training.h"
#include "balance_config.h"
#include <string.h>
const pp_training_scene pp_training_scenes[6]={
 {"晨风草原","高草丛",1,10,{16,19,10,13,29,32,1,4,7,21,23,25,27,35,37,39,52,56,58,77,83,84,128,133},0x477d48,0xa5d176},
 {"萤光密林","蕨叶间",11,20,{25,43,46,48,69,102,11,12,14,15,17,18,24,30,33,44,47,49,70,71,103,108,114,123,41,42},0x24634e,0x80c9a4},
 {"潮汐湿地","芦苇荡",21,30,{54,60,72,79,98,118,55,61,62,63,73,80,99,116,117,119,120,121,129,130,131,132,138,139,140,141,147,148},0x407f77,0xa8d8b6},
 {"赤岩峡谷","碎石坡",31,40,{27,50,66,74,95,111,28,51,57,59,67,68,75,76,78,81,82,104,105,106,107,112,115,126,127,142},0x975c45,0xdeae6b},
 {"霜雪山径","积雪间",41,50,{86,87,90,91,124,125,85,88,89,92,93,94,96,97,100,101,109,110,113,122,134,135,136,143},0x7696ae,0xd6eafa},
 {"星辉遗迹","符文石",51,60,{64,67,82,93,97,105,2,3,5,6,8,9,20,22,26,31,34,36,38,40,45,53,65,83,137,149},0x466a80,0x92d7dc}
};
unsigned pp_training_pool_count(unsigned scene){if(scene>=PP_TRAINING_SCENES)return 0;unsigned n=0;while(n<PP_TRAINING_POOL&&pp_training_scenes[scene].dex[n])n++;return n;}
static unsigned roll(pp_training *t,unsigned n){uint32_t x=t->rng;x^=x<<13;x^=x>>17;x^=x<<5;t->rng=x?x:1;return t->rng%n;}
static const pp_dex_entry *dex(unsigned national){for(unsigned i=0;i<PP_DEX_COUNT;i++)if(pp_pokedex[i].national_id==national)return &pp_pokedex[i];return &pp_pokedex[0];}
bool pp_training_pet(const pp_game *g,unsigned pick,pp_stored_pet *p){
 if(pick>=PP_PETS)return pp_warehouse_get(g,pick-PP_PETS,p);
 if(pick>=g->count)return false;
 *p=(pp_stored_pet){.pet=g->pets[pick],.growth=g->growth[pick],.identity=g->identity[pick],.declined=(g->evolution_declined>>pick)&1,.pats_today=g->pats_today[pick],.balance=g->pet_balance[pick]};return true;
}
void pp_training_init(pp_training *t,uint32_t seed){memset(t,0,sizeof(*t));t->rng=seed?seed:1;}
static void party(pp_training *t,const pp_game *g){
 memset(&t->battle,0,sizeof(t->battle));t->battle.scene=t->scene;t->battle.count=t->count;t->battle.traveler=-1;t->battle.rng=roll(t,UINT32_MAX)|1;
 for(unsigned i=0;i<t->count;i++){pp_stored_pet p;pp_training_pet(g,t->picks[i],&p);t->battle.pets[i]=p.pet;t->battle.growth[i]=p.growth;t->battle.identity[i]=p.identity;if(p.declined)t->battle.evolution_declined|=1u<<i;}
}
pp_result pp_training_start(pp_training *t,pp_game *g,unsigned scene,const unsigned *picks,unsigned count){
 if(scene>=6||!picks||count<1||count>3)return PP_INVALID;
 if(g->training.status==1||pp_training_growth_pending(g)||g->battling||pp_wild_opened(g)||(g->wild.phase==PP_WILD_WON&&g->wild.outcome==PP_WILD_CAPTURE_READY)||pp_needs_starter(g))return PP_BUSY;
 for(unsigned i=0;i<count;i++){
  pp_stored_pet p;if(picks[i]>=g->count||!pp_training_pet(g,picks[i],&p))return PP_INVALID;
  for(unsigned j=0;j<i;j++)if(picks[j]==picks[i])return PP_INVALID;
  if(picks[i]<PP_PETS&&((int)picks[i]==g->traveler||(g->drop_active&&picks[i]==g->drop_target)))return PP_BUSY;
  if(pp_training_busy(g,picks[i]) || (picks[i]>=PP_PETS&&g->fishing_partner==picks[i]-PP_PETS+1))return PP_BUSY;
  if(p.pet.sleeping||p.pet.health<30||p.pet.food<20||p.pet.energy<40)return PP_NOT_READY;
 }
 pp_training_close(t);t->count=count;t->scene=scene;t->phase=PP_TRAINING_SEARCH;t->elapsed=t->search_checks=t->clock_ms=t->rounds=t->total_xp=t->last_xp=0;
 memcpy(t->picks,picks,count*sizeof(*picks));party(t,g);return PP_OK;
}
static void prepare_encounter(pp_training *t,pp_game *g){
 party(t,g);const pp_training_scene *s=&pp_training_scenes[t->scene];
 /* Draw without replacement from the full regional roster. */
 unsigned pool[PP_TRAINING_POOL],size=0;for(unsigned n=0;n<pp_training_pool_count(t->scene);n++){bool recent=false;for(unsigned k=0;t->rounds&&k<t->count;k++)recent|=dex(s->dex[n])->art==t->foes[k].art;if(!recent)pool[size++]=n;}
 for(unsigned i=0;i<t->count;i++){
  unsigned n=i+roll(t,size-i),swap=pool[i];pool[i]=pool[n];pool[n]=swap;
  const pp_dex_entry *d=dex(s->dex[pool[i]]);unsigned levels[3];for(unsigned k=0;k<t->count;k++)levels[k]=t->battle.growth[k].level;
  for(unsigned k=0;k<t->count;k++)for(unsigned q=k+1;q<t->count;q++)if(levels[q]<levels[k]){unsigned v=levels[q];levels[q]=levels[k];levels[k]=v;}
  unsigned ref=levels[t->count/2],spread=ref<=5?1:2,lo=ref>spread?ref-spread:1,hi=ref<=5?ref:ref<=10?ref+1:ref+2;if(lo<s->min_level)lo=s->min_level;if(lo>s->max_level)lo=s->max_level;if(hi>s->max_level)hi=s->max_level;if(hi<lo)hi=lo;
  unsigned level=lo+roll(t,hi-lo+1);
  pp_game foe={0};foe.count=1;foe.pets[0].species=d->species;foe.growth[0]=(pp_growth){.level=level,.stage=d->stage};for(unsigned k=0;k<6;k++)foe.identity[0].iv[k]=15;
  t->foes[i]=(pp_gym_enemy){.name=d->name,.art=d->art,.level=level,.hp=pp_individual_stat(&foe,0,0),.attack=(pp_individual_stat(&foe,0,1)+pp_individual_stat(&foe,0,3))/2,.defense=(pp_individual_stat(&foe,0,2)+pp_individual_stat(&foe,0,4))/2};
  static const unsigned scales[3]=PP_BALANCE_NOVICE_SCALES;unsigned scale=scales[ref<=5?0:ref<=10?1:2];t->foes[i].hp=t->foes[i].hp*scale/100;t->foes[i].attack=t->foes[i].attack*scale/100;t->foes[i].defense=t->foes[i].defense*scale/100;
 }
 unsigned avg=0;for(unsigned i=0;i<t->count;i++)avg+=t->foes[i].level;avg/=t->count;
 t->arena=(pp_gym_def){.title=s->name,.team=t->foes,.win_xp=1+3*avg/10,.loss_xp=0};
 t->phase=PP_TRAINING_APPROACH;t->clock_ms=0;
}
static void encounter(pp_training *t){
 if(pp_battle_encounter_start(&t->battle,&t->arena,t->count)==PP_OK){t->phase=PP_TRAINING_FIGHT;t->elapsed=t->clock_ms=0;t->awarded=false;t->speed_remainder=0;}
}
void pp_training_advance(pp_training *t,pp_game *g,unsigned ms){
 /* The managed job supplies bounded steps, including background catch-up. */
 if(ms>250)ms=250;
 if(t->phase==PP_TRAINING_SEARCH){t->elapsed+=ms;unsigned check=t->elapsed/1000;
  if(check>t->search_checks){t->search_checks=check;if(check>=5&&(roll(t,100)<25||check>=16)){prepare_encounter(t,g);}}
 }else if(t->phase==PP_TRAINING_APPROACH){
  t->clock_ms+=ms;if(t->clock_ms>=PP_TRAINING_APPROACH_MS){t->clock_ms=0;t->phase=PP_TRAINING_ALERT;}
 }else if(t->phase==PP_TRAINING_ALERT){
  t->clock_ms+=ms;if(t->clock_ms>=2000)encounter(t);
 }else if(t->phase==PP_TRAINING_FIGHT){
  unsigned scaled=ms*3+t->speed_remainder;t->speed_remainder=scaled%2;
  t->clock_ms+=scaled/2;if(t->clock_ms>=1000){t->clock_ms-=1000;pp_tick(&t->battle,1,0);}
  if(!t->battle.battling){t->phase=PP_TRAINING_RESULT;t->elapsed=0;}
 }
}
pp_result pp_training_claim(pp_training *t,pp_game *g){
 if(t->phase!=PP_TRAINING_RESULT||t->battle.battling)return PP_NOT_READY;
 if(t->awarded)return PP_OK;
 for(unsigned i=0;i<t->count;i++){pp_stored_pet p;if(!pp_training_pet(g,t->picks[i],&p))return PP_INVALID;}
 unsigned enemy=0,defeated=0;for(unsigned i=0;i<t->count;i++){enemy+=t->foes[i].level;if(!t->battle.gym.enemy_hp[i])defeated++;}enemy/=t->count;
 t->last_xp=0;for(unsigned i=0;i<t->count;i++){
  unsigned pick=t->picks[i];if(pick>=g->count)return PP_INVALID;
  t->last_xp+=pp_award_battle_xp(g,pick,t->battle.growth[i].level,enemy,defeated,t->count,t->battle.gym.result==1,PP_TRAINING_XP_MULTIPLIER);
  pp_training_cost(g,pick,t->battle.gym.hp[i]==0);
 }
 t->battle.gym.reward_xp=t->last_xp;
 t->total_xp+=t->last_xp;t->rounds++;t->awarded=true;return PP_OK;
}
void pp_training_resume(pp_training *t){if(t->phase==PP_TRAINING_RESULT&&t->awarded){pp_battle_encounter_close(&t->battle);t->phase=PP_TRAINING_SEARCH;t->elapsed=t->search_checks=t->clock_ms=0;}}
void pp_training_close(pp_training *t){pp_battle_encounter_close(&t->battle);t->battle.battling=0;t->phase=PP_TRAINING_SELECT;}
