#pragma once
#include "lvgl.h"
#include "scene_fx.h"
/* One transparent drawing object; particles and damage stay in fixed storage. */
void pp_scene_effect_view_open(lv_obj_t *parent,const lv_image_dsc_t *sprites[3][8]);
void pp_scene_effect_view_update(const pp_scene_particle *particles,unsigned count);
