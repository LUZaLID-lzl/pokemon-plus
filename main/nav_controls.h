#pragma once
#include "lvgl.h"
#include <stdint.h>
LV_FONT_DECLARE(pp_font_14);
LV_FONT_DECLARE(pp_font_12);
/* Two-pixel cells, rounded stepped corners and a short cartoon depth lip. */
static const uint8_t pp_nav_span[16][2]={{7,8},{6,9},{5,9},{4,9},{3,13},{2,14},{1,15},{0,15},{0,15},{1,15},{2,14},{3,13},{4,9},{5,9},{6,9},{7,8}};
static inline bool pp_nav_cell(int x,int y) {
  return y>=0 && y<16 && x>=pp_nav_span[y][0] && x<=pp_nav_span[y][1];
}
static inline void pp_nav_draw(lv_event_t *e) {
  lv_obj_t *obj=lv_event_get_target_obj(e);lv_area_t a;lv_obj_get_coords(obj,&a);
  bool next=(uintptr_t)lv_event_get_user_data(e)!=0,pressed=lv_obj_has_state(obj,LV_STATE_PRESSED);
  lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_opa=LV_OPA_COVER;
  for(unsigned pass=0;pass<2;pass++)for(int row=0;row<16;row++)for(int col=pp_nav_span[row][0];col<=pp_nav_span[row][1];col++) {
    bool edge=!pp_nav_cell(col-1,row)||!pp_nav_cell(col+1,row)||!pp_nav_cell(col,row-1)||!pp_nav_cell(col,row+1);
    uint32_t c=!pass?0x112e36:edge?0x285448:!pp_nav_cell(col,row-2)?0xe2ffe5:!pp_nav_cell(col,row+2)?0x69b69a:pressed?0xd2f6cf:0xa9e4bf;
    d.bg_color=lv_color_hex(c);int x=6+(next?15-col:col)*2,y=5+row*2+(pass?(pressed?2:0):2);
    lv_area_t pixel={a.x1+x,a.y1+y,a.x1+x+1,a.y1+y+1};lv_draw_rect(lv_event_get_layer(e),&d,&pixel);
  }
}
static inline int pp_footer_inset(int row,int rows) {
  int edge=row<rows-1-row?row:rows-1-row;return edge==0?8:edge==1?4:edge==2?2:0;
}
static inline void pp_footer_draw(lv_event_t *e) {
  lv_obj_t *obj=lv_event_get_target_obj(e);lv_area_t a;lv_obj_get_coords(obj,&a);
  int w=lv_area_get_width(&a),h=lv_area_get_height(&a),rows=h/2;
  bool pressed=lv_obj_has_state(obj,LV_STATE_PRESSED),disabled=lv_obj_has_state(obj,LV_STATE_DISABLED);
  lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_opa=disabled?160:LV_OPA_COVER;
  for(unsigned pass=0;pass<2;pass++)for(int row=0;row<rows;row++) {
    int inset=pp_footer_inset(row,rows),y=a.y1+row*2+(pass?0:2);
    d.bg_color=lv_color_hex(!pass?0x112e36:row==rows-1?0x397d6c:0x81cbb0);
    lv_area_t strip={a.x1+inset,y,a.x1+w-1-inset,y+1};lv_draw_rect(lv_event_get_layer(e),&d,&strip);
    if(pass && row>0 && row<rows-1) {
      d.bg_color=lv_color_hex(pressed?0x3d7967:row==1?0x3c7462:0x28564e);
      strip.x1+=2;strip.x2-=2;lv_draw_rect(lv_event_get_layer(e),&d,&strip);
    }
  }
}
static inline void pp_nav_apply(lv_obj_t *b,bool next) {
  lv_obj_set_style_radius(b,0,0);lv_obj_set_style_bg_opa(b,0,0);
  lv_obj_set_style_bg_opa(b,0,LV_STATE_PRESSED);
  lv_obj_set_style_bg_grad_dir(b,LV_GRAD_DIR_NONE,0);
  lv_obj_set_style_border_width(b,0,0);lv_obj_set_style_shadow_width(b,0,0);
  lv_obj_set_style_outline_width(b,0,0);lv_obj_set_style_pad_all(b,0,0);
  if(lv_obj_get_child_count(b))lv_label_set_text(lv_obj_get_child(b,0),"");
  lv_obj_remove_event_cb(b,pp_nav_draw);lv_obj_remove_event_cb(b,pp_footer_draw);
  lv_obj_add_event_cb(b,pp_nav_draw,LV_EVENT_DRAW_MAIN,(void *)(uintptr_t)next);
}
static inline void pp_footer_back(lv_obj_t *b) {
  lv_obj_set_pos(b,158,298);lv_obj_set_size(b,44,44);pp_nav_apply(b,false);
}
static inline void pp_footer_text_style(lv_obj_t *b) {
  lv_obj_remove_event_cb(b,pp_nav_draw);lv_obj_remove_event_cb(b,pp_footer_draw);
  lv_obj_set_style_radius(b,0,0);lv_obj_set_style_pad_all(b,0,0);
  lv_obj_set_style_bg_opa(b,LV_OPA_TRANSP,0);lv_obj_set_style_bg_opa(b,LV_OPA_TRANSP,LV_STATE_PRESSED);
  lv_obj_set_style_bg_grad_dir(b,LV_GRAD_DIR_NONE,0);
  lv_obj_set_style_border_width(b,0,0);lv_obj_set_style_shadow_width(b,0,0);lv_obj_set_style_outline_width(b,0,0);
  lv_obj_add_event_cb(b,pp_footer_draw,LV_EVENT_DRAW_MAIN,NULL);
  lv_obj_update_layout(b);
  if(lv_obj_get_child_count(b)) {
    lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,lv_obj_get_width(b)-4);
    lv_obj_set_style_text_font(l,lv_obj_get_width(b)<96?&pp_font_12:&pp_font_14,0);
    lv_obj_set_style_text_color(l,lv_color_hex(0xe5e9e2),0);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);lv_obj_center(l);
  }
}
static inline void pp_footer_done(lv_obj_t *b) {
  lv_obj_set_pos(b,116,296);lv_obj_set_size(b,128,32);pp_footer_text_style(b);
}

