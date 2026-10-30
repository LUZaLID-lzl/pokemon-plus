#include "game.h"
#include "legacy_game_v8.h"
#include "moves.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static pp_game fresh(unsigned seed) {pp_game g;pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);return g;}
static unsigned dex(unsigned art) {for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].art==art)return d;assert(0);return 0;}
static void encounter(pp_game *g,unsigned art,unsigned level) {g->wild=(pp_wild_state){.phase=PP_WILD_PENDING,.dex=dex(art),.level=level,.scene=g->scene,.escape_at=g->seconds+15};assert(pp_valid(g));}
static void win(pp_game *g) {assert(pp_wild_start(g,0)==PP_OK);g->enemy_hp=1;pp_tick(g,3,0);assert(g->wild.phase==PP_WILD_WON && pp_valid(g));g->balls=1;assert(pp_wild_capture(g)==PP_OK);}
static uint32_t hash(const void *data,unsigned n) {const unsigned char *p=data;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;}
int main(void) {
 {pp_game pick=fresh(42);pick.count=5;
  for(unsigned i=1;i<5;i++){pick.pets[i]=pick.pets[0];pick.growth[i]=pick.growth[0];}
  const unsigned levels[]={10,30,30,50,60};for(unsigned i=0;i<5;i++)pick.growth[i].level=levels[i];
  pick.pets[4].sleeping=1;pick.traveler=3;assert(pp_wild_default_pet(&pick)==1);
  pick.pets[1].energy=2;assert(pp_wild_default_pet(&pick)==2);
  pick.pets[2].health=0;assert(pp_wild_default_pet(&pick)==0);
  pick.pets[0].sleeping=1;assert(pp_wild_default_pet(&pick)==PP_PETS);
  pick.count=0;assert(pp_wild_default_pet(&pick)==PP_PETS);
 }

 pp_game g=fresh(42);uint32_t before=pp_checksum(&g);pp_wild_scene(&g,false);assert(pp_checksum(&g)==before);
 pp_wild_scene(&g,true);unsigned target=pp_wild_steps_remaining(&g);assert(target>=400 && target<=700 && !g.wild.phase);
 pp_tick(&g,3600,0);pp_wild_scene(&g,true);assert(!g.wild.phase && pp_wild_steps_remaining(&g)==target);
 g.pets[0].sleeping=0;g.pets[0].energy=85;
 pp_record_steps(&g,target);pp_wild_scene(&g,true);assert(g.wild.phase==PP_WILD_PENDING && !pp_wild_opened(&g) && !g.battling);
 assert(pp_wild_alert_remaining(&g)==180);pp_game saved=g;assert(pp_valid(&saved));
 pp_game corrupt=g;corrupt.wild.escape_at=g.seconds+181;assert(!pp_valid(&corrupt));
 before=pp_checksum(&g);assert(pp_wild_start(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
 for(unsigned n=0;n<179;n++){pp_tick(&g,1,0);pp_wild_scene(&g,true);}assert(pp_wild_alert_remaining(&g)==1 && !pp_wild_opened(&g));
 assert(pp_wild_reveal(&g)==PP_OK && pp_wild_opened(&g));assert(pp_wild_reveal(&g)==PP_NOT_READY);
 pp_tick(&g,14,0);assert(pp_wild_opened(&g) && !g.battling);uint32_t coins=g.coins;unsigned count=g.count;
 pp_tick(&g,3600,0);assert(pp_wild_opened(&g) && !g.battling && g.coins==coins && g.count==count && pp_valid(&g));pp_wild_dismiss(&g);
 assert(pp_wild_start(&g,0)==PP_NOT_READY);assert(pp_wild_steps_remaining(&g)>=400);
 g=fresh(3);
 encounter(&g,2,16);g.wild.next_at=PP_WILD_STEP_MARK;g.wild.escape_at=g.seconds;g.wild_visible_left=0;assert(pp_wild_reveal(&g)==PP_NOT_READY && !g.battling);
 encounter(&g,3,16);g.pets[0].sleeping=1;before=pp_checksum(&g);assert(pp_wild_start(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);g.pets[0].sleeping=0;
 g.pets[0].energy=2;assert(pp_wild_start(&g,0)==PP_NOT_READY);g.pets[0].energy=85;
 /* Evolved exact form captured, actual family moves, only XP for fighter. */
 encounter(&g,6,16);assert(pp_wild_start(&g,0)==PP_OK);assert(pp_battle_move(&g,1,0)==pp_move_at(pp_pokedex[dex(6)].species,0));
 /* Same family counts even when the encounter has a different evolution. */
 g.enemy_hp=1;pp_tick(&g,3,0);assert(g.wild.outcome==PP_WILD_CAPTURE_READY && g.count==1 && g.wild.reward_coins==8 && g.wild.reward_xp==4 && pp_valid(&g));
 g.balls=1;assert(pp_wild_capture(&g)==PP_OK && g.count==2 && g.balls==0);
 pp_game restored=g;coins=g.coins;pp_tick(&restored,999,0);assert(restored.coins==coins && restored.wild.outcome==PP_WILD_CAUGHT);pp_wild_clear_result(&restored);pp_wild_clear_result(&restored);assert(restored.coins==coins && pp_valid(&restored));
 g=fresh(11);encounter(&g,9,36);assert(pp_wild_start(&g,0)==PP_OK);assert(pp_battle_move(&g,1,0)==pp_move_at(3,0));g.enemy_hp=1;pp_tick(&g,3,0);assert(pp_valid(&g));g.balls=1;assert(pp_wild_capture(&g)==PP_OK);assert(g.wild.outcome==PP_WILD_CAUGHT && g.count==2 && pp_art_id(&g,1)==9 && g.growth[1].level==36 && g.wild.reward_coins==8 && g.wild.reward_xp==4);
 /* No room: do not replace identities; egg reserves the last slot. */
 g=fresh(7);for(unsigned n=1;n<5;n++){g.pets[n]=g.pets[0];g.growth[n]=g.growth[0];}g.count=5;
 for(unsigned n=0;n<10;n++){g.reserves[n]=g.pets[0];g.reserve_growth[n]=g.growth[0];}g.reserve_count=10;g.egg_end=10000;
 pp_pet pets[5];memcpy(pets,g.pets,sizeof(pets));encounter(&g,9,36);win(&g);assert(g.wild.outcome==PP_WILD_CAUGHT && g.reserve_count==11 && g.count==5 && g.pets[1].species==pets[1].species);
 /* Free reserve slot gets the exact caught form. */
 g=fresh(7);for(unsigned n=1;n<5;n++){g.pets[n]=g.pets[0];g.growth[n]=g.growth[0];}g.count=5;encounter(&g,9,36);win(&g);assert(g.wild.outcome==PP_WILD_CAUGHT && g.reserve_count==1 && g.reserves[0].species==3 && g.reserve_growth[0].stage==2);
 pp_wild_clear_result(&g);encounter(&g,8,16);win(&g);assert(g.wild.outcome==PP_WILD_CAUGHT && g.reserve_count==2); /* reserve family's lower form */
 g=fresh(42);encounter(&g,9,36);assert(pp_wild_start(&g,0)==PP_OK);g.battle_hp=1;pp_tick(&g,3,0);assert(g.wild.phase==PP_WILD_LOST && g.coins==240 && g.count==1 && pp_valid(&g));
 /* Existing published save prefix stays byte-for-byte intact. */
 g=fresh(5);g.hatch_boosts=7;g.gym.badges=255;pp_game_v8 old;memcpy(&old,&g,sizeof(old));old.version=8;
 assert(pp_import_v8(&restored,&old,sizeof(old),hash(&old,sizeof(old))) && restored.hatch_boosts==7 && restored.gym.badges==255 && !restored.wild.phase && !restored.wild.next_at && pp_valid(&restored));
 before=pp_checksum(&restored);assert(!pp_import_v8(&restored,&old,sizeof(old),0) && pp_checksum(&restored)==before);
 encounter(&restored,9,36);pp_game invalid=restored;invalid.wild.dex=PP_DEX_COUNT;assert(!pp_valid(&invalid));invalid=restored;invalid.wild.level=PP_MAX_LEVEL+1;assert(!pp_valid(&invalid));invalid=restored;invalid.wild.phase=PP_WILD_BATTLE;assert(!pp_valid(&invalid));
 /* Hundreds of natural encounters: legal forms/levels, scene-only/no starter, battle result idempotence. */
 for(unsigned seed=1;seed<=400;seed++) {g=fresh(seed);g.growth[0].level=1+(seed%60);g.wild_queue=1;pp_wild_scene(&g,true);assert(g.wild.phase==PP_WILD_PENDING && g.wild.level<=g.growth[0].level && pp_valid(&g));
 assert(pp_wild_alert_remaining(&g)==180);assert(pp_wild_start(&g,0)==PP_NOT_READY);
 assert(pp_wild_reveal(&g)==PP_OK);assert(pp_wild_opened(&g));assert(pp_wild_start(&g,0)==PP_OK);for(unsigned n=0;n<65 && g.battling;n++){pp_tick(&g,3,0);assert(pp_valid(&g));}assert(!g.battling);coins=g.coins;count=g.count;pp_tick(&g,86400,0);assert(g.coins==coins && g.count==count && pp_valid(&g));}
 pp_init(&g,1);pp_wild_scene(&g,true);assert(!g.wild.next_at);
 g=fresh(2);g.coins=UINT_MAX-1;encounter(&g,1,1);win(&g);assert(g.coins==UINT_MAX && pp_valid(&g));
 puts("PASS: wild scene scheduling, 60s unclicked expiration and unlimited opened confirmation, explicit consent, 400 legal encounters, repeat family captures, ball settlement, concurrent egg, defeat, idempotent grants, overflow, frozen v8 migration and malformed saves");
}
