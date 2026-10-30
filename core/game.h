#ifndef PLUS_GAME_H
#define PLUS_GAME_H
#include <stdbool.h>
#include <stdint.h>
#include "pokedex_roster.h"
#include "travel_catalog.h"
#include "fish_catalog.h"
#define PP_PETS 5
#define PP_SPECIES 79
#define PP_EVOLUTIONS 69
#define PP_THUNDER_STONE_PRICE 200u
#define PP_MOON_STONE_PRICE 200u
#define PP_FIRE_STONE_PRICE 200u
#define PP_LEAF_STONE_PRICE 200u
#define PP_WATER_STONE_PRICE 200u
#define PP_LINK_CORD_PRICE 200u
#define PP_RETIRED_POTION 2u
#define PP_ITEMS 6
/* Keep the published six-slot prefix; the extra bag count is appended. */
#define PP_HATCH_BOOST PP_ITEMS
#define PP_BALL (PP_ITEMS + 1)
#define PP_TREE_ESSENCE (PP_ITEMS + 2)
#define PP_SHOP_ITEMS (PP_ITEMS + 3)
#define PP_NAME_BYTES 25
#define PP_NATURES 25
#define PP_DROP_SPOTS 24
#define PP_SCENES 13
#define PP_SCENE_REWARD 12
#define PP_SCENE_REQUIRED_MASK ((1u << PP_SCENE_REWARD) - 1u)
#define PP_TRAVEL_COLLECTION 583
#define PP_PURE_HEART 583
#define PP_GYM_CROWN 584
#define PP_DEX_TOME 585
#define PP_COLLECTION 586
/* Frozen v1-v12 bitset. Achievement ownership derives from saved progress. */
#define PP_STORED_COLLECTION 584
#define PP_SPECIAL_COLLECTION (PP_COLLECTION - PP_TRAVEL_COLLECTION)
#define PP_VERSION 17
#define PP_PAT_DAILY_LIMIT 20u
#define PP_GIFT_INTERVAL 600u
#define PP_GIFT_CHANCE 5u
#define PP_LEGEND_COUNT 5
#define PP_WILD_NOTICE_SECONDS 180u
#define PP_WILD_INTERVAL_MIN 400u
#define PP_WILD_INTERVAL_MAX 700u
/* Legacy saves may still contain a partly completed 1500–2500-step walk. */
#define PP_WILD_LEGACY_INTERVAL_MAX 2500u
#define PP_WILD_NOTICE_PAUSED (UINT64_MAX - 1)
/* Tagged remaining steps in the existing v12 field; no save layout change. */
#define PP_WILD_STEP_MARK (UINT64_C(1) << 63)
#define PP_WILD_BASE_COINS 8u
#define PP_WILD_BASE_XP 30u
#define PP_WILD_EXTRA_COINS 80u
#define PP_WILD_EXTRA_XP 60u
#define PP_MAX_LEVEL 60
/* Main battles: 3s at 1.5x. Legacy training already uses this cadence. */
#define PP_BATTLE_TURN_SECONDS 2u
#define PP_GYM_TEAM 3
#define PP_GYM_COUNT 8
#define PP_RESERVES 11
#define PP_TRAVEL_SECONDS 10800u
#define PP_TRAVEL_BASE_COINS 50u
#define PP_TRAVEL_EVENT_COINS 5u
#define PP_TRAVEL_BASE_XP 120u
#define PP_TRAVEL_LEVEL_XP 24u
#define PP_EGG_MIN_SECONDS 7200u
#define PP_EGG_MAX_SECONDS 14400u
#define PP_EGG_SECONDS PP_EGG_MAX_SECONDS
typedef enum {
  PP_OK,
  PP_NO_COINS,
  PP_EMPTY,
  PP_FULL,
  PP_BUSY,
  PP_NEED_TWO,
  PP_DAILY_LIMIT,
  PP_NOT_READY,
  PP_INVALID,
  PP_EVOLUTION_LOCKED,
  PP_NEED_THREE,
  PP_NO_EGG,
  PP_NO_BALL,
  PP_STORAGE_FULL
} pp_result;
typedef struct {
  uint8_t species, food, clean, health, mood, energy, sleeping;
} pp_pet;
typedef struct {
  const char *name;
  uint16_t price;
  uint8_t food, clean, health, mood;
} pp_item;
typedef struct {
  uint8_t level, stage;
  uint16_t xp;
} pp_growth;
typedef struct {
  uint64_t badges;
  uint32_t wins, reward_coins;
  uint16_t reward_xp, reward_item;
  int16_t hp[PP_GYM_TEAM], max_hp[PP_GYM_TEAM];
  int16_t enemy_hp[PP_GYM_TEAM], enemy_max[PP_GYM_TEAM];
  /* mode: 0 inactive, 1..PP_GYM_COUNT gym index+1; persisted layout unchanged. */
  uint8_t mode, team[PP_GYM_TEAM], ally, enemy, entered, result, first_clear;
} pp_gym_state;
enum { PP_WILD_NONE, PP_WILD_PENDING, PP_WILD_BATTLE, PP_WILD_WON, PP_WILD_LOST };
enum { PP_WILD_NO_REWARD, PP_WILD_CAUGHT, PP_WILD_DUPLICATE, PP_WILD_FULL, PP_WILD_EGG, PP_WILD_CAPTURE_READY };
typedef struct {
  /* NONE: tagged remaining steps; PENDING: marker until click, then next_at=0. */
  uint64_t next_at, escape_at;
  uint16_t dex, reward_coins, reward_xp;
  uint8_t phase, level, scene, outcome;
} pp_wild_state;
typedef struct {
  uint16_t eggs[PP_LEGEND_COUNT];
  uint8_t defeated, active, hatching;
} pp_legend_state;
typedef struct { uint16_t national_id; uint8_t level; const char *name; } pp_legend_def;
extern const pp_legend_def pp_legends[PP_LEGEND_COUNT];
/* IV order: HP, Attack, Defense, Sp. Attack, Sp. Defense, Speed. */
typedef struct { uint8_t nature, iv[6]; char nickname[PP_NAME_BYTES]; } pp_identity;
typedef struct { uint32_t xp_fraction; uint16_t energy_fraction; uint8_t battles, energy_discount; } pp_pet_balance;
typedef struct { pp_pet pet; uint8_t declined; pp_growth growth; pp_identity identity; uint8_t pats_today; pp_pet_balance balance; } pp_stored_pet;
extern const char *const pp_nature_names[PP_NATURES];
#define PP_TRAINING_DURATION_MS 3600000u
/* Accept existing v16 checkpoints and receipts from the former three-hour rule. */
#define PP_TRAINING_LEGACY_DURATION_MS 10800000u
/* Pointer-free v16 job checkpoint, committed atomically with awarded inventory. */
typedef struct {
 uint64_t anchor_seconds,battle_seconds,battle_next;
 uint32_t picks[3],elapsed_ms,phase_ms,clock_ms,search_checks,rng,loot_rng,loot_checks;
 uint32_t rounds,last_xp,total_xp,coins,items[PP_SHOP_ITEMS],walk_ms,ambient_ms,battle_rng,battle_round;
 pp_stored_pet party[3];
 pp_gym_state gym;
 struct {uint16_t hp,attack,defense;uint8_t art,level;} foes[3];
 int16_t hp[4];
 uint8_t status,scene,count,phase,awarded,battling,battle_pet,speed_remainder,last_item,pending_ms;
 uint16_t last_coins;
 uint8_t start_levels[3],end_levels[3],growth_seen,evolution_seen;
} pp_training_record;

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
  pp_training_record training;
  /* v17: individual care follows roster transfers; account counters are monotonic-day quotas. */
  pp_pet_balance pet_balance[PP_PETS], reserve_balance[PP_RESERVES];
  uint32_t wild_active_seconds;
  uint16_t travel_xp_snapshot;
  uint64_t wild_observed_seconds;
  uint16_t wild_steps_left, wild_visible_left, wild_delay, fish_today;
  uint8_t wild_queue, pat_coins_today, rematches_today;
  /* Runtime allocation only: never serialize addresses/capacity. */
  pp_stored_pet *overflow;
  uint32_t overflow_capacity;
} pp_game;
static inline bool pp_training_growth_pending(const pp_game *g){
 const pp_training_record *r=&g->training;if(r->status!=2)return false;
 for(unsigned i=0;i<r->count;i++)if(r->end_levels[i]>r->start_levels[i]&&(!(r->growth_seen&(1u<<i))||!(r->evolution_seen&(1u<<i))))return true;
 return false;
}
static inline bool pp_training_busy(const pp_game *g,unsigned pick){
 if(g->training.status!=1)return false;
 for(unsigned i=0;i<g->training.count;i++)if(g->training.picks[i]==pick)return true;
 return false;
}
unsigned pp_scene_required(unsigned scene);
unsigned pp_prosperity(const pp_game *);
unsigned pp_care_hp(const pp_game *,unsigned pet);
unsigned pp_battle_xp(const pp_pet *,unsigned level,unsigned enemy_level);
unsigned pp_award_battle_xp(pp_game *,unsigned pet,unsigned level,unsigned enemy_level,unsigned defeated,unsigned count,bool win,unsigned multiplier);
void pp_spend_energy(pp_game *,unsigned pet,unsigned amount);
void pp_training_cost(pp_game *,unsigned pet,bool fainted);
unsigned pp_training_supply(pp_game *,unsigned pet);
pp_result pp_hospital_treat(pp_game *,unsigned pet);
unsigned pp_warehouse_count(const pp_game *);
/* XP for either an active slot or warehouse slot; membership stays unchanged. */
pp_result pp_roster_gain_xp(pp_game *, bool warehouse, unsigned slot, unsigned xp);
pp_result pp_fishing_partner_select(pp_game *, unsigned warehouse_slot);
void pp_fishing_partner_clear(pp_game *);
bool pp_warehouse_get(const pp_game *, unsigned, pp_stored_pet *);
const char *pp_pet_name(const pp_game *, unsigned);
const char *pp_stored_name(const pp_stored_pet *);
unsigned pp_stored_art(const pp_stored_pet *);
pp_result pp_rename(pp_game *, bool warehouse, unsigned index, const char *name);
pp_result pp_release(pp_game *, unsigned index);
pp_result pp_wild_capture(pp_game *);
/* No gameplay capacity; allocation/storage failures leave pets and inventory intact. */
void pp_dispose(pp_game *);
void pp_set_storage_budget(unsigned bytes); /* Real persistence capacity, UINT_MAX on desktop. */
/* Initialize destinations with pp_init or {0}; clone instead of copying dynamic saves by value. */
bool pp_clone(pp_game *dest, const pp_game *src);
unsigned pp_save_size(const pp_game *);
bool pp_save_write(const pp_game *, void *, unsigned bytes, int64_t wall);
bool pp_save_read(pp_game *, const void *, unsigned bytes, int64_t *wall);
bool pp_import_v11(pp_game *, const void *, unsigned bytes, uint32_t checksum);
unsigned pp_individual_stat(const pp_game *, unsigned pet, unsigned stat);

