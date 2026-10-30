#pragma once
#include "lvgl.h"
/* One bounded canvas reused across page changes; init outside the frame loop. */
void pp_scene_view_init(void);
void pp_scene_view_blurred_set(lv_obj_t *image,const char *path);
void pp_scene_view_set(lv_obj_t *image,const char *path,unsigned scene,uint32_t elapsed);
void pp_scene_view_tick(lv_obj_t *image,unsigned scene,uint32_t elapsed);
void pp_scene_view_enabled(bool enabled);

/* Transparent native tree silhouette with colors synchronized to scenery. */
bool pp_scene_view_front_set(lv_obj_t *image,const char *mask_path);

/* Integer nearest-neighbor small landscapes reuse the existing RGB565 canvas. */
void pp_scene_view_pixel_set(lv_obj_t *image,const char *path,unsigned scale,int offset_x,int offset_y);

void pp_scene_view_pixel_blurred_set(lv_obj_t *image,const char *path,unsigned scale,int offset_x,int offset_y);

void pp_scene_view_panorama_set(lv_obj_t *image,const char *path,unsigned elapsed);
void pp_scene_view_panorama_tick(lv_obj_t *image,unsigned elapsed);

/* 360x112 travel strip: mirror-tiled in the existing page canvas, no extra heap. */
bool pp_scene_view_strip_set(lv_obj_t *image,const void *source,unsigned elapsed);
void pp_scene_view_strip_tick(lv_obj_t *image,unsigned elapsed);
