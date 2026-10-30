#include "game.h"
#include "legacy_game_v12.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static uint32_t hash(const void *data,unsigned n){const uint8_t *p=data;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;}
static void init(pp_game *g){pp_init(g,812);assert(pp_choose_starter(g,1)==PP_OK);g->coins=100000;}
static void exhaust(pp_game *g,unsigned p){for(unsigned i=0;i<20;i++)assert(pp_pat(g,p)==PP_OK);assert(pp_pat_remaining(g,p)==0);uint32_t before=pp_checksum(g);for(unsigned i=0;i<50;i++)assert(pp_pat(g,p)==PP_DAILY_LIMIT&&pp_checksum(g)==before);}
static void roundtrip(pp_game *g,pp_game *out){pp_tick(g,0,g->day);unsigned n=pp_save_size(g);void *blob=malloc(n);assert(blob&&pp_save_write(g,blob,n,1790000000));int64_t wall=0;assert(pp_save_read(out,blob,n,&wall)&&wall==1790000000);assert(pp_checksum(g)==pp_checksum(out));free(blob);}
int main(void){
 pp_game g={0},loaded={0};init(&g);pp_tick(&g,0,20261026);unsigned coins=g.coins;
 for(unsigned i=0;i<19;i++)assert(pp_pat(&g,0)==PP_OK&&g.coins==coins+(i<5?i+1:5)&&g.growth[0].xp==(i<5?i+1:5)&&pp_pat_remaining(&g,0)==19-i);
 assert(pp_pat(&g,0)==PP_OK&&g.coins==coins+5&&g.growth[0].level==1&&g.growth[0].xp==5);
 uint32_t before=pp_checksum(&g);assert(pp_pat(&g,0)==PP_DAILY_LIMIT&&pp_checksum(&g)==before);
 roundtrip(&g,&loaded);assert(pp_pat(&loaded,0)==PP_DAILY_LIMIT);pp_tick(&loaded,0,0);assert(!pp_pat_remaining(&loaded,0));pp_tick(&loaded,0,20261025);assert(!pp_pat_remaining(&loaded,0));pp_tick(&loaded,0,20261026);assert(!pp_pat_remaining(&loaded,0));
 pp_tick(&loaded,0,20261027);assert(pp_pat_remaining(&loaded,0)==20&&pp_pat(&loaded,0)==PP_OK);
 /* Dates unknown on first boot do not grant a second quota when RTC appears. */
 pp_dispose(&loaded);init(&loaded);exhaust(&loaded,0);pp_tick(&loaded,0,20261026);assert(!pp_pat_remaining(&loaded,0));pp_tick(&loaded,0,20261027);assert(pp_pat_remaining(&loaded,0)==20);
 /* Distinct individuals of the same species stay independent, including overflow. */
 for(unsigned i=0;i<19;i++)assert(pp_test_hatch(&g)==PP_OK);assert(g.overflow_count>0);
 g.pets[1].species=g.pets[0].species;assert(pp_pat_remaining(&g,1)==20);assert(pp_pat(&g,1)==PP_OK);
 assert(pp_replace_pet(&g,0,pp_warehouse_count(&g)-1)==PP_OK);assert(pp_pat_remaining(&g,0)==20);
 pp_stored_pet p;assert(pp_warehouse_get(&g,pp_warehouse_count(&g)-1,&p)&&p.pats_today==20);
 assert(pp_replace_pet(&g,0,pp_warehouse_count(&g)-1)==PP_OK);assert(pp_pat_remaining(&g,0)==0);
 assert(pp_reserve_pet(&g,0)==PP_OK);assert(g.pats_today[0]==1);assert(pp_warehouse_get(&g,pp_warehouse_count(&g)-1,&p)&&p.pats_today==20);
 assert(pp_replace_pet(&g,g.count,pp_warehouse_count(&g)-1)==PP_OK);assert(g.pats_today[4]==20);
 roundtrip(&g,&loaded);assert(loaded.pats_today[4]==20);pp_tick(&g,0,20261027);
 for(unsigned i=0;i<pp_warehouse_count(&g);i++){assert(pp_warehouse_get(&g,i,&p)&&!p.pats_today);}for(unsigned i=0;i<g.count;i++)assert(pp_pat_remaining(&g,i)==20);
 /* Bulk and travel consume the same cap, and skip actual travelers. */
 exhaust(&g,0);g.pats_today[1]=19;assert(pp_travel(&g,2)==PP_OK);coins=g.coins;unsigned used=g.pats_today[2];assert(pp_pat_all(&g)==3&&g.coins==coins+2&&g.pats_today[2]==used&&g.pats_today[0]==20&&g.pats_today[1]==20);
 g.traveler=-1;g.pets[1].species=1;g.growth[1].level=16;assert(pp_evolve(&g,1)==PP_OK);assert(!pp_pat_remaining(&g,1));
 g.pats_today[3]=20;g.tarot_card=5;g.tarot_day=g.tarot_effect_day=g.day;unsigned xp=g.growth[2].xp;assert(pp_pat(&g,2)==PP_OK&&g.growth[2].xp==xp+2&&g.pats_today[2]==1);
 /* Frozen v12 fixed/overflow records migrate without losing identity/progress. */
 unsigned fixed=offsetof(pp_game_v12,overflow),stride=sizeof(pp_stored_pet_v12),n=16+fixed+g.overflow_count*stride;
 uint8_t *blob=calloc(1,n);assert(blob);memcpy(blob+4,&n,4);int64_t wall=1790000000;memcpy(blob+8,&wall,8);memcpy(blob+16,&g,fixed);uint32_t version=12;memcpy(blob+20,&version,4);
 for(unsigned i=0;i<g.overflow_count;i++){const pp_stored_pet *q=&g.overflow[i];pp_stored_pet_v12 old={.pet=q->pet,.growth=q->growth,.identity=q->identity,.declined=q->declined};memcpy(blob+16+fixed+i*stride,&old,stride);}
 uint32_t sum=hash(blob+4,n-4);memcpy(blob,&sum,4);assert(pp_save_read(&loaded,blob,n,&wall)&&loaded.version==PP_VERSION&&loaded.coins==g.coins&&loaded.overflow_count==g.overflow_count&&loaded.day==g.day);
 for(unsigned i=0;i<g.count;i++){assert(!memcmp(&g.identity[i],&loaded.identity[i],sizeof(pp_identity)));assert(loaded.pats_today[i]==0);}
 for(unsigned i=0;i<g.overflow_count;i++){assert(!memcmp(&loaded.overflow[i].identity,&g.overflow[i].identity,sizeof(pp_identity)));assert(!loaded.overflow[i].pats_today);}
 before=pp_checksum(&loaded);blob[n-1]^=1;assert(!pp_save_read(&loaded,blob,n,NULL)&&pp_checksum(&loaded)==before);free(blob);
 roundtrip(&loaded,&g);assert(pp_valid(&g));g.pats_today[0]=21;assert(!pp_valid(&g));g.pats_today[0]=0;
 pp_dispose(&g);pp_dispose(&loaded);puts("PASS: daily per-individual 20/21 cap, no state mutation after limit, XP/coins, bulk/travel, same-species/swap/overflow/evolution, RTC adoption/rollback/day reset, reload and frozen v12 migration");
}
