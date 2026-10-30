#include "hospital_view.h"
#include "nav_controls.h"
#include "image_assets.h"
#include "pet_animation.h"
#include "hospital_scenery.h"
#include <stdio.h>
LV_FONT_DECLARE(pp_font_18);
static struct {
  lv_obj_t *root,*content,*status,*drop,*basket,*message;
  lv_timer_t *timer;pp_game *game;pp_hospital round;
  pp_fishing_frame_cb frame;void (*leave)(void);const char *assets;
  uint32_t last,ambient,scene_frame,clinic_time;bool on;unsigned shown;
  lv_obj_t *patients[3],*effects[3],*monitor;const void *patient_source[3];unsigned patient_slot[3],patient_count;
} view;
static const char *heart[]={"01100110","11111111","11111111","11111111","01111110","00111100","00011000"};
static const char *cloud[]={"000011110000","001111111000","011111111100","111111111110","111111111111","011111111110","000010010000","000100100000"};
static void show(void);
static lv_obj_t *shape(lv_obj_t *parent,int x,int y,int w,int h,unsigned c,unsigned radius) {
  lv_obj_t *o=lv_obj_create(parent);lv_obj_remove_style_all(o);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
  lv_obj_set_style_bg_color(o,lv_color_hex(c),0);lv_obj_set_style_bg_opa(o,255,0);lv_obj_set_style_radius(o,radius,0);
  lv_obj_remove_flag(o,LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_CLICKABLE);return o;
}
static lv_obj_t *caption(const char *s,int y,int w,const lv_font_t *font,unsigned tint) {
  lv_obj_t *l=lv_label_create(view.content);lv_label_set_text(l,s);lv_obj_set_pos(l,(360-w)/2,y);lv_obj_set_width(l,w);
  lv_obj_set_style_text_font(l,font,0);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_color(l,lv_color_hex(tint),0);return l;
}
static void set_text(lv_obj_t *l,const char *s){if(strcmp(lv_label_get_text(l),s))lv_label_set_text(l,s);}
static void pixel_alpha(lv_layer_t *layer,int x,int y,int w,int h,unsigned c,unsigned alpha) {
  lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=lv_color_hex(c);d.bg_opa=alpha;
  lv_area_t a={x,y,x+w-1,y+h-1};lv_draw_rect(layer,&d,&a);
}
static void pixel(lv_layer_t *layer,int x,int y,int w,int h,unsigned c){pixel_alpha(layer,x,y,w,h,c,255);}
static void healing(lv_event_t *e) {
  unsigned i=(uintptr_t)lv_event_get_user_data(e);lv_area_t a;lv_obj_get_coords(lv_event_get_target_obj(e),&a);
  lv_layer_t *layer=lv_event_get_layer(e);unsigned time=view.clinic_time;
  for(unsigned k=0;k<3;k++) {
    unsigned age=(time+i*470+k*610)%2400;int x=8+(int)((i*13+k*19)%52),y=62-(int)(age*44/2400);
    unsigned alpha=age<400?age*180/400:age>1800?(2400-age)*180/600:180;
    if(k==1) {
      for(unsigned row=0;row<7;row++)for(unsigned col=0;col<8;col++)if(heart[row][col]=='1')
        pixel_alpha(layer,a.x1+x+(int)col,a.y1+y+(int)row,1,1,0xffc0cb,alpha);
    }else {
      pixel_alpha(layer,a.x1+x+2,a.y1+y,2,8,k?0xf3e2a4:0xa2edcf,alpha);
      pixel_alpha(layer,a.x1+x,a.y1+y+3,6,2,k?0xf3e2a4:0xa2edcf,alpha);
    }
  }
}
static void clinic_tick(void) {
  view.clinic_time=view.ambient;
  for(unsigned i=0;i<view.patient_count;i++) {
    unsigned pose=view.round.phase==PP_HOSPITAL_WON?PP_POSE_HAPPY:PP_POSE_SLEEP;
    const void *source=view.frame(pp_art_id(view.game,view.patient_slot[i]),pose,view.ambient);
    if(source!=view.patient_source[i]){pp_image_set_src(view.patients[i],source);view.patient_source[i]=source;}
    lv_obj_invalidate(view.effects[i]);
  }
  if(view.monitor)lv_obj_set_style_bg_opa(view.monitor,120+(view.ambient/400%2)*135,0);
}
static void clinic(void) {
  for(unsigned i=0;i<sizeof(pp_hospital_furniture)/sizeof(pp_hospital_furniture[0]);i++) {
    const typeof(pp_hospital_furniture[0]) *r=&pp_hospital_furniture[i];shape(view.content,r->x,r->y,r->w,r->h,r->color,0);
  }
  for(unsigned i=0;i<sizeof(pp_hospital_supplies)/sizeof(pp_hospital_supplies[0]);i++) {
    char path[320];snprintf(path,sizeof(path),"%s/%s",view.assets,pp_hospital_supplies[i].path);
    lv_obj_t *im=lv_image_create(view.content);pp_image_set_src(im,path);lv_obj_set_pos(im,pp_hospital_supplies[i].x,pp_hospital_supplies[i].y);
  }
  view.monitor=shape(view.content,280,110,2,2,0xb4f0c9,0);
  for(unsigned slot=0;slot<view.game->count&&view.patient_count<3;slot++)if(view.game->traveler!=(int)slot) {
    unsigned i=view.patient_count++;view.patient_slot[i]=slot;
    view.patients[i]=lv_image_create(view.content);lv_obj_set_pos(view.patients[i],pp_hospital_patients_xy[i][0],pp_hospital_patients_xy[i][1]);
    view.effects[i]=shape(view.content,pp_hospital_patients_xy[i][0]-12,pp_hospital_patients_xy[i][1]-24,76,76,0,0);
    lv_obj_set_style_bg_opa(view.effects[i],0,0);lv_obj_add_event_cb(view.effects[i],healing,LV_EVENT_DRAW_MAIN,(void *)(uintptr_t)i);
  }
  clinic_tick();
}

