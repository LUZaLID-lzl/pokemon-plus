#pragma once
#include "game.h"
#include "lvgl.h"
/* Gym cadence; training drives this shared renderer at its own travel cadence. */
#define PP_BATTLE_VIEW_FRAME_MS 40u
typedef struct {
  bool active, casting, settled;
  unsigned species, side, particles, slot;
  int hp[2];
  unsigned art[2], alive_mask[2];
  unsigned actor[2], round, damage_side;
  int loss[2];
  bool damage_visible, immunity_visible;
  uint8_t relation_factor[2],relation_kind[2],relation_level[2];
  char impact_text[24];
  unsigned entrance_mask,entrance_ms,pet_opacity[2];
  char narration[128];
  unsigned narration_pending, narration_opacity;
  int narration_y;
} pp_battle_view_state;
void pp_battle_view_create(lv_obj_t *parent, pp_game *game, const char *root,
                           lv_obj_t *retry, lv_obj_t *back);
void pp_battle_view_close(void);
void pp_battle_view_tick(void);
void pp_battle_view_resync_clock(void);
/* Static current-form badges with names; also used by the pet information card. */
void pp_pet_type_badges_create(lv_obj_t *parent,unsigned art,int x,int y);
pp_battle_view_state pp_battle_view_probe(void);
#ifndef ESP_PLATFORM
/* Offline catalog preview only; never inserts a pet into game/save state. */
void pp_battle_view_catalog_preview(unsigned family, const char *frames_root,
                                   const uint16_t durations[6][8], unsigned once_mask, bool actor_check);
#endif
