#ifndef PP_LEGACY_GAME_V16_H
#define PP_LEGACY_GAME_V16_H
/* Frozen published v16 layouts. Do not alter. */
#include "game.h"
typedef struct { pp_pet pet; uint8_t declined; pp_growth growth; pp_identity identity; uint8_t pats_today; } pp_stored_pet_v16;
typedef struct {
 uint64_t anchor_seconds,battle_seconds,battle_next;
 uint32_t picks[3],elapsed_ms,phase_ms,clock_ms,search_checks,rng,loot_rng,loot_checks;
 uint32_t rounds,last_xp,total_xp,coins,items[PP_SHOP_ITEMS],walk_ms,ambient_ms,battle_rng,battle_round;
 pp_stored_pet_v16 party[3];
 pp_gym_state gym;
 struct {uint16_t hp,attack,defense;uint8_t art,level;} foes[3];
 int16_t hp[4];
 uint8_t status,scene,count,phase,awarded,battling,battle_pet,speed_remainder,last_item,pending_ms;
 uint16_t last_coins;
 uint8_t start_levels[3],end_levels[3],growth_seen,evolution_seen;
} pp_training_record_v16;
typedef struct {
  uint32_t magic, version, rng, revision, coins, stage, day, tarot_day,
      tarot_card;
  uint64_t seconds, travel_end, egg_end;
  uint32_t care_remainder, travel_events, travel_reward;
  int8_t traveler;
  uint8_t count, scene, digs_today, egg_shakes;
  uint16_t inventory[PP_ITEMS], scenes;
  uint8_t collected[(PP_STORED_COLLECTION + 7) / 8];
  pp_pet pets[PP_PETS];
  /* 0 inactive, 1 legacy center, 2..25 persisted random lawn spot. */
  uint8_t drop_item, drop_target, drop_active;
  uint64_t drop_end;
  uint8_t clock_ready, digging, dig_lives, dig_depth;
  uint32_t dig_reward;
  uint8_t battling, battle_pet;
  int16_t battle_hp, enemy_hp, battle_max_hp, enemy_max_hp;
  uint64_t battle_next;
  char log[4][96];
  pp_growth growth[PP_PETS];
  uint32_t battle_round;
  uint64_t total_steps;
  uint32_t steps_today, steps_rewarded, steps_coins, steps_xp;
  /* Per individual pet slot; permanent, persisted with its collectible reward. */
  uint64_t evolution_declined;
  /* Bench pets keep identity, growth and permanent evolution choice. */
  pp_pet reserves[PP_RESERVES];
  pp_growth reserve_growth[PP_RESERVES];
  uint16_t reserve_declined;
  uint8_t reserve_count, random_scene;
  uint32_t random_scene_day, tarot_effect_day;
  pp_gym_state gym;
  uint16_t hatch_boosts;
  pp_wild_state wild;
  pp_legend_state legend;
  /* Active garden time only; pending is an already-awarded persistent receipt. */
  struct { uint32_t active_seconds; uint16_t item, dex; uint8_t pending; } gift;
  /* v11 prefix above is frozen. First 11 warehouse entries remain in place. */
  _Alignas(uint64_t) pp_identity identity[PP_PETS], reserve_identity[PP_RESERVES];
  uint8_t dex_history[(PP_DEX_COUNT+7)/8], starter_chosen;
  uint16_t balls;
  uint32_t overflow_count;
  /* v12 fixed prefix above is frozen. Counts follow each individual. */
  _Alignas(void *) uint8_t pats_today[PP_PETS], reserve_pats_today[PP_RESERVES];
  /* v13 fixed prefix above is frozen; appended independent item stock. */
  _Alignas(void *) uint16_t tree_essences;
  /* v14 prefix frozen; 1-based warehouse slot stays assigned until changed. */
  _Alignas(void *) uint32_t fishing_partner, fish_caught[PP_FISH_COUNT];
  /* v15 prefix frozen; three-hour training job and receipt. */
  pp_training_record_v16 training;
  /* Runtime allocation only: never serialize addresses/capacity. */
  pp_stored_pet_v16 *overflow;
  uint32_t overflow_capacity;
} pp_game_v16;
#endif
