#pragma once
#include "game.h"
#include <assert.h>
/* Populate fixtures through the same ordinary-egg + boost flow as players. */
static inline pp_result pp_test_hatch(pp_game *g) {
  /* Catalog coverage fixtures use map 13's preserved global pool. */
  unsigned scene=g->scene;g->scene=12;
  assert(pp_buy(g,5)==PP_OK);
  assert(pp_use(g,5,0)==PP_OK);
  assert(pp_buy(g,PP_HATCH_BOOST)==PP_OK);
  pp_result result=pp_use(g,PP_HATCH_BOOST,0);g->scene=scene;return result;
}

static inline bool pp_test_ordinary_species(unsigned s) {
  for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].species==s && pp_is_legendary(pp_pokedex[d].national_id))return false;
  return true;
}
