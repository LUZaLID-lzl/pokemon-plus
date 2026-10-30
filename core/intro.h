#pragma once
#include <stdbool.h>
#include <stdint.h>
#define PP_INTRO_MS 11200u
#define PP_INTRO_PARTICLES 18
/* Read-only choreography: no game state, RNG or ownership changes. */
typedef struct { int x,y; unsigned pose,elapsed,opacity; bool left; } pp_intro_actor;
typedef struct { int x,y,size; unsigned type,opacity; } pp_intro_particle;
typedef struct {
  pp_intro_actor pets[3];
  pp_intro_particle particles[PP_INTRO_PARTICLES];
  unsigned fade,icons,caption,title;
  bool finished;
} pp_intro_state;
pp_intro_state pp_intro_sample(uint32_t age);
