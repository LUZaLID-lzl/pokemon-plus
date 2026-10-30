#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "balance_config.h"
/* Advance only to the next visible boundary; slow rendering never skips cells. */
static inline unsigned pp_shop_slot(unsigned slot) {
  static const unsigned grid[9]=PP_SHOP_GRID;
  return slot<9?grid[slot]:255;
}
typedef struct { unsigned slot,next;bool visible,done; } pp_shop_reel_view;
static inline pp_shop_reel_view pp_shop_reel_at(unsigned item,unsigned elapsed) {
  static const unsigned ring[8]=PP_SHOP_RING;
  unsigned target=0;while(target<7 && pp_shop_slot(ring[target])!=item)target++;
  unsigned steps=PP_SHOP_LAPS*8+target,deadline=0;
  for(unsigned step=0;step<steps;step++){
    unsigned remaining=steps-step;
    deadline+=PP_SHOP_STEP_MS+(remaining<=PP_SHOP_SLOW_STEPS?(PP_SHOP_SLOW_STEPS-remaining+1)*PP_SHOP_SLOW_STEP_MS:0);
    if(elapsed<deadline)return (pp_shop_reel_view){ring[step%8],deadline,true,false};
  }
  unsigned age=elapsed-deadline;
  if(age>=PP_SHOP_BLINK_MS*PP_SHOP_BLINK_COUNT*2)return (pp_shop_reel_view){ring[target],elapsed,true,true};
  unsigned flash=age/PP_SHOP_BLINK_MS;
  return (pp_shop_reel_view){ring[target],deadline+(flash+1)*PP_SHOP_BLINK_MS,flash%2==0,false};
}
static inline unsigned pp_shop_reel_slot(unsigned item,unsigned elapsed) {
  return pp_shop_reel_at(item,elapsed).slot;
}
static inline unsigned pp_shop_reel_advance(unsigned item,unsigned elapsed,unsigned delta) {
  pp_shop_reel_view view=pp_shop_reel_at(item,elapsed);
  if(view.done)return elapsed;
  unsigned remaining=view.next-elapsed;
  return elapsed+(delta<remaining?delta:remaining);
}
