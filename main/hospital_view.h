#pragma once
#include "hospital.h"
#include "fishing_view.h"
void pp_hospital_view_open(lv_obj_t *parent,pp_game *game,const char *assets,pp_fishing_frame_cb frame,void (*leave)(void));
void pp_hospital_view_screen(bool on);
pp_hospital pp_hospital_view_probe(void);
