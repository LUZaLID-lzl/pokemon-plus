#pragma once
#include <stdbool.h>
#include <stdint.h>
#define PP_SCENE_FX_MAX 48
#define PP_SCENE_FX_PARTS 7
#define PP_SCENE_FX_PERIOD 45000u
typedef struct {
  unsigned kind,chance,duration,count,color,speed;
} pp_scene_fx_config;
typedef enum {
  PP_FX_DOT, PP_FX_STAR, PP_FX_LEAF, PP_FX_RIPPLE,
  PP_FX_METEOR, PP_FX_CLOUD, PP_FX_RAIN, PP_FX_PETAL, PP_FX_GLOW,
  PP_FX_MAPLE, PP_FX_COMET, PP_FX_LAKE_RING,
  PP_FX_BAT_UP, PP_FX_BAT_FLAT, PP_FX_BAT_DOWN,
  PP_FX_TUMBLE_0, PP_FX_TUMBLE_1, PP_FX_TUMBLE_2, PP_FX_TUMBLE_3,
  PP_FX_TUMBLE_4, PP_FX_TUMBLE_5, PP_FX_TUMBLE_6, PP_FX_TUMBLE_7
} pp_scene_shape;
typedef struct {
  int x,y,w,h;
  unsigned color,opacity;
  pp_scene_shape shape;
  bool full_screen; /* Ambient overlays may span the circle behind the UI. */
} pp_scene_particle;
typedef struct {int x,y,w,h;unsigned opacity;} pp_scene_rect;
/* Visual-only seed: never consumes the game RNG or modifies save state. */
unsigned pp_scene_fx_sample(unsigned scene,uint32_t seed,uint32_t elapsed,bool preview,
                            pp_scene_particle out[PP_SCENE_FX_MAX]);
bool pp_scene_fx_safe(const pp_scene_particle *p);
/* Bitmap shapes return zero parts; the UI draws their preloaded sprite. */
unsigned pp_scene_fx_parts(const pp_scene_particle *p,pp_scene_rect out[PP_SCENE_FX_PARTS]);

/* Fixed page-entry allocation; no object creation during animation. */
unsigned pp_scene_fx_capacity(unsigned scene);
unsigned pp_scene_fx_part_limit(unsigned scene);
