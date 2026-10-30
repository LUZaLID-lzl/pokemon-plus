#include "warehouse_sort.h"
#include "hatch_fixture.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
 if(argc>1 && !strcmp(argv[1],"--keys")) {
  for(unsigned art=0;art<PP_DEX_COUNT;art++)for(unsigned level=1;level<=60;level++)for(unsigned mode=1;mode<=2;mode++)
   printf("%u %u %u %u\n",art,level,mode,pp_warehouse_sort_key(level,art,mode));
  return 0;
 }
 pp_game g;pp_init(&g,42);g.count=0;unsigned slots[9];
 assert(pp_warehouse_sorted_page(&g,1,0,9,slots));for(unsigned i=0;i<9;i++)assert(slots[i]==UINT32_MAX);
 pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);g.coins=1000000;
 assert(pp_warehouse_sorted_page(&g,2,0,9,slots)&&slots[0]==0&&slots[1]==UINT32_MAX);
 while(g.count+pp_warehouse_count(&g)<320)assert(pp_test_hatch(&g)==PP_OK);
 for(unsigned i=0;i<pp_warehouse_count(&g);i++){
  if(i<g.reserve_count)g.reserve_growth[i].level=1+(i*17)%60;
  else g.overflow[i-g.reserve_count].growth.level=1+(i*17)%60;
 }
 /* All five scene slots precede even level-60 stored pets. Stable ties use raw slot. */
 uint32_t checksum=pp_checksum(&g);
 for(unsigned mode=0;mode<3;mode++){
  bool seen[320]={0};unsigned previous_key=0,previous_slot=0;
  for(unsigned page=0;page<36;page++){
   assert(pp_warehouse_sorted_page(&g,mode,page,9,slots));
   for(unsigned i=0;i<9;i++){
    unsigned view=page*9+i,n=slots[i];if(view>=320){assert(n==UINT32_MAX);continue;}
    assert(n<320&&!seen[n]);seen[n]=true;
    if(view<g.count){assert(n==view);continue;}
    if(!mode){assert(n==view);continue;}
    pp_stored_pet p;assert(pp_warehouse_get(&g,n-g.count,&p));unsigned key=pp_warehouse_sort_key(p.growth.level,pp_stored_art(&p),mode);
    if(view>g.count)assert(key>previous_key||(key==previous_key&&n>previous_slot));
    previous_key=key;previous_slot=n;
   }
  }
  for(unsigned i=0;i<320;i++)assert(seen[i]);assert(pp_checksum(&g)==checksum);
 }
 /* View prefix also follows the actual active count below five. */
 assert(pp_reserve_pet(&g,4)==PP_OK);assert(pp_reserve_pet(&g,3)==PP_OK);
 assert(pp_warehouse_sorted_page(&g,1,0,9,slots));for(unsigned i=0;i<3;i++)assert(slots[i]==i);
 pp_dispose(&g);puts("PASS: native sorting 320 individuals, 36 pages, empty/single/3/5 scene pets, stable ties and checksum unchanged");
}
