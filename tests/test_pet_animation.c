#include "pet_animation.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
  unsigned counts=0;
  for (unsigned art=0;art<PP_ART_COUNT;art++) {
    assert(pp_pet_frame_offset(art)==counts);
    counts+=pp_pet_frame_count(art);
    for (unsigned a=0;a<6;a++) {
      unsigned count=pp_pet_frame_count(art)/6, total=0;
      for(unsigned i=0;i<count;i++) {
        unsigned d=pp_frame_durations[art][a][i];
        assert(pp_animation_frame(art,a*4,total)==a*count+i);
        assert(pp_animation_frame(art,a*4,total+d-1)==a*count+i);
        total+=d;
      }
      bool once=count==8&&(a==2||a==5);
      assert(pp_animation_frame(art,a*4,total)==a*count+(once?count-1:0));
      assert(pp_animation_frame(art,a*4,total+9999)<pp_pet_frame_count(art));
    }
  }
  assert(counts==PP_TOTAL_FRAMES);
  pp_animation_clock clock={0};
  assert(pp_animation_update(&clock,1,PP_POSE_WALK,9000)==8);
  assert(pp_animation_update(&clock,1,PP_POSE_WALK,9080)==9);
  assert(pp_animation_update(&clock,1,PP_POSE_ATTACK,9090)==40);
  assert(pp_animation_update(&clock,1,PP_POSE_ATTACK,10090)==47);
  assert(pp_animation_update(&clock,1,PP_POSE_IDLE,10091)==0);
  clock=(pp_animation_clock){0};
  assert(pp_animation_update(&clock,1,PP_POSE_WALK,UINT32_MAX-39)==8);
  assert(pp_animation_update(&clock,1,PP_POSE_WALK,40)==9);
  puts("PASS: species frame bounds, timing boundaries, loop/one-shot, action transitions and clock rollover");
}
