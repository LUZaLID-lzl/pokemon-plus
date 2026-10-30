#include "game.h"
#include "hatch_fixture.h"
#include "legacy_game_v4.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static void init(void){pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);pp_tick(&g,0,20261023);}
static void card(unsigned c){g.tarot_day=g.tarot_effect_day=g.day;g.tarot_card=c;}
int main(void){
 init();g.coins=100000;
 for(unsigned i=1;i<PP_PETS+PP_RESERVES;i++)assert(pp_test_hatch(&g)==PP_OK);
 assert(g.count==5&&g.reserve_count==11&&pp_valid(&g));
 bool seen[PP_SPECIES]={0};unsigned unique=0;
 for(unsigned i=0;i<g.count;i++){if(!seen[g.pets[i].species])unique++;seen[g.pets[i].species]=true;}
 for(unsigned i=0;i<g.reserve_count;i++){if(!seen[g.reserves[i].species])unique++;seen[g.reserves[i].species]=true;}
 /* More families than capacity: all sixteen slots must be distinct, rather than
  * requiring nineteen species to fit into sixteen saved companion slots. */
 unsigned expected=PP_SPECIES<PP_PETS+PP_RESERVES?PP_SPECIES:PP_PETS+PP_RESERVES;
 assert(unique==expected);
 uint32_t sum=pp_checksum(&g);g.inventory[5]=1;sum=pp_checksum(&g);assert(pp_use(&g,5,0)==PP_OK);g.egg_end=0;
 g.pets[2].mood=20;g.growth[2].level=35;g.growth[2].xp=12;
 g.evolution_declined=1u<<2;g.collected[PP_PURE_HEART/8]|=1u<<(PP_PURE_HEART%8);
 pp_pet pet=g.pets[2];pp_growth growth=g.growth[2];
 assert(pp_replace_pet(&g,2,7)==PP_OK&&g.reserve_declined==(1u<<7));
 assert(!g.evolution_declined&&pp_valid(&g));
 assert(pp_replace_pet(&g,2,7)==PP_OK&&g.evolution_declined==(1u<<2));
 assert(!memcmp(&pet,&g.pets[2],sizeof pet)&&!memcmp(&growth,&g.growth[2],sizeof growth));
 assert(pp_travel(&g,0)==PP_OK);sum=pp_checksum(&g);
 assert(pp_replace_pet(&g,0,0)==PP_BUSY&&pp_reserve_pet(&g,1)==PP_BUSY&&pp_checksum(&g)==sum);
 assert(pp_replace_pet(&g,1,0)==PP_OK&&g.traveler==0&&pp_valid(&g));
 unsigned coins=g.coins;assert(pp_pat_all(&g)==4&&g.coins==coins+4);
 g.traveler=-1;g.reserve_count=10;g.reserve_declined=0;
 assert(pp_reserve_pet(&g,2)==PP_OK&&g.count==4&&g.reserve_count==11);
 assert(g.reserve_declined==(1u<<10)&&!g.evolution_declined);
 assert(pp_replace_pet(&g,g.count,10)==PP_OK&&g.count==5&&g.reserve_count==10);
 assert(g.evolution_declined==(1u<<4)&&pp_valid(&g));
 pp_game saved=g;assert(pp_valid(&saved)&&pp_checksum(&saved)==pp_checksum(&g));
 init();g.scenes=(1u<<2)|1;g.scene=0;assert(pp_random_scene(&g,true)==PP_OK&&g.scene==2);
 uint32_t rng=g.rng;pp_random_scene(&g,false);pp_random_scene(&g,true);
 assert(g.rng==rng&&g.scene==2);pp_tick(&g,0,20261024);assert(g.scene==0);
 pp_tick(&g,0,20261023);rng=g.rng;assert(g.day==20261024&&!g.clock_ready);
 pp_tick(&g,0,20261024);assert(g.rng==rng&&g.scene==0);
 pp_random_scene(&g,false);pp_tick(&g,0,20261025);assert(g.scene==0);
 init();pp_random_scene(&g,true);assert(g.scene==0&&g.scenes==1);
 for(unsigned c=0;c<8;c++){
   init();card(c);assert(pp_tarot_active(&g,c));
   for(unsigned b=0;b<8;b++)assert(pp_tarot_active(&g,b)==(b==c));
   pp_tick(&g,0,20261024);assert(!pp_tarot_active(&g,c));
   pp_tick(&g,0,20261023);assert(!pp_tarot_active(&g,c));
 }
 init();card(0);g.pets[0].energy=80;pp_tick(&g,600,g.day);pp_tick(&g,600,g.day);assert(g.pets[0].energy==83);
 init();card(1);pp_record_steps(&g,1000);assert(g.steps_xp==22&&g.steps_coins==30);pp_record_steps(&g,0);assert(g.steps_xp==22);
 init();card(2);g.pets[0].sleeping=1;g.pets[0].energy=20;pp_tick(&g,600,g.day);assert(g.pets[0].energy==25);
 init();g.coins=1000;pp_test_hatch(&g);pp_travel(&g,0);pp_tick(&g,PP_TRAVEL_SECONDS,g.day);card(3);
 coins=g.coins;assert(pp_travel_describe(&g).pending_coins==96);pp_claim(&g);assert(g.coins-coins==96);
 init();card(4);pp_game normal=g;normal.tarot_effect_day=0;pp_battle(&normal,0);pp_battle(&g,0);pp_tick(&g,2,g.day);pp_tick(&normal,2,normal.day);
 assert(g.enemy_hp<normal.enemy_hp);
 init();card(5);g.pets[0].mood=20;pp_pat_all(&g);assert(g.pets[0].mood==24&&g.growth[0].xp==2);
 init();card(6);g.pets[0].food=0;pp_use(&g,0,0);pp_tick(&g,4,g.day);assert(g.pets[0].food==46);
 init();card(7);pp_record_steps(&g,3500);assert(g.steps_coins==96&&g.steps_xp==60);pp_record_steps(&g,500);assert(g.steps_coins==128);
 pp_tick(&g,0,20261024);pp_record_steps(&g,1000);assert(g.steps_coins==30);
 init();g.coins=987;g.total_steps=42;card(3);g.growth[0].level=20;g.evolution_declined=1;g.collected[PP_PURE_HEART/8]|=1u<<(PP_PURE_HEART%8);
 pp_game_v4 old;memcpy(&old,&g,sizeof old);old.version=4;uint32_t hash=2166136261u;
 for(unsigned i=0;i<sizeof old;i++)hash=(hash^((unsigned char*)&old)[i])*16777619u;
 pp_game dest;assert(pp_import_v4(&dest,&old,sizeof old,hash)&&pp_valid(&dest));
 assert(dest.coins==987&&dest.total_steps==42&&dest.evolution_declined==1&&dest.growth[0].level==20);
 assert(!dest.reserve_count&&!dest.random_scene&&!dest.tarot_effect_day);
 assert(!pp_import_v4(&dest,&old,sizeof old,hash+1));
 puts("PASS: v4 migration, full bench and atomic identity/choice swap, bulk pat excludes traveler, daily scene no rerolls/locked scenes, all eight tarot effects and expiry");
}
