#pragma once
#include "fishing.h"
#include "lvgl.h"
typedef const void *(*pp_fishing_frame_cb)(unsigned art,unsigned pose,uint32_t elapsed);
void pp_fishing_view_open(lv_obj_t *parent,pp_game *game,const char *assets,pp_fishing_frame_cb frame,void (*leave)(void));
pp_fishing pp_fishing_view_probe(void);
unsigned pp_fishing_view_mode(void);
void pp_fishing_view_screen(bool screen_on);
