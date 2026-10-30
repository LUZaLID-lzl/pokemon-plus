#pragma once
#include "moves.h"
#include "pet_types.h"
#include <stdlib.h>

enum { PP_WAREHOUSE_DEFAULT, PP_WAREHOUSE_LEVEL, PP_WAREHOUSE_TYPE, PP_WAREHOUSE_SORT_COUNT };
typedef struct { unsigned slot, key; } pp_warehouse_sort_entry;
/* Normal, Fire, Water, Electric, Grass, Ice, Fighting, Poison, Ground,
 * Flying, Psychic, Bug, Rock, Ghost, Dragon, Dark, Steel, Fairy. */
static inline unsigned pp_warehouse_sort_key(unsigned level,unsigned art,unsigned mode) {
  static const unsigned char ranks[]={3,4,1,2,0,11,7,9,8,12,17,10,6,5,16,13,15,14};
  unsigned descending=60-(level>60?60:level);
  if(mode!=PP_WAREHOUSE_TYPE || art>=PP_DEX_COUNT)return descending;
  const int8_t *types=pp_pet_types[art];
  unsigned secondary=types[1]<0?0:1+ranks[(unsigned)types[1]];
  return (ranks[(unsigned)types[0]]*19+secondary)*61+descending;
}
static inline int pp_warehouse_sort_compare(const void *a,const void *b) {
  const pp_warehouse_sort_entry *x=a,*y=b;
  if(x->key!=y->key)return x->key<y->key?-1:1;
  return x->slot<y->slot?-1:x->slot>y->slot;
}
/* A view of the real slots, never a mutation of the save or active party.
 * Allocate only while opening a sorted page; retain only its visible slots. */
static inline bool pp_warehouse_sorted_page(const pp_game *game,unsigned mode,unsigned page,
                                           unsigned capacity,unsigned *slots) {
  unsigned stored=pp_warehouse_count(game),total=game->count+stored,start=page*capacity;
  for(unsigned i=0;i<capacity;i++)slots[i]=start+i<total?start+i:UINT32_MAX;
  if(!mode || !stored || start>=total)return true;
  if(SIZE_MAX/stored<sizeof(pp_warehouse_sort_entry))return false;
  pp_warehouse_sort_entry *entries=malloc((size_t)stored*sizeof(*entries));
  if(!entries)return false;
  for(unsigned i=0;i<stored;i++) {
    pp_stored_pet pet;pp_warehouse_get(game,i,&pet);
    entries[i]=(pp_warehouse_sort_entry){i,pp_warehouse_sort_key(pet.growth.level,pp_stored_art(&pet),mode)};
  }
  qsort(entries,stored,sizeof(*entries),pp_warehouse_sort_compare);
  for(unsigned i=0;i<capacity && start+i<total;i++)
    if(start+i>=game->count)slots[i]=game->count+entries[start+i-game->count].slot;
  free(entries);return true;
}
