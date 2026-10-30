#include "legacy_game_v16.h"
#include "fishing.h"
#include "legacy_game_v14.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
static pp_game g,restored;
static uint32_t hash(const void *data,unsigned n){const unsigned char *p=data;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;}
int main(void){
 pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);
 assert(PP_FISH_COUNT>=27);unsigned base=0;for(unsigned i=0;i<PP_FISH_COUNT;i++)base+=pp_fish_catalog[i].weight;assert(base==1000);
 for(unsigned q=0;q<3;q++){
  unsigned counts[PP_FISH_COUNT]={0},total=pp_fishing_weight_total(q);
  for(unsigned n=0;n<total;n++)counts[pp_fishing_pick(n,q)]++;
  for(unsigned i=0;i<PP_FISH_COUNT;i++)assert(counts[i]==pp_fishing_weight(i,q));
 }
 assert((double)pp_fishing_weight(26,2)/pp_fishing_weight_total(2)>(double)pp_fishing_weight(26,0)/pp_fishing_weight_total(0));
 for(unsigned offset=0;offset<=25;offset++){
  pp_fishing f;pp_fishing_init(&f,42+offset);uint32_t before=g.coins,revision=g.revision;unsigned used=g.fish_today;
  pp_fishing_press(&f,&g,UINT32_MAX-3000u);assert(f.wait_ms>=6000&&f.wait_ms<=10000&&f.target>=30&&f.target<=70);
  uint32_t bite=f.started+f.wait_ms;pp_fishing_press(&f,&g,bite-1);assert(f.phase==PP_FISH_WAIT&&g.coins==before);
  pp_fishing_press(&f,&g,bite+(f.target+offset)*10);
  assert(f.phase==PP_FISH_RESULT&&f.caught==(offset<=18));
  unsigned base=pp_fish_catalog[f.fish].coins/6;if(base<2)base=2;unsigned expected=f.caught?base+f.quality:0;if(f.caught&&used>=15){expected/=5;if(!expected)expected=1;}
  assert(f.reward==expected&&g.coins==before+expected&&g.revision==revision+!!f.caught&&pp_valid(&g));
  if(f.caught)assert(g.fish_caught[f.fish]);
  pp_fishing_tick(&f,bite+60000);assert(g.coins==before+expected);
  pp_fishing_press(&f,&g,bite+60001);assert(f.phase==PP_FISH_WAIT&&!f.caught&&g.coins==before+expected);
  pp_fishing_tick(&f,f.started+f.wait_ms+PP_FISH_REEL_MS);assert(f.phase==PP_FISH_RESULT&&!f.caught&&!f.reward);
 }
 pp_fishing f;pp_fishing_init(&f,42);pp_fishing_press(&f,&g,0);g.coins=UINT32_MAX;uint32_t rev=g.revision;
 pp_fishing_press(&f,&g,f.wait_ms+f.target*10);assert(f.caught&&!f.reward&&g.fish_caught[f.fish]&&g.revision==rev+1);
 g.fish_caught[f.fish]=UINT32_MAX;pp_fishing_init(&f,42);pp_fishing_press(&f,&g,0);pp_fishing_press(&f,&g,f.wait_ms+f.target*10);assert(g.fish_caught[f.fish]==UINT32_MAX);
 for(unsigned n=0;n<22;n++)assert(pp_test_hatch(&g)==PP_OK);
 assert(pp_fishing_partner_select(&g,13)==PP_OK&&g.fishing_partner==14);assert(pp_rename(&g,true,13,"angler")==PP_OK);uint32_t before=pp_checksum(&g);
 assert(pp_release(&g,13)==PP_BUSY&&pp_replace_pet(&g,0,13)==PP_BUSY&&pp_checksum(&g)==before);
 assert(pp_release(&g,1)==PP_OK&&g.fishing_partner==13);pp_stored_pet partner;pp_warehouse_get(&g,12,&partner);assert(!strcmp(partner.identity.nickname,"angler"));
 assert(pp_reserve_pet(&g,0)==PP_OK&&pp_replace_pet(&g,g.count,0)==PP_OK&&g.fishing_partner==12);pp_warehouse_get(&g,11,&partner);assert(!strcmp(partner.identity.nickname,"angler"));
 g.tree_essences=7;pp_tick(&g,0,g.day);unsigned bytes=pp_save_size(&g);void *data=malloc(bytes);assert(data&&pp_save_write(&g,data,bytes,1790000000)&&pp_save_read(&restored,data,bytes,NULL));free(data);
 assert(pp_checksum(&g)==pp_checksum(&restored)&&restored.fishing_partner==12);
 /* Frozen v14 with overflow + essence migrates without inventing catches or selection. */
 unsigned fixed=offsetof(pp_game_v14,overflow),size=16+fixed+g.overflow_count*sizeof(pp_stored_pet_v16);unsigned char *old=calloc(1,size);assert(old);memcpy(old+4,&size,4);memcpy(old+16,&g,fixed);uint32_t v=14;memcpy(old+20,&v,4);for(unsigned i=0;i<g.overflow_count;i++)memcpy(old+16+fixed+i*sizeof(pp_stored_pet_v16),&g.overflow[i],sizeof(pp_stored_pet_v16));uint32_t h=hash(old+4,size-4);memcpy(old,&h,4);
 assert(pp_save_read(&restored,old,size,NULL)&&!restored.fishing_partner&&restored.tree_essences==g.tree_essences&&restored.overflow_count==g.overflow_count);for(unsigned i=0;i<PP_FISH_COUNT;i++)assert(!restored.fish_caught[i]);free(old);
 pp_fishing_partner_clear(&g);assert(!g.fishing_partner&&pp_release(&g,11)==PP_OK);before=pp_checksum(&g);assert(pp_fishing_partner_select(&g,pp_warehouse_count(&g))==PP_INVALID&&pp_checksum(&g)==before);
 pp_dispose(&g);pp_dispose(&restored);puts("PASS:27 fish exact weighted distributions/precision, timer rollover, once-only catches/coins and saturation; pinned warehouse identity/compaction/protection; v15 roundtrip and frozen v14 overflow migration");
}
