#pragma once
#include "lvgl.h"
#include "intro.h"
typedef const void *(*pp_intro_frame_fn)(unsigned actor,unsigned pose,unsigned elapsed,bool left);
void pp_intro_view_open(lv_obj_t *parent,const void *scene,const void *types[3],pp_intro_frame_fn frame,void (*done)(void));
void pp_intro_view_close(void);
void pp_intro_view_tick(void);
pp_intro_state pp_intro_view_probe(void);
