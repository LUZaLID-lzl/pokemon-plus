#include "game.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(void){
 const unsigned required[]={1,3,6,10,15,22,30,40,52,66,82,100,125};
 for(unsigned n=1;n<=151;n++){
  pp_game g={0};pp_init(&g,n);assert(pp_choose_starter(&g,1)==PP_OK);memset(g.dex_history,0,sizeof(g.dex_history));
  /* Use actual Pokédex indices; starter is already included. */
  unsigned starter=0;for(;pp_pokedex[starter].national_id!=1;starter++){}g.dex_history[starter/8]|=1u<<(starter%8);
  unsigned used=1;for(unsigned d=0;used<n;d++)if(d!=starter){g.dex_history[d/8]|=1u<<(d%8);used++;}
  unsigned coins=g.coins,rng=g.rng;pp_tick(&g,0,0);assert(pp_prosperity(&g)==n);
  for(unsigned scene=0;scene<13;scene++){assert(pp_scene_required(scene)==required[scene]);assert(pp_scene(&g,scene)==(n>=required[scene]?PP_OK:PP_NOT_READY));assert(g.coins==coins&&g.rng==rng&&pp_valid(&g));}
  unsigned scenes=g.scenes;memset(g.dex_history,0,sizeof(g.dex_history));pp_tick(&g,0,0);assert(g.scenes==scenes); /* Ownership is never revoked. */
  unsigned size=pp_save_size(&g);void *blob=malloc(size);assert(blob&&pp_save_write(&g,blob,size,123));pp_game copy={0};assert(pp_save_read(&copy,blob,size,NULL)&&copy.scenes==scenes);free(blob);pp_dispose(&copy);pp_dispose(&g);
 }
 puts("PASS: all 151 prosperity levels, exact 13 thresholds, free unlock, no RNG spending, previous ownership and reload");
}
