#include "game.h"
#include "legacy_game_v9.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game fresh(unsigned seed){pp_game g;pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);g.growth[0]=(pp_growth){.level=60,.stage=2};return g;}
static void victory(pp_game *g,unsigned legend){assert(pp_legend_start(g,legend,0)==PP_OK);g->enemy_hp=1;pp_tick(g,3,0);assert(g->wild.phase==PP_WILD_WON && g->wild.outcome==PP_WILD_EGG && pp_valid(g));}
static uint32_t hash(const void *data,unsigned n){const unsigned char *p=data;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;}
int main(void){
 unsigned ids[]={144,145,146,150,151};
 for(unsigned d=1;d<=151;d++){bool expected=false;for(unsigned i=0;i<5;i++)expected|=d==ids[i];assert(pp_is_legendary(d)==expected);}
 for(unsigned mask=0;mask<255;mask=(mask<<1)|1){pp_game g=fresh(1);g.gym.badges=mask;uint32_t before=pp_checksum(&g);for(unsigned i=0;i<5;i++)assert(pp_legend_start(&g,i,0)==PP_NOT_READY);assert(pp_checksum(&g)==before && pp_valid(&g));}
 for(unsigned seed=1;seed<=1200;seed++){
  pp_game g=fresh(seed);g.inventory[5]=1;g.hatch_boosts=1;assert(pp_use(&g,5,0)==PP_OK);assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK);
  for(unsigned i=0;i<5;i++)assert(g.pets[1].species!=pp_pokedex[pp_legend_dex(i)].species);
  g.wild.next_at=PP_WILD_STEP_MARK;pp_wild_scene(&g,true);assert(!pp_is_legendary(pp_pokedex[g.wild.dex].national_id));assert(pp_valid(&g));
 }
 for(unsigned n=0;n<5;n++){
  pp_game g=fresh(n+1);g.gym.badges=255;unsigned count=g.count;uint32_t coins=g.coins;victory(&g,n);
  assert(g.count==count && g.coins==coins && g.legend.eggs[n]==1 && g.legend.defeated==(1u<<n));
  pp_game saved=g;pp_tick(&saved,86400,0);assert(saved.legend.eggs[n]==1);pp_wild_clear_result(&g);pp_wild_clear_result(&g);assert(!g.legend.active && g.legend.eggs[n]==1 && pp_valid(&g));
  assert(pp_legend_egg_use(&g,n)==PP_OK && !g.legend.eggs[n] && g.legend.hatching==n+1 && !g.egg_shakes);
  uint64_t end=g.egg_end;assert(end-g.seconds>=7200 && end-g.seconds<=14400);saved=g;assert(pp_valid(&saved));
  assert(pp_shake(&g)==PP_OK);pp_tick(&g,(unsigned)(end-g.seconds-1),0);assert(g.count==count && g.egg_end);pp_tick(&g,1,0);
  assert(g.count==count+1 && !g.egg_end && !g.legend.hatching && g.pets[count].species==pp_pokedex[pp_legend_dex(n)].species && g.growth[count].level==1 && pp_valid(&g));
  saved.hatch_boosts=1;assert(pp_use(&saved,PP_HATCH_BOOST,0)==PP_OK);assert(saved.pets[count].species==g.pets[count].species && !saved.legend.hatching && pp_valid(&saved));
  saved=g;g.pets[0].energy=100;victory(&g,n);assert(g.legend.eggs[n]==1 && g.count==saved.count);pp_wild_clear_result(&g);
  g.pets[0].energy=100;assert(pp_legend_start(&g,n,0)==PP_OK);g.battle_hp=1;pp_tick(&g,3,0);assert(g.wild.phase==PP_WILD_LOST && g.legend.eggs[n]==1 && pp_valid(&g));
  pp_wild_clear_result(&g);g.legend.eggs[n]=65535;g.pets[0].energy=100;uint32_t before=pp_checksum(&g);assert(pp_legend_start(&g,n,0)==PP_FULL && pp_checksum(&g)==before);
 }
 pp_game g=fresh(55);g.gym.badges=255;g.legend.eggs[0]=2;g.inventory[5]=1;assert(pp_use(&g,5,0)==PP_OK);uint32_t before=pp_checksum(&g);assert(pp_legend_egg_use(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.egg_end=0;
 for(unsigned i=1;i<5;i++){g.pets[i]=g.pets[0];g.growth[i]=g.growth[0];}g.count=5;for(unsigned i=0;i<11;i++){g.reserves[i]=g.pets[0];g.reserve_growth[i]=g.growth[0];}g.reserve_count=11;
 assert(pp_legend_egg_use(&g,0)==PP_OK);g.hatch_boosts=1;assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK && pp_warehouse_count(&g)==12 && g.overflow[0].pet.species==pp_pokedex[pp_legend_dex(0)].species && pp_valid(&g));pp_dispose(&g);
 g=fresh(13);g.gym.badges=127;g.coins=987;g.pets[0].species=77;g.growth[0].stage=0;pp_game_v9 old;memcpy(&old,&g,sizeof(old));old.version=9;pp_game restored;
 assert(pp_import_v9(&restored,&old,sizeof(old),hash(&old,sizeof(old))) && restored.version==PP_VERSION && restored.coins==987 && restored.pets[0].species==77 && !restored.legend.active && pp_valid(&restored));
 before=pp_checksum(&restored);assert(!pp_import_v9(&restored,&old,sizeof(old),0) && pp_checksum(&restored)==before);
 old.wild=(pp_wild_state){.phase=PP_WILD_PENDING,.dex=pp_legend_dex(0),.level=60,.escape_at=old.seconds+15};assert(pp_import_v9(&restored,&old,sizeof(old),hash(&old,sizeof(old))) && !restored.wild.phase && restored.pets[0].species==77);
 g=fresh(8);g.gym.badges=255;g.legend.hatching=6;assert(!pp_valid(&g));g.legend.hatching=1;assert(!pp_valid(&g));g.legend.hatching=0;assert(pp_legend_start(&g,3,0)==PP_OK);g.legend.active=1;assert(!pp_valid(&g));
 for(unsigned n=0;n<5;n++){g=fresh(77+n);g.gym.badges=255;assert(pp_legend_start(&g,n,0)==PP_OK);for(unsigned turn=0;turn<65&&g.battling;turn++){pp_tick(&g,3,0);assert(pp_valid(&g));}assert(!g.battling);}
 puts("PASS: Gen I 4 legendary + Mew, 1200 ordinary hatch/encounter exclusions, eight-badge lock, five real battles, exact eggs/timers/boost/reserves, no direct capture, repeat wins, atomic capacity/overflow, once-only receipt and frozen v9 migration");
}
