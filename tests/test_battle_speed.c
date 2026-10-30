#include "game.h"
#include <assert.h>
#include <stdio.h>

static pp_game fresh(void) {
  pp_game g;
  pp_init(&g,42);
  assert(pp_choose_starter(&g,1)==PP_OK);
  g.growth[0].level=60;
  return g;
}
static void cadence(pp_game *g) {
  /* No early hit, exact first/next deadlines, resumable mid-round save. */
  assert(g->battle_next==g->seconds+2 && !g->battle_round);
  int hp=g->battle_hp,enemy=g->enemy_hp;
  pp_tick(g,1,0);
  assert(!g->battle_round && hp==g->battle_hp && enemy==g->enemy_hp);
  pp_game saved=*g;
  pp_tick(g,1,0);pp_tick(&saved,1,0);
  assert(g->battle_round==1 && g->battle_next==g->seconds+2);
  assert(pp_checksum(g)==pp_checksum(&saved));
  pp_tick(g,1,0);assert(g->battle_round==1);
  pp_tick(g,1,0);assert(g->battle_round==2 && pp_valid(g));
}
int main(void) {
  const unsigned team[]={0,1,2};
  pp_game g=fresh();
  g.count=3;
  for(unsigned i=1;i<3;i++){g.pets[i]=g.pets[0];g.growth[i]=g.growth[0];}
  assert(pp_gym_start(&g,team)==PP_OK);
  cadence(&g);
  /* Knockout and entrance retain separate turns at the faster cadence. */
  g.enemy_hp=g.gym.enemy_hp[0]=1;g.battle_round=0;
  pp_tick(&g,2,0);assert(!g.enemy_hp && g.gym.enemy==0);
  unsigned round=g.battle_round;
  pp_tick(&g,1,0);assert(g.gym.enemy==0);
  pp_tick(&g,1,0);assert(g.gym.enemy==1 && g.battle_round==round);
  assert(g.enemy_hp==g.gym.enemy_max[1]);
  pp_tick(&g,2,0);assert(g.battle_round==round+1 && pp_valid(&g));
  g=fresh();
  g.wild=(pp_wild_state){.phase=PP_WILD_PENDING,.dex=0,.level=60,.escape_at=15};
  assert(pp_wild_start(&g,0)==PP_OK);cadence(&g);
  /* Once-only rewards, including an old saved 3s deadline. */
  g.battle_next=g.seconds+3;g.enemy_hp=1;g.battle_round=0;
  pp_tick(&g,2,0);assert(g.battling);
  pp_tick(&g,1,0);assert(!g.battling && g.wild.phase==PP_WILD_WON);
  unsigned coins=g.coins,xp=g.growth[0].xp;
  pp_tick(&g,20,0);assert(g.coins==coins && g.growth[0].xp==xp);
  for(unsigned i=0;i<PP_LEGEND_COUNT;i++) {
    g=fresh();g.gym.badges=255;
    assert(pp_legend_start(&g,i,0)==PP_OK);cadence(&g);
    g.enemy_hp=1;g.battle_round=0;
    pp_tick(&g,2,0);assert(!g.battling && g.legend.eggs[i]==1);
    pp_tick(&g,20,0);assert(g.legend.eggs[i]==1 && pp_valid(&g));
  }
  puts("PASS: 1.5x gym/wild/five legendary cadence, exact deadlines, save resume, isolated relay entrance and once-only rewards");
}
