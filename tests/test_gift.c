#include "game.h"
#include "legacy_game_v10.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game fresh(unsigned seed){pp_game g;pp_init(&g,seed);assert(pp_choose_starter(&g,1)==PP_OK);return g;}
static uint32_t hash(const void *data,unsigned n){const unsigned char *p=data;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;}
static void wait_gift(pp_game *g){for(unsigned i=0;i<200000 && !g->gift.pending;i++)pp_gift_scene(g,true,5);assert(g->gift.pending && pp_valid(g));}
int main(void){
 pp_game g=fresh(1);uint32_t before=pp_checksum(&g);
 pp_gift_scene(&g,false,600);pp_gift_scene(&g,true,600);pp_gift_scene(&g,true,0);assert(pp_checksum(&g)==before);
 pp_tick(&g,86400,0);assert(!g.gift.pending && !g.gift.active_seconds);
 g=fresh(4);g.pets[0].sleeping=1;before=pp_checksum(&g);pp_gift_scene(&g,true,5);assert(pp_checksum(&g)==before);
 g.pets[0].sleeping=0;g.pets[0].health=29;before=pp_checksum(&g);pp_gift_scene(&g,true,5);assert(pp_checksum(&g)==before);
 for(unsigned busy=0;busy<5;busy++){g=fresh(4);if(busy==0)g.battling=1;if(busy==1)g.gym.mode=1;if(busy==2)g.drop_active=1;if(busy==3)g.digging=1;if(busy==4)g.wild.phase=PP_WILD_PENDING;before=pp_checksum(&g);pp_gift_scene(&g,true,5);assert(pp_checksum(&g)==before);}
 g=fresh(42);for(unsigned i=0;i<119;i++)pp_gift_scene(&g,true,5);assert(!g.gift.pending && g.gift.active_seconds==595);wait_gift(&g);
 unsigned item=g.gift.item,total=pp_collected(&g);assert(total==1 && item<PP_TRAVEL_COLLECTION && pp_has(&g,item) && !pp_has(&g,PP_PURE_HEART));
 before=pp_checksum(&g);for(unsigned i=0;i<1000;i++){pp_gift_scene(&g,true,5);pp_wild_scene(&g,true);}assert(pp_checksum(&g)==before);
 pp_game saved=g;pp_tick(&g,86400,0);assert(g.gift.pending && pp_collected(&g)==total && pp_valid(&g));g=saved;
 pp_gift_ack(&g);before=pp_checksum(&g);pp_gift_ack(&g);assert(pp_checksum(&g)==before && !g.gift.pending && pp_has(&g,item));
 /* Only one missing item remains; it must be selected regardless of quality. */
 memset(g.collected,255,sizeof(g.collected));g.collected[PP_PURE_HEART/8]&=0x7f;g.collected[582/8]&=~(1u<<(582%8));wait_gift(&g);assert(g.gift.item==582 && !pp_has(&g,PP_PURE_HEART));
 pp_gift_ack(&g);before=pp_checksum(&g);for(unsigned i=0;i<1000;i++)pp_gift_scene(&g,true,5);assert(pp_checksum(&g)==before);
 /* A traveling pet is never the sender; a second awake resident can gift. */
 g=fresh(9);g.count=2;g.pets[1]=g.pets[0];g.pets[1].species=2;g.growth[1]=g.growth[0];g.traveler=0;wait_gift(&g);assert(pp_pokedex[g.gift.dex].species==2);
 /* Deterministic large sample checks a low, once-per-window probability. */
 unsigned found=0,quality[5]={0};
 for(unsigned seed=1;seed<=10000;seed++){g=fresh(seed);g.gift.active_seconds=599;pp_gift_scene(&g,true,1);if(g.gift.pending){found++;quality[pp_collectibles[g.gift.item].quality]++;}assert(pp_valid(&g));}
 assert(found>=400 && found<=600);for(unsigned q=0;q<5;q++)assert(quality[q]);printf("Gift chances: %u/10000; qualities %u/%u/%u/%u/%u\n",found,quality[0],quality[1],quality[2],quality[3],quality[4]);
 /* Frozen v10 migration preserves existing collections, pets and legendary eggs. */
 g=fresh(3);g.gym.badges=255;g.legend.eggs[2]=4;g.coins=456;g.collected[0]=1;
 pp_game_v10 old;memcpy(&old,&g,sizeof(old));old.version=10;pp_game restored;
 assert(pp_import_v10(&restored,&old,sizeof(old),hash(&old,sizeof(old))) && pp_valid(&restored));
 assert(restored.coins==456 && restored.legend.eggs[2]==4 && pp_has(&restored,0) && !restored.gift.pending && !restored.gift.active_seconds);
 before=pp_checksum(&restored);assert(!pp_import_v10(&restored,&old,sizeof(old),0) && pp_checksum(&restored)==before);
 g=fresh(2);g.gift.pending=1;assert(!pp_valid(&g));g.gift.pending=0;g.gift.active_seconds=600;assert(!pp_valid(&g));
 puts("PASS: gift visible-time 10min/5% gate, busy/sleep/health/travel exclusions, missing-only/all-collected/special exclusion, atomic persistent once-only receipt and frozen v10 migration");
}
