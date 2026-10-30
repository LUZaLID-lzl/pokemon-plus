#include "fishing_view.h"
#include "nav_controls.h"
#include "image_assets.h"
#include "pet_animation.h"
#include "scene_view.h"
#include "fishing_scenery.h"
#include <stdio.h>
LV_FONT_DECLARE(pp_font_18);

static struct {
  lv_obj_t *root,*status,*balance,*hint,*reward,*fish,*halo,*bobber,*needle,*band,*perfect,*button,*button_label,*pet,*sea;
  lv_obj_t *rod,*rod_edge,*line;
  lv_point_precise_t rod_points[2],line_points[2];
  lv_timer_t *timer;
  pp_game *game;
  pp_fishing round;
  pp_fishing_frame_cb frame;
  void (*leave)(void);
  const char *assets;
  unsigned mode,page,detail;
  bool paused,screen_on;
  uint32_t paused_at,ambient_time,last_tick;
  const void *fish_sources[PP_FISH_COUNT],*shown_fish,*shown_pet;
} view;
static const unsigned tints[]={0xa5d6c0,0x8ccae0,0xcda9e5,0xf0ca75};
static const char *rarities[]={"常见","少见","稀有","珍奇"};
static void show(unsigned mode);
static lv_obj_t *shape(lv_obj_t *parent,int x,int y,int w,int h,unsigned c,int radius){
  lv_obj_t *o=lv_obj_create(parent);lv_obj_remove_style_all(o);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
  lv_obj_set_style_bg_color(o,lv_color_hex(c),0);lv_obj_set_style_bg_opa(o,255,0);lv_obj_set_style_radius(o,radius,0);
  lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);return o;
}
static lv_obj_t *caption_at(lv_obj_t *parent,const char *s,int x,int y,int w,const lv_font_t *font,unsigned c){
  lv_obj_t *o=lv_label_create(parent);lv_label_set_text(o,s);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);
  lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_color(o,lv_color_hex(c),0);return o;
}
static lv_obj_t *caption(const char *s,int x,int y,int w,const lv_font_t *font,unsigned c){return caption_at(view.root,s,x,y,w,font,c);}
static void set_text(lv_obj_t *o,const char *s){if(strcmp(lv_label_get_text(o),s))lv_label_set_text(o,s);}
static void pause_sync(void){
  bool paused=!view.screen_on||view.mode!=0;
  if(paused&&!view.paused){view.paused_at=lv_tick_get();view.paused=true;}
  else if(!paused&&view.paused){view.round.started+=lv_tick_get()-view.paused_at;view.paused=false;}
}
static void action(lv_event_t *e){
  unsigned cmd=(uintptr_t)lv_event_get_user_data(e);
  if(cmd==1){if(view.mode)show(0);else view.leave();return;}
  if(cmd==2){if(!view.paused)pp_fishing_press(&view.round,view.game,lv_tick_get());return;}
  if(cmd==3){view.page=0;show(1);return;}
  if(cmd==4){view.page=0;show(2);return;}
  if(cmd==5){pp_fishing_partner_clear(view.game);show(0);return;}
  if(cmd==6||cmd==7){unsigned total=view.mode==1?pp_warehouse_count(view.game):PP_FISH_COUNT,pages=(total+5)/6;if(pages)view.page=(view.page+(cmd==7?1:pages-1))%pages;show(view.mode);return;}
  if(cmd>=100&&cmd<106){unsigned slot=view.page*6+cmd-100;if(pp_fishing_partner_select(view.game,slot)==PP_OK)show(0);return;}
  if(cmd>=200&&cmd<206){view.detail=view.page*6+cmd-200;show(3);return;}
  if(cmd==8)show(2);
}
static lv_obj_t *control(const char *s,int x,int y,int w,int h,unsigned cmd){
  lv_obj_t *o=lv_button_create(view.root);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);lv_obj_set_style_shadow_width(o,0,0);
  lv_obj_add_event_cb(o,action,LV_EVENT_CLICKED,(void *)(uintptr_t)cmd);
  lv_obj_t *l=lv_label_create(o);lv_label_set_text(l,s);pp_footer_text_style(o);return o;
}
static void icon_control(const char *name,int y,unsigned cmd){
  lv_obj_t *b=control("",298,y,44,44,cmd);lv_obj_clean(b);lv_obj_remove_event_cb(b,pp_footer_draw);
  lv_obj_set_style_bg_opa(b,0,0);lv_obj_set_style_bg_opa(b,0,LV_STATE_PRESSED);
  char path[320];snprintf(path,sizeof(path),"%s/fishing/%s.bin",view.assets,name);
  lv_obj_t *im=lv_image_create(b);pp_image_set_src(im,path);lv_obj_set_pos(im,6,6);
  lv_obj_remove_flag(im,LV_OBJ_FLAG_CLICKABLE);
}
static void update(void){
  uint32_t now=lv_tick_get(),delta=now-view.last_tick;view.last_tick=now;
  if(!view.root||view.paused)return;
  view.ambient_time+=delta;pp_fishing_tick(&view.round,now);
  pp_fishing *f=&view.round;char s[96];
  snprintf(s,sizeof(s),"%u 金币",(unsigned)view.game->coins);set_text(view.balance,s);
  set_text(view.status,f->phase==PP_FISH_READY?"准备抛竿":f->phase==PP_FISH_WAIT?"等待咬钩…":f->phase==PP_FISH_REEL?"咬钩了！":f->caught?"":"鱼溜走了");
  set_text(view.hint,f->phase==PP_FISH_REEL?"绿色区域收竿":f->phase==PP_FISH_WAIT?"水下有鱼在靠近":"");
  set_text(view.button_label,f->phase==PP_FISH_READY?"抛竿":f->phase==PP_FISH_WAIT?"等待咬钩":f->phase==PP_FISH_REEL?"收竿":"再来一次");
  if(f->phase==PP_FISH_WAIT)lv_obj_add_state(view.button,LV_STATE_DISABLED);else lv_obj_remove_state(view.button,LV_STATE_DISABLED);
  bool meter=f->phase==PP_FISH_REEL;
  lv_obj_set_style_opa(view.band,meter?255:60,0);lv_obj_set_style_opa(view.perfect,meter?255:60,0);
  lv_obj_set_x(view.band,78+(int)(f->target?f->target:50)*204/100-37);lv_obj_set_x(view.perfect,78+(int)(f->target?f->target:50)*204/100-8);
  lv_obj_set_x(view.needle,78+(int)f->needle*204/100-2);
  if(meter)lv_obj_remove_flag(view.needle,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(view.needle,LV_OBJ_FLAG_HIDDEN);
  int bob=f->phase==PP_FISH_WAIT?(int)((view.ambient_time/350)%3):meter?(int)((view.ambient_time/100)%5):0;
  lv_obj_set_y(view.bobber,150+bob);
  int bend=meter?(int)((view.ambient_time/120)%3)*2:0;
  view.rod_points[0]=(lv_point_precise_t){103,158};view.rod_points[1]=(lv_point_precise_t){146-bend,119+bend};
  view.line_points[0]=view.rod_points[1];view.line_points[1]=(lv_point_precise_t){216,151+bob};
  lv_obj_invalidate(view.rod);lv_obj_invalidate(view.rod_edge);lv_obj_invalidate(view.line);
  lv_line_set_points(view.rod,view.rod_points,2);lv_line_set_points(view.rod_edge,view.rod_points,2);lv_line_set_points(view.line,view.line_points,2);
  pp_scene_view_tick(view.sea,PP_FISHING_SCENERY,view.ambient_time);
  pp_stored_pet partner;
  if(view.game->fishing_partner&&pp_warehouse_get(view.game,view.game->fishing_partner-1,&partner)){
    unsigned pose=f->caught?PP_POSE_HAPPY:meter?PP_POSE_ATTACK:PP_POSE_IDLE;
    const void *src=view.frame(pp_stored_art(&partner),pose,view.ambient_time);
    if(src!=view.shown_pet){view.shown_pet=src;pp_image_set_src(view.pet,src);}lv_obj_remove_flag(view.pet,LV_OBJ_FLAG_HIDDEN);
  }else lv_obj_add_flag(view.pet,LV_OBJ_FLAG_HIDDEN);
  if(f->caught){
    if(view.shown_fish!=view.fish_sources[f->fish]){view.shown_fish=view.fish_sources[f->fish];pp_image_set_src(view.fish,view.shown_fish);}
    lv_obj_remove_flag(view.fish,LV_OBJ_FLAG_HIDDEN);lv_obj_remove_flag(view.halo,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(view.bobber,LV_OBJ_FLAG_HIDDEN);
    snprintf(s,sizeof(s),"%s  +%u 金币",pp_fish_catalog[f->fish].name,f->reward);
  }else{lv_obj_add_flag(view.fish,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(view.halo,LV_OBJ_FLAG_HIDDEN);lv_obj_remove_flag(view.bobber,LV_OBJ_FLAG_HIDDEN);s[0]=0;}
  set_text(view.reward,s);
}
static void pond(void){
  char path[320];snprintf(path,sizeof(path),"%s/fishing/pond.bin",view.assets);
  view.sea=lv_image_create(view.root);pp_scene_view_set(view.sea,path,PP_FISHING_SCENERY,view.ambient_time);lv_obj_set_pos(view.sea,0,0);
  shape(view.root,126,18,108,20,0x153b3e,10);view.balance=caption("",126,21,108,&pp_font_12,0xe8c57c);
  icon_control("partner-icon",136,3);icon_control("dex-icon",180,4);
  shape(view.root,76,169,30,4,0x153e3c,2);view.pet=lv_image_create(view.root);lv_obj_set_pos(view.pet,68,125);
  shape(view.root,100,157,8,18,0x776441,2);shape(view.root,101,157,6,3,0xcfb374,0);
  view.rod_edge=lv_line_create(view.root);lv_line_set_points(view.rod_edge,view.rod_points,2);lv_obj_set_style_line_color(view.rod_edge,lv_color_hex(0x163942),0);lv_obj_set_style_line_width(view.rod_edge,5,0);
  view.rod=lv_line_create(view.root);lv_line_set_points(view.rod,view.rod_points,2);lv_obj_set_style_line_color(view.rod,lv_color_hex(0xcfb374),0);lv_obj_set_style_line_width(view.rod,2,0);
  view.line=lv_line_create(view.root);lv_line_set_points(view.line,view.line_points,2);lv_obj_set_style_line_color(view.line,lv_color_hex(0xe9e3c8),0);lv_obj_set_style_line_width(view.line,1,0);
  view.bobber=shape(view.root,211,150,10,14,0xf1e8cd,2);shape(view.bobber,0,0,10,7,0xe77668,2);
  view.halo=shape(view.root,166,108,108,86,0x0d313c,40);lv_obj_set_style_bg_opa(view.halo,180,0);
  view.fish=lv_image_create(view.root);lv_obj_set_pos(view.fish,172,119);
  view.status=caption("",70,212,220,&pp_font_14,0xd5ede7);view.reward=caption("",60,212,240,&pp_font_14,0xf3d18d);
  shape(view.root,76,238,208,16,0x061d27,5);view.band=shape(view.root,143,240,74,12,0x499e80,2);view.perfect=shape(view.root,172,240,16,12,0xb5e5a0,1);view.needle=shape(view.root,76,234,4,24,0xfff0be,1);
  view.hint=caption("",60,191,240,&pp_font_12,0xd5e9de);
  lv_obj_t *floating[]={view.hint,view.status,view.reward};
  for(unsigned i=0;i<3;i++){lv_obj_set_style_text_outline_stroke_color(floating[i],lv_color_hex(0x102d3b),0);lv_obj_set_style_text_outline_stroke_width(floating[i],1,0);}
  view.button=control("抛竿",116,260,128,32,2);view.button_label=lv_obj_get_child(view.button,0);
  pp_footer_back(control("",158,298,44,44,1));view.shown_fish=view.shown_pet=NULL;
}
static void pages(void){
  unsigned total=view.mode==1?pp_warehouse_count(view.game):PP_FISH_COUNT,pages=(total+5)/6;
  if(pages&&view.page>=pages)view.page=pages-1;
  caption(view.mode==1?"选择陪钓伙伴":"鱼图鉴",70,35,220,&pp_font_18,0xe5eada);char s[96];
  if(view.mode==1)snprintf(s,sizeof(s),"仓库伙伴 · %u / %u",pages?view.page+1:0,pages);
  else {unsigned seen=0;for(unsigned i=0;i<PP_FISH_COUNT;i++)seen+=view.game->fish_caught[i]>0;snprintf(s,sizeof(s),"已发现 %u / %u · %u / %u",seen,PP_FISH_COUNT,view.page+1,pages);}
  caption(s,55,64,250,&pp_font_12,0x9bc1b3);
  if(!total){caption("仓库暂无伙伴",70,142,220,&pp_font_14,0xc9ddd0);caption("可在仓库收回场景伙伴",55,169,250,&pp_font_12,0x9bc1b3);}
  for(unsigned i=0;i<6&&view.page*6+i<total;i++){
    unsigned n=view.page*6+i,cols=total-view.page*6-(i/3)*3;if(cols>3)cols=3;
    int x=(360-(int)cols*78+6)/2+(i%3)*78,y=94+(i/3)*82;
    lv_obj_t *b=control("",x,y,72,76,(view.mode==1?100:200)+i);lv_obj_clean(b);lv_obj_remove_event_cb(b,pp_footer_draw);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x173d40),0);lv_obj_set_style_bg_opa(b,255,0);lv_obj_set_style_radius(b,12,0);lv_obj_set_style_border_width(b,1,0);lv_obj_set_style_border_color(b,lv_color_hex(0x426d65),0);
    if(view.mode==1){pp_stored_pet pet;pp_warehouse_get(view.game,n,&pet);lv_obj_t *im=lv_image_create(b);pp_image_set_src(im,view.frame(pp_stored_art(&pet),PP_POSE_IDLE,0));lv_obj_set_pos(im,12,0);
      lv_obj_t *name=caption_at(b,pp_stored_name(&pet),2,46,68,&pp_font_12,0xe4eddb);lv_label_set_long_mode(name,LV_LABEL_LONG_DOT);lv_obj_set_height(name,15);
      snprintf(s,sizeof(s),view.game->fishing_partner==n+1?"陪钓中":"Lv.%u",pet.growth.level);caption_at(b,s,0,61,72,&pp_font_12,0x9bd8b4);
      if(view.game->fishing_partner==n+1){lv_obj_set_style_border_width(b,2,0);lv_obj_set_style_border_color(b,lv_color_hex(0x9bd8b4),0);}
    }else{
      /* 96px art stays native; the card uses a dedicated 48px offline sprite. */
      char path[320];snprintf(path,sizeof(path),"%s/fishing/thumb-%u.bin",view.assets,n);lv_obj_t *im=lv_image_create(b);pp_image_set_src(im,path);lv_obj_set_pos(im,12,5);
      if(!view.game->fish_caught[n]){lv_obj_set_style_image_recolor(im,lv_color_hex(0x5a7777),0);lv_obj_set_style_image_recolor_opa(im,255,0);}
      caption_at(b,view.game->fish_caught[n]?pp_fish_catalog[n].name:"???",0,43,72,&pp_font_12,tints[pp_fish_catalog[n].rarity]);
      snprintf(s,sizeof(s),"%u次",(unsigned)view.game->fish_caught[n]);caption_at(b,s,0,59,72,&pp_font_12,0x94baac);
    }
  }
  if(pages>1)for(unsigned side=0;side<2;side++){lv_obj_t *b=control("",side?298:18,158,44,44,side?7:6);pp_nav_apply(b,side);}
  if(view.mode==1&&view.game->fishing_partner)control("暂不陪钓",116,260,128,32,5);
  pp_footer_back(control("",158,298,44,44,1));
}
static void detail(void){
  unsigned id=view.detail;const pp_fish_entry *f=&pp_fish_catalog[id];char s[96];caption("鱼图鉴",100,35,160,&pp_font_18,0xe5eada);
  snprintf(s,sizeof(s),"No.%02u · %s",id+1,rarities[f->rarity]);caption(s,65,65,230,&pp_font_12,tints[f->rarity]);
  lv_obj_t *im=lv_image_create(view.root);pp_image_set_src(im,view.fish_sources[id]);lv_obj_set_pos(im,132,100);
  caption(f->name,50,175,260,&pp_font_18,0xe5eada);
  snprintf(s,sizeof(s),"捕获 %u 次 · %u 金币",(unsigned)view.game->fish_caught[id],f->coins);caption(s,50,207,260,&pp_font_12,0xb9cdb7);
  snprintf(s,sizeof(s),"普通概率 %u.%u%%",f->weight/10,f->weight%10);caption(s,50,231,260,&pp_font_12,0x9bc1b3);
  unsigned chance=pp_fishing_weight(id,2)*1000/pp_fishing_weight_total(2);
  snprintf(s,sizeof(s),"精准概率 %u.%u%%",chance/10,chance%10);caption(s,50,254,260,&pp_font_12,0xf0d295);
  lv_obj_t *b=control("",158,298,44,44,8);pp_footer_back(b);
}
static void show(unsigned mode){
  view.mode=mode;pause_sync();lv_obj_clean(view.root);view.last_tick=lv_tick_get();
  if(mode==0){pond();update();}else if(mode<3)pages();else detail();
}
static void tick(lv_timer_t *timer){(void)timer;update();}
static void deleted(lv_event_t *e){if(lv_event_get_target(e)!=view.root)return;if(view.timer)lv_timer_delete(view.timer);view.timer=NULL;view.root=NULL;}
void pp_fishing_view_open(lv_obj_t *parent,pp_game *game,const char *assets,pp_fishing_frame_cb frame,void (*leave)(void)){
  view=(typeof(view)){.game=game,.assets=assets,.frame=frame,.leave=leave,.screen_on=true,.last_tick=lv_tick_get()};
  pp_fishing_init(&view.round,game->rng^lv_tick_get());
  for(unsigned i=0;i<PP_FISH_COUNT;i++){char path[320];snprintf(path,sizeof(path),"%s/fishing/fish-%u.bin",assets,i);view.fish_sources[i]=pp_image_descriptor(path);}
  view.root=shape(parent,0,0,360,360,0x102d3b,0);lv_obj_add_event_cb(view.root,deleted,LV_EVENT_DELETE,NULL);show(0);view.timer=lv_timer_create(tick,PP_FRAME_MS,NULL);
}
pp_fishing pp_fishing_view_probe(void){return view.round;}
unsigned pp_fishing_view_mode(void){return view.mode;}
void pp_fishing_view_screen(bool screen_on){if(!view.root)return;view.screen_on=screen_on;pause_sync();view.last_tick=lv_tick_get();}