static void sprite(lv_event_t *e) {
  lv_obj_t *o=lv_event_get_target_obj(e);lv_area_t a;lv_obj_get_coords(o,&a);lv_layer_t *layer=lv_event_get_layer(e);
  unsigned kind=(uintptr_t)lv_event_get_user_data(e);
  if(kind==2) {
    pixel(layer,a.x1+2,a.y1+4,44,18,0x173e43);pixel(layer,a.x1,a.y1,48,6,0xe9c986);
    pixel(layer,a.x1+4,a.y1+6,40,12,0x8bd3b4);pixel(layer,a.x1+8,a.y1+8,32,4,0xc9eed5);
    pixel(layer,a.x1+18,a.y1+7,12,8,0xf5e3b3);return;
  }
  bool hazard=pp_hospital_cloud(&view.round);unsigned rows=hazard?8:7,cols=hazard?12:8,scale=hazard?2:3;
  const char **mask=hazard?cloud:heart;
  for(unsigned y=0;y<rows;y++)for(unsigned x=0;x<cols;x++)if(mask[y][x]=='1') {
    bool edge=!x||x+1==cols||!y||y+1==rows||mask[y][x-1]!='1'||mask[y][x+1]!='1'||mask[y-1][x]!='1'||mask[y+1][x]!='1';
    unsigned tint=edge?0x183b43:hazard?0x8998b3:y<3?0xffb5ac:0xf0808d;
    pixel(layer,a.x1+x*scale,a.y1+y*scale,scale,scale,tint);
  }
}
static unsigned treatment_pick;
static void action(lv_event_t *e) {
  unsigned cmd=(uintptr_t)lv_event_get_user_data(e);if(!view.on)return;
  if(cmd==1){view.leave();return;}
  if(cmd==3){if(view.game->count)treatment_pick=(treatment_pick+1)%view.game->count;show();return;}
  if(cmd==4){pp_result result=pp_hospital_treat(view.game,treatment_pick);if(result!=PP_OK){set_text(view.message,result==PP_NO_COINS?"金币不足":result==PP_NOT_READY?"健康已满":"伙伴忙碌中");return;}show();set_text(view.message,"健康已恢复");return;}
  if(cmd>=10&&cmd<13){pp_hospital_select(&view.round,cmd-10);return;}
  if(cmd==2) {
    if(view.round.phase==PP_HOSPITAL_WON){view.leave();return;}
    pp_result r=pp_hospital_available(view.game);
    if(r!=PP_OK){set_text(view.message,r==PP_BUSY?"伙伴忙碌中，请稍后再来":"暂时没有可护理的伙伴");return;}
    pp_hospital_start(&view.round);view.last=lv_tick_get();show();
  }
}
static lv_obj_t *control(const char *s,int x,int y,int w,int h,unsigned cmd) {
  lv_obj_t *b=lv_button_create(view.content);lv_obj_set_pos(b,x,y);lv_obj_set_size(b,w,h);
  lv_obj_t *l=lv_label_create(b);lv_label_set_text(l,s);pp_footer_text_style(b);
  lv_obj_add_event_cb(b,action,LV_EVENT_CLICKED,(void *)(uintptr_t)cmd);return b;
}
static void show(void) {
  lv_obj_clean(view.content);view.status=view.drop=view.basket=view.message=NULL;view.shown=view.round.phase;
  view.patient_count=0;view.monitor=NULL;memset(view.patient_source,0,sizeof(view.patient_source));view.scene_frame=UINT32_MAX;
  caption("宠物医院",38,200,&pp_font_18,0xf4eed9);
  pp_hospital *h=&view.round;
  if(h->phase==PP_HOSPITAL_PLAY) {
    lv_obj_t *status_panel=shape(view.content,54,66,252,26,0x123940,10);lv_obj_set_style_bg_opa(status_panel,225,0);
    view.status=caption("",70,250,&pp_font_14,0xc3e4d2);
    lv_obj_t *panel=shape(view.content,64,98,232,148,0x123940,12);lv_obj_set_style_bg_opa(panel,210,0);
    for(unsigned i=0;i<3;i++) {
      for(int y=108;y<220;y+=12)shape(view.content,103+(int)i*76,y,2,4,0x426e6b,0);
      lv_obj_t *b=control("",68+(int)i*76,104,72,142,10+i);lv_obj_clean(b);lv_obj_remove_event_cb(b,pp_footer_draw);
      lv_obj_set_style_bg_opa(b,0,0);lv_obj_set_style_bg_opa(b,0,LV_STATE_PRESSED);
    }
    view.drop=shape(view.content,92,106,24,24,0,0);lv_obj_set_style_bg_opa(view.drop,0,0);lv_obj_add_event_cb(view.drop,sprite,LV_EVENT_DRAW_MAIN,NULL);
    view.basket=shape(view.content,156,222,48,24,0,0);lv_obj_set_style_bg_opa(view.basket,0,0);lv_obj_add_event_cb(view.basket,sprite,LV_EVENT_DRAW_MAIN,(void *)2);
    caption("点击轨道移动",261,260,&pp_font_12,0xf4e5bf);
  } else {
    clinic();
    char path[320];snprintf(path,sizeof(path),"%s/hospital/joy.bin",view.assets);
    lv_obj_t *nurse=lv_image_create(view.content);pp_image_set_src(nurse,path);lv_obj_set_pos(nurse,132,88);
    if(h->phase==PP_HOSPITAL_WON) {
      lv_obj_t *panel=shape(view.content,132,222,96,24,0x123940,10);lv_obj_set_style_bg_opa(panel,225,0);
      caption("健康已护理",225,96,&pp_font_14,0xf0e6cb);
      control("完成",116,260,128,32,2);
    } else {
      view.message=caption("免费健康至60 · 治疗至100",72,260,&pp_font_12,0xeed59c);
      if(treatment_pick>=view.game->count)treatment_pick=0;
      char patient[80];snprintf(patient,sizeof(patient),"%s · 健康 %u",pp_pet_name(view.game,treatment_pick),view.game->pets[treatment_pick].health);
      control(patient,88,222,184,28,3);
      control(h->phase==PP_HOSPITAL_MISSED?"重新护理":"免费护理",72,260,108,32,2);
      control("治疗 20金币",184,260,108,32,4);
    }
  }
  lv_obj_t *back=control("",158,298,44,44,1);pp_footer_back(back);
}
static void tick(lv_timer_t *timer) {
  (void)timer;uint32_t now=lv_tick_get(),delta=now-view.last;view.last=now;
  if(!view.root||!view.on)return;
  pp_hospital *h=&view.round;pp_hospital_advance(h,delta);
  if(h->phase==PP_HOSPITAL_WON&&!h->claimed) {
    if(pp_hospital_claim(h,view.game)!=PP_OK){h->phase=PP_HOSPITAL_MISSED;}
  }
  if(view.shown!=h->phase)show();
  if(h->phase!=PP_HOSPITAL_PLAY){
    view.ambient+=delta;unsigned frame=view.ambient/PP_FRAME_MS;
    if(frame!=view.scene_frame){view.scene_frame=frame;clinic_tick();}
    return;
  }
  char s[80];unsigned seconds=(PP_HOSPITAL_ROUNDS*PP_HOSPITAL_SLOT_MS-h->elapsed+999)/1000;
  snprintf(s,sizeof(s),"爱心 %u / 6 · 还剩 %u 秒",h->score,seconds);set_text(view.status,s);
  lv_obj_set_pos(view.basket,80+(int)h->basket*76,222);
  unsigned age=h->elapsed-h->slot*PP_HOSPITAL_SLOT_MS;
  if(h->resolved)lv_obj_add_flag(view.drop,LV_OBJ_FLAG_HIDDEN);
  else {lv_obj_remove_flag(view.drop,LV_OBJ_FLAG_HIDDEN);lv_obj_set_pos(view.drop,92+(int)h->lane*76,106+(int)(116*age/PP_HOSPITAL_FALL_MS));lv_obj_invalidate(view.drop);}
}
static void deleted(lv_event_t *e) {
  if(lv_event_get_target(e)!=view.root)return;
  if(view.timer)lv_timer_delete(view.timer);
  view.timer=NULL;view.root=NULL;
}
void pp_hospital_view_open(lv_obj_t *parent,pp_game *game,const char *assets,pp_fishing_frame_cb frame,void (*leave)(void)) {
  treatment_pick=0;view=(typeof(view)){.game=game,.assets=assets,.frame=frame,.leave=leave,.on=true,.last=lv_tick_get()};
  pp_hospital_init(&view.round,game->rng^lv_tick_get()^0x48454152u);
  view.root=shape(parent,0,0,360,360,0x173c40,0);lv_obj_add_event_cb(view.root,deleted,LV_EVENT_DELETE,NULL);
  char path[320];snprintf(path,sizeof(path),"%s/hospital/room.bin",assets);
  lv_obj_t *background=lv_image_create(view.root);pp_image_set_src(background,path);
  view.content=shape(view.root,0,0,360,360,0,0);lv_obj_set_style_bg_opa(view.content,0,0);show();
  view.timer=lv_timer_create(tick,PP_FRAME_MS,NULL);
}
void pp_hospital_view_screen(bool on){view.on=on;view.last=lv_tick_get();}
pp_hospital pp_hospital_view_probe(void){return view.round;}
