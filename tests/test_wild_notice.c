#include "game.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
static void save(pp_game *g){unsigned n=pp_save_size(g);void *p=malloc(n);assert(pp_valid(g)&&pp_save_write(g,p,n,123));pp_game other={0};assert(pp_save_read(&other,p,n,NULL));free(p);pp_dispose(g);*g=other;}
static void visible(pp_game *g,unsigned seconds,bool active){for(unsigned i=0;i<seconds;i++){pp_tick(g,1,0);pp_wild_scene(g,active);}}
int main(void){
 pp_game g={0};pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);pp_record_steps(&g,2100);assert(g.wild_queue==3);pp_wild_scene(&g,true);assert(pp_wild_alert_remaining(&g)==180);
 unsigned dex=g.wild.dex,rng=g.rng;visible(&g,179,true);assert(pp_wild_alert_remaining(&g)==1);save(&g);visible(&g,3600,false);assert(pp_wild_alert_remaining(&g)==1&&g.wild.dex==dex&&g.rng==rng);
 g.scenes|=4;assert(pp_scene(&g,2)==PP_OK&&g.wild.dex==dex);visible(&g,1,true);assert(!g.wild.phase&&g.wild_queue==2&&pp_wild_reveal(&g)==PP_NOT_READY);
 visible(&g,60,true);assert(!g.wild.phase);visible(&g,1,true);assert(g.wild.phase==PP_WILD_PENDING);assert(pp_wild_reveal(&g)==PP_OK);save(&g);visible(&g,300,true);assert(pp_wild_opened(&g));pp_wild_dismiss(&g);assert(g.wild_queue==1);
 pp_dispose(&g);pp_init(&g,44);assert(pp_choose_starter(&g,1)==PP_OK);visible(&g,1199,true);assert(!g.wild_queue);visible(&g,1,true);assert(g.wild_queue==1&&g.wild.phase==PP_WILD_PENDING);assert(pp_wild_steps_remaining(&g)>=400);
 pp_dispose(&g);puts("PASS: visible 180 seconds, menu/sleep/reload pause, fixed individual across scenes, queue cooldown and activity fallback");
}
