#include "training_view.h"
#include "game_ui.h"
#include "battle_view.h"
#include "nav_controls.h"
#include "image_assets.h"
#include "scene_view.h"
#include "pet_animation.h"
#include "training_scenery.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
LV_FONT_DECLARE(pp_font_18);
static struct {
 lv_obj_t *root,*content,*background,*status,*found,*pets[3],*shadows[3],*levels[3],*alerts[3],*scenery[2],*back,*foe,*foe_shadow;lv_timer_t *timer;
 pp_game *game;pp_training_frame_cb frame;void (*leave)(void);const char *assets;
 unsigned mode,scene,picks[3],count,list,last_phase;uint32_t last,ambient,walk_ms,result_ms;bool on;
 const void *source[3],*foe_source;char error[96];
} view;
static pp_training session;
static pp_game *job_game;
static lv_timer_t *job_timer;
static uint32_t job_tick,job_elapsed;
static uint64_t job_seconds;
static void tick(lv_timer_t *timer);
static void job_clock(void){
 job_tick=lv_tick_get();job_elapsed=job_game->training.elapsed_ms+job_game->training.pending_ms;
 uint64_t offline=job_game->seconds>=job_game->training.anchor_seconds?(job_game->seconds-job_game->training.anchor_seconds)*1000:0;
 if(offline>PP_TRAINING_DURATION_MS)offline=PP_TRAINING_DURATION_MS;
 if(offline>job_elapsed)job_elapsed=offline;
 job_seconds=job_game->seconds;
}
static void job_tick_cb(lv_timer_t *timer){
 (void)timer;if(!job_game)return;
 if(job_game->training.status!=1){if(view.root)tick(NULL);return;}
 uint64_t real=(uint32_t)(lv_tick_get()-job_tick),game=job_game->seconds>=job_seconds?(job_game->seconds-job_seconds)*1000:0;
 /* Whole seconds have an unrelated phase: never let an ordinary second tick
  * jump ahead and replace an unfinished enemy reply. Rebase only for actual
  * offline/time catch-up; then keep advancing from the millisecond clock. */
 if(game>real+1000){
  uint64_t caught=(uint64_t)job_elapsed+game;job_elapsed=caught>PP_TRAINING_DURATION_MS?PP_TRAINING_DURATION_MS:caught;
  job_tick=lv_tick_get();job_seconds=job_game->seconds;real=0;
 }
 uint64_t target=job_elapsed+real;if(target>PP_TRAINING_DURATION_MS)target=PP_TRAINING_DURATION_MS;
 unsigned processed=job_game->training.elapsed_ms+job_game->training.pending_ms;
 if(target>processed){unsigned ms=target-processed;if(ms>60000)ms=60000;pp_training_job_advance_auto(&session,job_game,ms,pp_ui_auto_supply());}
 if(view.root)tick(NULL);
}
void pp_training_view_bind(pp_game *g){
 if(job_timer){lv_timer_delete(job_timer);}
 job_game=g;pp_training_restore(&session,g);job_clock();
 job_timer=lv_timer_create(job_tick_cb,PP_TRAINING_VIEW_FRAME_MS,NULL);
}
static void show(void);
static lv_obj_t *shape(lv_obj_t *parent,int x,int y,int w,int h,unsigned tint,unsigned alpha){
 lv_obj_t *o=lv_obj_create(parent);lv_obj_remove_style_all(o);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
 lv_obj_set_style_bg_color(o,lv_color_hex(tint),0);lv_obj_set_style_bg_opa(o,alpha,0);lv_obj_set_style_radius(o,8,0);lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);return o;
}
static lv_obj_t *caption(const char *s,int x,int y,int w,const lv_font_t *font,unsigned tint){lv_obj_t *o=lv_label_create(view.content);lv_label_set_text(o,s);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_color(o,lv_color_hex(tint),0);return o;}
static void action(lv_event_t *e){
 unsigned cmd=(uintptr_t)lv_event_get_user_data(e);if(!view.on)return;
 if(cmd==1){
  if(view.mode>=2){view.leave();}
  else if(view.mode==1){view.mode=0;show();}else {view.leave();}
  return;
 }
 if(cmd==6){if(view.game->training.status==1){pp_training_job_finish(&session,view.game);view.mode=4;show();tick(NULL);}return;}
 if(cmd==5){view.mode=0;view.count=0;show();return;}
 if(cmd==2){
  if(!view.mode){view.mode=1;view.list=0;view.error[0]=0;show();return;}
  if(view.mode==1){pp_result r=pp_training_job_start(&session,view.game,view.scene,view.picks,view.count);
   if(r!=PP_OK){snprintf(view.error,sizeof(view.error),"%s",!view.count?"请选择1至3位伙伴":r==PP_NOT_READY?"需健康30、饱腹20、精力40":r==PP_BUSY?"伙伴忙碌中，请稍后再来":"请选择有效伙伴");show();return;}
   job_clock();view.mode=2;view.last=lv_tick_get();view.ambient=view.walk_ms=0;show();
  }return;
 }
 if(cmd==3||cmd==4){
  if(!view.mode)view.scene=(view.scene+(cmd==3?5:1))%6;
  else if(view.mode==1){unsigned pages=(view.game->count+8)/9;if(pages)view.list=(view.list+(cmd==3?pages-1:1))%pages;}
  show();return;
 }
 if(cmd>=100){
  view.error[0]=0;unsigned ordinal=cmd-100,pick=ordinal;if(pick>=view.game->count)return;
  unsigned i=0;while(i<view.count&&view.picks[i]!=pick)i++;
  if(i<view.count){for(unsigned j=i;j+1<view.count;j++)view.picks[j]=view.picks[j+1];view.count--;}
  else if(view.count<3)view.picks[view.count++]=pick;
  else {snprintf(view.error,sizeof(view.error),"最多选择3位伙伴");}
  show();
 }
}
static lv_obj_t *control(const char *s,int x,int y,int w,int h,unsigned cmd){lv_obj_t *b=lv_button_create(view.content);lv_obj_set_pos(b,x,y);lv_obj_set_size(b,w,h);lv_label_set_text(lv_label_create(b),s);pp_footer_text_style(b);lv_obj_add_event_cb(b,action,LV_EVENT_CLICKED,(void *)(uintptr_t)cmd);return b;}
static void pixel(lv_layer_t *layer,int x,int y,int w,int h,unsigned color,unsigned alpha){lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=lv_color_hex(color);d.bg_opa=alpha;lv_area_t a={x,y,x+w-1,y+h-1};lv_draw_rect(layer,&d,&a);}
static void scenery_paint(void *data,int x,int y,int w,int h,unsigned tint,unsigned alpha){
 pixel(data,x,y,w,h,tint,alpha);
}
static void scenery_draw(lv_event_t *e){
 bool front=(uintptr_t)lv_event_get_user_data(e)!=0;
 pp_training_scenery(view.scene,view.ambient,front,scenery_paint,lv_event_get_layer(e));
 if(front&&(session.phase==PP_TRAINING_SEARCH||session.phase==PP_TRAINING_APPROACH))pp_training_battle_elements(view.scene,view.walk_ms,scenery_paint,lv_event_get_layer(e));
}
static void scenery_layer(unsigned front){
 view.scenery[front]=shape(view.content,0,0,360,360,0,0);lv_obj_set_style_radius(view.scenery[front],0,0);
 lv_obj_add_event_cb(view.scenery[front],scenery_draw,LV_EVENT_DRAW_MAIN,(void *)(uintptr_t)front);
}
static void progress_text(void){
 unsigned seconds=(view.game->training.elapsed_ms>=PP_TRAINING_DURATION_MS?0:(PP_TRAINING_DURATION_MS-view.game->training.elapsed_ms+999)/1000),items=0;
 for(unsigned i=0;i<PP_SHOP_ITEMS;i++)items+=view.game->training.items[i];
 char line[128];snprintf(line,sizeof(line),"%02u:%02u:%02u · 金币 +%u · 物品 %u",seconds/3600,seconds/60%60,seconds%60,(unsigned)view.game->training.coins,items);
 lv_obj_t *label=lv_obj_get_user_data(view.root);if(label)lv_label_set_text(label,line);
}
static void search_tick(void){
 pp_scene_view_panorama_tick(view.background,view.walk_ms);
 bool visible=session.phase==PP_TRAINING_APPROACH||session.phase==PP_TRAINING_ALERT;
 if(visible){
  int x=354-(int)((session.phase==PP_TRAINING_ALERT?PP_TRAINING_APPROACH_MS:(session.clock_ms+view.game->training.pending_ms))*144/PP_TRAINING_APPROACH_MS);
  const void *src=view.frame(session.foes[0].art,PP_POSE_IDLE,view.ambient,true);
  if(src!=view.foe_source){pp_image_set_src(view.foe,src);view.foe_source=src;}
  lv_obj_set_pos(view.foe,x,154);lv_obj_set_pos(view.foe_shadow,x+11,194);
  lv_obj_remove_flag(view.foe,LV_OBJ_FLAG_HIDDEN);lv_obj_remove_flag(view.foe_shadow,LV_OBJ_FLAG_HIDDEN);
 }else{lv_obj_add_flag(view.foe,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(view.foe_shadow,LV_OBJ_FLAG_HIDDEN);}

 for(unsigned i=0;i<session.count;i++){
  int x,y;bool left;pp_training_position(view.scene,i,session.count,view.walk_ms,&x,&y,&left);
  bool alert=session.phase==PP_TRAINING_ALERT;
  const void *src=view.frame(pp_art_id(&session.battle,i),alert?PP_POSE_IDLE:PP_POSE_WALK,alert?0:view.walk_ms*3/4+i*170,left);
  if(src!=view.source[i]){pp_image_set_src(view.pets[i],src);view.source[i]=src;}
  lv_obj_set_pos(view.pets[i],x,y);lv_obj_set_pos(view.shadows[i],x+11,y+40);
  lv_obj_set_pos(view.levels[i],x+2,y+45);lv_obj_set_pos(view.alerts[i],x+14,y-24);
  if(alert)lv_obj_remove_flag(view.alerts[i],LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(view.alerts[i],LV_OBJ_FLAG_HIDDEN);
 }
}
/* Result cards reuse native-size, preloaded shop/HUD artwork. */
static void result_icon(const char *name,int x,int y){
 char path[320];snprintf(path,sizeof(path),"%s/%s.bin",view.assets,name);
 lv_obj_t *im=lv_image_create(view.content);pp_image_set_src(im,path);lv_obj_set_pos(im,x,y);
}
static void result_stat(unsigned index,const char *name,const char *icon,unsigned value,bool plus,unsigned tint){
 int x=64+(index%2)*120,y=76+(index/2)*44;
 lv_obj_t *tile=shape(view.content,x,y,112,40,0x102b39,235);
 lv_obj_set_style_border_width(tile,1,0);lv_obj_set_style_border_color(tile,lv_color_hex(tint),0);lv_obj_set_style_border_opa(tile,52,0);
 result_icon(icon,x+8,y+8);
 lv_obj_t *l=caption(name,x+40,y+3,67,&pp_font_12,0xa4babc);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_LEFT,0);
 char line[24];snprintf(line,sizeof(line),"%s%u",plus?"+":"",value);
 l=caption(line,x+40,y+17,69,strlen(line)>6?&pp_font_14:&pp_font_18,tint);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_LEFT,0);lv_label_set_long_mode(l,LV_LABEL_LONG_CLIP);
}
static void exit_draw(lv_event_t *e){
 lv_obj_t *o=lv_event_get_target_obj(e);lv_area_t a;lv_obj_get_coords(o,&a);bool pressed=lv_obj_has_state(o,LV_STATE_PRESSED);
 for(unsigned pass=0;pass<2;pass++)for(int y=0;y<16;y++)for(int x=0;x<16;x++)if(pp_training_exit_cell(x,y)){
  bool edge=!pp_training_exit_cell(x-1,y)||!pp_training_exit_cell(x+1,y)||!pp_training_exit_cell(x,y-1)||!pp_training_exit_cell(x,y+1);
  unsigned c=!pass?0x112e36:edge?0x285448:!pp_training_exit_cell(x,y-2)?0xe2ffe5:!pp_training_exit_cell(x,y+2)?0x69b69a:pressed?0xd2f6cf:0xa9e4bf;
  pixel(lv_event_get_layer(e),a.x1+6+x*2,a.y1+5+y*2+(pass?(pressed?2:0):2),2,2,c,255);
 }
}
static void exit_control(void){
 lv_obj_set_pos(view.back,PP_TRAINING_BACK_X,PP_TRAINING_NAV_Y);lv_obj_t *b=control("",PP_TRAINING_EXIT_X,PP_TRAINING_NAV_Y,44,44,6);pp_nav_apply(b,false);lv_obj_remove_event_cb(b,pp_nav_draw);lv_obj_add_event_cb(b,exit_draw,LV_EVENT_DRAW_MAIN,NULL);
}
static void result_show(void){
 const pp_training_record *r=&view.game->training;unsigned items=0,count=0;
 for(unsigned i=0;i<PP_SHOP_ITEMS;i++){items+=r->items[i];if(r->items[i])count++;}
 shape(view.content,112,44,16,1,0xe8c98e,160);shape(view.content,232,44,16,1,0xe8c98e,160);
 caption("历练成果",70,32,220,&pp_font_18,0xffe8b1);
 unsigned seconds=(r->elapsed_ms+999)/1000,minutes=(seconds+59)/60;char line[96];snprintf(line,sizeof(line),seconds<60?"%s · %u秒":minutes%60?"%s · %u分钟":"%s · %u小时",pp_training_scenes[view.scene].name,seconds<60?seconds:minutes%60?minutes:minutes/60);caption(line,60,57,240,&pp_font_12,0xc0d3cb);
 result_stat(0,"战斗","icons-small/9",r->rounds,false,0xecc68b);
 result_stat(1,"经验","icons-small/14",r->total_xp,true,0xb5e2c7);
 result_stat(2,"金币","hud/coin-large",r->coins,true,0xffdb80);
 result_stat(3,"物品","icons-small/6",items,false,0xb8d7ee);
 shape(view.content,76,166,208,1,0xa7c5bb,65);
 if(count){
  shape(view.content,52,171,256,83,0x102b39,215);
  unsigned n=0;
  for(unsigned i=0;i<PP_SHOP_ITEMS;i++)if(r->items[i]){
   unsigned row=n/3,cols=count-row*3;if(cols>3)cols=3;
   int x=(360-(int)cols*84)/2+(n%3)*84,y=174+row*26;char icon[64];
   if(i<PP_ITEMS)snprintf(icon,sizeof(icon),"icons-small/%u",i);
   else snprintf(icon,sizeof(icon),"items/%s-small",i==PP_HATCH_BOOST?"hatch-boost":i==PP_BALL?"capture-ball":"tree-essence");
   result_icon(icon,x,y+1);
   lv_obj_t *l=caption(pp_items[i].name,x+26,y,56,&pp_font_12,0xd9e7df);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_LEFT,0);
   snprintf(line,sizeof(line),"×%u",(unsigned)r->items[i]);l=caption(line,x+26,y+13,56,&pp_font_12,0xf5dca3);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_LEFT,0);n++;
  }
 }else{
  result_icon("icons-small/6",168,186);caption("途中没有发现物品",60,220,240,&pp_font_12,0xb4c8c5);
 }
 control("再次历练",116,260,128,32,5);view.back=control("",158,298,44,44,1);pp_footer_back(view.back);
}
static void show(void){
 pp_battle_view_close();lv_obj_clean(view.content);memset(view.pets,0,sizeof(view.pets));memset(view.source,0,sizeof(view.source));view.status=view.found=NULL;memset(view.scenery,0,sizeof(view.scenery));
 const pp_training_scene *s=&pp_training_scenes[view.scene];char path[320],text[128];snprintf(path,sizeof(path),"%s/training/%u.bin",view.assets,view.scene);if(view.mode<2||view.mode==4)pp_scene_view_pixel_blurred_set(view.background,path,2,0,0);else pp_scene_view_panorama_set(view.background,path,view.walk_ms);
 if(view.mode==4){result_show();return;}
 if(view.mode==3){
  shape(view.content,0,0,360,360,0x102b39,220);
  lv_obj_t *body=shape(view.content,0,64,360,215,0,0);lv_obj_set_style_radius(body,0,0);
  lv_obj_t *retry=control("",210,296,52,32,999);lv_obj_add_flag(retry,LV_OBJ_FLAG_HIDDEN);
  view.back=control("",158,298,44,44,1);pp_footer_back(view.back);
  pp_battle_view_create(body,&session.battle,view.assets,retry,view.back);exit_control();return;
 }
 /* A compact title and separate level chip keep the round crown open. */
 shape(view.content,108,49,18,1,pp_training_themes[view.scene][1],110);
 shape(view.content,234,49,18,1,pp_training_themes[view.scene][1],110);
 caption(view.mode==1?"选择伙伴":s->name,70,37,220,&pp_font_18,0xf3edcf);
 if(!view.mode){
  snprintf(text,sizeof(text),"Lv.%u–%u · %u种",s->min_level,s->max_level,pp_training_pool_count(view.scene));
  lv_obj_t *chip=shape(view.content,110,65,140,20,pp_training_themes[view.scene][0],170);
  lv_obj_set_style_radius(chip,10,0);
  lv_obj_set_style_border_color(chip,lv_color_hex(pp_training_themes[view.scene][1]),0);
  lv_obj_set_style_border_width(chip,1,0);lv_obj_set_style_border_opa(chip,45,0);
  caption(text,112,69,136,&pp_font_12,0xd6e1cb);
 }else{
  if(view.mode==1)snprintf(text,sizeof(text),"%s",view.error);
  else snprintf(text,sizeof(text),"Lv.%u–%u · %s",s->min_level,s->max_level,s->search);
  if(view.mode==1&&!view.error[0])snprintf(text,sizeof(text),"已选 %u / 3 · 仅庭院伙伴",view.count);
  caption(text,52,68,256,&pp_font_12,0xb9cfbe);
 }
 if(!view.mode){

  for(unsigned i=0;i<6;i++){const pp_dex_entry *d=NULL;for(unsigned j=0;j<PP_DEX_COUNT;j++)if(pp_pokedex[j].national_id==s->dex[i])d=&pp_pokedex[j];if(!d)continue;
   int x=80+(int)(i%3)*76,y=99+(int)(i/3)*70;shape(view.content,x-3,y-3,66,66,pp_training_themes[view.scene][0],135);lv_obj_t *im=lv_image_create(view.content);pp_image_set_src(im,view.frame(d->art,PP_POSE_IDLE,0,false));lv_obj_set_pos(im,x+6,y);caption(d->name,x-5,y+46,70,&pp_font_12,0xe0e3ca);
  }
  control("选择伙伴",116,260,128,32,2);
 }else if(view.mode==1){
  unsigned total=view.game->count,start=view.list*9,n=total>start?total-start:0;if(n>9)n=9;
  for(unsigned i=0;i<n;i++){unsigned ord=start+i,pick=ord;pp_stored_pet p;pp_training_pet(view.game,pick,&p);
   unsigned row=i/3,col=i%3,cols=n-row*3;if(cols>3)cols=3;int x=(360-(int)cols*76+4)/2+(int)col*76,y=88+(int)row*54;
   unsigned order=0;for(unsigned j=0;j<view.count;j++)if(view.picks[j]==pick)order=j+1;
   lv_obj_t *tile=control("",x,y,72,52,100+ord);lv_obj_remove_event_cb(tile,pp_footer_draw);lv_obj_set_style_bg_opa(tile,220,0);lv_obj_set_style_bg_color(tile,lv_color_hex(order?0x477352:0x163d43),0);lv_obj_set_style_radius(tile,8,0);lv_obj_clean(tile);
   char avatar[320];snprintf(avatar,sizeof(avatar),"%s/avatars/%u-0.bin",view.assets,pp_stored_art(&p));lv_obj_t *im=lv_image_create(tile);pp_image_set_src(im,avatar);lv_obj_set_pos(im,0,-3);
   snprintf(text,sizeof(text),"%s",pp_stored_name(&p));lv_obj_t *name=lv_label_create(tile);lv_label_set_text(name,text);lv_obj_set_pos(name,2,32);lv_obj_set_width(name,68);lv_obj_set_style_text_align(name,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_font(name,&pp_font_12,0);lv_label_set_long_mode(name,LV_LABEL_LONG_CLIP);
   snprintf(text,sizeof(text),"Lv.%u",p.growth.level);
   lv_obj_t *l=lv_label_create(tile);lv_label_set_text(l,text);lv_obj_set_pos(l,33,8);lv_obj_set_width(l,38);lv_obj_set_style_text_font(l,&pp_font_12,0);lv_obj_set_style_text_color(l,lv_color_hex(order?0xffedab:0xbad2cd),0);
  }
  if(!total)caption("暂无可出战伙伴",60,153,240,&pp_font_14,0xeed7ac);
  control("进入历练",116,260,128,32,2);
 }else{
  scenery_layer(0);
  for(unsigned i=0;i<session.count;i++){view.shadows[i]=shape(view.content,0,0,26,5,0x12252a,45);view.pets[i]=lv_image_create(view.content);}
  view.foe_shadow=shape(view.content,0,0,26,5,0x12252a,45);view.foe=lv_image_create(view.content);view.foe_source=NULL;
  scenery_layer(1);
  /* Follow each original individual; text stays upright when the sprite turns. */
  for(unsigned i=0;i<session.count;i++){
   pp_stored_pet p;pp_training_pet(view.game,session.picks[i],&p);
   view.levels[i]=shape(view.content,0,0,44,16,pp_training_themes[view.scene][0],210);
   lv_obj_t *l=lv_label_create(view.levels[i]);snprintf(text,sizeof(text),"Lv.%u",p.growth.level);lv_label_set_text(l,text);
   lv_obj_set_size(l,44,14);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_font(l,&pp_font_12,0);lv_obj_set_style_text_color(l,lv_color_hex(0xffe9af),0);
   view.alerts[i]=shape(view.content,0,0,20,22,0xffe6a2,255);
   shape(view.alerts[i],8,20,5,5,0xffe6a2,255);
   l=lv_label_create(view.alerts[i]);lv_label_set_text(l,"!");lv_obj_set_style_text_font(l,&pp_font_18,0);lv_obj_set_style_text_color(l,lv_color_hex(0x69422c),0);lv_obj_center(l);
  }
  shape(view.content,48,229,264,57,pp_training_themes[view.scene][0],180);
  snprintf(text,sizeof(text),"战斗 %u 次 · 经验 +%u XP",session.rounds,session.total_xp);
  view.status=caption(text,54,233,252,&pp_font_14,0xf3e2b8);
  view.found=caption("",54,251,252,&pp_font_12,0xd8e2cb);
  search_tick();
 }
 if(view.mode<2 && (!view.mode||view.game->count>9))for(unsigned side=0;side<2;side++){lv_obj_t *b=control("",side?298:18,158,44,44,side?4:3);pp_nav_apply(b,side);}
 view.back=control("",158,298,44,44,1);pp_footer_back(view.back);if(view.mode==2)exit_control();
}
static void tick(lv_timer_t *timer){
 (void)timer;view.last=lv_tick_get();if(!view.on)return;
 lv_obj_t *progress=lv_obj_get_user_data(view.root);if(progress){if(view.mode==2||view.mode==3){lv_obj_remove_flag(progress,LV_OBJ_FLAG_HIDDEN);progress_text();}else lv_obj_add_flag(progress,LV_OBJ_FLAG_HIDDEN);}
 if(view.mode<2)return;
 /* Render the unprocessed remainder, without changing deterministic 40ms logic. */
 unsigned remainder=view.game->training.status==1?view.game->training.pending_ms:0;
 view.walk_ms=view.game->training.walk_ms+((session.phase==PP_TRAINING_SEARCH||session.phase==PP_TRAINING_APPROACH)?remainder:0);
 view.ambient=view.game->training.ambient_ms+remainder;
 unsigned next=view.game->training.status==2?4:(session.phase==PP_TRAINING_FIGHT||session.phase==PP_TRAINING_RESULT)?3:2;
 if(view.mode!=next){view.mode=next;show();}
 if(view.mode==2){
  search_tick();lv_area_t area={18,85,340,240};lv_obj_invalidate_area(view.scenery[0],&area);lv_obj_invalidate_area(view.scenery[1],&area);
  char line[96];snprintf(line,sizeof(line),"战斗 %u 次 · 经验 +%u XP",session.rounds,session.total_xp);lv_label_set_text(view.status,line);
  pp_training_discovery(&view.game->training,line,sizeof(line));
  lv_label_set_text(view.found,line);
 }
 if(view.mode==3){pp_battle_view_tick();lv_obj_set_pos(view.back,PP_TRAINING_BACK_X,PP_TRAINING_NAV_Y);}
}
static void deleted(lv_event_t *e){if(lv_event_get_target(e)!=view.root)return;if(view.timer)lv_timer_delete(view.timer);pp_battle_view_close();view.timer=NULL;view.root=NULL;}
void pp_training_view_open(lv_obj_t *parent,pp_game *g,const char *assets,pp_training_frame_cb frame,void (*leave)(void)){
 view=(typeof(view)){.game=g,.assets=assets,.frame=frame,.leave=leave,.on=true,.last=lv_tick_get()};
 if(job_game!=g)pp_training_view_bind(g);
 if(g->training.status){view.scene=g->training.scene;view.count=g->training.count;memcpy(view.picks,g->training.picks,sizeof(view.picks));view.mode=g->training.status==2?4:(session.phase==PP_TRAINING_FIGHT||session.phase==PP_TRAINING_RESULT)?3:2;view.walk_ms=g->training.walk_ms;view.ambient=g->training.ambient_ms;}
 /* Load new compact variants at entry, never during result-frame updates. */
 for(unsigned i=0;i<2;i++){char path[320];snprintf(path,sizeof(path),"%s/items/%s-small.bin",assets,i?"capture-ball":"hatch-boost");pp_image_preload(path);}
 view.root=shape(parent,0,0,360,360,0x132d37,255);lv_obj_set_style_radius(view.root,0,0);lv_obj_add_event_cb(view.root,deleted,LV_EVENT_DELETE,NULL);
 view.background=lv_image_create(view.root);lv_obj_set_pos(view.background,0,0);
 view.content=shape(view.root,0,0,360,360,0,0);lv_obj_set_style_radius(view.content,0,0);show();
 lv_obj_t *progress=lv_label_create(view.root);lv_obj_set_pos(progress,54,267);lv_obj_set_width(progress,252);lv_obj_set_style_text_align(progress,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_font(progress,&pp_font_12,0);lv_obj_set_style_text_color(progress,lv_color_hex(0xd4dbb8),0);lv_obj_set_user_data(view.root,progress);
 tick(NULL);
}
void pp_training_view_screen(bool on){view.on=on;view.last=lv_tick_get();if(view.root&&view.mode==3)pp_battle_view_resync_clock();}
const pp_training *pp_training_view_probe(void){return &session;}
unsigned pp_training_view_mode(void){return view.mode;}
pp_training_actor pp_training_view_actor(unsigned i){
 if(i>=session.count||view.mode!=2)return (pp_training_actor){0};
 lv_obj_update_layout(view.content);
 int x,y;bool left;pp_training_position(view.scene,i,session.count,view.walk_ms,&x,&y,&left);
 return (pp_training_actor){.x=lv_obj_get_x(view.pets[i]),.y=lv_obj_get_y(view.pets[i]),.level_x=lv_obj_get_x(view.levels[i]),.level_y=lv_obj_get_y(view.levels[i]),.left=left,.alert=!lv_obj_has_flag(view.alerts[i],LV_OBJ_FLAG_HIDDEN),.source=lv_image_get_src(view.pets[i]),.walk_ms=view.walk_ms};
}
