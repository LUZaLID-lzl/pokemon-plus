#include "training.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "training_events.h"
#include "balance_config.h"

static uint32_t loot_next(pp_training_record *r){
 uint32_t x=r->loot_rng;x^=x<<13;x^=x>>17;x^=x<<5;return r->loot_rng=x?x:1;
}
static const char *foe_name(unsigned art){
 for(unsigned i=0;i<PP_DEX_COUNT;i++)if(pp_pokedex[i].art==art)return pp_pokedex[i].name;
 return pp_pokedex[0].name;
}
void pp_training_checkpoint(pp_training *t,pp_game *g){
 pp_training_record *r=&g->training;
 r->scene=t->scene;r->count=t->count;r->phase=t->phase;
 memcpy(r->picks,t->picks,sizeof(r->picks));r->phase_ms=t->elapsed;r->clock_ms=t->clock_ms;
 r->search_checks=t->search_checks;r->rng=t->rng;r->rounds=t->rounds;r->last_xp=t->last_xp;
 r->total_xp=t->total_xp;r->awarded=t->awarded;r->speed_remainder=t->speed_remainder;
 for(unsigned i=0;i<t->count;i++){
  pp_training_pet(&t->battle,i,&r->party[i]);
  r->foes[i].art=t->foes[i].art;r->foes[i].level=t->foes[i].level;
  r->foes[i].hp=t->foes[i].hp;r->foes[i].attack=t->foes[i].attack;r->foes[i].defense=t->foes[i].defense;
 }
 r->gym=t->battle.gym;r->battle_seconds=t->battle.seconds;r->battle_next=t->battle.battle_next;
 r->battle_rng=t->battle.rng;r->battle_round=t->battle.battle_round;r->battling=t->battle.battling;r->battle_pet=t->battle.battle_pet;
 r->hp[0]=t->battle.battle_hp;r->hp[1]=t->battle.enemy_hp;r->hp[2]=t->battle.battle_max_hp;r->hp[3]=t->battle.enemy_max_hp;
}
bool pp_training_restore(pp_training *t,pp_game *g){
 pp_training_close(t);pp_training_init(t,g->rng^0x74726169);
 const pp_training_record *r=&g->training;if(!r->status)return false;
 t->scene=r->scene;t->count=r->count;t->phase=r->phase;t->elapsed=r->phase_ms;t->clock_ms=r->clock_ms;
 t->search_checks=r->search_checks;t->rng=r->rng;t->rounds=r->rounds;t->last_xp=r->last_xp;
 t->total_xp=r->total_xp;t->awarded=r->awarded;t->speed_remainder=r->speed_remainder;memcpy(t->picks,r->picks,sizeof(t->picks));
 t->battle.count=t->count;t->battle.scene=t->scene;t->battle.traveler=-1;
 unsigned level=0,ally_level=0;
 for(unsigned i=0;i<t->count;i++){
  t->battle.pets[i]=r->party[i].pet;t->battle.growth[i]=r->party[i].growth;t->battle.identity[i]=r->party[i].identity;t->battle.pet_balance[i]=r->party[i].balance;
  if(r->party[i].declined)t->battle.evolution_declined|=1u<<i;
  t->foes[i]=(pp_gym_enemy){foe_name(r->foes[i].art),r->foes[i].level,r->foes[i].art,r->foes[i].hp,r->foes[i].attack,r->foes[i].defense};
  level+=r->foes[i].level;ally_level+=r->party[i].growth.level;
 }
 level/=t->count;(void)ally_level;
 t->arena=(pp_gym_def){.title=pp_training_scenes[t->scene].name,.team=t->foes,.win_xp=1+3*level/10,.loss_xp=0};
 if(t->phase==PP_TRAINING_FIGHT||t->phase==PP_TRAINING_RESULT){
  if(pp_battle_encounter_start(&t->battle,&t->arena,t->count)!=PP_OK)return false;
 }
 t->battle.gym=r->gym;t->battle.seconds=r->battle_seconds;t->battle.battle_next=r->battle_next;
 t->battle.rng=r->battle_rng;t->battle.battle_round=r->battle_round;t->battle.battling=r->battling;t->battle.battle_pet=r->battle_pet;
 t->battle.battle_hp=r->hp[0];t->battle.enemy_hp=r->hp[1];t->battle.battle_max_hp=r->hp[2];t->battle.enemy_max_hp=r->hp[3];
 /* Older running jobs may already exceed the new limit. Preserve earned rewards. */
 if(r->status==1&&r->elapsed_ms>=PP_TRAINING_DURATION_MS){g->training.pending_ms=0;pp_training_job_finish(t,g);}
 return true;
}
pp_result pp_training_job_start(pp_training *t,pp_game *g,unsigned scene,const unsigned *picks,unsigned count){
 pp_result result=pp_training_start(t,g,scene,picks,count);if(result!=PP_OK)return result;
 memset(&g->training,0,sizeof(g->training));g->training.status=1;g->training.anchor_seconds=g->seconds;g->training.loot_rng=t->rng^0x9e3779b9u;
 if(!g->training.loot_rng)g->training.loot_rng=1;
 for(unsigned i=0;i<count;i++)g->training.start_levels[i]=g->training.end_levels[i]=t->battle.growth[i].level;
 pp_training_checkpoint(t,g);g->revision++;return PP_OK;
}
/* Events share the existing loot receipt and individual save transaction. */
static void care_event(pp_game *g,unsigned kind){
 pp_training_record *r=&g->training;pp_pet *target=NULL;unsigned deficit=0;
 for(unsigned i=0;i<r->count;i++)if(r->picks[i]<g->count){pp_pet *p=&g->pets[r->picks[i]];unsigned value=kind==0?p->energy:kind==1?p->food:kind==2?p->clean:p->mood;if(100-value>deficit){target=p;deficit=100-value;}}
 if(target){uint8_t *value=kind==0?&target->energy:kind==1?&target->food:kind==2?&target->clean:&target->mood;*value+=deficit<2?deficit:2;g->revision++;}
}
void pp_training_discovery(const pp_training_record *r,char *out,size_t size){
 if(!size)return;
 out[0]=0;
 if(r->scene>=6||r->elapsed_ms<60000)return;
 /* Only the first twelve seconds of a five-minute receipt show its result. */
 if(r->loot_checks>=5&&r->elapsed_ms%300000<12000){
  if(r->last_item)snprintf(out,size,"发现 %s ×1",pp_items[r->last_item-1].name);
  else if(r->last_coins)snprintf(out,size,"发现 金币 +%u",r->last_coins);
  else if(r->loot_rng%100>=60)snprintf(out,size,"%s",training_care_events[(r->loot_rng%100-60)/10]);
  if(out[0])return;
 }
 unsigned minute=r->elapsed_ms/60000;snprintf(out,size,"%s",training_events[r->scene][(minute-1+r->scene)%6]);
}
static void discover(pp_game *g){
 pp_training_record *r=&g->training;unsigned chance=loot_next(r)%100;
 r->last_item=0;r->last_coins=0;
 if(chance<50){
  unsigned coins=5+loot_next(r)%4;if(coins>UINT32_MAX-g->coins)coins=UINT32_MAX-g->coins;
  g->coins+=coins;r->coins+=coins;r->last_coins=coins;
 }else if(chance<60){
  static const unsigned weights[5]=PP_TRAINING_ITEM_WEIGHTS;unsigned roll=loot_next(r)%100,item=0;
  while(item<4&&roll>=weights[item]){roll-=weights[item];item++;}
  uint16_t *stock=item<PP_ITEMS?&g->inventory[item]:item==PP_HATCH_BOOST?&g->hatch_boosts:item==PP_BALL?&g->balls:&g->tree_essences;
  if(*stock<UINT16_MAX){(*stock)++;r->items[item]++;r->last_item=item+1;}
 }else care_event(g,(chance-60)/10);
 if(r->last_item||r->last_coins)g->revision++;
}
void pp_training_job_finish(pp_training *t,pp_game *g){
 if(g->training.status!=1)return;
 if(t->phase==PP_TRAINING_RESULT)pp_training_claim(t,g);
 for(unsigned i=0;i<t->count;i++){pp_stored_pet p;if(pp_training_pet(g,t->picks[i],&p))g->training.end_levels[i]=p.growth.level;}
 pp_training_close(t);g->training.pending_ms=0;g->training.status=2;pp_training_checkpoint(t,g);g->revision++;
}
void pp_training_job_advance(pp_training *t,pp_game *g,unsigned ms){pp_training_job_advance_auto(t,g,ms,false);}
static bool can_continue(const pp_training *t,const pp_game *g){
 for(unsigned i=0;i<t->count;i++){unsigned pick=t->picks[i];if(pick>=g->count||g->pets[pick].energy<20||g->pets[pick].food<20||g->pets[pick].health<30||g->pets[pick].sleeping)return false;}return true;
}
static void supply(const pp_training *t,pp_game *g){for(unsigned i=0;i<t->count;i++)pp_training_supply(g,t->picks[i]);}
void pp_training_job_advance_auto(pp_training *t,pp_game *g,unsigned ms,bool auto_supply){
 if(g->training.status!=1)return;
 pp_training_record *r=&g->training;
 if(r->elapsed_ms>=PP_TRAINING_DURATION_MS){r->pending_ms=0;pp_training_job_finish(t,g);return;}
 unsigned remaining=PP_TRAINING_DURATION_MS-r->elapsed_ms-r->pending_ms;if(ms>remaining)ms=remaining;
 ms+=r->pending_ms;r->pending_ms=ms%40;ms-=r->pending_ms;
 unsigned checkpoint=r->elapsed_ms/30000;
 while(ms&&r->status==1){
  unsigned step=ms>40?40:ms;ms-=step;
  if(auto_supply&&t->phase==PP_TRAINING_SEARCH){supply(t,g);if(!can_continue(t,g)){pp_training_job_finish(t,g);break;}}
  if(t->phase==PP_TRAINING_SEARCH||t->phase==PP_TRAINING_APPROACH)r->walk_ms+=step;
  r->ambient_ms+=step;r->elapsed_ms+=step;
  pp_training_advance(t,g,step);
  if(t->phase==PP_TRAINING_RESULT){
   pp_training_claim(t,g);t->elapsed+=step;
   if(t->elapsed>=3200){if(auto_supply)supply(t,g);if(can_continue(t,g))pp_training_resume(t);else pp_training_job_finish(t,g);}
  }
  if(r->elapsed_ms/60000>r->loot_checks){r->loot_checks++;if(r->loot_checks%5==0)discover(g);
   if(r->loot_checks%15==0)for(unsigned i=0;i<t->count;i++){unsigned pick=t->picks[i];if(pick<g->count&&g->pets[pick].clean<40){unsigned loss=g->pets[pick].clean<20?2:1;g->pets[pick].health=g->pets[pick].health>loss?g->pets[pick].health-loss:0;}}}
 }
 if(r->elapsed_ms/30000!=checkpoint)g->revision++;
 if(r->elapsed_ms==PP_TRAINING_DURATION_MS)pp_training_job_finish(t,g);
 else pp_training_checkpoint(t,g);
}
