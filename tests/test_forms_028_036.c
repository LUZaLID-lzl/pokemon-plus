#include "game.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static pp_game g;
static void init(unsigned species) {
  pp_init(&g,42);pp_choose_starter(&g,1);
  g.pets[0].species=species;g.coins=500;
}
int main(void) {
  const unsigned families[]={10,11,12};
  const unsigned levels[]={22,16,16},arts[]={27,29,32};
  for(unsigned i=0;i<3;i++) {
    init(families[i]);g.growth[0].level=levels[i]-1;
    uint32_t before=pp_checksum(&g);
    assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
    g.growth[0].level++;g.traveler=0;before=pp_checksum(&g);
    assert(pp_evolve(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
    assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==arts[i] && pp_valid(&g));
    assert(pp_dex_count(&g)==2);
    if(i) {assert(!pp_evolution_level(&g,0));assert(!strcmp(pp_evolution_stone(&g,0),"月之石"));}
    else assert(!pp_evolution_stone(&g,0));
  }
  const unsigned stone_species[]={11,12,13},stage[]={1,1,0},dest[]={30,33,35};
  for(unsigned i=0;i<3;i++) {
    init(stone_species[i]);g.growth[0].stage=stage[i];g.growth[0].level=i==2?1:16;
    assert(!strcmp(pp_evolution_stone(&g,0),"月之石"));
    g.coins=199;uint32_t before=pp_checksum(&g);
    assert(pp_evolve_stone(&g,0)==PP_NO_COINS && pp_checksum(&g)==before);
    assert(pp_evolve_thunder_stone(&g,0)==PP_INVALID);
    assert(pp_evolve(&g,0)==PP_INVALID);
    g.coins=500;g.drop_active=1;g.drop_target=0;before=pp_checksum(&g);
    assert(pp_evolve_stone(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.drop_active=0;
    uint16_t stock[PP_ITEMS];memcpy(stock,g.inventory,sizeof(stock));
    assert(pp_evolve_stone(&g,0)==PP_OK && g.coins==300 && pp_art_id(&g,0)==dest[i]);
    assert(!memcmp(stock,g.inventory,sizeof(stock)) && pp_valid(&g));
    assert(strstr(g.log[3],"月之石"));before=pp_checksum(&g);
    assert(!pp_evolution_stone(&g,0) && pp_evolve_stone(&g,0)==PP_INVALID && pp_checksum(&g)==before);
    pp_game saved=g;assert(pp_valid(&saved) && pp_art_id(&saved,0)==dest[i]);
    if(i<2){saved.growth[0].level=15;assert(!pp_valid(&saved));}
    init(stone_species[i]);g.growth[0].stage=stage[i];g.growth[0].level=i==2?1:16;
    g.coins=0;assert(pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));
    unsigned art=pp_art_id(&g,0);g.coins=1000;before=pp_checksum(&g);
    assert(pp_evolve_stone(&g,0)==PP_EVOLUTION_LOCKED && pp_checksum(&g)==before && pp_art_id(&g,0)==art);
    /* Another individual of the same form remains eligible. */
    g.count=2;g.pets[1]=g.pets[0];g.growth[1]=g.growth[0];
    assert(pp_evolve_stone(&g,1)==PP_OK && pp_evolution_locked(&g,0) && !pp_evolution_locked(&g,1));
  }
  for(unsigned species=11;species<PP_SPECIES;species++)for(unsigned slot=0;slot<4;slot++) {
    const pp_move *m=pp_move_at(species,slot);assert(m && m>=pp_moves+44 && m<pp_moves+PP_MOVE_COUNT);
  }
  puts("PASS: 027/029/032 level gates, all moon-stone stages, atomic costs/activity/final guards, valid saves/history, per-individual permanent refusal and original inventory layout");
}
