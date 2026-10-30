#include "moves.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  bool seen[PP_MOVE_CATALOG_COUNT]={false};
  for(unsigned family=0;family<PP_MOVE_FAMILY_COUNT;family++) {
    assert(pp_move_family_names[family] && *pp_move_family_names[family]);
    for(unsigned slot=0;slot<PP_MOVE_SLOTS;slot++) {
      const pp_move *m=pp_move_catalog_at(family,slot);
      assert(m && m>=pp_moves && m<pp_moves+PP_MOVE_CATALOG_COUNT);
      unsigned id=(unsigned)(m-pp_moves);
      assert(!seen[id]);seen[id]=true;
      assert(m->name && *m->name && m->type<=PP_DRAGON);
      if(family<PP_SPECIES)assert(pp_move_at(family,slot)==m);
      else {assert(!pp_move_at(family,slot));assert(id>=272);}
      if(family>=68)assert(id==272+slot*11+family-68);
      if(family==67)assert(id==268+slot);
    }
  }
  for(unsigned i=0;i<PP_MOVE_CATALOG_COUNT;i++)assert(seen[i]);
  assert(!pp_move_catalog_at(PP_MOVE_FAMILY_COUNT,0));
  assert(!pp_move_catalog_at(0,PP_MOVE_SLOTS));
  assert(!pp_move_catalog_at(UINT_MAX,UINT_MAX));
  assert(!pp_move_at(PP_SPECIES,0));
  assert(!strcmp(pp_move_catalog_at(76,1)->name,"龙卷风"));
  assert(pp_move_catalog_at(76,1)->type==PP_DRAGON);
  assert(pp_move_catalog_at(71,2)->type==PP_DARK);
  puts("PASS:316 catalog skills, unique family-slot mapping, runtime boundary, dragon/dark and invalid input");
}
