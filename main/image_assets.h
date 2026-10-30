#pragma once
#include "lvgl.h"
#define PP_IMAGE_CENTERED_PET LV_IMAGE_FLAGS_USER1
unsigned pp_image_canvas_width(const lv_image_dsc_t *);
unsigned pp_image_canvas_height(const lv_image_dsc_t *);
uint32_t pp_image_indexed_pixel(const lv_image_dsc_t *, unsigned x, unsigned y);
typedef struct {
  unsigned file_reads, memory_hits, images;
  size_t bytes;
} pp_image_stats;
bool pp_image_bank_init(void);
/* NULL means the bank was unavailable or did not match this application. */
const char *pp_image_bank_version(void);
void pp_image_preload(const char *path);
void pp_image_set_src(lv_obj_t *object, const void *source);
/* Resolve a preloaded image without transferring ownership. */
const lv_image_dsc_t *pp_image_descriptor(const void *source);
pp_image_stats pp_image_probe(void);
