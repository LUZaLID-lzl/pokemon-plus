#pragma once
#include <stdbool.h>
#include <stdint.h>
#define PP_HOME_ACTORS 5u
typedef enum {PP_HOME_NONE,PP_HOME_PAT,PP_HOME_CALL,PP_HOME_EXPLORE,PP_HOME_HOP,
 PP_HOME_GREET,PP_HOME_STROLL,PP_HOME_REST,PP_HOME_WELCOME} pp_home_behavior;
typedef struct {int x,y;unsigned mood,energy;bool available,left;} pp_home_actor;
typedef struct {bool active,release,left;int x,y,jump;unsigned pose,symbol,kind;} pp_home_output;
typedef struct {
 unsigned kind,partner;int x,y,tx,ty;bool left;
 uint32_t start,approach,duration;
} pp_home_activity;
typedef struct {
 pp_home_activity pets[PP_HOME_ACTORS];uint32_t rng,next_solo,next_pair,last_call;
 bool called;unsigned scene;
} pp_home_behavior_state;
/* Presentation-only: caller owns time, inputs and a local RNG; never touches saves. */
void pp_home_behavior_init(pp_home_behavior_state *,uint32_t seed,uint32_t now,unsigned scene);
void pp_home_behavior_cancel(pp_home_behavior_state *,unsigned pet);
void pp_home_behavior_clear(pp_home_behavior_state *);
bool pp_home_behavior_request(pp_home_behavior_state *,uint32_t now,unsigned kind,
 unsigned a,unsigned b,int x,int y,const pp_home_actor *,unsigned count);
int pp_home_behavior_call(pp_home_behavior_state *,uint32_t now,int x,int y,const pp_home_actor *,unsigned count);
void pp_home_behavior_tick(pp_home_behavior_state *,uint32_t now,const pp_home_actor *,unsigned count,
 bool automatic,pp_home_output out[PP_HOME_ACTORS]);

/* Bounded closed patrol. Reflect at the lawn edge instead of clipping the walk. */
unsigned pp_home_patrol(uint32_t elapsed,unsigned slot,bool tired,int origin_x,int origin_y,
 bool facing,int *x,int *y,bool *left);
unsigned pp_home_scene_patrol(unsigned scene,uint32_t elapsed,unsigned slot,bool tired,
 int origin_x,int origin_y,bool facing,int *x,int *y,bool *left);
