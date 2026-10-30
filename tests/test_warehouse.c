#include "game.h"
#include "legacy_game_v11.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
static uint32_t hash(const void *data,unsigned n){const unsigned char *p=data;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;}
static void victory(pp_game *g,unsigned dex){g->pets[0].energy=100;g->wild=(pp_wild_state){.dex=dex,.phase=PP_WILD_PENDING,.level=1,.escape_at=g->seconds+15};assert(pp_wild_start(g,0)==PP_OK);g->enemy_hp=1;pp_tick(g,3,0);assert(g->wild.outcome==PP_WILD_CAPTURE_READY&&pp_valid(g));}
int main(void){
 pp_game g;pp_init(&g,512);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=1000000;
 pp_identity first=g.identity[0];assert(pp_rename(&g,false,0,"firelong")==PP_OK);
 assert(!strcmp(pp_pet_name(&g,0),"firelong"));assert(pp_rename(&g,false,0,"123456789")==PP_INVALID);assert(pp_rename(&g,false,0,"bad\nname")==PP_INVALID);
 assert(pp_rename(&g,false,0,"\xc0\x80")==PP_INVALID);assert(pp_rename(&g,false,0,"abcdefgh")==PP_OK);
 assert(pp_rename(&g,false,0,"小火")==PP_INVALID);assert(pp_rename(&g,false,0,"Fire2")==PP_INVALID);assert(pp_rename(&g,false,0,"Fire long")==PP_INVALID);
 assert(pp_rename(&g,false,0,"")==PP_OK);assert(!strcmp(pp_pet_name(&g,0),pp_form_name(&g,0)));assert(pp_rename(&g,false,0,"abcdefgh")==PP_OK);
 uint32_t unchanged=pp_checksum(&g);
 assert(pp_rename(&g,false,0,"AbCdEfGh")==PP_INVALID && pp_checksum(&g)==unchanged);
 first=g.identity[0];
 for(unsigned n=1;n<320;n++){assert(pp_test_hatch(&g)==PP_OK);assert(pp_valid(&g));}
 assert(g.count==5&&pp_warehouse_count(&g)==315&&g.overflow_count==304);
 assert(pp_reserve_pet(&g,0)==PP_OK);pp_stored_pet p;assert(pp_warehouse_get(&g,315,&p));assert(!memcmp(&p.identity,&first,sizeof(first)));
 assert(pp_replace_pet(&g,g.count,315)==PP_OK);assert(!memcmp(&g.identity[4],&first,sizeof(first))&&g.count==5);
 assert(pp_rename(&g,true,300,"star")==PP_OK);pp_warehouse_get(&g,300,&p);assert(!strcmp(pp_stored_name(&p),"star"));
 /* Courtyard replacement preserves the independent egg and swaps the chosen individual. */
 g.inventory[5]=1;assert(pp_use(&g,5,0)==PP_OK);uint64_t egg_end=g.egg_end;unsigned shakes=g.egg_shakes;pp_identity old_id=g.identity[2];pp_stored_pet incoming;assert(pp_warehouse_get(&g,1,&incoming));
 assert(pp_replace_pet(&g,2,1)==PP_OK&&g.egg_end==egg_end&&g.egg_shakes==shakes);assert(!memcmp(&g.identity[2],&incoming.identity,sizeof(pp_identity)));pp_warehouse_get(&g,1,&p);assert(!memcmp(&p.identity,&old_id,sizeof(pp_identity)));
 g.hatch_boosts=1;assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK);assert(pp_release(&g,pp_warehouse_count(&g)-1)==PP_OK);
 /* Deep snapshots and pointer-free persistence: stable checksums; no aliasing. */
 pp_game snapshot={0};assert(pp_clone(&snapshot,&g));assert(snapshot.overflow!=g.overflow&&pp_checksum(&snapshot)==pp_checksum(&g));
 pp_tick(&g,0,g.day);unsigned bytes=pp_save_size(&g);void *blob=malloc(bytes);assert(blob&&pp_save_write(&g,blob,bytes,1790000000));
 pp_game restored;pp_init(&restored,1);int64_t wall=0;assert(pp_save_read(&restored,blob,bytes,&wall)&&wall==1790000000&&pp_checksum(&restored)==pp_checksum(&g));
 assert(pp_rename(&g,true,300,"tea")==PP_OK);pp_warehouse_get(&snapshot,300,&p);assert(!strcmp(p.identity.nickname,"star"));
 uint32_t before=pp_checksum(&restored);((unsigned char *)blob)[bytes-1]^=1;assert(!pp_save_read(&restored,blob,bytes,&wall)&&pp_checksum(&restored)==before);free(blob);
 /* Previous v12 names remain readable without accepting new non-English input. */
 snprintf(g.identity[1].nickname,PP_NAME_BYTES,"OldName");
 snprintf(g.identity[0].nickname,PP_NAME_BYTES,"小火");bytes=pp_save_size(&g);blob=malloc(bytes);assert(blob&&pp_save_write(&g,blob,bytes,1790000000));assert(pp_save_read(&restored,blob,bytes,&wall));assert(!strcmp(restored.identity[0].nickname,"小火")&&!strcmp(restored.identity[1].nickname,"OldName"));free(blob);
 /* Release shifts metadata and keeps all historical forms. */
 unsigned owned=pp_dex_count(&g);assert(pp_release(&g,300)==PP_OK&&pp_warehouse_count(&g)==314&&pp_dex_count(&g)==owned&&pp_valid(&g));
 while(pp_warehouse_count(&g))assert(pp_release(&g,0)==PP_OK);assert(pp_dex_count(&g)==owned&&pp_valid(&g));
 assert(pp_release(&g,0)==PP_INVALID);
 /* Two captures of the same family are retained independently; no ball means no mutation. */
 unsigned dex=0;for(;dex<PP_DEX_COUNT;dex++)if(pp_pokedex[dex].national_id==1)break;
 victory(&g,dex);before=pp_checksum(&g);assert(pp_wild_capture(&g)==PP_NO_BALL&&pp_checksum(&g)==before);
 assert(pp_buy(&g,PP_BALL)==PP_OK&&g.balls==1);assert(pp_wild_capture(&g)==PP_OK&&g.balls==0&&pp_warehouse_count(&g)==1);
 before=pp_checksum(&g);assert(pp_wild_capture(&g)==PP_INVALID&&pp_checksum(&g)==before);pp_wild_clear_result(&g);
 victory(&g,dex);g.balls=1;assert(pp_wild_capture(&g)==PP_OK&&pp_warehouse_count(&g)==2);pp_wild_clear_result(&g);
 /* Real storage exhaustion is atomic, retains the egg and the capture receipt. */
 while(pp_warehouse_count(&g)<11)assert(pp_test_hatch(&g)==PP_OK);
 pp_set_storage_budget(pp_save_size(&g));g.inventory[5]=1;assert(pp_use(&g,5,0)==PP_OK);g.hatch_boosts=1;
 before=pp_checksum(&g);assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_STORAGE_FULL&&pp_checksum(&g)==before);
 victory(&g,dex);g.balls=1;before=pp_checksum(&g);assert(pp_wild_capture(&g)==PP_STORAGE_FULL&&pp_checksum(&g)==before);pp_wild_clear_result(&g);
 pp_set_storage_budget(UINT_MAX);assert(pp_use(&g,PP_HATCH_BOOST,0)==PP_OK&&pp_warehouse_count(&g)==12);
 /* Free a real slot while a won capture is waiting, then retry without losing the receipt. */
 victory(&g,dex);g.balls=1;pp_set_storage_budget(pp_save_size(&g));before=pp_checksum(&g);assert(pp_wild_capture(&g)==PP_STORAGE_FULL&&pp_checksum(&g)==before);
 assert(pp_release(&g,0)==PP_OK&&g.wild.outcome==PP_WILD_CAPTURE_READY);assert(pp_wild_capture(&g)==PP_OK&&g.balls==0&&pp_valid(&g));pp_wild_clear_result(&g);pp_set_storage_budget(UINT_MAX);
 /* Every IV and nature axis affects the corresponding calculated stat. */
 g.growth[0].level=60;memset(g.identity[0].iv,0,6);g.identity[0].nature=0;unsigned low[6];for(unsigned i=0;i<6;i++)low[i]=pp_individual_stat(&g,0,i);
 memset(g.identity[0].iv,31,6);for(unsigned i=0;i<6;i++)assert(pp_individual_stat(&g,0,i)==low[i]+31);
 g.identity[0].nature=3;assert(pp_individual_stat(&g,0,1)>(low[1]+31)&&pp_individual_stat(&g,0,3)<low[3]+31);
 /* Published v11 prefix preserves every old pet/egg/receipt, stable identity after resave. */
 pp_game oldgame;pp_init(&oldgame,77);pp_choose_starter(&oldgame,4);oldgame.coins=4321;oldgame.hatch_boosts=2;
 pp_game_v11 old;memcpy(&old,&oldgame,sizeof(old));old.version=11;
 pp_game migrated={0};assert(pp_import_v11(&migrated,&old,sizeof(old),hash(&old,sizeof(old)))&&migrated.coins==4321&&migrated.hatch_boosts==2&&pp_valid(&migrated));
 pp_game_v11 check;memcpy(&check,&migrated,sizeof(check));check.version=11;assert(!memcmp(&check,&old,sizeof(old)));
 old.count=255;assert(!pp_import_v11(&migrated,&old,sizeof(old),hash(&old,sizeof(old))));
 pp_dispose(&g);pp_dispose(&restored);pp_dispose(&snapshot);pp_dispose(&oldgame);pp_dispose(&migrated);
 puts("PASS: 320 independent pets, unlimited gameplay warehouse, lowercase-only nickname and legacy UTF-8/mixed-case, metadata swaps/releases, historical dex, deep snapshots, corrupt/legacy saves, duplicate ball captures, six IV/nature stats, atomic physical storage exhaustion");
}
