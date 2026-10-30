#pragma once
#include "game.h"
#include "scene_fx.h"
#define PP_HOME_CONFLICT_MIN_MS 180000u
#define PP_HOME_CONFLICT_MAX_MS 300000u
#define PP_HOME_CONFLICT_CHANCE 25u
#define PP_HOME_CONFLICT_IMPACT_MS 700u
#define PP_HOME_CONFLICT_DURATION_MS 1800u
#define PP_HOME_CONFLICT_HEALTH 3u
#define PP_HOME_CONFLICT_MOOD 5u
#define PP_HOME_CONFLICT_PARTICLES 16u
typedef enum {PP_CONFLICT_NONE,PP_CONFLICT_BEGIN,PP_CONFLICT_HIT,
 PP_CONFLICT_END,PP_CONFLICT_CANCEL} pp_conflict_event;
/* Ephemeral scene activity, outside the persisted v13 game structure. */
typedef struct {
 uint32_t rng,next_check,started;
 unsigned attacker,target,slot,type,health_loss,mood_loss;
 bool active,hit,suspended;
} pp_home_conflict;
bool pp_nature_feisty(unsigned nature);
void pp_home_conflict_init(pp_home_conflict *,uint32_t seed,uint32_t now);
void pp_home_conflict_pause(pp_home_conflict *,uint32_t now);
bool pp_home_conflict_start(pp_home_conflict *,const pp_game *,uint32_t now,
 unsigned attacker,unsigned target,unsigned slot);
/* Caller enables only on a visible, unobstructed courtyard. Mask excludes
 * pets currently touched or performing another scene interaction. */
pp_conflict_event pp_home_conflict_tick(pp_home_conflict *,pp_game *,uint32_t now,
 bool enabled,unsigned eligible_mask);
unsigned pp_home_conflict_sample(const pp_home_conflict *,uint32_t now,
 int source_x,int source_y,int target_x,int target_y,
 pp_scene_particle out[PP_HOME_CONFLICT_PARTICLES]);
