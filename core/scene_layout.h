#pragma once
#include <stdbool.h>
#include <stdint.h>
#define PP_MIRROR_LAKE_SCENE 12u
#define PP_LAKE_FRONT_TOP 214
#define PP_LAKE_ROOT_LEFT 86
#define PP_LAKE_ROOT_RIGHT 274
/* Screen-space top-left positions for48px sprites. Back arc goes behind
 * the crown; front arc clears the roots and stays above navigation labels. */
static inline void pp_lake_patrol(uint32_t elapsed,int *x,int *y,bool *left) {
  static const int points[12][2]={{40,145},{76,116},{138,116},{200,116},
    {264,145},{272,178},{272,PP_LAKE_FRONT_TOP},{220,PP_LAKE_FRONT_TOP},
    {156,PP_LAKE_FRONT_TOP},{88,PP_LAKE_FRONT_TOP},{40,PP_LAKE_FRONT_TOP},{40,178}};
  unsigned phase=elapsed%24000u,a=phase/2000u,b=(a+1)%12,t=phase%2000u;
  *x=points[a][0]+(points[b][0]-points[a][0])*(int)t/2000;
  *y=points[a][1]+(points[b][1]-points[a][1])*(int)t/2000;
  *left=points[b][0]<points[a][0];
}
static inline bool pp_lake_in_front(int y) {return y+42>=254;}
static inline int pp_lake_shore_x(unsigned spot) {
  static const int x[4]={44,60,268,284};return x[spot%4];
}
static inline void pp_lake_approach(int from_x,int from_y,int to_x,int to_y,
                                    unsigned elapsed,int *x,int *y) {
  if(elapsed>2400)elapsed=2400;
  /* Leave a central arc sideways before changing depth, never through the altar. */
  if(from_x+24>=PP_LAKE_ROOT_LEFT-6 && from_x+24<=PP_LAKE_ROOT_RIGHT+6) {
    int shore=from_x<156?40:272;
    if(elapsed<800) {
      *x=from_x+(shore-from_x)*(int)elapsed/800;*y=from_y;return;
    }
    from_x=shore;elapsed=(elapsed-800)*2400/1600;
  }
  if((from_x<156)==(to_x<156)) {
    *x=from_x+(to_x-from_x)*(int)elapsed/2400;
    *y=from_y+(to_y-from_y)*(int)elapsed/2400;
    return;
  }
  /* Opposite shores: first go in front of the altar, then across, then up. */
  unsigned segment=elapsed==2400?2:elapsed/800,t=elapsed==2400?800:elapsed%800;
  int ax=segment<2?from_x:to_x,ay=segment?PP_LAKE_FRONT_TOP:from_y;
  int bx=segment?to_x:from_x,by=segment<2?PP_LAKE_FRONT_TOP:to_y;
  *x=ax+(bx-ax)*(int)t/800;*y=ay+(by-ay)*(int)t/800;
}

/* Rejoin after feeding/waking without restarting at the opposite shore. */
static inline unsigned pp_lake_nearest_phase(int x,int y) {
  unsigned best=0;int best_distance=1000000;
  for(unsigned phase=0;phase<24000;phase+=120) {
    int px,py;bool left;pp_lake_patrol(phase,&px,&py,&left);
    int distance=(px-x)*(px-x)+(py-y)*(py-y);
    if(distance<best_distance){best_distance=distance;best=phase;}
  }
  return best;
}
