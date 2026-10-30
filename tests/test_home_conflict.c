#include "home_conflict.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pp_game g;
static void fixture(void){
 pp_dispose(&g);pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);
 g.count=3;
 for(unsigned i=0;i<3;i++){
  g.pets[i]=g.pets[0];g.growth[i]=g.growth[0];g.identity[i]=g.identity[0];
  g.pets[i].species=i;g.identity[i].nature=i?20:3;
 }
}
int main(void){
 pp_home_conflict s;
 for(unsigned n=0;n<25;n++)assert(pp_nature_feisty(n)==(n==3||n==4||n==8||n==11||n==24));
 fixture();pp_home_conflict_init(&s,42,0);assert(s.next_check>=180000&&s.next_check<=300000);
 unsigned checksum=pp_checksum(&g),rng=g.rng;
 assert(pp_home_conflict_start(&s,&g,1000,0,1,0));
 assert(pp_checksum(&g)==checksum);
 assert(pp_home_conflict_tick(&s,&g,1699,true,7)==PP_CONFLICT_NONE);
 assert(pp_checksum(&g)==checksum);
 unsigned health=g.pets[1].health,mood=g.pets[1].mood,rev=g.revision;
 assert(pp_home_conflict_tick(&s,&g,1700,true,7)==PP_CONFLICT_HIT);
 assert(g.pets[1].health==health-3&&g.pets[1].mood==mood-5&&g.revision==rev+1&&g.rng==rng);
 checksum=pp_checksum(&g);
 for(unsigned t=1700;t<2800;t++)assert(pp_home_conflict_tick(&s,&g,t,true,7)==PP_CONFLICT_NONE);
 assert(pp_checksum(&g)==checksum);assert(pp_home_conflict_tick(&s,&g,2800,true,7)==PP_CONFLICT_END);
 unsigned bytes=pp_save_size(&g);void *saved=malloc(bytes);assert(saved);int64_t wall;
 assert(pp_save_write(&g,saved,bytes,1234)&&pp_save_read(&g,saved,bytes,&wall));free(saved);
 assert(g.pets[1].health==health-3&&g.pets[1].mood==mood-5&&pp_valid(&g));
 /* No invisible hits after menu/screen transitions or a missed animation. */
 for(unsigned block=0;block<8;block++){
  fixture();pp_home_conflict_init(&s,42,0);assert(pp_home_conflict_start(&s,&g,0,0,1,0));
  if(block==1)g.pets[1].sleeping=1;
  if(block==2)g.traveler=1;
  if(block==3)g.drop_active=1;
  if(block==4)g.battling=1;
  if(block==5)g.wild.phase=PP_WILD_PENDING;
  if(block==6)g.pets[0].energy=29;
  checksum=pp_checksum(&g);
  assert(pp_home_conflict_tick(&s,&g,block==7?2000:700,block!=0,7)==(block==7?PP_CONFLICT_END:PP_CONFLICT_CANCEL));
  assert(pp_checksum(&g)==checksum&&!s.active);
 }
 fixture();g.pets[1].health=2;g.pets[1].mood=2;pp_home_conflict_init(&s,42,0);
 assert(pp_home_conflict_start(&s,&g,0,0,1,0));assert(pp_home_conflict_tick(&s,&g,700,true,7)==PP_CONFLICT_HIT);
 assert(g.pets[1].health==1&&g.pets[1].mood==0&&s.health_loss==1&&s.mood_loss==2);
 pp_home_conflict_init(&s,42,0);assert(!pp_home_conflict_start(&s,&g,0,0,1,0));
 assert(!pp_home_conflict_start(&s,&g,0,1,0,0));assert(!pp_home_conflict_start(&s,&g,0,0,0,0));
 /* Actual low-frequency scheduling, no dependence on frame rate, game RNG or
  * hidden elapsed time. Advancing straight to due checks never stacks hits. */
 unsigned attacks=0,checks=0;
 for(unsigned seed=0;seed<32;seed++){
  fixture();rng=g.rng;checksum=pp_checksum(&g);pp_home_conflict_init(&s,seed,UINT32_MAX-50000);uint32_t previous=UINT32_MAX-50000;
  for(unsigned i=0;i<64;i++){
   uint32_t now=s.next_check,span=now-previous;assert(span>=180000&&span<=300000);previous=now;
   pp_conflict_event e=pp_home_conflict_tick(&s,&g,now,true,7);checks++;
   if(e==PP_CONFLICT_BEGIN){attacks++;assert(s.attacker==0&&s.target!=0&&pp_move_at(g.pets[0].species,s.slot)->power);
    assert(pp_home_conflict_tick(&s,&g,now+1800,true,7)==PP_CONFLICT_END);}
  }
  assert(g.rng==rng&&pp_checksum(&g)==checksum);
 }
 assert(attacks>checks/6&&attacks<checks/3);
 /* Every skill type and both directions produce bounded local particles. */
 uint32_t fx_hash=2166136261u;
 for(unsigned type=0;type<18;type++)for(unsigned dir=0;dir<2;dir++)for(unsigned t=0;t<1800;t+=33){
  s=(pp_home_conflict){.active=true,.type=type};pp_scene_particle p[16];
  unsigned count=pp_home_conflict_sample(&s,t,dir?296:64,139,dir?64:296,238,p);assert(count<=16);
  for(unsigned n=0;n<count;n++){int fields[]={p[n].x,p[n].y,p[n].w,p[n].h,(int)p[n].color,(int)p[n].opacity};
   for(unsigned f=0;f<6;f++)fx_hash=(fx_hash^(uint32_t)fields[f])*16777619u;
   assert(p[n].opacity<=255&&p[n].x>=40&&p[n].x+p[n].w<=320&&p[n].y>=110&&p[n].y+p[n].h<=263);}
 }
 printf("SAMPLES %u %u %u\n",checks,attacks,fx_hash);
 pp_dispose(&g);puts("PASS: feisty natures, rare local scheduling, impact-only once, health floor, save/reload, busy/sleep/travel/wake cancellation, rollover and all18 skill particles");
}
