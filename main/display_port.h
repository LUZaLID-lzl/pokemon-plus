#pragma once
#include "lvgl.h"
/* Uses only the audited board BSP for LCD/touch setup and timing. */
lv_display_t *pp_display_start(lv_indev_t **input);
void pp_display_input_enable(bool enabled);
void pp_display_log_stats(void);
