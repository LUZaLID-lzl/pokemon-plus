#pragma once
#include "game.h"
/* Frozen published v7 layout. Do not change pp_gym_state layout. */
typedef struct {
  uint32_t magic, version, rng, revision, coins, stage, day, tarot_day,
      tarot_card;
  uint64_t seconds, travel_end, egg_end;
  uint32_t care_remainder, travel_events, travel_reward;
  int8_t traveler;
  uint8_t count, scene, digs_today, egg_shakes;
  uint16_t inventory[PP_ITEMS], scenes;
  uint8_t collected[(584 + 7) / 8];
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
} pp_game_v7;