/* Sorting companion to the back arrow; same 2px mint cells and depth lip. */
static inline bool pp_nav_sort_cell(int x,int y,unsigned mode) {
  if(x<0||x>=16||y<0||y>=16)return false;
  bool arrow=(x>=2&&x<=4&&y>=1&&y<=10)||(y==10&&x<=6)||(y==11&&x>=1&&x<=5)||(y==12&&x>=2&&x<=4)||(y==13&&x==3);
  if(x<8)return arrow;
  if(mode==2)return ((x<=10)||(x>=13))&&((y>=2&&y<=4)||(y>=10&&y<=12));
  return (y>=1&&y<=3)||(y>=6&&y<=8&&x<=(mode==1?13:15))||(y>=11&&y<=13&&x<=(mode==1?11:15));
}
static inline void pp_nav_sort_draw(lv_event_t *e) {
  lv_obj_t *obj=lv_event_get_target_obj(e);lv_area_t a;lv_obj_get_coords(obj,&a);
  unsigned mode=(uintptr_t)lv_event_get_user_data(e);bool pressed=lv_obj_has_state(obj,LV_STATE_PRESSED);
  lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_opa=LV_OPA_COVER;
  for(unsigned pass=0;pass<2;pass++)for(int y=0;y<16;y++)for(int x=0;x<16;x++)if(pp_nav_sort_cell(x,y,mode)) {
    bool edge=!pp_nav_sort_cell(x-1,y,mode)||!pp_nav_sort_cell(x+1,y,mode)||!pp_nav_sort_cell(x,y-1,mode)||!pp_nav_sort_cell(x,y+1,mode);
    unsigned c=!pass?0x112e36:edge?0x285448:!pp_nav_sort_cell(x,y-2,mode)?0xe2ffe5:!pp_nav_sort_cell(x,y+2,mode)?0x69b69a:pressed?0xd2f6cf:0xa9e4bf;
    d.bg_color=lv_color_hex(c);int px=a.x1+6+x*2,py=a.y1+5+y*2+(pass?(pressed?2:0):2);
    lv_area_t pixel={px,py,px+1,py+1};lv_draw_rect(lv_event_get_layer(e),&d,&pixel);
  }
}
