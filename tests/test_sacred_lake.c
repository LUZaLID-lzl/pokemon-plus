#include "game.h"
#include "scene_layout.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
/* Reviewed09 altar bounds, including the flared left and right roots. */
static bool altar(int x,int y) {return x>=86 && x<=274 && y>=204 && y<=253;}
int main(int argc,char **argv) {
 if(argc>1 && !strcmp(argv[1],"--samples")) {
  for(unsigned t=0;t<=48000;t+=40) {
   int x,y;bool left;pp_lake_patrol(t,&x,&y,&left);
   printf("P %u %d %d %u %u\n",t,x,y,left,pp_lake_in_front(y));
  }
  for(unsigned t=0;t<24000;t+=600)for(unsigned spot=0;spot<24;spot++) {
   int x,y;bool left;pp_lake_patrol(t,&x,&y,&left);
   int dx=pp_lake_shore_x(spot)-8,dy=124+(int)(spot/4)*16;
   for(unsigned elapsed=0;elapsed<=2400;elapsed+=40) {
    int px,py;pp_lake_approach(x,y,dx,dy,elapsed,&px,&py);
    printf("A %d %d %d %d %u %d %d\n",x,y,dx,dy,elapsed,px,py);
   }
  }
  return 0;
 }
 if(argc>1) {
  unsigned elapsed;assert(sscanf(argv[1],"%u",&elapsed)==1);
  int x,y;bool left;pp_lake_patrol(elapsed,&x,&y,&left);
  printf("%d %d %u\n",x,y,left);return 0;
 }
 pp_game game;pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);game.coins=1000;
 uint32_t before=game.coins;assert(pp_scene(&game,12)==PP_NOT_READY && game.coins==before);
 for(unsigned d=0;d<125;d++)game.dex_history[d/8]|=1u<<(d%8);assert(pp_scene(&game,12)==PP_OK);
 assert(game.coins==before && game.scenes&(1u<<12) && pp_valid(&game));
 before=game.coins;assert(pp_scene(&game,12)==PP_OK && game.coins==before);
 pp_game restored=game;assert(pp_valid(&restored) && restored.scene==12 && !strcmp(pp_scene_names[12],"神树镜湖"));
 assert(pp_scene(&game,13)==PP_INVALID);
 game.clock_ready=true;game.day=20261024;game.scenes=1|(1u<<12);game.scene=0;
 assert(pp_random_scene(&game,true)==PP_OK && game.scene==12);
 int last_x,last_y;bool initial_left;pp_lake_patrol(0,&last_x,&last_y,&initial_left);
 for(uint32_t t=0;t<=48000;t+=40) {
  int x,y;bool left;pp_lake_patrol(t,&x,&y,&left);
  assert(x>=40 && x<=272 && y>=116 && y<=214 && !altar(x+24,y+42));
  assert(x-last_x<=2 && last_x-x<=2 && y-last_y<=2 && last_y-y<=2);last_x=x;last_y=y;
 }
 unsigned behind=0,front=0;
 for(unsigned t=0;t<24000;t+=120) {
  int x,y;bool left;pp_lake_patrol(t,&x,&y,&left);
  if(pp_lake_in_front(y))front++;else behind++;
  unsigned near=pp_lake_nearest_phase(x,y);int nx,ny;pp_lake_patrol(near,&nx,&ny,&left);
  assert((nx-x)*(nx-x)+(ny-y)*(ny-y)<=4);
  for(unsigned spot=0;spot<24;spot++)for(unsigned elapsed=0;elapsed<=2400;elapsed+=40) {
    game.drop_active=spot+2;int dx,dy,px,py;pp_drop_position(&game,&dx,&dy);
    pp_lake_approach(x,y,dx-8,dy-16,elapsed,&px,&py);assert(!altar(px+24,py+42));
  }
 }
 assert(behind>20 && front>20);
 assert(!pp_lake_in_front(211) && pp_lake_in_front(212));
 for(unsigned spot=0;spot<24;spot++) {
  int x,y,ex,ey;game.drop_active=spot+2;pp_drop_position(&game,&x,&y);
  assert(x>=44 && x<=284 && y>=140 && y<=220 && !altar(x+16,y+24));
  game.egg_end=7200+spot*60;pp_egg_position(&game,&ex,&ey);assert(ex==155 && ey==198); /* Recessed into the altar,30px left of the previous root cradle. */
  for(unsigned side=0;side<2;side++)for(unsigned elapsed=0;elapsed<=2400;elapsed+=40) {
   int px,py;pp_lake_approach(side?266:44,150,x-8,y-16,elapsed,&px,&py);
   assert(!altar(px+24,py+42));
   if(elapsed==2400)assert(px==x-8 && py==y-16);
  }
 }
 puts("PASS:13th scene purchase/save/daily selection, continuous lake patrol, altar avoidance,24 shore items and fixed root egg nest");
}
