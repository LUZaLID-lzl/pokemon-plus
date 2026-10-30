#pragma once
#include "training.h"
#include "lvgl.h"
/* Travel cadence; simulation checkpoints remain fixed at 40ms. */
#define PP_TRAINING_VIEW_FRAME_MS 33u
#define PP_TRAINING_BACK_X 158
#define PP_TRAINING_EXIT_X 208
#define PP_TRAINING_NAV_Y 288
typedef const void *(*pp_training_frame_cb)(unsigned,unsigned,uint32_t,bool);
typedef struct {int x,y,level_x,level_y;bool left,alert;const void *source;unsigned walk_ms;} pp_training_actor;
void pp_training_view_bind(pp_game *);
void pp_training_view_open(lv_obj_t *,pp_game *,const char *,pp_training_frame_cb,void (*leave)(void));
void pp_training_view_screen(bool);
const pp_training *pp_training_view_probe(void);
unsigned pp_training_view_mode(void);
pp_training_actor pp_training_view_actor(unsigned);