typedef struct {
  uint16_t national_id;
  uint8_t species, stage, art;
  const char *name;
} pp_dex_entry;
extern const pp_dex_entry pp_pokedex[PP_DEX_COUNT];
bool pp_dex_owned(const pp_game *, unsigned entry);
unsigned pp_dex_count(const pp_game *);
/* National number order; acquiring a form never moves its catalog position. */
void pp_dex_order(const pp_game *, unsigned order[PP_DEX_COUNT]);
#define PP_QUALITY_COUNT 6
#define PP_QUALITY_ALL PP_QUALITY_COUNT
typedef struct {
  const char *name;
  uint8_t quality, scene;
} pp_collection_entry;
typedef struct {
  const char *name;
  uint32_t color;
  uint16_t count;
  uint8_t target_percent;
} pp_quality_entry;
extern const pp_collection_entry pp_collectibles[PP_COLLECTION];
extern const pp_quality_entry pp_qualities[PP_QUALITY_COUNT];
/* ALL includes the exclusive special gift; travel randomness never includes it. */
unsigned pp_collection_total(unsigned quality);
unsigned pp_collection_owned(const pp_game *, unsigned quality);
unsigned pp_collection_id(unsigned quality, unsigned index);
extern const pp_item pp_items[PP_SHOP_ITEMS];
extern const char *const pp_pet_names[PP_SPECIES];
extern const char *const pp_scene_names[PP_SCENES];
extern const char *const pp_tarot_names[8];
void pp_init(pp_game *, uint32_t seed);
bool pp_needs_starter(const pp_game *);
pp_result pp_choose_starter(pp_game *, unsigned national_id);
typedef struct {
  uint8_t species, level, from_stage, art;
  const char *name;
} pp_evolution_template;
extern const pp_evolution_template pp_evolutions[PP_EVOLUTIONS];
unsigned pp_xp_needed(unsigned level);
unsigned pp_training_xp(unsigned level, bool win);
void pp_gain_xp(pp_game *, unsigned pet, unsigned xp);
unsigned pp_evolution_level(const pp_game *, unsigned pet);
pp_result pp_evolve(pp_game *, unsigned pet);
bool pp_evolution_locked(const pp_game *, unsigned pet);
pp_result pp_evolution_offer(const pp_game *, unsigned pet);
/* Original courtyard or warehouse pick; preview never mutates the individual. */
bool pp_roster_evolution_preview(const pp_game *,unsigned pick,pp_game *preview);
pp_result pp_roster_evolution_choose(pp_game *,unsigned pick,bool decline,unsigned branch);
pp_result pp_decline_evolution(pp_game *, unsigned pet);
/* Purchased and consumed on the selected Pikachu; no save inventory slot. */
pp_result pp_evolve_thunder_stone(pp_game *, unsigned pet);
/* Current-form evolution item (stone or link cord), atomic purchase/use. */
const char *pp_evolution_stone(const pp_game *, unsigned pet);
/* Eevee branch 1=water, 2=thunder, 3=fire; no implicit default. */
const char *pp_evolution_branch_name(unsigned branch);
pp_result pp_evolve_branch(pp_game *, unsigned pet, unsigned branch);
pp_result pp_evolve_stone(pp_game *, unsigned pet);
const char *pp_form_name(const pp_game *, unsigned pet);
unsigned pp_art_id(const pp_game *, unsigned pet);
unsigned pp_base_art(unsigned species);
void pp_record_steps(pp_game *, unsigned steps);
unsigned pp_step_coin_rate(const pp_game *);
unsigned pp_step_xp_rate(const pp_game *);
void pp_gift_scene(pp_game *, bool active, unsigned seconds);
void pp_gift_ack(pp_game *);
bool pp_import_v10(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_import_v9(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_import_v8(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_import_v7(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_import_v6(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_import_v5(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_import_v4(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_import_v3(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_import_v2(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_import_v1(pp_game *, const void *, unsigned bytes, uint32_t checksum);
bool pp_valid(const pp_game *);
void pp_tick(pp_game *, uint32_t seconds, uint32_t day);
unsigned pp_pat_remaining(const pp_game *, unsigned pet);
pp_result pp_pat(pp_game *, unsigned pet);
unsigned pp_pat_all(pp_game *);
pp_result pp_replace_pet(pp_game *, unsigned slot, unsigned reserve);
pp_result pp_reserve_pet(pp_game *, unsigned slot);
pp_result pp_random_scene(pp_game *, bool enabled);
bool pp_tarot_active(const pp_game *, unsigned card);
extern const char *const pp_tarot_effects[8];
unsigned pp_item_count(const pp_game *, unsigned item);
bool pp_item_enabled(unsigned item);
void pp_shop_retire_legacy(pp_game *);
pp_result pp_shop_draw(pp_game *, unsigned *item);
pp_result pp_buy(pp_game *, unsigned item);
pp_result pp_use(pp_game *, unsigned item, unsigned target);
/* Bag care: lowest relevant stat among courtyard pets not travelling. */
pp_result pp_use_auto(pp_game *, unsigned item);
void pp_drop_position(const pp_game *, int *x, int *y);
void pp_egg_position(const pp_game *, int *x, int *y);
pp_result pp_travel(pp_game *, unsigned pet);
unsigned pp_travel_destination(const pp_game *);
const char *pp_travel_name(unsigned destination);
typedef struct {
  bool active, ready, walking;
  unsigned remaining, events, pending_coins;
  const char *activity, *event;
} pp_travel_status;
pp_travel_status pp_travel_describe(const pp_game *);
/* Transient receipt; rewards and completion commit together, never in a view. */
typedef struct {
  unsigned pet, destination, item, coins, xp;
  pp_growth before, after;
} pp_travel_reward;
pp_result pp_claim_travel(pp_game *, pp_travel_reward *);
pp_result pp_claim(pp_game *);
unsigned pp_travel_xp(const pp_game *);
/* Scene scheduler is called only while the home scene is visible and awake. */
void pp_wild_scene(pp_game *, bool active);
/* next_at is the reveal deadline while pending; old v9 pending saves have it in the past. */
unsigned pp_wild_alert_remaining(const pp_game *);
unsigned pp_wild_steps_remaining(const pp_game *);
void pp_wild_screen(pp_game *, bool screen_on);
pp_result pp_wild_reveal(pp_game *);
bool pp_habitat_has(unsigned scene, unsigned species);
bool pp_wild_opened(const pp_game *);
bool pp_wild_eligible(const pp_game *, unsigned pet);
/* Highest-level eligible courtyard partner; ties keep roster order, PP_PETS if none. */
unsigned pp_wild_default_pet(const pp_game *);
pp_result pp_wild_start(pp_game *, unsigned pet);
void pp_wild_dismiss(pp_game *);
void pp_wild_clear_result(pp_game *);
bool pp_is_legendary(unsigned national_id); /* Includes Gen I mythical Mew. */
unsigned pp_legend_dex(unsigned legend);
bool pp_legend_unlocked(const pp_game *);
pp_result pp_legend_start(pp_game *, unsigned legend, unsigned pet);
pp_result pp_legend_egg_use(pp_game *, unsigned legend);
pp_result pp_battle(pp_game *, unsigned pet); /* Legacy renderer/test compatibility. */
typedef struct { const char *name; uint8_t level, art; uint16_t hp, attack, defense; } pp_gym_enemy;
typedef struct {
  const char *title, *city, *leader, *type, *badge;
  const char *leader_art, *badge_art, *display_art, *map_art, *arena_art;
  const pp_gym_enemy *team;
  uint16_t first_coins, rematch_coins, win_xp, loss_xp;
  uint8_t rare_percent, epic_percent;
} pp_gym_def;
/* One transient encounter uses a separate runtime game, never a saved mode. */
bool pp_battle_encounter(const pp_game *);
unsigned pp_battle_team_count(const pp_game *);
pp_result pp_battle_encounter_start(pp_game *, const pp_gym_def *, unsigned count);
void pp_battle_encounter_close(pp_game *);
extern const pp_gym_enemy pp_pewter_team[PP_GYM_TEAM];
extern const pp_gym_def pp_gyms[PP_GYM_COUNT];
const pp_gym_def *pp_gym_current(const pp_game *);
unsigned pp_gym_recommended_level(unsigned gym);
bool pp_gym_unlocked(const pp_game *, unsigned gym);
pp_result pp_gym_start_at(pp_game *, unsigned gym, const unsigned team[PP_GYM_TEAM]);
pp_result pp_gym_start(pp_game *, const unsigned team[PP_GYM_TEAM]);
unsigned pp_gym_hp(const pp_game *, unsigned pet);
unsigned pp_gym_effectiveness(const pp_game *, bool enemy, unsigned slot);
unsigned pp_gym_damage(const pp_game *, bool enemy, unsigned slot, unsigned variance);
void pp_gym_leave(pp_game *); /* A running challenge continues when navigating away. */
pp_result pp_dig_start(pp_game *);
pp_result pp_shake(pp_game *);
pp_result pp_dig_stop(pp_game *);
pp_result pp_tarot(pp_game *);
pp_result pp_scene(pp_game *, unsigned scene);
unsigned pp_collected(const pp_game *);
bool pp_has(const pp_game *, unsigned id);
const char *pp_message(pp_result);
uint32_t pp_checksum(const pp_game *);
#endif
