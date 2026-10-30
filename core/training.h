#pragma once
#include "game.h"
#include <stddef.h>
#define PP_TRAINING_SCENES 6
#define PP_TRAINING_POOL 32
#define PP_TRAINING_WAREHOUSE PP_PETS
/* Picks 0..4 are courtyard slots; PP_PETS+n refers to warehouse n. */
typedef struct {const char *name,*search;uint8_t min_level,max_level;uint16_t dex[PP_TRAINING_POOL];uint32_t grass,light;} pp_training_scene;
extern const pp_training_scene pp_training_scenes[PP_TRAINING_SCENES];
enum {PP_TRAINING_SELECT,PP_TRAINING_SEARCH,PP_TRAINING_FIGHT,PP_TRAINING_RESULT,PP_TRAINING_ALERT,PP_TRAINING_APPROACH};
#define PP_TRAINING_APPROACH_MS 1800u
typedef struct {
  pp_game battle;
  pp_gym_enemy foes[3];pp_gym_def arena;
  unsigned picks[3],count,scene,phase,elapsed,clock_ms,search_checks,rounds,last_xp,total_xp;
  uint32_t rng;unsigned speed_remainder;bool awarded;
} pp_training;
void pp_training_init(pp_training *,uint32_t seed);
bool pp_training_pet(const pp_game *,unsigned pick,pp_stored_pet *);
pp_result pp_training_start(pp_training *,pp_game *,unsigned scene,const unsigned *,unsigned count);
void pp_training_advance(pp_training *,pp_game *,unsigned ms);
pp_result pp_training_claim(pp_training *,pp_game *);
void pp_training_resume(pp_training *);
void pp_training_close(pp_training *);

/* Managed jobs continue independently of the visible screen. */
void pp_training_checkpoint(pp_training *,pp_game *);
bool pp_training_restore(pp_training *,pp_game *);
pp_result pp_training_job_start(pp_training *,pp_game *,unsigned,const unsigned *,unsigned);
void pp_training_job_advance(pp_training *,pp_game *,unsigned);
void pp_training_job_finish(pp_training *,pp_game *);

unsigned pp_training_pool_count(unsigned scene);
void pp_training_discovery(const pp_training_record *,char *,size_t);

void pp_training_job_advance_auto(pp_training *,pp_game *,unsigned ms,bool auto_supply);
