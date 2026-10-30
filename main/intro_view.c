#include "intro_view.h"
#include "nav_controls.h"
#include "image_assets.h"
LV_FONT_DECLARE(pp_font_14);
LV_FONT_DECLARE(pp_font_intro_24);
LV_FONT_DECLARE(pp_font_intro_32);
LV_FONT_DECLARE(pp_font_intro_20);
static lv_obj_t *pets[3],*types[3],*particles[PP_INTRO_PARTICLES],*fade;
/* Fixed glyph layers: bevel and extrusion without blur or frame-time images. */
typedef struct { lv_obj_t *layers[24]; unsigned count,alpha[24],last; } raised_text;
static raised_text title, title_plus, caption;
static const void *sources[3];
static pp_intro_frame_fn get_frame;
static void (*on_done)(void);
static uint32_t started;
static bool active;
static lv_obj_t *plain(lv_obj_t *p,int x,int y,int w,int h) {
  lv_obj_t *o=lv_obj_create(p);lv_obj_remove_style_all(o);
  lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
  lv_obj_remove_flag(o,LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_CLICKABLE);
  return o;
}
static lv_obj_t *words(lv_obj_t *p,const char *s,int y,const lv_font_t *font,unsigned color) {
  lv_obj_t *o=lv_label_create(p);lv_label_set_text(o,s);lv_obj_set_pos(o,40,y);lv_obj_set_width(o,280);
  lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_font(o,font,0);
  lv_obj_set_style_text_color(o,lv_color_hex(color),0);return o;
}
static void raised_layer(raised_text *t,lv_obj_t *parent,const char *s,int y,const lv_font_t *font,int dx,int dy,unsigned color,unsigned alpha) {
  unsigned i=t->count++;
  t->layers[i]=words(parent,s,y+dy,font,color);
  lv_obj_set_x(t->layers[i],40+dx);t->alpha[i]=alpha;
  lv_obj_set_style_text_opa(t->layers[i],0,0);
}
static void raised_alpha(raised_text *t,unsigned alpha) {
  if(t->last==alpha)return;
  t->last=alpha;
  for(unsigned i=0;i<t->count;i++)lv_obj_set_style_text_opa(t->layers[i],alpha*t->alpha[i]/255,0);
}
static void raised_open(raised_text *t,lv_obj_t *parent,const char *s,int y,const lv_font_t *font,bool title_style) {
  *t=(raised_text){.last=256};
  if(title_style) {
    const int dx[]={-1,0,1,-1,1,-1,0,1},dy[]={-1,-1,-1,0,0,1,1,1};
    raised_layer(t,parent,s,y,font,4,5,0x091b28,180);
    for(unsigned i=0;i<8;i++)raised_layer(t,parent,s,y,font,3+dx[i],3+dy[i],0x142635,255);
    raised_layer(t,parent,s,y,font,3,3,0x53446e,255);
    raised_layer(t,parent,s,y,font,2,2,0x716092,255);
    raised_layer(t,parent,s,y,font,1,1,0x9880bd,255);
    for(unsigned i=0;i<8;i++)raised_layer(t,parent,s,y,font,dx[i],dy[i],0x263447,255);
    raised_layer(t,parent,s,y,font,-1,-1,0xfff0ff,255);
    raised_layer(t,parent,s,y,font,0,0,0xd5c6ff,255);
  } else {
    raised_layer(t,parent,s,y,font,1,2,0x14392f,230);
    raised_layer(t,parent,s,y,font,0,1,0x5c8470,255);
    raised_layer(t,parent,s,y,font,0,0,0xe2eadb,255);
  }
}
static void skip(lv_event_t *e) {(void)e;if(active && on_done)on_done();}
void pp_intro_view_close(void) {active=false;get_frame=NULL;on_done=NULL;}
pp_intro_state pp_intro_view_probe(void) {return pp_intro_sample(active?lv_tick_get()-started:PP_INTRO_MS);}
void pp_intro_view_tick(void) {
  if(!active)return;
  pp_intro_state s=pp_intro_view_probe();
  if(s.finished){if(on_done)on_done();return;}
  for(unsigned i=0;i<3;i++) {
    pp_intro_actor *p=&s.pets[i];
    const void *src=get_frame(i,p->pose,p->elapsed,p->left);
    if(src!=sources[i]){pp_image_set_src(pets[i],src);sources[i]=src;}
    lv_obj_set_pos(pets[i],p->x,p->y);lv_obj_set_style_opa(pets[i],p->opacity,0);
    lv_obj_set_pos(types[i],p->x+8,125);lv_obj_set_style_opa(types[i],s.icons,0);
  }
  const unsigned colors[]={0x9cdb95,0xffc078,0x9bd8ef};
  for(unsigned i=0;i<PP_INTRO_PARTICLES;i++) {
    pp_intro_particle *p=&s.particles[i];lv_obj_t *o=particles[i];
    lv_obj_set_pos(o,p->x,p->y);lv_obj_set_size(o,p->size?p->size:1,p->size?p->size:1);
    lv_obj_set_style_bg_color(o,lv_color_hex(colors[p->type]),0);
    lv_obj_set_style_border_color(o,lv_color_hex(colors[p->type]),0);
    lv_obj_set_style_border_width(o,p->type==2?1:0,0);
    lv_obj_set_style_radius(o,p->type==2?3:1,0);
    lv_obj_set_style_bg_opa(o,p->type==2?25:255,0);lv_obj_set_style_opa(o,p->opacity,0);
  }
  raised_alpha(&title,s.title);raised_alpha(&title_plus,s.title);raised_alpha(&caption,s.caption);
  lv_obj_set_style_bg_opa(fade,s.fade,0);
}
void pp_intro_view_open(lv_obj_t *parent,const void *scene,const void *type_sources[3],pp_intro_frame_fn frame,void (*done)(void)) {
  started=lv_tick_get();get_frame=frame;on_done=done;active=true;
  lv_obj_t *bg=lv_image_create(parent);pp_image_set_src(bg,scene);lv_obj_set_pos(bg,0,0);
  lv_obj_t *scrim=plain(parent,0,0,360,360);lv_obj_set_style_bg_color(scrim,lv_color_hex(0x080e17),0);lv_obj_set_style_bg_opa(scrim,110,0);
  raised_open(&title,parent,"POKEMON",44,&pp_font_intro_32,true);
  raised_open(&title_plus,parent,"PLUS",80,&pp_font_intro_24,true);
  for(unsigned i=0;i<3;i++) {
    pets[i]=lv_image_create(parent);sources[i]=NULL;
    types[i]=lv_image_create(parent);pp_image_set_src(types[i],type_sources[i]);
  }
  for(unsigned i=0;i<PP_INTRO_PARTICLES;i++)particles[i]=plain(parent,0,0,1,1);
  raised_open(&caption,parent,"一起开始冒险吧",242,&pp_font_intro_20,false);
  fade=plain(parent,0,0,360,360);lv_obj_set_style_bg_color(fade,lv_color_hex(0x080e17),0);
  lv_obj_t *b=lv_button_create(parent);lv_obj_remove_style_all(b);
  lv_obj_set_pos(b,148,293);lv_obj_set_size(b,64,32);
  lv_obj_set_style_radius(b,16,0);lv_obj_set_style_bg_color(b,lv_color_hex(0x142a34),0);lv_obj_set_style_bg_opa(b,200,0);
  lv_obj_add_event_cb(b,skip,LV_EVENT_CLICKED,NULL);
  lv_obj_t *l=lv_label_create(b);lv_label_set_text(l,">>");lv_obj_set_style_text_font(l,&pp_font_14,0);lv_obj_set_style_text_color(l,lv_color_hex(0xa0b9bf),0);lv_obj_center(l);pp_footer_done(b);
  pp_intro_view_tick();
}
