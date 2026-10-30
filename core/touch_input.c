#include "touch_input.h"
void pp_touch_reset(pp_touch_input *input) {
  input->head=input->count=0;
  input->latest.down=input->delivered.down=false;
  input->ignore_until_release=true;
}
void pp_touch_sample(pp_touch_input *input,bool down,int x,int y) {
  if(down && (x<0 || y<0 || x>=360 || y>=360)) return;
  if(input->ignore_until_release) {
    if(!down)input->ignore_until_release=false;
    return;
  }
  pp_touch_point point=down?(pp_touch_point){x,y,true}:input->latest;
  point.down=down;
  if(down!=input->latest.down) {
    if(input->count==PP_TOUCH_EDGES) {
      /* Never leave LVGL pressed or replay old taps against a new page. */
      input->overflows++;
      pp_touch_reset(input);
      input->edges[0]=input->delivered;input->edges[0].down=false;
      input->count=1;
      return;
    }
    input->edges[(input->head+input->count)%PP_TOUCH_EDGES]=point;
    input->count++;
    if(down)input->presses++;else input->releases++;
  }
  input->latest=point;
}
pp_touch_point pp_touch_read(pp_touch_input *input,bool *more) {
  if(input->count) {
    input->delivered=input->edges[input->head];
    input->head=(input->head+1)%PP_TOUCH_EDGES;input->count--;
  } else input->delivered=input->latest;
  *more=input->count>0;
  return input->delivered;
}
