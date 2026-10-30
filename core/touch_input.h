#pragma once
#include <stdbool.h>
#include <stdint.h>
#define PP_TOUCH_EDGES 16
#define PP_TOUCH_POLL_MS 10
#define PP_DISPLAY_SYNC_TIMEOUT_MS 25
#define PP_DISPLAY_SYNC_RETRY_MS 1000
typedef struct { int16_t x,y; bool down; } pp_touch_point;
typedef struct {
  pp_touch_point edges[PP_TOUCH_EDGES], latest, delivered;
  unsigned head,count,overflows,presses,releases;
  bool ignore_until_release;
} pp_touch_input;
/* Caller serializes sampling/read/reset; no LVGL or hardware dependencies. */
void pp_touch_reset(pp_touch_input *input);
void pp_touch_sample(pp_touch_input *input,bool down,int x,int y);
pp_touch_point pp_touch_read(pp_touch_input *input,bool *more);
