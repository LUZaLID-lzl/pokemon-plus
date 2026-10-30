#pragma once
#include "game.h"
#include "lvgl.h"
void pp_ui_create(pp_game *game);
void pp_ui_bind_input(lv_indev_t *input);
void pp_ui_refresh(void);
void pp_ui_screen(bool on);
void pp_ui_clock(const char *clock);
void pp_ui_assets(const char *root);
void pp_ui_pat_all(void);
void pp_ui_boot_hold(void);
void pp_ui_open(unsigned page);
void pp_ui_notice(const char *message);
void pp_ui_hardware(bool assets, bool imu);
unsigned pp_ui_page(void);
unsigned pp_ui_app_page(void);
unsigned pp_ui_scene_selected(void);

bool pp_ui_evolving(void);
typedef struct {
  bool active, receipt, claimable, ready, confirmation;
  unsigned coins, xp, before_level, after_level;
  int x, y, width, height;
  unsigned motion_ms,effects;
  const void *background_source;
  char activity[128], event[128], reward[128];
} pp_travel_view_state;
pp_travel_view_state pp_ui_travel_probe(void);
typedef struct {
  bool active, ready, choice, declined, level;
  unsigned age, before_art, current_art, tint, silhouette;
} pp_evolution_view_state;
pp_evolution_view_state pp_ui_evolution_probe(void);

typedef struct {
  int x, y, bubble_x, bubble_y, drop_x, drop_y, ground_y;
  int bubble_icon,bubble_width,bubble_height;char bubble_text[96];
  unsigned speaker, pose, frame, behavior, symbol;
  unsigned drop_width,drop_height,drop_opacity,drop_age,egg_width,egg_height;
  int egg_x,egg_y;bool egg_dropping;
  bool left, bubble_visible, drop_visible, visible, sleeping, nest_front;
} pp_home_state;
pp_home_state pp_ui_home_probe(unsigned pet);
typedef struct {bool active,hit;unsigned age,attacker,target,slot,particles;} pp_home_conflict_state;
pp_home_conflict_state pp_ui_home_conflict_probe(void);
#ifndef ESP_PLATFORM
bool pp_ui_home_conflict_preview(unsigned attacker,unsigned target,unsigned slot);
#endif

unsigned pp_ui_pokedex_page(void);

/* Deterministic preview capture only; normal UI always uses chance scheduling. */
void pp_ui_scene_fx_preview(bool enabled);

unsigned pp_ui_collection_quality(void);
unsigned pp_ui_collection_page(void);

/* Separate from game save/version; mark only after completion or skip. */
void pp_ui_onboarding(bool seen);
bool pp_ui_intro_seen(void);

typedef struct { bool active, cleared, welcoming; unsigned gym; } pp_gym_map_state;
pp_gym_map_state pp_ui_gym_map_probe(void);

void pp_ui_scene_motion(bool enabled);

bool pp_ui_gift_pending(void);
bool pp_ui_wild_alert(void);
bool pp_ui_wild_pending(void);
bool pp_ui_wild_capturing(void);
unsigned pp_ui_wild_fighter(void);

/* Preview comparison: disable only particle overlays, preserving scenery motion. */
void pp_ui_scene_fx_enabled(bool enabled);

/* Presentation preferences are stored independently of the game save. */
enum { PP_UI_SHOW_TIME=1, PP_UI_SHOW_COINS=2, PP_UI_SHOW_BATTERY=4, PP_UI_DISPLAY_DEFAULT=7 };
#include "battery.h"
void pp_ui_battery(pp_battery_reading reading);
void pp_ui_display_load(unsigned flags);
unsigned pp_ui_display_options(void);
void pp_ui_auto_supply_load(unsigned enabled);
bool pp_ui_auto_supply(void);

/* Shared-renderer presentation fixtures; no rewards or saved gameplay changes. */
bool pp_ui_home_interaction(unsigned kind,unsigned first,unsigned second,int x,int y);
