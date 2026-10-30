#include "touch_input.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
  pp_touch_input input={0};bool more;
  /* Both edges arrive while the render thread is busy: preserve a full tap. */
  pp_touch_sample(&input,true,128,300);pp_touch_sample(&input,false,0,0);
  pp_touch_point p=pp_touch_read(&input,&more);assert(p.down && p.x==128 && p.y==300 && more);
  p=pp_touch_read(&input,&more);assert(!p.down && p.x==128 && p.y==300 && !more);
  /* Several quick taps remain separate clicks rather than one held press. */
  for(int i=0;i<6;i++){pp_touch_sample(&input,true,90+i,160);pp_touch_sample(&input,false,0,0);}
  for(int i=0;i<6;i++) {
    p=pp_touch_read(&input,&more);assert(p.down && p.x==90+i);
    p=pp_touch_read(&input,&more);assert(!p.down && p.x==90+i);
  }
  assert(!more && input.presses==7 && input.releases==7);
  /* Motion coalesces without filling the bounded edge queue. */
  pp_touch_sample(&input,true,20,30);p=pp_touch_read(&input,&more);assert(p.down);
  for(int i=0;i<300;i++)pp_touch_sample(&input,true,i,50);
  assert(!input.count);p=pp_touch_read(&input,&more);assert(p.down && p.x==299);
  pp_touch_sample(&input,true,-1,50);pp_touch_sample(&input,true,360,50);
  p=pp_touch_read(&input,&more);assert(p.down && p.x==299);
  /* Sleep/wake ignores a contact already held; a new tap is required. */
  pp_touch_reset(&input);pp_touch_sample(&input,true,90,160);
  p=pp_touch_read(&input,&more);assert(!p.down && !more);
  pp_touch_sample(&input,false,0,0);pp_touch_sample(&input,true,90,160);
  p=pp_touch_read(&input,&more);assert(p.down);
  pp_touch_sample(&input,false,0,0);p=pp_touch_read(&input,&more);assert(!p.down);
  /* Overflow cancels stale taps safely; it never leaves a stuck press. */
  for(int i=0;i<9;i++){pp_touch_sample(&input,true,90,160);pp_touch_sample(&input,false,0,0);}
  assert(input.overflows==1);p=pp_touch_read(&input,&more);assert(!p.down && !more);
  pp_touch_sample(&input,false,0,0);pp_touch_sample(&input,true,100,160);
  p=pp_touch_read(&input,&more);assert(p.down && p.x==100);
  puts("PASS: short tap survives busy renderer, rapid taps stay distinct, motion coalesces, invalid coordinates ignored, wake/overflow cannot replay stale clicks or stick pressed");
}
