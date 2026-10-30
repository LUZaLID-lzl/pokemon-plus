#ifndef PLUS_LEGACY_GAME_V1_H
#define PLUS_LEGACY_GAME_V1_H
#include "game.h"
/* Frozen 0.1–0.3 save layout; never edit fields or order. */
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
  uint8_t drop_item, drop_target, drop_active;
  uint64_t drop_end;
  uint8_t clock_ready, digging, dig_lives, dig_depth;
  uint32_t dig_reward;
  uint8_t battling, battle_pet;
  int16_t battle_hp, enemy_hp, battle_max_hp, enemy_max_hp;
  uint64_t battle_next;
  char log[4][96];
} pp_game_v1;
#endif
