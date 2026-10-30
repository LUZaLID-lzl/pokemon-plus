#include "game.h"
#include "scene_nests.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {
  pp_game game;pp_init(&game,42);
  assert(pp_choose_starter(&game,1)==PP_OK);
  for(unsigned scene=0;scene<PP_SCENES;scene++) {
    game.scene=scene;int first_x=-1,first_y=-1;
    for(unsigned egg=0;egg<100;egg++) {
      game.egg_end=game.seconds+7200u+(uint64_t)egg*60u;
      game.legend.hatching=egg%6;game.egg_shakes=egg%4;
      uint32_t before=pp_checksum(&game);int x,y;
      pp_egg_position(&game,&x,&y);
      assert(before==pp_checksum(&game));
      if(!egg){first_x=x;first_y=y;}
      assert(x==first_x && y==first_y);
      assert(x>=32 && x+32<=328 && y>=115 && y+32<258);
      if(scene==12)assert(x==155 && y==198);
      /* Cradles leave at least12px above the home labels at y253. */
      assert(pp_scene_nest_positions[scene][1]+42<=241);
      pp_game loaded=game;int lx,ly;pp_egg_position(&loaded,&lx,&ly);
      assert(lx==x && ly==y);
      loaded.scene=(scene+1)%PP_SCENES;pp_egg_position(&loaded,&lx,&ly);
      loaded.scene=scene;pp_egg_position(&loaded,&lx,&ly);assert(lx==x && ly==y);
    }
    if(argc>1 && !strcmp(argv[1],"--samples"))printf("%u %d %d\n",scene,first_x,first_y);
  }
  pp_dispose(&game);
  if(argc==1)puts("PASS: 13 fixed scene nests; 1300 ordinary/legendary eggs independent of time/shakes/RNG; saved egg and scene round-trip anchors");
}
