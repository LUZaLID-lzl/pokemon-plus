#include "game.h"
#include "moves.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pp_game g;
static void init(unsigned species,unsigned stage,unsigned level) {
  pp_init(&g,42);pp_choose_starter(&g,1);g.pets[0].species=species;
  g.growth[0].stage=stage;g.growth[0].level=level;g.coins=500;assert(pp_valid(&g));
}
int main(void) {
  const unsigned species[]={14,15,17},stages[]={0,0,1},levels[]={1,1,21},arts[]={37,39,44};
  const char *names[]={"火之石","月之石","叶之石"};
  for(unsigned i=0;i<3;i++) {
    init(species[i],stages[i],levels[i]);assert(!strcmp(pp_evolution_stone(&g,0),names[i]));
    assert(pp_evolve(&g,0)==PP_INVALID && pp_evolve_thunder_stone(&g,0)==PP_INVALID);
    g.coins=199;uint32_t before=pp_checksum(&g);
    assert(pp_evolve_stone(&g,0)==PP_NO_COINS && pp_checksum(&g)==before);
    g.coins=200;g.traveler=0;before=pp_checksum(&g);
    assert(pp_evolve_stone(&g,0)==PP_BUSY && pp_checksum(&g)==before);g.traveler=-1;
    uint16_t stock[PP_ITEMS];memcpy(stock,g.inventory,sizeof(stock));
    assert(pp_evolve_stone(&g,0)==PP_OK && !g.coins && pp_art_id(&g,0)==arts[i] && pp_valid(&g));
    assert(!memcmp(stock,g.inventory,sizeof(stock)) && strstr(g.log[3],names[i]));
    before=pp_checksum(&g);assert(pp_evolve_stone(&g,0)==PP_INVALID && pp_checksum(&g)==before);
    pp_game saved=g;assert(pp_valid(&saved));
    init(species[i],stages[i],levels[i]);g.coins=0;
    assert(pp_evolution_offer(&g,0)==PP_OK && pp_decline_evolution(&g,0)==PP_OK && pp_has(&g,PP_PURE_HEART));
    unsigned art=pp_art_id(&g,0);g.coins=1000;before=pp_checksum(&g);
    assert(pp_evolve_stone(&g,0)==PP_EVOLUTION_LOCKED && pp_art_id(&g,0)==art && pp_checksum(&g)==before);
    saved=g;assert(pp_valid(&saved) && pp_evolution_locked(&saved,0));
  }
  const unsigned families[]={16,17},gates[]={22,21},dest[]={41,43};
  for(unsigned i=0;i<2;i++) {
    init(families[i],0,gates[i]-1);uint32_t before=pp_checksum(&g);
    assert(pp_evolve(&g,0)==PP_NOT_READY && pp_checksum(&g)==before);
    g.growth[0].level++;assert(pp_evolve(&g,0)==PP_OK && pp_art_id(&g,0)==dest[i] && pp_valid(&g));
    if(i){assert(!strcmp(pp_evolution_stone(&g,0),"叶之石"));assert(pp_evolve_stone(&g,0)==PP_OK && pp_art_id(&g,0)==44 && pp_valid(&g));g.growth[0].level=20;assert(!pp_valid(&g));}
    else assert(pp_evolve(&g,0)==PP_INVALID);
  }
  init(29,1,PP_MAX_LEVEL);uint32_t before=pp_checksum(&g);
  assert(pp_art_id(&g,0)==72 && !pp_evolution_stone(&g,0) && pp_evolve(&g,0)==PP_INVALID && pp_checksum(&g)==before);
  g.growth[0].stage=2;assert(!pp_valid(&g));
  for(unsigned s=14;s<19;s++)for(unsigned slot=0;slot<4;slot++)assert(pp_move_at(s,slot)==&pp_moves[56+slot*5+s-14]);
  puts("PASS: fire/moon/leaf stone atomic cost, busy/final/permanent-refusal guards; bat/plant level gates; valid Gloom/Vileplume history and Tentacruel final-form guard; 20 new move mappings");
}
