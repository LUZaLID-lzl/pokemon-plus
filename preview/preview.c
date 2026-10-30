#define _POSIX_C_SOURCE 200809L
#include "battle_view.h"
#include "item_drop.h"
#include "shop_reel.h"
#include "game_ui.h"
#include "home_behavior.h"
#include "home_navigation.h"
#include "scene_layout.h"
#include "scene_nests.h"
#include "scene_view.h"
#include "../tests/hatch_fixture.h"
#include "intro_view.h"
#include "fishing_view.h"
#include "hospital_view.h"
#include "pet_dialogue.h"
LV_FONT_DECLARE(pp_font_12);
LV_FONT_DECLARE(pp_font_18);
#include "training_view.h"
#include "image_assets.h"
#include "moves.h"
#include "warehouse_sort.h"
#include "battle_type_icons.h"
#include "battle_relation.h"
#include "pedometer.h"
#include "pet_animation.h"
#include "touch_input.h"
#include "simulator_window.h"
#include "random_seed.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <wchar.h>
#endif
static pp_game game;
static bool raw_display;
static uint16_t export_frame[360 * 360];
/* A stone stage retains the level gates of all preceding stages. */
static unsigned form_level(unsigned species,unsigned stage) {
  unsigned level=1;
  for(unsigned i=0;i<PP_EVOLUTIONS;i++) {
    const pp_evolution_template *t=&pp_evolutions[i];
    if(t->species==species && t->from_stage<stage && t->level>level)level=t->level;
  }
  return level;
}
static bool form_stone(unsigned species,unsigned stage) {
  if(species==67 && !stage)return true;
  for(unsigned i=0;i<PP_EVOLUTIONS;i++) {
    const pp_evolution_template *t=&pp_evolutions[i];
    if(t->species==species && t->from_stage==stage && !t->level)return true;
  }
  return false;
}
static uint32_t day_now(void) {
  time_t now = time(NULL);
  struct tm t;
#ifdef _WIN32
  localtime_s(&t, &now);
#else
  localtime_r(&now, &t);
#endif
  return (t.tm_year + 1900) * 10000u + (t.tm_mon + 1) * 100u + t.tm_mday;
}
static void clock_now(void) {
  time_t now = time(NULL);
  struct tm t;
  char s[8];
#ifdef _WIN32
  localtime_s(&t, &now);
#else
  localtime_r(&now, &t);
#endif
  strftime(s, sizeof(s), "%H:%M", &t);
  pp_ui_clock(s);
}
static void fixture(void) {
  game.coins = 2000;
  for (unsigned i = 0; i < 3; i++) {
    pp_buy(&game, 5);
    pp_use(&game, 5, 0);
    pp_shake(&game);
    pp_tick(&game, PP_EGG_SECONDS, day_now());
  }
  for (unsigned i = 0; i < game.count; i++) {
    game.pets[i].food = 85;
    game.pets[i].clean = 90;
    game.pets[i].health = 100;
    game.pets[i].mood = 85;
    game.pets[i].energy = 90;
    game.pets[i].sleeping = 0;
  }
}
static void gym_fixture(void) {
  pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);
  game.count=5;
  const unsigned species[]={1,2,3,6,7};
  for(unsigned i=0;i<5;i++) {
    game.pets[i]=game.pets[0];game.pets[i].species=species[i];game.pets[i].energy=100;
    game.growth[i]=(pp_growth){.level=12};
  }
  assert(pp_valid(&game));
}
static int keys(void *unused, SDL_Event *event) {
  (void)unused;
  if (event->type == SDL_KEYDOWN && !event->key.repeat) {
    int k = event->key.keysym.sym;
    if (k == SDLK_ESCAPE)
      pp_ui_open(0);
    if (k == SDLK_k) pp_record_steps(&game,100);
    if (k == SDLK_s)
      pp_shake(&game);
    if (k == SDLK_SPACE)
      pp_tick(&game, 3600, day_now());
    if (k >= SDLK_0 && k <= SDLK_9)
      pp_ui_open(k - SDLK_0);
    pp_ui_refresh();
  }
  return 1;
}
static void shortcut(unsigned id) {
  if(id==PP_SHORTCUT_STEPS)pp_record_steps(&game,100);
  else if(id==PP_SHORTCUT_SHAKE)pp_shake(&game);
  else if(id==PP_SHORTCUT_HOUR)pp_tick(&game,3600,day_now());
  else if(id==PP_SHORTCUT_HOME)pp_ui_open(0);
  else if(id>=100 && id<110)pp_ui_open(id-100);
  pp_ui_refresh();
}
static void shell_pump(unsigned ms) {
  for(unsigned t=0;t<ms;t+=5) {
    pp_shell_poll();lv_tick_inc(5);lv_timer_handler();pp_shell_render();
  }
}
static void shell_tap(int x,int y) {
  SDL_Event e={0};e.type=SDL_MOUSEBUTTONDOWN;e.button.button=SDL_BUTTON_LEFT;e.button.x=x;e.button.y=y;
  SDL_PushEvent(&e);shell_pump(80);
  e.type=SDL_MOUSEBUTTONUP;SDL_PushEvent(&e);shell_pump(80);
}
/* Exercise actual LVGL switches and visibility, rather than a settings mock. */
static unsigned collect_switches(lv_obj_t *parent,lv_obj_t **out,unsigned count) {
  for(unsigned i=0;i<lv_obj_get_child_count(parent);i++) {
    lv_obj_t *child=lv_obj_get_child(parent,i);
    if(lv_obj_check_type(child,&lv_switch_class)){if(count<5)out[count]=child;count++;}
    count=collect_switches(child,out,count);
  }
  return count;
}
static unsigned visible_labels(lv_obj_t *parent,const char *text) {
  unsigned count=0;
  for(unsigned i=0;i<lv_obj_get_child_count(parent);i++) {
    lv_obj_t *child=lv_obj_get_child(parent,i);
    if(!lv_obj_is_visible(child))continue;
    if(lv_obj_check_type(child,&lv_label_class) && !strcmp(lv_label_get_text(child),text))count++;
    count+=visible_labels(child,text);
  }
  return count;
}
static bool check_training_actors(const char *root,const pp_training *t){
 for(unsigned i=0;i<t->count;i++){
  pp_training_actor a=pp_training_view_actor(i);assert(a.level_x==a.x+2&&a.level_y==a.y+45);
  assert(a.alert==(t->phase==PP_TRAINING_ALERT));
  unsigned frame=pp_animation_frame(pp_art_id(&t->battle,i),a.alert?PP_POSE_IDLE:PP_POSE_WALK,a.alert?0:a.walk_ms*3/4+i*170);
  char path[320];snprintf(path,sizeof(path),"S:%s/%s/%u-%u.bin",root,a.left?"pets-left":"pets",pp_art_id(&t->battle,i),frame);
  if(a.source!=pp_image_descriptor(path)){fprintf(stderr,"direction source mismatch %s walk=%u left=%u alert=%u actual=%p expected=%p\n",path,a.walk_ms,a.left,a.alert,a.source,(void *)pp_image_descriptor(path));abort();}
  for(unsigned corner=0;corner<4;corner++){int dx=a.level_x+(corner%2?43:0)-180,dy=a.level_y+(corner/2?15:0)-180;assert(dx*dx+dy*dy<=168*168);}
 }
 return pp_training_view_actor(0).left;
}
static lv_obj_t *label_parent(lv_obj_t *parent,const char *text) {
  for(unsigned i=0;i<lv_obj_get_child_count(parent);i++){
    lv_obj_t *child=lv_obj_get_child(parent,i);
    if(lv_obj_check_type(child,&lv_label_class) && !strcmp(lv_label_get_text(child),text))return parent;
    lv_obj_t *found=label_parent(child,text);if(found)return found;
  }
  return NULL;
}
static unsigned battery_fill_width(lv_obj_t *group) {
  assert(group && lv_obj_get_child_count(group)==4);
  lv_obj_t *fill=lv_obj_get_child(group,2);
  return lv_obj_has_flag(fill,LV_OBJ_FLAG_HIDDEN)?0:(unsigned)lv_obj_get_width(fill);
}
static unsigned battery_bolt_pixels(lv_obj_t *group) {
  lv_refr_now(lv_display_get_default());lv_area_t a;lv_obj_get_coords(group,&a);
  unsigned count=0;const uint16_t gold=(31u<<11)|(60u<<5)|21u;
  for(int y=a.y1+5;y<=a.y1+11;y++)for(int x=a.x1+5;x<=a.x1+17;x++)
    count+=export_frame[y*360+x]==gold;
  return count;
}
/* Encounter actions must fit the actual ball, including the 2px pixel shadow. */
static unsigned check_wild_ball_controls(lv_obj_t *parent) {
  unsigned count=0;
  for(unsigned i=0;i<lv_obj_get_child_count(parent);i++) {
    lv_obj_t *child=lv_obj_get_child(parent,i);if(!lv_obj_is_visible(child))continue;
    if(lv_obj_check_type(child,&lv_button_class) && lv_obj_get_child_count(child)) {
      const char *s=lv_label_get_text(lv_obj_get_child(child,0));
      if(!strcmp(s,"放走")||!strcmp(s,"对战")) {
        lv_area_t ball,a;lv_obj_get_coords(lv_obj_get_parent(child),&ball);lv_obj_get_coords(child,&a);
        assert(ball.x1==38 && ball.y1==34 && lv_area_get_width(&ball)==284);
        int xs[]={a.x1,a.x2},ys[]={a.y1,a.y2+2};
        for(unsigned x=0;x<2;x++)for(unsigned y=0;y<2;y++){int dx=xs[x]-180,dy=ys[y]-176;assert(dx*dx+dy*dy<=139*139);}
        count++;
      }
    }
    count+=check_wild_ball_controls(child);
  }
  return count;
}
static lv_obj_t *visible_image(lv_obj_t *parent,const void *source) {
  for(unsigned i=0;i<lv_obj_get_child_count(parent);i++) {
    lv_obj_t *child=lv_obj_get_child(parent,i);
    if(!lv_obj_is_visible(child))continue;
    if(lv_obj_check_type(child,&lv_image_class) && lv_image_get_src(child)==source)return child;
    lv_obj_t *found=visible_image(child,source);if(found)return found;
  }
  return NULL;
}
static unsigned battle_type_images(lv_obj_t *parent,unsigned art[2],unsigned *found) {
  unsigned count=0;
  for(unsigned n=0;n<lv_obj_get_child_count(parent);n++) {
    lv_obj_t *o=lv_obj_get_child(parent,n);if(!lv_obj_is_visible(o))continue;
    if(lv_obj_check_type(o,&lv_image_class)) {
      const void *src=lv_image_get_src(o);
      if(lv_image_src_get_type(src)==LV_IMAGE_SRC_VARIABLE) {
        const lv_image_dsc_t *im=src;
        if(im->header.w==16&&im->header.h==16&&im->header.cf==LV_COLOR_FORMAT_ARGB8888) {
          lv_area_t a;lv_obj_get_coords(o,&a);bool matched=false;
          for(unsigned side=0;side<2;side++)for(unsigned i=0;i<2;i++) {
            int type=pp_pet_types[art[side]][i];if(type<0)continue;
            unsigned total=pp_pet_types[art[side]][1]<0?1:2;
            int x=side?224+(int)i*18:136-(int)total*18+2+(int)i*18;
            if(a.x1==x&&a.y1==64&&!memcmp(im->data,pp_battle_type_pixels[type],1024)) {
              assert(!(*found&(1u<<(side*2+i))));*found|=1u<<(side*2+i);matched=true;
            }
          }
          assert(matched);count++;
        }
      }
    }
    count+=battle_type_images(o,art,found);
  }
  return count;
}
static void check_battle_types(void) {
  pp_battle_view_state view=pp_battle_view_probe();unsigned found=0,expected=0,mask=0;
  for(unsigned side=0;side<2;side++)for(unsigned i=0;i<2;i++)if(pp_pet_types[view.art[side]][i]>=0){expected++;mask|=1u<<(side*2+i);}
  assert(battle_type_images(lv_screen_active(),view.art,&found)==expected&&found==mask);
  assert(!visible_labels(lv_screen_active(),"VS")&&!visible_labels(lv_screen_active(),"免疫"));
  for(unsigned side=0;side<2;side++) {
    unsigned factor=pp_gym_effectiveness(&game,side,view.slot),power=pp_battle_move_power(&game,side,view.slot);
    assert(view.relation_factor[side]==factor&&view.relation_kind[side]==pp_relation_kind(factor,power));
  }
}
/* Keep every visible action target inside the round display with 12px to spare.
 * Check bounds rather than a rectangular screenshot alone: the watch clips arcs. */
static void check_safe_buttons(lv_obj_t *parent) {
  for (unsigned i=0;i<lv_obj_get_child_count(parent);i++) {
    lv_obj_t *child=lv_obj_get_child(parent,i);
    if (!lv_obj_is_visible(child)) continue;
    if (lv_obj_check_type(child,&lv_button_class)) {
      lv_area_t a; lv_obj_get_coords(child,&a);
      int xs[]={a.x1,a.x2}, ys[]={a.y1,a.y2};
      for (unsigned x=0;x<2;x++) for (unsigned y=0;y<2;y++) {
        int dx=xs[x]-180,dy=ys[y]-180;
        if (dx*dx+dy*dy>168*168) {
          fprintf(stderr,"Button outside circular safe area: page %u, (%d,%d)-(%d,%d)\n",pp_ui_page(),a.x1,a.y1,a.x2,a.y2);
          exit(1);
        }
      }
    }
    check_safe_buttons(child);
  }
}
static void screenshot(const char *path) {
  lv_obj_update_layout(lv_screen_active());
  check_safe_buttons(lv_screen_active());
  bool overlay = false;
  for (unsigned i=0;i<lv_obj_get_child_count(lv_layer_top());i++)
    if (lv_obj_is_visible(lv_obj_get_child(lv_layer_top(),i))) overlay = true;
  if (raw_display || overlay) {
    lv_refr_now(lv_display_get_default());
    FILE *f=fopen(path,"wb"); if (!f) exit(1);
    fprintf(f,"P6\n360 360\n255\n");
    for(unsigned i=0;i<360*360;i++) {
      uint16_t c=export_frame[i];
      uint8_t rgb[]={((c>>11)&31)*255/31,((c>>5)&63)*255/63,(c&31)*255/31};
      int x=i%360-180,y=i/360-180;
      if(!raw_display && x*x+y*y>180*180)memset(rgb,0,3);
      fwrite(rgb,1,3,f);
    }
    fclose(f);return;
  }
  lv_draw_buf_t *snap =
      lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_RGB888);
  if (!snap) {
    fprintf(stderr, "Snapshot failed\n");
    exit(1);
  }
  FILE *f = fopen(path, "wb");
  if (!f)
    exit(1);
  fprintf(f, "P6\n360 360\n255\n");
  for (unsigned y = 0; y < 360; y++)
    for (unsigned x = 0; x < 360; x++) {
      const uint8_t *p = snap->data + y * snap->header.stride + x * 3;
      uint8_t rgb[] = {p[2], p[1], p[0]};
      if ((int)(x - 180) * (int)(x - 180) + (int)(y - 180) * (int)(y - 180) >
          180 * 180)
        memset(rgb, 0, 3);
      fwrite(rgb, 1, 3, f);
    }
  fclose(f);
  lv_draw_buf_destroy(snap);
}
/* Export uses an offscreen LVGL display, independent of desktop window events. */
static uint8_t export_pixels[360 * 360 * 2];
static bool export_rows[360];
static struct { uint64_t pixels,flushes,full,frames; } scene_metrics;
static bool scene_measuring;
static void export_flush(lv_display_t *display,const lv_area_t *area,uint8_t *pixels) {
  unsigned width=lv_area_get_width(area);
  if(scene_measuring) {
    scene_metrics.pixels+=(uint64_t)width*lv_area_get_height(area);
    scene_metrics.flushes++;
    if(width==360 && lv_area_get_height(area)==360)scene_metrics.full++;
    if(lv_display_flush_is_last(display))scene_metrics.frames++;
  }
  for(int y=area->y1;y<=area->y2;y++) {
    export_rows[y]=true;
    memcpy(export_frame+y*360+area->x1,pixels+(y-area->y1)*width*2,width*2);
  }
  lv_display_flush_ready(display);
}
static bool pressed;
static lv_point_t point;
static void pointer(lv_indev_t *in, lv_indev_data_t *data) {
  (void)in;
  data->point = point;
  data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
static pp_touch_input queued_input;
static void queued_pointer(lv_indev_t *in,lv_indev_data_t *data) {
  bool more;
  pp_touch_point p=pp_touch_read(&queued_input,&more);
  data->point=(lv_point_t){p.x,p.y};
  data->state=p.down?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
  data->continue_reading=more;
  if(p.down)lv_obj_update_layout(lv_display_get_screen_active(lv_indev_get_display(in)));
}
/* A complete physical tap arrives without any intervening LVGL input read. */
static void queue_tap(int x,int y) {
  pp_touch_sample(&queued_input,true,x,y);
  pp_touch_sample(&queued_input,false,0,0);
}
static void pump(unsigned ms) {
  for (unsigned i = 0; i < ms; i += 10) {
    lv_tick_inc(ms - i < 10 ? ms - i : 10);
    lv_timer_handler();
  }
}
/* Real shared renderer, full-capacity partial buffer as on the ESP32. Counts
 * transferred pixels, not desktop FPS; excludes entry/asset loading costs. */
static void scene_profile(const char *path) {
  FILE *out=fopen(path,"w");assert(out);
  fprintf(out,"{\"durationMs\":3000,\"samples\":[");
  for(unsigned scene=0;scene<13;scene++)for(unsigned fx=0;fx<2;fx++) {
    game.scene=scene;game.scenes|=1u<<scene;
    pp_ui_scene_fx_preview(true);pp_ui_scene_fx_enabled(fx!=0);
    pp_ui_open(0);pump(1200);
    memset(&scene_metrics,0,sizeof(scene_metrics));scene_measuring=true;
    uint64_t start=SDL_GetPerformanceCounter();for(unsigned ms=0;ms<3000;ms+=33)pump(ms+33>3000?3000-ms:33);
    uint64_t ticks=SDL_GetPerformanceCounter()-start;scene_measuring=false;
    fprintf(out,"%s{\"scene\":%u,\"effects\":%s,\"frames\":%llu,\"pixels\":%llu,\"flushes\":%llu,\"fullFrames\":%llu,\"hostMs\":%.3f}",
      scene||fx?",":"",scene,fx?"true":"false",(unsigned long long)scene_metrics.frames,
      (unsigned long long)scene_metrics.pixels,(unsigned long long)scene_metrics.flushes,
      (unsigned long long)scene_metrics.full,(double)ticks*1000/SDL_GetPerformanceFrequency());
  }
  fprintf(out,"]}\n");fclose(out);puts("PASS: 13-scene effects on/off real-render pixel/flush profile (host time is not device FPS)");
}
static void nest_walk_smoke(void){
 for(unsigned scene=0;scene<13;scene++){
  game.scene=scene;game.scenes|=1u<<scene;game.egg_end=game.seconds+7200;
  pp_ui_open(0);pump(100);pp_image_stats images=pp_image_probe();uint32_t checksum=pp_checksum(&game);
  unsigned moved[5]={0};pp_home_state previous[5];
  for(unsigned p=0;p<game.count;p++)previous[p]=pp_ui_home_probe(p);
  for(unsigned frame=0;frame<380;frame++){
   if(frame==60)assert(pp_ui_home_interaction(PP_HOME_CALL,0,0,pp_scene_nest_positions[scene][0],pp_scene_nest_positions[scene][1]-16));
   if(frame==180)pp_ui_home_interaction(PP_HOME_STROLL,1,2,0,0);
   if(frame==290)pp_ui_home_interaction(PP_HOME_HOP,0,0,0,0);
   pump(100);
   for(unsigned p=0;p<game.count;p++){
    pp_home_state h=pp_ui_home_probe(p);
    assert(!pp_home_nest_blocked(scene,h.x,h.ground_y));
    assert(h.nest_front==(h.ground_y>=pp_scene_nest_positions[scene][1]&&(scene!=12||pp_lake_in_front(h.ground_y))));
    if(abs(h.x-previous[p].x)>12||abs(h.ground_y-previous[p].ground_y)>12){
     fprintf(stderr,"Nest continuity scene%u frame%u pet%u kind%u %d,%d -> %d,%d\n",scene,frame,p,h.behavior,previous[p].x,previous[p].ground_y,h.x,h.ground_y);abort();
    }
    moved[p]+=h.x!=previous[p].x||h.ground_y!=previous[p].ground_y;previous[p]=h;
   }
  }
  for(unsigned p=0;p<game.count;p++)assert(moved[p]>30);
  assert(pp_image_probe().file_reads==images.file_reads&&pp_checksum(&game)==checksum);
 }
 puts("PASS: all13 real LVGL scenes / five pets / 38s patrol, nest-target call, stroll and hop; ground collision, actual nest depth, continuity, zero image reads/save/RNG changes");
}
static void scene_damage_smoke(void) {
  uint16_t *partial=malloc(sizeof(export_frame));assert(partial);
  uint32_t rng=game.rng;
  for(unsigned scene=0;scene<13;scene++) {
    game.scene=scene;game.scenes|=1u<<scene;
    pp_ui_scene_fx_preview(true);pp_ui_scene_fx_enabled(true);pp_ui_open(0);pump(33);
    pp_image_stats before=pp_image_probe();
    unsigned moved=0;pp_home_state previous=pp_ui_home_probe(0);
    for(unsigned frame=0;frame<100;frame++) {
      if(frame==60)pp_ui_scene_fx_enabled(false);
      if(frame==75)pp_ui_scene_fx_enabled(true);
      pump(33);lv_refr_now(lv_display_get_default());
      memcpy(partial,export_frame,sizeof(export_frame));
      lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());
      if(memcmp(partial,export_frame,sizeof(export_frame))) {
        fprintf(stderr,"Scene partial/full mismatch scene=%u frame=%u\n",scene,frame);abort();
      }
      pp_home_state pet=pp_ui_home_probe(0);
      moved+=pet.x!=previous.x || pet.y!=previous.y;previous=pet;
    }
    assert(moved>5 && pp_image_probe().file_reads==before.file_reads);
    pp_ui_open(1);pump(33);pp_ui_open(0);pump(33);
  }
  assert(game.rng==rng);
  free(partial);puts("PASS: 1300 actual partial frames equal full repaint, all13 scenes, effects stop/restart/menu return, pet movement and zero extra image reads/RNG changes");
}
static void tap(lv_indev_t *in, int x, int y, unsigned ms) {
  lv_obj_update_layout(lv_screen_active());
  point = (lv_point_t){x, y};
  pressed = true;
  lv_indev_read(in);
  pump(ms);
  pressed = false;
  lv_indev_read(in);
  pump(100);
}
/* Bag slots are four columns with fixed row starts. */
static void tap_bag_slot(lv_indev_t *in,unsigned slot,unsigned ms) {
  assert(slot<12);tap(in,90+(slot%4)*60,134+(slot/4)*62,ms);
}
static void tap_bag_item(lv_indev_t *in,unsigned item,unsigned ms) {
  unsigned slot=0;
  for(unsigned i=0;i<item;i++)slot+=i<PP_SHOP_ITEMS?(pp_item_enabled(i)&&pp_item_count(&game,i)>0):game.legend.eggs[i-PP_SHOP_ITEMS]>0;
  if(slot>=12)tap(in,320,180,80);
  tap_bag_slot(in,slot%12,ms);
}
/* Shared APP geometry: 9 square tiles, then the remaining 3 entries. */
static void tap_app(lv_indev_t *in,unsigned entry){
  assert(pp_ui_page()==1 && entry<14);
  unsigned page=entry/9,slot=entry%9;
  if(pp_ui_app_page()!=page)tap(in,320,180,80);
  unsigned cols=page&&slot/3==1?2:3;
  tap(in,page?(360-(int)cols*84+20)/2+(int)(slot%3)*84+32:106+(int)(slot%3)*74,page?136+(int)(slot/3)*84:100+(int)(slot/3)*72,80);
}
static void check_travel_return(lv_indev_t *in,int x) {
  memset(export_rows,0,sizeof(export_rows));
  tap(in,x,304,80);assert(pp_ui_page()==1);
  for(unsigned y=0;y<360;y++)assert(export_rows[y]);
  /* Drain the current frame before comparing it with a forced repaint. */
  lv_refr_now(lv_display_get_default());
  static uint16_t returned[360*360];
  memcpy(returned,export_frame,sizeof(returned));
  lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());
  assert(!memcmp(returned,export_frame,sizeof(returned)));
  tap(in,180,312,80);assert(pp_ui_page()==0);
}
static void check_partial_frame(void){
  static uint16_t partial[360*360];
  lv_refr_now(lv_display_get_default());memcpy(partial,export_frame,sizeof(partial));
  lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());
  assert(!memcmp(partial,export_frame,sizeof(partial)));
}
/* All encounter entry points share the native release renderer. Test actual
 * entry state and frames rather than only checking the shared implementation. */
static void check_battle_release(void) {
  pp_battle_view_state entry=pp_battle_view_probe();
  assert(entry.active && entry.entrance_mask==3 && !entry.casting);
  assert(!entry.pet_opacity[0] && !entry.pet_opacity[1]);
  unsigned reads=pp_image_probe().file_reads;
  uint32_t before=pp_checksum(&game);
  unsigned clock=entry.entrance_ms;
  pp_ui_screen(false);pump(1000);
  assert(pp_battle_view_probe().entrance_ms==clock);
  pp_ui_screen(true);pump(450);
  entry=pp_battle_view_probe();
  assert(entry.entrance_mask==3 && !entry.casting);
  assert(entry.pet_opacity[0]>0 && entry.pet_opacity[0]<255);
  assert(entry.pet_opacity[1]>0 && entry.pet_opacity[1]<255);
  pump(650);entry=pp_battle_view_probe();
  assert(!entry.entrance_mask && entry.pet_opacity[0]==255 && entry.pet_opacity[1]==255);
  assert(pp_checksum(&game)==before && pp_image_probe().file_reads==reads);
}
static void wild_fixture(const char *phase) {
  pp_init(&game,42);pp_choose_starter(&game,1);
  /* Squirtle walkthrough belongs to the coast's regional pool. */
  game.scene=2;game.scenes|=1u<<2;
  game.growth[0].level=20;game.pets[0].health=100;
  unsigned dex=0;while(dex<PP_DEX_COUNT && pp_pokedex[dex].art!=3)dex++;
  game.wild=(pp_wild_state){.phase=PP_WILD_PENDING,.dex=dex,.level=8,.scene=game.scene,.escape_at=game.seconds+15};
  if(!strcmp(phase,"alert")){game.wild.next_at=PP_WILD_STEP_MARK;game.wild.escape_at=game.seconds+PP_WILD_NOTICE_SECONDS;return;}
  if(!strcmp(phase,"pending"))return;
  if(!strcmp(phase,"duplicate"))game.pets[0].species=3;
  if(!strcmp(phase,"full")) {
    for(unsigned p=1;p<5;p++){game.pets[p]=game.pets[0];game.growth[p]=game.growth[0];}game.count=5;
    for(unsigned p=0;p<11;p++){game.reserves[p]=game.pets[0];game.reserve_growth[p]=game.growth[0];}game.reserve_count=11;
  }
  assert(pp_wild_start(&game,0)==PP_OK);
  if(!strcmp(phase,"battle"))return;
  if(!strcmp(phase,"lost")){game.battle_hp=1;pp_tick(&game,3,0);}
  else {game.enemy_hp=1;pp_tick(&game,3,0);}
  if(!strcmp(phase,"success")){game.balls=1;assert(pp_wild_capture(&game)==PP_OK);}
  if(!strcmp(phase,"caught-ball"))game.balls=1;
  assert(pp_valid(&game));
}
int main(int argc, char **argv) {
  bool conflict_smoke=false,conflict_preview=false;
  bool achievements_smoke=false, pat_smoke=false, interaction_smoke=false;
  unsigned interaction_kind=0;
  bool reproducible=false,seed_set=false;
  uint32_t seed=0;
  const char *hatch_audit=NULL;
  bool shell_smoke=false, shell=false;
  const char *shell_output=NULL,*scene_profile_path=NULL;
  const char *root = NULL, *shot = NULL, *activity = NULL, *frames_dir = NULL, *clock_override = NULL;
  unsigned collection_quality=PP_QUALITY_ALL,gift_quality=PP_QUALITY_ALL;
  unsigned travel_destination=PP_TRAVEL_DESTINATIONS;
  unsigned evo_branch_choice=1;
  unsigned catalog_family=PP_MOVE_FAMILY_COUNT;
  const char *catalog_frames=NULL;
  bool warehouse_smoke=false, warehouse_film=false;
  bool catalog_actor_check=false, gift_smoke=false, gift_preview=false, menu_smoke=false;
  const char *menu_state=NULL;
  unsigned page = 0, species = 0, at_ms = 0, frame_count = 32, move_slot = 0, scene = 0, growth_stage = 0, frame_step = 125;
  bool cerulean_smoke=false, vermilion_smoke=false, celadon_smoke=false, fuchsia_smoke=false, saffron_smoke=false, cinnabar_smoke=false, viridian_smoke=false;
  bool gym_map_smoke=false, notice_smoke=false, wild_smoke=false;
  bool battle_types_smoke=false;bool immunity_smoke=false;bool completion_smoke=false;bool training_growth_smoke=false;bool training_profile=false;bool training_smoke=false;const char *training_phase=NULL;unsigned training_scene=0,training_party=2;
  bool hospital_smoke=false;const char *hospital_phase=NULL;
  bool fishing_smoke=false;const char *fishing_phase=NULL;
  const char *wild_phase=NULL,*legend_phase=NULL;unsigned legend_index=0;bool legend_smoke=false;
  const char *notice_text=NULL;
  bool long_home_smoke=false;bool item_drop_smoke=false;bool item_smoke=false;bool hatch_smoke=false, gym_smoke=false, intro_smoke=false, intro_seen=false, touch_smoke=false, home_smoke=false;
  bool scene_set=false,settings_smoke=false,scene_damage=false;
  unsigned display_flags=PP_UI_DISPLAY_DEFAULT;
  int battery_percent=72;bool battery_charging=false;
  bool demo = false, smoke = false, ambient_smoke = false, story_smoke = false, evolution_smoke = false, travel_smoke = false;
  for (int i = 1; i < argc; i++) {
    size_t length=strlen(argv[i]);
    if(!strcmp(argv[i],"--smoke") || (length>6 && !strncmp(argv[i],"--",2) && !strcmp(argv[i]+length-6,"-smoke")))reproducible=true;
    if(!strcmp(argv[i],"--item-drop-smoke"))item_drop_smoke=true;
    else if(!strcmp(argv[i],"--long-home-smoke") || !strcmp(argv[i],"--boot-home-smoke"))long_home_smoke=true;
    if(!strcmp(argv[i],"--interaction-smoke"))interaction_smoke=true;
    if(!strcmp(argv[i],"--home-interaction")&&i+1<argc){
      const char *name=argv[++i];const char *names[]={"none","pat","call","explore","hop","greet","stroll","rest","welcome"};
      for(unsigned k=1;k<=8;k++)if(!strcmp(name,names[k]))interaction_kind=k;
      if(!interaction_kind)return 2;
    }
    if(!strcmp(argv[i],"--home-conflict-smoke"))conflict_smoke=true;
    else if(!strcmp(argv[i],"--pat-smoke"))pat_smoke=true;
    else if(!strcmp(argv[i],"--shell-smoke"))shell_smoke=true;
    else if(!strcmp(argv[i],"--achievements-smoke"))achievements_smoke=true;
    else if(!strcmp(argv[i],"--seed") && i+1<argc) {
      char *end;errno=0;unsigned long parsed=strtoul(argv[++i],&end,10);
      if(!argv[i][0] || argv[i][0]=='-' || *end || errno || parsed>UINT32_MAX)return 2;
      seed=(uint32_t)parsed;seed_set=true;
    }
    else if(!strcmp(argv[i],"--hatch-audit") && i+1<argc) {
      hatch_audit=argv[++i];
      if(strcmp(hatch_audit,"natural") && strcmp(hatch_audit,"boost"))return 2;
    }
    else if(!strcmp(argv[i],"--shell-output") && i+1<argc)shell_output=argv[++i];
    else if(!strcmp(argv[i],"--scene-profile") && i+1<argc){scene_profile_path=argv[++i];reproducible=true;}
    else if(!strcmp(argv[i],"--scene-damage-smoke"))scene_damage=true;
    else if(!strcmp(argv[i],"--warehouse-smoke"))warehouse_smoke=true;
    else if(!strcmp(argv[i],"--warehouse-film"))warehouse_film=true;
    else if (!strcmp(argv[i], "--assets") && i + 1 < argc)
      root = argv[++i];
    else if (!strcmp(argv[i], "--screenshot") && i + 1 < argc)
      shot = argv[++i];
    else if (!strcmp(argv[i], "--frames-dir") && i + 1 < argc)
      frames_dir = argv[++i];
    else if (!strcmp(argv[i], "--species") && i + 1 < argc)
      species = (unsigned)atoi(argv[++i]) % PP_SPECIES;
    else if (!strcmp(argv[i], "--catalog-family") && i + 1 < argc)
      catalog_family=(unsigned)atoi(argv[++i]);
    else if (!strcmp(argv[i], "--catalog-frames") && i + 1 < argc)
      catalog_frames=argv[++i];
    else if (!strcmp(argv[i], "--catalog-actor-check"))
      catalog_actor_check=true;
    else if (!strcmp(argv[i], "--evolution-branch") && i+1<argc)
      evo_branch_choice=(unsigned)atoi(argv[++i]);
    else if (!strcmp(argv[i], "--growth-stage") && i + 1 < argc)
      growth_stage = (unsigned)atoi(argv[++i]) % 4;
    else if (!strcmp(argv[i], "--frame-step") && i + 1 < argc)
      frame_step = (unsigned)atoi(argv[++i]);
    else if (!strcmp(argv[i], "--at-ms") && i + 1 < argc)
      at_ms = (unsigned)atoi(argv[++i]);
    else if (!strcmp(argv[i], "--frame-count") && i + 1 < argc)
      frame_count = (unsigned)atoi(argv[++i]);
    else if (!strcmp(argv[i], "--collection-quality") && i + 1 < argc)
      collection_quality=(unsigned)atoi(argv[++i]) % 7;
    else if (!strcmp(argv[i], "--page") && i + 1 < argc)
      page = atoi(argv[++i]);
    else if (!strcmp(argv[i], "--clock") && i + 1 < argc)
      clock_override = argv[++i];
    else if (!strcmp(argv[i], "--scene") && i + 1 < argc)
      {scene = (unsigned)atoi(argv[++i]) % PP_SCENES;scene_set=true;}
    else if (!strcmp(argv[i], "--travel-destination") && i + 1 < argc)
      travel_destination=(unsigned)atoi(argv[++i]) % PP_TRAVEL_DESTINATIONS;
    else if (!strcmp(argv[i], "--move-slot") && i + 1 < argc)
      move_slot = (unsigned)atoi(argv[++i]) % PP_MOVE_SLOTS;
    else if (!strcmp(argv[i], "--activity") && i + 1 < argc)
      activity = argv[++i];
    else if (!strcmp(argv[i], "--notice") && i+1<argc)
      notice_text=argv[++i];
    else if(!strcmp(argv[i],"--menu-smoke"))menu_smoke=true;
    else if(!strcmp(argv[i],"--battle-types-smoke"))battle_types_smoke=true;
    else if(!strcmp(argv[i],"--immunity-smoke"))immunity_smoke=true;
    else if(!strcmp(argv[i],"--completion-smoke"))completion_smoke=training_growth_smoke=true;
    else if(!strcmp(argv[i],"--training-growth-smoke"))training_growth_smoke=true;
    else if(!strcmp(argv[i],"--training-smoke"))training_smoke=true;
    else if(!strcmp(argv[i],"--training-profile"))training_profile=training_smoke=reproducible=true;
    else if(!strcmp(argv[i],"--training") && i+1<argc)training_phase=argv[++i];
    else if(!strcmp(argv[i],"--training-party") && i+1<argc)training_party=(unsigned)atoi(argv[++i]);
    else if(!strcmp(argv[i],"--training-scene") && i+1<argc)training_scene=(unsigned)atoi(argv[++i])%6;
    else if(!strcmp(argv[i],"--hospital-smoke"))hospital_smoke=true;
    else if(!strcmp(argv[i],"--hospital") && i+1<argc)hospital_phase=argv[++i];
    else if(!strcmp(argv[i],"--fishing-smoke"))fishing_smoke=true;
    else if(!strcmp(argv[i],"--fishing") && i+1<argc)fishing_phase=argv[++i];
    else if(!strcmp(argv[i],"--menu-scene-state") && i+1<argc)menu_state=argv[++i];
    else if(!strcmp(argv[i],"--gift"))gift_preview=true;
    else if(!strcmp(argv[i],"--gift-quality") && i+1<argc){gift_preview=true;gift_quality=(unsigned)atoi(argv[++i])%5;}
    else if(!strcmp(argv[i],"--gift-smoke")){gift_smoke=true;gift_preview=true;}
    else if(!strcmp(argv[i],"--legend") && i+1<argc)legend_phase=argv[++i];
    else if(!strcmp(argv[i],"--legend-index") && i+1<argc)legend_index=(unsigned)atoi(argv[++i])%PP_LEGEND_COUNT;
    else if(!strcmp(argv[i],"--legend-smoke"))legend_smoke=true;
    else if (!strcmp(argv[i], "--wild-smoke"))wild_smoke=true;
    else if (!strcmp(argv[i], "--wild") && i+1<argc)wild_phase=argv[++i];
    else if (!strcmp(argv[i], "--notice-smoke"))
      notice_smoke=true;
    else if (!strcmp(argv[i], "--demo"))
      demo = true;
    else if (!strcmp(argv[i], "--scene-still"))
      pp_ui_scene_motion(false);
    else if (!strcmp(argv[i], "--scene-fx-off"))
      pp_ui_scene_fx_enabled(false);
    else if (!strcmp(argv[i], "--ambient"))
      pp_ui_scene_fx_preview(true);
    else if (!strcmp(argv[i], "--viridian-smoke")) {gym_smoke=true;viridian_smoke=true;}
    else if (!strcmp(argv[i], "--cinnabar-smoke")) {gym_smoke=true;cinnabar_smoke=true;}
    else if (!strcmp(argv[i], "--saffron-smoke")) {gym_smoke=true;saffron_smoke=true;}
    else if (!strcmp(argv[i], "--fuchsia-smoke")) {gym_smoke=true;fuchsia_smoke=true;}
    else if (!strcmp(argv[i], "--celadon-smoke")) {gym_smoke=true;celadon_smoke=true;}
    else if (!strcmp(argv[i], "--vermilion-smoke")) {gym_smoke=true;vermilion_smoke=true;}
    else if (!strcmp(argv[i], "--cerulean-smoke")) {gym_smoke=true;cerulean_smoke=true;}
    else if (!strcmp(argv[i], "--gym-map-smoke")) {gym_map_smoke=true;}
    else if (!strcmp(argv[i], "--gym-smoke"))
      gym_smoke=true;
    else if (!strcmp(argv[i], "--battle-story-smoke"))
      story_smoke = true;
    else if (!strcmp(argv[i], "--evolution-smoke"))
      evolution_smoke = true;
    else if (!strcmp(argv[i], "--travel-smoke"))
      travel_smoke = true;
    else if (!strcmp(argv[i], "--ambient-smoke")) {
      ambient_smoke = true;
      pp_ui_scene_fx_preview(true);
    }
    else if (!strcmp(argv[i],"--settings-smoke"))settings_smoke=true;
    else if (!strcmp(argv[i],"--battery") && i+1<argc)battery_percent=atoi(argv[++i]);
    else if (!strcmp(argv[i],"--battery-charging"))battery_charging=true;
    else if (!strcmp(argv[i],"--hud-display") && i+1<argc)display_flags=(unsigned)atoi(argv[++i]);
    else if (!strcmp(argv[i], "--raw-display"))
      raw_display = true;
    else if (!strcmp(argv[i], "--home-smoke"))
      home_smoke=true;
    else if (!strcmp(argv[i], "--touch-smoke"))
      touch_smoke=true;
    else if (!strcmp(argv[i], "--item-smoke"))item_smoke=true;
    else if (!strcmp(argv[i], "--hatch-smoke"))
      hatch_smoke=true;
    else if (!strcmp(argv[i], "--intro-smoke"))
      intro_smoke=true;
    else if (!strcmp(argv[i], "--intro-seen"))
      intro_seen=true;
    else if (!strcmp(argv[i], "--smoke"))
      smoke = true;
  }
  if(achievements_smoke)activity="achievement-special";
  if(catalog_frames) {
    /* Use a valid throwaway simulation; only the offline actor/move view is
     * overridden. Pending families never enter the runtime roster or save. */
    if(catalog_family>=PP_MOVE_FAMILY_COUNT || !activity || strcmp(activity,"battle") || (!shot && !frames_dir))return 2;
    char path[640];snprintf(path,sizeof(path),"%s/profile.bin",catalog_frames);
    uint16_t durations[6][8],once_mask=0;
    FILE *profile=fopen(path,"rb");
    if(!profile)return 2;
    bool valid=fread(durations,sizeof(durations),1,profile)==1 && fread(&once_mask,sizeof(once_mask),1,profile)==1;
    fclose(profile);
    if(!valid)return 2;
    for(unsigned a=0;a<6;a++)for(unsigned f=0;f<8;f++)if(!durations[a][f])return 2;
    pp_battle_view_catalog_preview(catalog_family,catalog_frames,durations,once_mask,catalog_actor_check);
    species=1;
  }
  reproducible=reproducible || demo || shot || frames_dir || activity || notice_text || menu_state || wild_phase || legend_phase || gift_preview;
  if(!seed_set)seed=pp_simulator_seed(reproducible);
  pp_init(&game, seed);
  pp_tick(&game, 0, day_now());
  if(hatch_audit) {
    /* Headless audit of the normal startup seed and actual purchased eggs. */
    assert(pp_choose_starter(&game,1)==PP_OK);game.coins=100000;
    printf("{\"seed\":%lu,\"pets\":[",(unsigned long)seed);
    for(unsigned n=0;n<4;n++) {
      if(!strcmp(hatch_audit,"boost"))assert(pp_test_hatch(&game)==PP_OK);
      else {
        assert(pp_buy(&game,5)==PP_OK && pp_use(&game,5,0)==PP_OK && pp_shake(&game)==PP_OK);
        pp_tick(&game,(unsigned)(game.egg_end-game.seconds),day_now());
      }
      assert(game.count==n+2 && pp_valid(&game));
      printf("%s%u",n?",":"",game.pets[n+1].species);
    }
    puts("]}");pp_dispose(&game);return 0;
  }
  conflict_preview=activity&&!strcmp(activity,"home-conflict");
  if (demo)
    fixture();
  if(training_growth_smoke || training_smoke || training_phase){game.coins=5000;pp_choose_starter(&game,1);for(unsigned n=0;n<9;n++)assert(pp_test_hatch(&game)==PP_OK);for(unsigned n=0;n<game.count;n++){game.growth[n].level=training_profile?training_scene*10+5:30;game.pets[n].health=game.pets[n].energy=90;}for(unsigned n=0;n<game.reserve_count;n++)game.reserve_growth[n].level=training_profile?training_scene*10+5:30;game.evolution_declined=(1u<<game.count)-1;game.collected[PP_PURE_HEART/8]|=1u<<(PP_PURE_HEART%8);page=training_smoke?1:33;intro_seen=true;}
  if(hospital_smoke || hospital_phase){game.coins=5000;pp_choose_starter(&game,1);for(unsigned n=0;n<6;n++)assert(pp_test_hatch(&game)==PP_OK);for(unsigned n=0;n<game.count;n++){game.pets[n].food=20;game.pets[n].health=30;game.pets[n].clean=40;game.pets[n].mood=50;game.pets[n].energy=60;}page=hospital_smoke?1:32;intro_seen=true;}
  if(fishing_smoke || fishing_phase){game.coins=5000;pp_choose_starter(&game,1);for(unsigned n=0;n<12;n++)assert(pp_test_hatch(&game)==PP_OK);assert(pp_fishing_partner_select(&game,0)==PP_OK);if(fishing_phase&&(!strncmp(fishing_phase,"dex-",4)||!strcmp(fishing_phase,"detail-last")))for(unsigned i=0;i<PP_FISH_COUNT;i++)game.fish_caught[i]=i+1;page=fishing_smoke?1:31;intro_seen=true;}
  game.scene = scene;
  game.pets[0].species = species;
  game.growth[0].stage = growth_stage;
  if(conflict_smoke||conflict_preview){
    pp_dispose(&game);pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);game.count=3;
    for(unsigned i=0;i<3;i++){game.pets[i]=game.pets[0];game.growth[i]=game.growth[0];game.identity[i]=game.identity[0];game.pets[i].species=i;game.identity[i].nature=i?20:3;}
    game.scene=scene;game.scenes|=1u<<scene;page=0;intro_seen=true;
  }
  if(long_home_smoke){pp_choose_starter(&game,1);game.coins=10000;assert(pp_test_hatch(&game)==PP_OK);intro_seen=true;}
  if(item_smoke||item_drop_smoke){pp_choose_starter(&game,1);game.coins=10000;intro_seen=true;}
  if(battle_types_smoke || immunity_smoke){assert(pp_choose_starter(&game,1)==PP_OK);intro_seen=true;page=1;}
  if(hatch_smoke){assert(pp_choose_starter(&game,1)==PP_OK);game.coins=1000;}
  if (story_smoke || evolution_smoke || travel_smoke) {
    game.count = 2;
    game.pets[1] = game.pets[0];
    game.pets[1].species = 1;
  }
  if(settings_smoke){pp_choose_starter(&game,1);game.coins=1234;page=11;}
  if (ambient_smoke) {
    pp_choose_starter(&game, 1);
    game.scene = scene_set?scene:5;game.scenes|=1u<<game.scene;
    page = 0;
  }
  if(home_smoke) {
    pp_choose_starter(&game,1);game.count=3;game.coins=2000;
    for(unsigned i=1;i<3;i++){game.pets[i]=game.pets[0];game.growth[i]=game.growth[0];}
    game.pets[1].species=2;game.pets[2].species=3;game.pets[2].sleeping=1;
  }
  if(scene_profile_path || scene_damage){gym_fixture();page=0;intro_seen=true;}
  game.growth[0].level=training_profile?training_scene*10+5:form_level(species,growth_stage);
  if (activity) {
    bool evo_activity=!strcmp(activity,"evolution-stone") || !strcmp(activity,"evolution") ||
      !strcmp(activity,"evolution-decline") || !strcmp(activity,"evolution-choice");
    if (evo_activity && form_stone(species,growth_stage)) {
      game.count=2;game.pets[1].species=1;game.coins=500;page=4;
    } else if (!strcmp(activity,"heart-collection") || !strcmp(activity,"heart-detail")) {
      pp_choose_starter(&game,1);game.growth[0].level=16;assert(pp_decline_evolution(&game,0)==PP_OK);page=8;
    } else if (!strcmp(activity,"collection-demo") || !strcmp(activity,"collection-detail")) {
      pp_choose_starter(&game,1);page=8;
      for(unsigned id=0;id<PP_STORED_COLLECTION;id++) if(id%7==0 || pp_collectibles[id].quality>=4) game.collected[id/8]|=1u<<(id%8);
    } else if (!strcmp(activity,"warehouse") || !strcmp(activity,"reserves")) {
      pp_dispose(&game);pp_init(&game,42);pp_choose_starter(&game,1);game.coins=100000;
    while(pp_warehouse_count(&game)<17)assert(pp_test_hatch(&game)==PP_OK);
      const char *names[]={"tea","tuan","firelong","star","Gulu","Pipi"};
      for(unsigned n=0;n<11;n++){pp_rename(&game,true,n,names[n%6]);game.reserve_growth[n].level=1+n%5;}
      page=16;
    } else if (!strcmp(activity,"legacy-reserves")) {
      pp_choose_starter(&game,1);game.coins=100000;
      while(game.count+game.reserve_count<PP_PETS+PP_RESERVES) assert(pp_test_hatch(&game)==PP_OK);
      page=16;
    } else if (!strcmp(activity,"settings-random")) {
      game.scenes=(1u<<PP_SCENES)-1;pp_random_scene(&game,true);page=11;
    } else if (!strcmp(activity,"tarot")) {
      pp_tarot(&game);page=10;
    } else if (!strcmp(activity,"pokedex")) {
      pp_init(&game,42);pp_choose_starter(&game,1);page=15;
    } else if (!strcmp(activity,"steps")) {
      pp_record_steps(&game,4321); page=14;
    } else if (!strcmp(activity, "travel") || !strcmp(activity, "travel-ready") || !strcmp(activity, "travel-receipt") || !strcmp(activity,"travel-complete")) {
      if(!strcmp(activity,"travel-complete"))for(unsigned id=0;id<PP_TRAVEL_COLLECTION;id++)game.collected[id/8]|=1u<<(id%8);
      pp_travel(&game, 0);
      /* Only the capture fixture requests a particular destination. */
      if(travel_destination<PP_TRAVEL_DESTINATIONS)
        while(pp_travel_destination(&game)!=travel_destination) {game.seconds++;game.travel_end++;}
      pp_tick(&game, !strcmp(activity,"travel") ? 1801 : PP_TRAVEL_SECONDS, day_now());
      page = 5;
    } else if (!strcmp(activity, "battle")) {
      pp_battle(&game, 0);
      game.battle_round = move_slot;
      page = 6;
    } else if ((!strcmp(activity, "evolution") || !strcmp(activity,"evolution-decline") || !strcmp(activity,"evolution-choice"))) {
      unsigned target = pp_evolution_level(&game,0);
      assert(target>1);
      game.growth[0].level = target-1;
      game.growth[0].xp = pp_xp_needed(target-1)-1;
      pp_gain_xp(&game, 0, 1);
      page = 0;
    } else if (!strcmp(activity, "dig")) {
      pp_dig_start(&game);
      pp_shake(&game);
      pp_shake(&game);
      page = 7;
    } else if (!strcmp(activity, "egg")) {
      game.inventory[5]++;
      pp_use(&game, 5, 0);
      page = 0;
    } else if (!strcmp(activity, "sleep")) {
      game.pets[0].sleeping=1;page=0;
    } else if (!strcmp(activity, "feed")) {
      pp_use(&game, 0, 0);
      page = 0;
    }
  }
  bool viridian=viridian_smoke || (activity && !strncmp(activity,"viridian-",9));
  bool cinnabar=cinnabar_smoke || (activity && !strncmp(activity,"cinnabar-",9));
  bool saffron=saffron_smoke || (activity && !strncmp(activity,"saffron-",8));
  bool fuchsia=fuchsia_smoke || (activity && !strncmp(activity,"fuchsia-",8));
  bool celadon=celadon_smoke || (activity && !strncmp(activity,"celadon-",8));
  bool vermilion=vermilion_smoke || (activity && !strncmp(activity,"vermilion-",10));
  bool cerulean=cerulean_smoke || (activity && !strncmp(activity,"cerulean-",9));
  const char *gym_stage=activity?(viridian?activity+9:cinnabar?activity+9:saffron?activity+8:fuchsia?activity+8:celadon?activity+8:vermilion?activity+10:cerulean?activity+9:!strncmp(activity,"gym-",4)?activity+4:""):"";
  if(gym_smoke || gym_map_smoke || (activity && (!strncmp(activity,"gym-",4) || cerulean || vermilion || celadon || fuchsia || saffron || cinnabar || viridian))) {
    gym_fixture();page=17;
    unsigned gym=viridian?7:cinnabar?6:saffron?5:fuchsia?4:celadon?3:vermilion?2:cerulean?1:0;
    if(cerulean){game.gym.badges=1;game.pets[1].species=0;for(unsigned i=0;i<5;i++){game.growth[i].level=20;unsigned sp=game.pets[i].species;game.growth[i].stage=(sp==1 || sp==3 || sp==6)?1:0;}}
    if(cerulean_smoke){game.pets[2].species=1;game.growth[2].stage=1;}
    if(vermilion){
      game.gym.badges=3;
      const unsigned sp[]={10,20,1,3,7};
      for(unsigned i=0;i<5;i++){game.pets[i].species=sp[i];game.growth[i]=(pp_growth){.level=28,.stage=i==1?0:1};}
    }
    if(celadon){
      game.gym.badges=7;
      const unsigned sp[]={2,6,14,3,7};
      for(unsigned i=0;i<5;i++){game.pets[i].species=sp[i];game.growth[i]=(pp_growth){.level=36,.stage=1};}
    }
    if(fuchsia){
      game.gym.badges=15;
      const unsigned sp[]={26,41,10,3,7};
      for(unsigned i=0;i<5;i++){game.pets[i].species=sp[i];game.growth[i]=(pp_growth){.level=44,.stage=i==3?2:1};}
    }
    if(saffron){
      game.gym.badges=31;
      const unsigned sp[]={58,39,62,3,4};const unsigned stages[]={0,2,0,2,2};
      for(unsigned i=0;i<5;i++){game.pets[i].species=sp[i];game.growth[i]=(pp_growth){.level=50,.stage=stages[i]};}
    }
    if(cinnabar){
      game.gym.badges=63;
      const unsigned sp[]={3,54,64,30,10};const unsigned stages[]={2,1,1,2,1};
      for(unsigned i=0;i<5;i++){game.pets[i].species=sp[i];game.growth[i]=(pp_growth){.level=55,.stage=stages[i]};}
    }
    if(viridian){
      game.gym.badges=127;
      const unsigned sp[]={1,3,56,64,67};const unsigned stages[]={2,2,1,1,1};
      for(unsigned i=0;i<5;i++){game.pets[i].species=sp[i];game.growth[i]=(pp_growth){.level=60,.stage=stages[i]};}
    }
    if(cerulean_smoke || vermilion_smoke || celadon_smoke || fuchsia_smoke || saffron_smoke || cinnabar_smoke || viridian_smoke || !strcmp(gym_stage,"locked"))game.gym.badges=(1u<<(gym?gym-1:0))-1;
    /* High-level battle fixtures must not be intercepted by an unrelated
     * pending evolution choice before their first pointer action. */
    if(gym_smoke){game.evolution_declined=(1u<<game.count)-1u;game.collected[PP_PURE_HEART/8]|=1u<<(PP_PURE_HEART%8);}
    if(activity) {
      if(!strcmp(gym_stage,"cleared")){game.gym.badges=(1u<<(gym+1))-1;page=17;}
      else if(!strcmp(gym_stage,"detail"))page=18;
      else if(!strcmp(gym_stage,"team"))page=19;
      else if(!strcmp(gym_stage,"empty")){game.count=2;page=19;}
      else if(!strcmp(gym_stage,"badges") || !strcmp(gym_stage,"badges-pewter") || !strcmp(gym_stage,"badges-cerulean") || !strcmp(gym_stage,"badges-vermilion") || !strcmp(gym_stage,"badges-celadon") || !strcmp(gym_stage,"badges-fuchsia") || !strcmp(gym_stage,"badges-saffron") || !strcmp(gym_stage,"badges-cinnabar")){game.gym.badges=(1u<<(gym+1))-1;page=20;}
      else if(!strcmp(gym_stage,"battle") || !strcmp(gym_stage,"faint") || !strcmp(gym_stage,"win") || !strcmp(gym_stage,"loss") || !strcmp(gym_stage,"rematch")) {
        if(!strcmp(gym_stage,"loss"))for(unsigned i=0;i<3;i++){game.pets[i].species=2;game.growth[i]=(pp_growth){.level=1};}
        unsigned team[]={1,2,0};assert(pp_gym_start_at(&game,gym,team)==PP_OK);page=6;
        if(!strcmp(gym_stage,"faint")){if(vermilion)game.battle_round=1;game.battle_hp=game.gym.hp[0]=1;pp_tick(&game,3,0);}
        if(!strcmp(gym_stage,"win") || !strcmp(gym_stage,"loss") || !strcmp(gym_stage,"rematch")) {
          for(unsigned i=0;i<600 && game.battling;i++)pp_tick(&game,3,0);
          if(!strcmp(gym_stage,"rematch")) {
            for(unsigned i=0;i<3;i++)game.pets[i].energy=100;
            assert(pp_gym_start_at(&game,gym,team)==PP_OK);
            for(unsigned i=0;i<600 && game.battling;i++)pp_tick(&game,3,0);
          }
          assert(!game.battling && game.gym.result);page=21;
        }
      }
    }
  }
  if(legend_smoke)legend_phase="list";
  if(legend_phase){
    pp_init(&game,42);pp_choose_starter(&game,1);game.growth[0]=(pp_growth){.level=60,.stage=2};game.gym.badges=255;page=24;
    if(!strcmp(legend_phase,"locked") || !strcmp(legend_phase,"locked-detail"))game.gym.badges=127;
    if(!strcmp(legend_phase,"cleared"))game.legend.defeated=31;
    if(!strcmp(legend_phase,"party")) {
      for(unsigned i=1;i<5;i++){game.pets[i]=game.pets[0];game.pets[i].species=i==4?0:i+1;game.growth[i]=(pp_growth){.level=60};}game.count=5;
    }
    if(!strcmp(legend_phase,"apps"))page=1;
    if(!strcmp(legend_phase,"bag") || !strcmp(legend_phase,"egg") || !strcmp(legend_phase,"hatched")){
      for(unsigned i=0;i<PP_LEGEND_COUNT;i++)game.legend.eggs[i]=1;
      page=2;
      if(strcmp(legend_phase,"bag")){
        assert(pp_legend_egg_use(&game,legend_index)==PP_OK);page=0;
      }
    }
    if(!strcmp(legend_phase,"battle") || !strcmp(legend_phase,"won") || !strcmp(legend_phase,"lost")){
      assert(pp_legend_start(&game,legend_index,0)==PP_OK);page=6;
      if(strcmp(legend_phase,"battle")){if(!strcmp(legend_phase,"won"))game.enemy_hp=1;else game.battle_hp=1;pp_tick(&game,3,0);page=23;}
    }
    assert(pp_valid(&game));
  }
  if(gift_preview){
    pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);
    for(unsigned n=0;n<10000 && !game.gift.pending;n++)pp_gift_scene(&game,true,5);
    if(gift_quality<5){
      unsigned item=pp_collection_id(gift_quality,0);
      for(unsigned i=0;i<PP_TRAVEL_COLLECTION;i++)if(pp_collectibles[i].quality==gift_quality && strlen(pp_collectibles[i].name)>strlen(pp_collectibles[item].name))item=i;
      game.gift.item=item;game.collected[item/8]|=1u<<(item%8);
    }
    assert(game.gift.pending && pp_valid(&game));page=0;
  }
  if(menu_smoke || menu_state){
    pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);game.coins=2000;
    if(menu_state){
      assert(!strcmp(menu_state,"active") || !strcmp(menu_state,"owned") || !strcmp(menu_state,"locked"));
      game.scene=!strcmp(menu_state,"active")?scene:(scene+1)%PP_SCENES;
      game.scenes=(1u<<game.scene) | (strcmp(menu_state,"locked")?1u<<scene:0);page=9;
    }else page=1;
  }
  if(wild_smoke)wild_fixture("pending");
  if(wild_phase){wild_fixture(wild_phase);page=(!strcmp(wild_phase,"pending") || !strcmp(wild_phase,"alert"))?0:!strcmp(wild_phase,"battle")?6:23;}
  if(activity && !strncmp(activity,"achievement-",12)) {
    pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);
    game.gym.badges=255;
    for(unsigned d=0;d<PP_DEX_COUNT;d++)game.dex_history[d/8]|=1u<<(d%8);
    if(!strcmp(activity,"achievement-locked") || achievements_smoke) {
      game.gym.badges=127;
      game.dex_history[(PP_DEX_COUNT-1)/8]&=~(1u<<((PP_DEX_COUNT-1)%8));
    }
    collection_quality=5;page=8;assert(pp_valid(&game));
  }
  lv_init();
#ifdef __ANDROID__
  if(!root) {
    static char android_root[320];
    snprintf(android_root,sizeof(android_root),"%s/resources",SDL_AndroidGetInternalStoragePath());
    root=android_root;
  }
#endif
#ifdef _WIN32
  if (!root) {
    /* Use a wide Windows path for the executable directory, then ASCII
     * relative asset names: extraction into Chinese/user folders also works. */
    wchar_t executable[32768];
    DWORD length = GetModuleFileNameW(NULL, executable, 32768);
    wchar_t *slash = length && length < 32768 ? wcsrchr(executable, L'\\') : NULL;
    if (!slash) {
      MessageBoxW(NULL, L"Cannot locate the application folder.", L"Pokemon Plus", MB_OK | MB_ICONERROR);
      return 1;
    }
    *slash = L'\0';
    if (!SetCurrentDirectoryW(executable)) {
      MessageBoxW(NULL, L"Cannot open the application folder.", L"Pokemon Plus", MB_OK | MB_ICONERROR);
      return 1;
    }
    root = "resources";
    FILE *check = fopen("resources/scenes/0.bin", "rb");
    if (!check) {
      MessageBoxW(NULL, L"Resources are missing. Extract the entire ZIP before starting PokemonPlus.exe.", L"Pokemon Plus", MB_OK | MB_ICONERROR);
      return 1;
    }
    fclose(check);
  }
#endif
  if (root) {
    char path[320];
    snprintf(path, sizeof(path), "S:%s", root);
    pp_ui_assets(path);
  }
  lv_indev_t *mouse = NULL;
  if (shell_smoke) {
    shell=pp_shell_create(shortcut);
    if(!shell){fprintf(stderr,"SDL shell: %s\n",SDL_GetError());return 1;}
    pp_dispose(&game);pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);
  } else if (battle_types_smoke || immunity_smoke || hospital_smoke || hospital_phase || long_home_smoke || home_smoke || item_smoke || item_drop_smoke || training_growth_smoke || training_smoke || interaction_smoke || interaction_kind || pat_smoke || scene_damage || scene_profile_path || achievements_smoke || warehouse_smoke || settings_smoke || menu_smoke || gift_smoke || shot || frames_dir || travel_smoke || gym_smoke || notice_smoke || wild_smoke || legend_smoke) {
    lv_display_t *display = lv_display_create(360,360);
    lv_display_set_color_format(display,LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display,export_pixels,NULL,(training_profile||scene_damage||scene_profile_path||travel_smoke||item_smoke)?sizeof(export_pixels):360*20*2,LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display,export_flush);
  } else if(smoke || touch_smoke || home_smoke || intro_smoke || evolution_smoke || ambient_smoke || story_smoke || gym_map_smoke || hatch_smoke) {
    lv_sdl_window_create(360,360);
    lv_sdl_window_set_title(lv_display_get_default(),"Pokemon Plus - 360x360");
    mouse = lv_sdl_mouse_create();
    lv_indev_set_long_press_time(mouse,1000);
    SDL_AddEventWatch(keys,NULL);
  } else {
    shell=pp_shell_create(shortcut);
    if(!shell){fprintf(stderr,"SDL shell: %s\n",SDL_GetError());return 1;}
  }
  /* Establish the deterministic clock before opening timed views. SDL sets
   * a wall-clock callback; changing it after intro started can underflow its
   * epoch when asset preload takes longer than the first test sample. */
  if(battle_types_smoke || immunity_smoke || hospital_smoke || hospital_phase || long_home_smoke || item_smoke || item_drop_smoke || training_growth_smoke || training_smoke || scene_damage || scene_profile_path || achievements_smoke || shell_smoke || warehouse_smoke || settings_smoke || menu_smoke || menu_state || gift_preview || legend_phase || wild_smoke || notice_smoke || gym_smoke || gym_map_smoke || smoke || touch_smoke || home_smoke || intro_smoke || evolution_smoke || travel_smoke || ambient_smoke || story_smoke || shot || frames_dir)
    lv_tick_set_cb(NULL);
  pp_ui_onboarding(battle_types_smoke || immunity_smoke || training_growth_smoke || interaction_smoke || interaction_kind || pat_smoke || achievements_smoke || shell_smoke || warehouse_smoke || settings_smoke || menu_smoke || menu_state || gift_preview || legend_phase || wild_smoke || wild_phase || gym_smoke || intro_seen || smoke || touch_smoke);
  pp_ui_display_load(display_flags);
  pp_ui_battery((pp_battery_reading){.valid=battery_percent>=0&&battery_percent<=100,.percent=battery_percent,.charging=battery_charging});
  pp_ui_create(&game);
  if(long_home_smoke){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pp_ui_bind_input(in);
    const unsigned pages[]={1,2,3,4,5,8,9,10,11,14,15,16,17,18,19,20,24,25,26,27,31,32,33};
    for(unsigned n=0;n<sizeof(pages)/sizeof(pages[0]);n++){
      pp_ui_open(pages[n]);pump(100);uint32_t before=pp_checksum(&game);
      tap(in,180,52,1150);assert(pp_ui_page()==pages[n] && pp_checksum(&game)==before);
      pp_ui_boot_hold();pump(80);assert(pp_ui_page()==0 && pp_checksum(&game)==before);
    }
    unsigned coins=game.coins,stock=game.inventory[0];
    pp_ui_open(3);pump(100);pp_ui_boot_hold();pump(100);assert(pp_ui_page()==0 && game.coins==coins && game.inventory[0]==stock);
    pp_ui_open(3);pump(100);tap(in,100,125,80);assert(game.inventory[0]==stock+1 && game.coins<coins);
    pp_ui_open(11);pump(100);unsigned options=pp_ui_display_options();pp_ui_boot_hold();pump(100);assert(pp_ui_page()==0 && pp_ui_display_options()==options);
    unsigned pats=game.pats_today[0];pp_ui_boot_hold();assert(pp_ui_page()==0 && game.pats_today[0]==pats+1);
    pp_home_state pet=pp_ui_home_probe(0);tap(in,pet.x+24,pet.y+24,1150);assert(pp_ui_page()==4);
    uint32_t before=pp_checksum(&game);pp_ui_screen(false);pp_ui_boot_hold();assert(pp_ui_page()==4 && pp_checksum(&game)==before);pp_ui_screen(true);
    assert(pp_travel(&game,0)==PP_OK);pp_tick(&game,PP_TRAVEL_SECONDS,day_now());pp_ui_open(5);pump(100);
    coins=game.coins;assert(pp_ui_travel_probe().confirmation);tap(in,180,52,1150);assert(pp_ui_page()==5 && pp_ui_travel_probe().confirmation);
    pp_ui_boot_hold();pump(80);assert(pp_ui_page()==0 && game.coins==coins && game.travel_end);
    pp_ui_refresh();assert(pp_ui_page()==0);pp_ui_open(5);assert(pp_ui_travel_probe().confirmation);
    lv_indev_delete(in);puts("PASS: 23 pages ignore touchscreen hold and BOOT returns home without mutations; purchases/settings, home BOOT pat, pet info, screen-off guard and pending travel confirmation preserved");return 0;
  }
  pp_ui_hardware(true, false);
  pp_ui_open(page);
  if(legend_phase && !strcmp(legend_phase,"hatched")) {
    game.hatch_boosts=1;assert(pp_use(&game,PP_HATCH_BOOST,0)==PP_OK);
    pp_ui_refresh();assert(pp_ui_page()==30);
  }
  if(item_drop_smoke){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    for(unsigned scene=0;scene<PP_SCENES;scene++){
      for(unsigned n=0;n<6;n++){
        unsigned item=n<5?n:PP_TREE_ESSENCE;if(item==PP_RETIRED_POTION)continue;
        pp_init(&game,42);pp_choose_starter(&game,1);game.scene=scene;game.scenes=(1u<<PP_SCENES)-1;
        if(item==PP_TREE_ESSENCE)game.tree_essences=1;else game.inventory[item]=1;
        pp_ui_create(&game);pp_ui_open(2);pump(100);
        tap_bag_item(in,item,40);
        assert(pp_ui_page()==0 && game.drop_active && game.drop_item==item && !pp_item_count(&game,item));
        pp_home_state h=pp_ui_home_probe(0);assert(h.drop_visible && h.drop_width==24 && h.drop_height==24 && h.drop_y<140);
        unsigned reads=pp_image_probe().file_reads;uint32_t before=pp_checksum(&game);
        int dx,dy;pp_drop_position(&game,&dx,&dy);int tx=dx-8,ty=dy-16;pp_home_ground_project(scene,&tx,&ty);dx=tx+8;dy=ty+16;
        int land_y=dy+8;
        if(scene==0 && n==0 && frames_dir){
          for(unsigned frame=0;frame<18;frame++){
            char path[640];snprintf(path,sizeof(path),"%s/drop-%02u.ppm",frames_dir,frame);screenshot(path);pump(50);check_partial_frame();
          }
        }else{
          pump(200-h.drop_age);h=pp_ui_home_probe(0);int falling_y=h.drop_y;
          pp_ui_screen(false);pump(1000);assert(pp_ui_home_probe(0).drop_y==falling_y);pp_ui_screen(true);
          pp_ui_open(3);pump(1000);pp_ui_open(0);assert(pp_ui_home_probe(0).drop_y==falling_y);
          int bounced_y=land_y;bool touched=false;
          while(pp_ui_home_probe(0).drop_age<720){
            pump(10);check_partial_frame();h=pp_ui_home_probe(0);
            if(h.drop_age>=400){if(h.drop_y<land_y-8)touched=true;if(h.drop_y<bounced_y)bounced_y=h.drop_y;}
          }
          assert(touched&&bounced_y>=land_y-12);
          h=pp_ui_home_probe(0);assert(h.drop_x==dx+4&&h.drop_y==land_y&&h.drop_opacity==255);
        }
        assert(pp_checksum(&game)==before && pp_image_probe().file_reads==reads);
        if(scene==0&&frames_dir){h=pp_ui_home_probe(0);if(h.drop_age<4000)pump(4000-h.drop_age);}
        pp_tick(&game,4,0);pp_ui_refresh();assert(!game.drop_active&&!pp_ui_home_probe(0).drop_visible);
        h=pp_ui_home_probe(0);pp_talk_topic topic=pp_talk_catalog[pp_talk_item(item)];
        assert(h.bubble_visible&&h.speaker==0&&h.bubble_icon==topic.icon);
        bool found=false;for(unsigned line=0;line<topic.count;line++)if(!strcmp(h.bubble_text,pp_talk_words[topic.first+line]))found=true;
        lv_point_t measured;lv_text_get_size(&measured,h.bubble_text,&pp_font_12,0,0,172,LV_TEXT_FLAG_NONE);
        assert(found&&h.bubble_width==measured.x+48&&h.bubble_height>=44);
        assert(h.bubble_x>=36&&h.bubble_x+h.bubble_width<=324&&h.bubble_y>=99);
        before=pp_checksum(&game);reads=pp_image_probe().file_reads;
        for(unsigned f=0;f<12;f++){pump(33);check_partial_frame();}
        assert(pp_checksum(&game)==before&&pp_image_probe().file_reads==reads);
        if(scene==0&&frames_dir){char path[640];snprintf(path,sizeof(path),"%s/pet-care-%u.ppm",frames_dir,item);screenshot(path);}
        if(scene==0&&item==1){
          char previous[96];snprintf(previous,sizeof(previous),"%s",h.bubble_text);
          game.inventory[1]=1;pp_ui_open(2);tap_bag_item(in,1,40);
          assert(game.drop_active&&!pp_ui_home_probe(0).bubble_visible);
          pp_tick(&game,4,0);pp_ui_refresh();h=pp_ui_home_probe(0);
          assert(h.bubble_visible&&h.bubble_icon==1&&strcmp(previous,h.bubble_text));
          puts("PASS: repeated soap clears old speech during use, then shows a different cleaning response");
        }
        pp_ui_open(1);pump(4000);pp_ui_open(0);assert(pp_ui_home_probe(0).bubble_visible);
        pump(3800);assert(!pp_ui_home_probe(0).bubble_visible);
      }
      pp_init(&game,42);pp_choose_starter(&game,1);game.scene=scene;game.scenes=(1u<<PP_SCENES)-1;game.inventory[5]=1;
      pp_ui_create(&game);pp_ui_open(2);pump(100);tap_bag_item(in,5,40);
      pp_home_state h=pp_ui_home_probe(0);assert(game.egg_end&&h.egg_dropping&&h.egg_width==32&&h.egg_height==32);
      unsigned reads=pp_image_probe().file_reads;uint32_t before=pp_checksum(&game);
      pump(200);h=pp_ui_home_probe(0);int egg_y=h.egg_y;
      pp_ui_screen(false);pump(1000);assert(pp_ui_home_probe(0).egg_y==egg_y);pp_ui_screen(true);
      pp_ui_open(3);pump(1000);pp_ui_open(0);assert(pp_ui_home_probe(0).egg_y==egg_y);
      for(unsigned f=0;f<16;f++){pump(50);check_partial_frame();}
      h=pp_ui_home_probe(0);int ex,ey;pp_egg_position(&game,&ex,&ey);
      assert(!h.egg_dropping&&h.egg_y==ey&&h.egg_width==32);
      pp_ui_open(3);pump(1000);pp_ui_open(0);assert(!pp_ui_home_probe(0).egg_dropping&&pp_ui_home_probe(0).egg_y==ey);
      pp_ui_create(&game);pp_ui_open(0);assert(!pp_ui_home_probe(0).egg_dropping&&pp_ui_home_probe(0).egg_y==ey);
      assert(pp_checksum(&game)==before&&pp_image_probe().file_reads==reads);
      printf("PASS: scene %u: six 24px bag items fall/bounce, screen/menu pause, unchanged state/reads, consumption once; 32px egg fall/settle/no replay, actual partial/full pixel equality\n",scene);
    }
    lv_indev_delete(in);return 0;
  }
  if(item_smoke) {
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(100);
    /* Every shop cell must route to exactly its own item, including row3. */
    for(unsigned slot=0;slot<9;slot++) {
      if(slot==4)continue;unsigned i=pp_shop_slot(slot);
      pp_ui_open(3);pump(80);check_safe_buttons(lv_screen_active());
      unsigned coins=game.coins,stock=pp_item_count(&game,i);tap(in,100+(slot%3)*80,133+(slot/3)*66,80);
      assert(pp_ui_page()==3&&game.coins==coins-pp_items[i].price&&pp_item_count(&game,i)==stock+1);
    }
    assert(visible_labels(lv_screen_active(),"药水")==0&&visible_labels(lv_screen_active(),"抽奖")==0);
    unsigned paid=game.coins,stock_before[9];for(unsigned i=0;i<9;i++)stock_before[i]=pp_item_count(&game,i);
    tap(in,180,199,40);assert(game.coins==paid-PP_SHOP_PRICE&&pp_ui_page()==3);
    uint32_t settled=pp_checksum(&game);tap(in,180,199,40);tap(in,100,133,40);pp_ui_boot_hold();
    assert(game.coins==paid-PP_SHOP_PRICE&&pp_checksum(&game)==settled&&pp_ui_page()==3);
    pp_ui_screen(false);pump(1200);pp_ui_screen(true);
    unsigned winner=99;for(unsigned i=0;i<9;i++)if(pp_item_count(&game,i)==stock_before[i]+1){assert(winner==99);winner=i;}
    assert(pp_item_enabled(winner));unsigned reads=pp_image_probe().file_reads;
    char won[64];snprintf(won,sizeof(won),"抽中%sx1",pp_items[winner].name);
    for(unsigned frame=0;frame<240 && !visible_labels(lv_layer_top(),won);frame++){
      pump(33);check_partial_frame();
      if(frames_dir){char path[640];snprintf(path,sizeof(path),"%s/lottery-%03u.ppm",frames_dir,frame);screenshot(path);}
    }
    assert(visible_labels(lv_layer_top(),won)==1&&pp_checksum(&game)==settled&&pp_image_probe().file_reads==reads);
    unsigned saved_coins=game.coins;game.coins=0;uint32_t error_before=pp_checksum(&game);tap(in,180,199,40);
    assert(pp_checksum(&game)==error_before&&visible_labels(lv_layer_top(),"金币不足")==1);game.coins=saved_coins;
    pump(3400);
    /* Sample actual rendered border pixels after long, irregular UI stalls. */
    static const unsigned ring[8]=PP_SHOP_RING;
    for(unsigned cadence=0;cadence<3;cadence++){
      unsigned old_stock[9],paid=game.coins;for(unsigned i=0;i<9;i++)old_stock[i]=pp_item_count(&game,i);
      point=(lv_point_t){180,199};pressed=true;lv_indev_read(in);pump(20);pressed=false;lv_indev_read(in);
      assert(game.coins==paid-PP_SHOP_PRICE);check_partial_frame();
      assert(export_frame[(98+16)*360+65]==((31u<<11)|(54u<<5)|16u));
      if(frames_dir&&cadence==0){char path[640];snprintf(path,sizeof(path),"%s/slow-%03u.ppm",frames_dir,0u);screenshot(path);}
      unsigned winner=99;for(unsigned i=0;i<9;i++)if(pp_item_count(&game,i)==old_stock[i]+1)winner=i;
      assert(winner<9);unsigned target=0;while(pp_shop_slot(ring[target])!=winner)target++;
      char won[64];snprintf(won,sizeof(won),"抽中%sx1",pp_items[winner].name);
      unsigned steps=0,flashes=0,seen=1,last=0,frames=0;bool was_visible=true;
      uint32_t settled=pp_checksum(&game);unsigned reads=pp_image_probe().file_reads;
      while(!visible_labels(lv_layer_top(),won)){
        unsigned delta=cadence==0?132:cadence==1?450:(unsigned[]){33,99,1200,66,250}[frames%5];
        lv_tick_inc(delta);lv_timer_handler();check_partial_frame();
        int lit=-1;const uint16_t gold=(31u<<11)|(54u<<5)|16u;
        for(unsigned slot=0;slot<9;slot++)if(slot!=4 && export_frame[(98+slot/3*68+16)*360+64+slot%3*80+1]==gold){assert(lit<0);lit=(int)slot;}
        if(lit>=0){
          unsigned index=0;while(index<8&&ring[index]!=(unsigned)lit)index++;assert(index<8);
          seen|=1u<<index;
          if((unsigned)lit!=last){unsigned old=0;while(ring[old]!=last)old++;assert((unsigned)lit==ring[(old+1)%8]);steps++;}
          last=(unsigned)lit;
        }else if(was_visible){flashes++;assert(last==ring[target]&&steps==24+target);}
        was_visible=lit>=0;
        assert(++frames<400&&pp_checksum(&game)==settled&&pp_image_probe().file_reads==reads);
        if(frames_dir&&cadence==0){char path[640];snprintf(path,sizeof(path),"%s/slow-%03u.ppm",frames_dir,frames);screenshot(path);}
      }
      assert(seen==255&&steps==24+target&&flashes==3&&was_visible&&last==ring[target]);
      printf("PASS: actual LCD frame cadence%u: %u consecutive clockwise steps, all8 cells, three winner off/on flashes, exactly-once award, unchanged reads\n",cadence,steps);
      pump(3400);
    }
    if(frames_dir){char path[640];snprintf(path,sizeof(path),"%s/shop.ppm",frames_dir);screenshot(path);}
    pp_ui_open(2);pump(100);check_safe_buttons(lv_screen_active());
    for(unsigned i=0;i<PP_SHOP_ITEMS;i++)assert(visible_labels(lv_screen_active(),pp_items[i].name)==0);
    if(frames_dir){char path[640];snprintf(path,sizeof(path),"%s/bag.ppm",frames_dir);screenshot(path);}
    game.pets[0].food=game.pets[0].clean=game.pets[0].health=game.pets[0].mood=game.pets[0].energy=5;
    unsigned stock=game.tree_essences;tap_bag_item(in,8,80);
    assert(pp_ui_page()==0&&game.drop_active&&game.drop_item==PP_TREE_ESSENCE&&game.tree_essences==stock-1);
    pump(2600);if(frames_dir){char path[640];snprintf(path,sizeof(path),"%s/essence-use.ppm",frames_dir);screenshot(path);}
    pp_tick(&game,4,0);pp_ui_refresh();const pp_pet *p=&game.pets[0];
    assert(p->food==100&&p->clean==100&&p->health==100&&p->mood==100&&p->energy==100&&pp_valid(&game));
    game.tree_essences=1;game.legend.eggs[0]=1;pp_ui_open(2);pump(3400);check_safe_buttons(lv_screen_active());
    assert(!visible_labels(lv_screen_active(),"神兽蛋")&&!visible_labels(lv_screen_active(),"道具")&&!visible_labels(lv_screen_active(),"1 / 2"));
    if(frames_dir){char path[640];snprintf(path,sizeof(path),"%s/bag-ten.ppm",frames_dir);screenshot(path);}
    for(unsigned i=0;i<5;i++)game.legend.eggs[i]=1;
    pp_ui_open(2);pump(80);assert(visible_labels(lv_screen_active(),"1 / 2")==1);
    uint32_t before=pp_checksum(&game);tap(in,320,180,80);assert(pp_ui_page()==2&&visible_labels(lv_screen_active(),"2 / 2")==1);
    check_safe_buttons(lv_screen_active());
    tap(in,40,180,80);assert(visible_labels(lv_screen_active(),"1 / 2")==1&&pp_checksum(&game)==before);
    tap(in,40,180,80);assert(visible_labels(lv_screen_active(),"2 / 2")==1&&pp_checksum(&game)==before);
    memset(game.legend.eggs,0,sizeof(game.legend.eggs));pp_ui_open(2);pump(80);assert(!visible_labels(lv_screen_active(),"2 / 2")&&!visible_labels(lv_screen_active(),"1 / 2"));
    for(unsigned target=0;target<PP_LEGEND_COUNT;target++) {
      for(unsigned i=0;i<5;i++)game.legend.eggs[i]=1;
      pp_ui_open(0);pp_ui_open(2);pump(80);assert(visible_labels(lv_screen_active(),"1 / 2")==1);
      if(!target&&frames_dir){char path[640];snprintf(path,sizeof(path),"%s/bag-first.ppm",frames_dir);screenshot(path);}
      if(!target||target>=4){tap(in,320,180,80);assert(visible_labels(lv_screen_active(),"2 / 2")==1);check_safe_buttons(lv_screen_active());}
      for(unsigned i=0;i<5;i++)assert(visible_labels(lv_screen_active(),pp_legends[i].name)==0);
      if(!target&&frames_dir){char path[640];snprintf(path,sizeof(path),"%s/legend-bag.ppm",frames_dir);screenshot(path);tap(in,40,180,80);}
      else if(!target)tap(in,40,180,80);
      tap_bag_slot(in,(PP_SHOP_ITEMS-1+target)%12,80);assert(pp_ui_page()==0&&game.legend.hatching==target+1&&!game.legend.eggs[target]);
      game.egg_end=0;game.legend.hatching=0;game.egg_shakes=0;pump(3400);
    }
    memset(game.legend.eggs,0,sizeof(game.legend.eggs));pp_ui_open(2);pump(80);
    assert(!visible_labels(lv_screen_active(),"1 / 2")&&!visible_labels(lv_screen_active(),"2 / 2"));
    memset(game.inventory,0,sizeof(game.inventory));game.hatch_boosts=game.balls=game.tree_essences=0;
    pp_ui_open(2);pump(100);assert(visible_labels(lv_screen_active(),"背包空空的")==1);
    assert(!visible_labels(lv_screen_active(),"×0")&&!visible_labels(lv_screen_active(),"1 / 2"));check_safe_buttons(lv_screen_active());
    if(frames_dir){char path[640];snprintf(path,sizeof(path),"%s/bag-empty.ppm",frames_dir);screenshot(path);}
    puts("PASS: eight shop targets, central draw, 33ms and stalled partial/full frames, repeated-click/BOOT guard, screen-off pause and exact single award, no bag categories/names, owned-only 4-column 0/8/9/13 entries, 12 per page, wrap/clamp, pure pagination, all5 egg targets and circular safety; full essence use");
    lv_indev_delete(in);pp_dispose(&game);return 0;
  }
  if(conflict_preview){pump(50);assert(pp_ui_home_conflict_preview(0,1,move_slot));}
  if(wild_phase && !strcmp(wild_phase,"capture")) {
    game.balls=1;pp_ui_open(23);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    tap(in,220,302,40);lv_indev_delete(in);
    assert(pp_ui_wild_capturing() && game.wild.outcome==PP_WILD_CAUGHT);
  }
  clock_now();
  if(scene_profile_path){scene_profile(scene_profile_path);pp_dispose(&game);return 0;}
  if(scene_damage){scene_damage_smoke();nest_walk_smoke();pp_dispose(&game);return 0;}
  if(conflict_smoke){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    for(unsigned scene_id=0;scene_id<13;scene_id++){
      game.scene=scene_id;game.scenes|=1u<<scene_id;game.pets[1].health=100;game.pets[1].mood=80;pp_ui_open(0);pump(100);
      pp_image_stats images=pp_image_probe();unsigned health=game.pets[1].health,mood=game.pets[1].mood,rng=game.rng;
      pp_home_state before[2]={pp_ui_home_probe(0),pp_ui_home_probe(1)};
      assert(pp_ui_home_conflict_preview(0,1,scene_id%4));pump(500);
      pp_home_conflict_state c=pp_ui_home_conflict_probe();assert(c.active&&!c.hit&&c.particles==16);
      assert(pp_ui_home_probe(0).pose==PP_POSE_ATTACK&&pp_ui_home_probe(0).symbol==4);
      assert(game.pets[1].health==health&&game.pets[1].mood==mood);
      for(unsigned i=0;i<2;i++){pp_home_state p=pp_ui_home_probe(i);assert(p.x==before[i].x&&p.y==before[i].y);}
      if(frames_dir&&scene_id==0){char path[640];snprintf(path,sizeof(path),"%s/cast.ppm",frames_dir);screenshot(path);}
      pump(300);assert(pp_ui_home_conflict_probe().hit&&game.pets[1].health==health-3&&game.pets[1].mood==mood-5);
      if(frames_dir&&scene_id==0){char path[640];snprintf(path,sizeof(path),"%s/impact.ppm",frames_dir);screenshot(path);}
      pump(1200);assert(!pp_ui_home_conflict_probe().active&&game.pets[1].health==health-3&&game.pets[1].mood==mood-5&&game.rng==rng);
      pp_home_state after[2]={pp_ui_home_probe(0),pp_ui_home_probe(1)};pump(33);
      for(unsigned i=0;i<2;i++){pp_home_state p=pp_ui_home_probe(i);assert(abs(p.x-after[i].x)<=3&&abs(p.y-after[i].y)<=3);}
      assert(pp_image_probe().file_reads==images.file_reads);check_safe_buttons(lv_screen_active());
      /* New transparent skill pixels and recoil must erase correctly with
       * the small partial framebuffer, including the final cleared frame. */
      static uint16_t partial[360*360];unsigned objects=lv_obj_get_child_count(lv_screen_active());
      assert(pp_ui_home_conflict_preview(0,1,scene_id%4));
      for(unsigned frame=0;frame<50;frame++){
        pump(33);lv_refr_now(lv_display_get_default());memcpy(partial,export_frame,sizeof(partial));
        lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());
        assert(!memcmp(partial,export_frame,sizeof(partial)));
      }
      assert(!pp_ui_home_conflict_probe().active && lv_obj_get_child_count(lv_screen_active())==objects);
      assert(game.pets[1].health==health-6 && game.pets[1].mood==mood-10 && pp_image_probe().file_reads==images.file_reads);
    }
    for(unsigned interrupt=0;interrupt<3;interrupt++){
      pp_ui_open(0);pump(100);unsigned health=game.pets[1].health,mood=game.pets[1].mood;
      assert(pp_ui_home_conflict_preview(0,1,0));pump(300);
      if(interrupt==0)pp_ui_open(1);
      if(interrupt==1)pp_ui_screen(false);
      if(interrupt==2)game.pets[1].sleeping=1;
      pump(2000);assert(!pp_ui_home_conflict_probe().active&&game.pets[1].health==health&&game.pets[1].mood==mood);
      game.pets[1].sleeping=0;pp_ui_screen(true);pp_ui_open(0);pump(100);
    }
    game.identity[0].nature=20;assert(!pp_ui_home_conflict_preview(0,1,0));game.identity[0].nature=3;
    game.pets[1].sleeping=1;assert(!pp_ui_home_conflict_preview(0,1,0));game.pets[1].sleeping=0;
    assert(pp_ui_home_conflict_preview(0,1,0));pump(800);unsigned health=game.pets[1].health;pp_ui_open(1);pump(100);pp_ui_open(0);pump(2000);assert(game.pets[1].health==health);
    puts("PASS:13 scenes actual skill cast/impact/once-only health and mood,650 partial/full frame comparisons, stable positions, fixed objects/zero frame reads, menu/screen/sleep interruption, no resumed damage");return 0;
  }
  if(shell_smoke) {
    const int sizes[][2]={{300,420},{440,600},{800,960},{1000,600}};
    shell_pump(100);
    for(unsigned s=0;s<4;s++) {
      pp_shell_resize(sizes[s][0],sizes[s][1]);shell_pump(100);
      int x,y,gx,gy;pp_shell_game_point(180,180,&x,&y);
      assert(pp_shell_map(x,y,&gx,&gy) && abs(gx-180)<3 && abs(gy-180)<3);
      pp_shell_game_point(0,0,&x,&y);assert(!pp_shell_map(x,y,&gx,&gy));
      uint32_t before=pp_checksum(&game);shell_tap(x,y);assert(pp_checksum(&game)==before);
      pp_shell_game_point(128,306,&x,&y);shell_tap(x,y);assert(pp_ui_page()==1);
      pp_shell_button_point(PP_SHORTCUT_HOME,&x,&y);shell_tap(x,y);assert(pp_ui_page()==0);
      unsigned steps=game.total_steps;pp_shell_button_point(PP_SHORTCUT_STEPS,&x,&y);shell_tap(x,y);assert(game.total_steps==steps+100);
      unsigned old_zoom=pp_shell_zoom();pp_shell_button_point(4,&x,&y);shell_tap(x,y);assert(pp_shell_zoom()==old_zoom-10);
      pp_shell_button_point(5,&x,&y);shell_tap(x,y);assert(pp_shell_zoom()==old_zoom);
      if(shell_output){char path[640];snprintf(path,sizeof(path),"%s/shell-%u.bmp",shell_output,s);assert(pp_shell_capture(path));}
    }
    game.coins=1000;assert(pp_buy(&game,5)==PP_OK);assert(pp_use(&game,5,0)==PP_OK);
    unsigned shakes=game.egg_shakes;int x,y;pp_shell_button_point(PP_SHORTCUT_SHAKE,&x,&y);shell_tap(x,y);assert(game.egg_shakes>shakes);
    uint64_t seconds=game.seconds;pp_shell_button_point(PP_SHORTCUT_HOUR,&x,&y);shell_tap(x,y);assert(game.seconds==seconds+3600);
    pp_shell_button_point(6,&x,&y);shell_tap(x,y);assert(pp_shell_zoom()==100);
    puts("PASS: circular hit mask; four window sizes; scaled game input; four real shortcuts; zoom down/up/reset");
    pp_shell_destroy();pp_dispose(&game);return 0;
  }
  if(clock_override)pp_ui_clock(clock_override);
  if(achievements_smoke) {
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(120);
    tap(in,274,93,80);assert(pp_ui_collection_quality()==5);
    unsigned reads=pp_image_probe().file_reads;
    tap(in,116,121,80);assert(pp_ui_page()==12);
    assert(visible_labels(lv_screen_active(),"八徽之冠")==1 && visible_labels(lv_screen_active(),"特殊 · 未获得")==1);
    check_safe_buttons(lv_screen_active());tap(in,180,310,80);
    game.gym.badges=255;game.revision++;pp_ui_refresh();pump(120);
    tap(in,116,121,80);assert(visible_labels(lv_screen_active(),"特殊 · 已收藏")==1);tap(in,180,310,80);
    tap(in,141,121,80);assert(visible_labels(lv_screen_active(),"万象图鉴")==1 && visible_labels(lv_screen_active(),"特殊 · 未获得")==1);tap(in,180,310,80);
    game.dex_history[(PP_DEX_COUNT-1)/8]|=1u<<((PP_DEX_COUNT-1)%8);game.revision++;pp_ui_refresh();pump(120);
    tap(in,141,121,80);assert(visible_labels(lv_screen_active(),"特殊 · 已收藏")==1);check_safe_buttons(lv_screen_active());
    unsigned checksum=pp_checksum(&game);pump(2000);assert(pp_checksum(&game)==checksum && pp_image_probe().file_reads==reads);
    tap(in,180,310,80);assert(pp_ui_page()==8 && pp_ui_collection_quality()==5 && pp_collection_owned(&game,5)==2);
    lv_indev_delete(in);puts("PASS: two achievement real pointer details, locked/earned states, special filter/return, circular safety, no read-time mutation or new image reads");return 0;
  }
  if(interaction_smoke || interaction_kind){
    pp_dispose(&game);pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);
    for(unsigned i=1;i<3;i++){game.pets[i]=game.pets[0];game.pets[i].species=i+1;game.growth[i]=game.growth[0];}game.count=3;
    pp_ui_create(&game);pp_ui_open(0);pump(100);
    if(clock_override)pp_ui_clock(clock_override);else clock_now();
    if(interaction_kind)assert(pp_ui_home_interaction(interaction_kind,0,1,220,200));
  }
  if(interaction_smoke){
    if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);lv_indev_set_long_press_time(in,1000);
    uint32_t checksum=pp_checksum(&game),rng=game.rng;unsigned reads=pp_image_probe().file_reads;
    for(unsigned kind=1;kind<=8;kind++){
      assert(pp_ui_home_interaction(kind,0,1,220,200));pump(100);
      assert(pp_ui_home_probe(0).behavior==kind);
      pp_home_state previous[3];for(unsigned p=0;p<3;p++)previous[p]=pp_ui_home_probe(p);
      for(unsigned ms=0;ms<18000;ms+=100){pump(100);for(unsigned p=0;p<3;p++){
        pp_home_state next=pp_ui_home_probe(p);if(abs(next.x-previous[p].x)>12||abs(next.y-previous[p].y)>12)fprintf(stderr,"spacing continuity kind%u time%u pet%u %d,%d -> %d,%d\n",kind,ms,p,previous[p].x,previous[p].y,next.x,next.y);assert(abs(next.x-previous[p].x)<=12&&abs(next.y-previous[p].y)<=12);
        assert(next.x>=40&&next.x<=272&&next.y>=111&&next.y<=214);previous[p]=next;
      }}
      assert(pp_checksum(&game)==checksum&&game.rng==rng&&pp_image_probe().file_reads==reads);
    }
    /* Lake activities must also rejoin the existing tree patrol without a snap. */
    game.scene=12;game.scenes|=1u<<12;pp_ui_open(0);pump(100);reads=pp_image_probe().file_reads;
    for(unsigned kind=1;kind<=8;kind++){
      assert(pp_ui_home_interaction(kind,0,1,220,200));pump(100);
      pp_home_state previous[3];for(unsigned p=0;p<3;p++)previous[p]=pp_ui_home_probe(p);
      for(unsigned ms=0;ms<26000;ms+=100){pump(100);for(unsigned p=0;p<3;p++){
        pp_home_state next=pp_ui_home_probe(p);
        if(abs(next.x-previous[p].x)>12||abs(next.y-previous[p].y)>12)fprintf(stderr,"Lake interaction jump kind%u p%u %d,%d -> %d,%d\n",kind,p,previous[p].x,previous[p].y,next.x,next.y);
        assert(abs(next.x-previous[p].x)<=12&&abs(next.y-previous[p].y)<=12);previous[p]=next;
      }}
      assert(pp_image_probe().file_reads==reads);
    }
    game.scene=0;pp_ui_open(0);pump(100);checksum=pp_checksum(&game);
    /* Nearby empty ground routes to a pet; navigation and pet gestures keep priority. */
    pp_ui_open(0);pump(100);
    bool called=false;for(int y=140;y<=240&&!called;y+=20)for(int x=70;x<=290&&!called;x+=20){
      bool free=true;for(unsigned p=0;p<3;p++){pp_home_state h=pp_ui_home_probe(p);if(x>=h.x-12&&x<=h.x+60&&y>=h.y-12&&y<=h.y+60)free=false;}
      if(!free)continue;
      tap(in,x,y,80);for(unsigned p=0;p<3;p++)called|=pp_ui_home_probe(p).behavior==PP_HOME_CALL;
    }
    assert(called);assert(pp_checksum(&game)==checksum);
    pp_home_state h=pp_ui_home_probe(0);tap(in,h.x+24,h.y+24,80);assert(game.pats_today[0]==1);pump(100);
    assert(pp_ui_home_probe(0).behavior==PP_HOME_PAT&&pp_ui_home_probe(0).symbol==1);
    pump(2400);h=pp_ui_home_probe(0);tap(in,h.x+24,h.y+24,1200);assert(pp_ui_page()==4&&game.pats_today[0]==1);
    pp_ui_open(0);pump(100);assert(pp_ui_home_interaction(PP_HOME_GREET,0,1,0,0));
    game.pets[0].sleeping=1;pp_ui_refresh();pump(200);assert(!pp_ui_home_probe(0).behavior&&!pp_ui_home_probe(1).behavior&&pp_ui_home_probe(0).pose==PP_POSE_SLEEP);
    game.pets[0].sleeping=0;pp_ui_refresh();pump(100);assert(pp_ui_home_interaction(PP_HOME_STROLL,0,1,0,0));
    game.inventory[0]=1;assert(pp_use(&game,0,0)==PP_OK);pp_ui_refresh();pump(2500);
    assert(!pp_ui_home_probe(0).behavior&&!pp_ui_home_probe(1).behavior&&pp_ui_home_probe(0).pose==PP_POSE_EAT);
    pp_tick(&game,4,day_now());pp_ui_refresh();pump(1800);
    /* Feed at all24 legal spots, then verify that resuming the patrol never
     * keeps WALK selected while pinned at a lawn edge. Also vary low energy. */
    unsigned longest_stall=0;
    for(unsigned spot=0;spot<24;spot++){
      game.pets[0].energy=spot%2?29:80;game.inventory[0]=1;
      assert(pp_use(&game,0,0)==PP_OK);game.drop_active=spot+2;pp_ui_refresh();pump(2500);
      int tx,ty;pp_drop_position(&game,&tx,&ty);h=pp_ui_home_probe(0);
      tx-=8;ty-=16;pp_home_ground_project(game.scene,&tx,&ty);
      assert(h.x==tx&&h.y==ty&&h.pose==PP_POSE_EAT);
      pp_tick(&game,4,game.day);pp_ui_refresh();pump(1600);
      unsigned stalled=0;pp_home_state previous=pp_ui_home_probe(0);
      for(unsigned ms=0;ms<16000;ms+=33){pump(33);pp_home_state next=pp_ui_home_probe(0);
        assert(abs(next.x-previous.x)<=6&&abs(next.y-previous.y)<=6);
        stalled=next.pose==PP_POSE_WALK&&next.x==previous.x&&next.y==previous.y?stalled+33:0;
        if(stalled>longest_stall)longest_stall=stalled;
        if(stalled>250)fprintf(stderr,"Stalled after feeding spot%u: x%d y%d behavior%u for%ums\n",spot,next.x,next.y,next.behavior,stalled);
        assert(stalled<=250);previous=next;
      }
    }
    printf("PASS: all24 feeding spots resume moving patrols; maximum unchanged WALK position %ums\n",longest_stall);
    pp_ui_open(1);pump(61000);pp_ui_open(0);pump(200);bool welcome=false;
    for(unsigned p=0;p<3;p++)welcome|=pp_ui_home_probe(p).behavior==PP_HOME_WELCOME;
    assert(welcome);
    pp_ui_open(1);pump(1000);pp_ui_open(0);pump(100);for(unsigned p=0;p<3;p++)assert(pp_ui_home_probe(p).behavior!=PP_HOME_WELCOME);
    assert(pp_image_probe().file_reads==reads);check_safe_buttons(lv_screen_active());lv_indev_delete(in);
    puts("PASS: eight shared-renderer behaviors, empty-ground pointer call, pat/long-press priority, sleeping/feeding paired interruption, welcome cooldown, continuous positions, no auto rewards/RNG/save changes or image reads");return 0;
  }
  if(pat_smoke){
    pp_dispose(&game);pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);pp_tick(&game,0,day_now());
    pp_ui_onboarding(true);pp_ui_create(&game);pp_ui_open(0);pump(100);if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    pp_home_state p=pp_ui_home_probe(0);unsigned coins=game.coins;
    for(unsigned i=0;i<20;i++){p=pp_ui_home_probe(0);tap(in,p.x+24,p.y+24,80);}
    assert(game.coins==coins+20 && game.pats_today[0]==20);pump(3000);
    uint32_t before=pp_checksum(&game);p=pp_ui_home_probe(0);tap(in,p.x+24,p.y+24,80);
    assert(pp_checksum(&game)==before && pp_ui_home_probe(0).pose!=PP_POSE_HAPPY && !visible_labels(lv_screen_active(),"♥ +2     +1"));pp_ui_pat_all();assert(pp_checksum(&game)==before && pp_ui_home_probe(0).pose!=PP_POSE_HAPPY);
    /* Long press keeps opening status even after the daily short-click cap. */
    p=pp_ui_home_probe(0);tap(in,p.x+24,p.y+24,1200);assert(pp_ui_page()==4 && pp_checksum(&game)==before);
    pp_ui_open(0);pp_tick(&game,0,game.day+1);pp_ui_refresh();pump(120);p=pp_ui_home_probe(0);tap(in,p.x+24,p.y+24,80);
    assert(game.coins==coins+21 && game.pats_today[0]==1);lv_indev_delete(in);
    puts("PASS: real pet pointer20/21 cap, BOOT respects same cap, long-press status after cap and next-day refill");pp_dispose(&game);return 0;
  }
  if(warehouse_smoke){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    pp_dispose(&game);pp_init(&game,42);pp_choose_starter(&game,1);game.coins=100000;
      pp_ui_create(&game);pp_ui_open(1);pump(100);if(pp_ui_app_page())tap(in,40,180,80);tap_app(in,3);
    assert(pp_ui_page()==16&&visible_labels(lv_screen_active(),"1只    1 / 1")==1&&visible_labels(lv_screen_active(),"Lv.1")==1&&visible_labels(lv_screen_active(),"Lv.1 场景")==0&&visible_labels(lv_screen_active(),"仓库空空的")==0);check_safe_buttons(lv_screen_active());
    if(frames_dir){char path[512];snprintf(path,sizeof(path),"%s/warehouse-single.ppm",frames_dir);screenshot(path);}
    tap(in,180,120,80);assert(pp_ui_page()==26&&visible_labels(lv_screen_active(),"放生")==0&&visible_labels(lv_screen_active(),"收回")==0);tap(in,180,312,80);assert(pp_ui_page()==16);
    while(game.count+pp_warehouse_count(&game)<9)assert(pp_test_hatch(&game)==PP_OK);
    pp_ui_open(16);pump(100);assert(visible_labels(lv_screen_active(),"9只    1 / 1")==1 && visible_labels(lv_screen_active(),"Lv.1")==9);
    assert(pp_test_hatch(&game)==PP_OK);pp_ui_open(16);pump(100);
    assert(visible_labels(lv_screen_active(),"10只    1 / 2")==1 && visible_labels(lv_screen_active(),"Lv.1")==9);
    tap(in,40,180,80);assert(visible_labels(lv_screen_active(),"10只    2 / 2")==1 && visible_labels(lv_screen_active(),"Lv.1")==1);check_safe_buttons(lv_screen_active());
    tap(in,320,180,80);assert(visible_labels(lv_screen_active(),"10只    1 / 2")==1);
    while(pp_warehouse_count(&game)<17)assert(pp_test_hatch(&game)==PP_OK);
    pp_rename(&game,true,0,"tuan");pp_ui_create(&game);pp_ui_open(1);pump(100);if(pp_ui_app_page())tap(in,40,180,80);assert(pp_ui_app_page()==0);
    tap_app(in,3);assert(pp_ui_page()==16);check_safe_buttons(lv_screen_active());
    if(frames_dir){char path[512];snprintf(path,sizeof(path),"%s/warehouse.ppm",frames_dir);screenshot(path);}
    for(unsigned i=0;i<9;i++){
      pp_stored_pet pet;if(i<game.count)pet=(pp_stored_pet){.pet=game.pets[i],.growth=game.growth[i],.identity=game.identity[i]};else assert(pp_warehouse_get(&game,i-game.count,&pet));
      char name[64];snprintf(name,sizeof(name),"%s",pp_stored_name(&pet));
      tap(in,102+(i%3)*78,117+(i/3)*68,80);assert(pp_ui_page()==26&&visible_labels(lv_screen_active(),name)==1);check_safe_buttons(lv_screen_active());
      tap(in,180,312,80);assert(pp_ui_page()==16 && visible_labels(lv_screen_active(),"22只    1 / 3")==1);
    }
    tap(in,320,180,80);assert(visible_labels(lv_screen_active(),"22只    2 / 3")==1 && visible_labels(lv_screen_active(),"Lv.1")==9);
    tap(in,320,180,80);assert(visible_labels(lv_screen_active(),"22只    3 / 3")==1 && visible_labels(lv_screen_active(),"Lv.1")==4);check_safe_buttons(lv_screen_active());
    if(frames_dir){char path[512];snprintf(path,sizeof(path),"%s/warehouse-last.ppm",frames_dir);screenshot(path);}
    for(unsigned i=0;i<4;i++){unsigned columns=4-i;if(columns>3)columns=3;tap(in,(360-columns*78+6)/2+36,117,80);assert(pp_ui_page()==26);tap(in,180,276,80);assert(pp_ui_page()==28);tap(in,180,266,80);assert(pp_ui_page()==16);}
    assert(visible_labels(lv_screen_active(),"18只    2 / 2")==1 && visible_labels(lv_screen_active(),"Lv.1")==9);
    tap(in,40,180,80);assert(visible_labels(lv_screen_active(),"18只    1 / 2")==1);
    tap(in,258,185,80);assert(pp_ui_page()==26);check_safe_buttons(lv_screen_active());
    if(frames_dir){char path[512];snprintf(path,sizeof(path),"%s/individual.ppm",frames_dir);screenshot(path);}
    tap(in,110,276,80);assert(pp_ui_page()==27);check_safe_buttons(lv_screen_active());
    tap(in,180,272,80); /* clear */
    assert(!visible_labels(lv_screen_active(),"ABC")&&!visible_labels(lv_screen_active(),"abc"));
    tap(in,152,197,80); /* f */
    tap(in,247,157,80); /* i */tap(in,139,157,80); /* r */tap(in,112,157,80); /* e */
    tap(in,274,197,80); /* l */tap(in,274,157,80); /* o */tap(in,233,237,80); /* n */tap(in,179,197,80); /* g */
    if(frames_dir){char path[512];snprintf(path,sizeof(path),"%s/name-english.ppm",frames_dir);screenshot(path);}
    tap(in,85,157,80); /* ninth letter is ignored */
    tap(in,260,272,80);assert(pp_ui_page()==26);
    pp_stored_pet p;pp_warehouse_get(&game,0,&p);assert(!strcmp(p.identity.nickname,"firelong"));pump(3200);
    unsigned owned=pp_dex_count(&game),count=pp_warehouse_count(&game);
    tap(in,180,276,80);assert(pp_ui_page()==28);check_safe_buttons(lv_screen_active());tap(in,180,312,80);assert(pp_ui_page()==26&&pp_warehouse_count(&game)==count);
    tap(in,180,276,80);tap(in,180,266,80);assert(pp_ui_page()==16&&pp_warehouse_count(&game)==count-1&&pp_dex_count(&game)==owned);
    tap(in,258,185,80);pp_stored_pet incoming;pp_warehouse_get(&game,0,&incoming);pp_identity outgoing=game.identity[3];
    tap(in,248,276,80);assert(pp_ui_page()==29);check_safe_buttons(lv_screen_active());
    if(frames_dir){char path[512];snprintf(path,sizeof(path),"%s/replacement.ppm",frames_dir);screenshot(path);}
    assert(pp_travel(&game,0)==PP_OK);uint64_t trip_end=game.travel_end;
    pp_ui_refresh();pump(150);
    tap(in,141,198,80);
    assert(pp_ui_page()==0&&pp_valid(&game)&&game.traveler==0&&game.travel_end==trip_end);assert(!memcmp(&game.identity[3],&incoming.identity,sizeof(pp_identity)));pp_warehouse_get(&game,0,&p);assert(!memcmp(&p.identity,&outgoing,sizeof(pp_identity)));
    game.traveler=-1; /* End the fixture trip before testing warehouse compaction. */
    pp_ui_open(1);pump(100);if(pp_ui_app_page())tap(in,40,180,80);tap_app(in,3);assert(pp_ui_page()==16);tap(in,180-(int)((game.count<3?game.count:3)-1)*39,120,80);assert(pp_ui_page()==26&&visible_labels(lv_screen_active(),"收回")==1);
    pp_identity stored_identity=game.identity[0];unsigned original_count=pp_warehouse_count(&game);tap(in,224,276,80);assert(pp_ui_page()==16&&game.count==4&&pp_warehouse_count(&game)==original_count+1);
    tap(in,180,185,80);assert(pp_ui_page()==26&&visible_labels(lv_screen_active(),"上场")==1);tap(in,248,276,80);assert(pp_ui_page()==0&&game.count==5&&pp_warehouse_count(&game)==original_count);
    pp_warehouse_get(&game,original_count-1,&p);assert(!memcmp(&p.identity,&stored_identity,sizeof(pp_identity)));
    pp_ui_open(3);pump(100);check_safe_buttons(lv_screen_active());unsigned coins=game.coins;tap(in,180,265,80);assert(game.balls==1&&game.coins==coins-pp_items[PP_BALL].price);
    pp_dispose(&game);wild_fixture("won");game.balls=0;pp_ui_open(23);pump(100);assert(game.count==1&&game.wild.outcome==PP_WILD_CAPTURE_READY);check_safe_buttons(lv_screen_active());
    tap(in,220,302,80);assert(pp_ui_page()==3&&game.wild.outcome==PP_WILD_CAPTURE_READY);tap(in,180,265,80);assert(game.balls==1);tap(in,180,312,80);assert(pp_ui_page()==23&&game.count==1);
    tap(in,220,302,80);assert(game.balls==0&&game.count==2&&game.wild.outcome==PP_WILD_CAUGHT);pump(3000);tap(in,180,312,80);assert(!game.wild.phase&&pp_ui_page()==0);
    /* Three partners train, a fourth travels, and only the fifth is idle. */
    pp_dispose(&game);pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);game.coins=100000;
    while(game.count+pp_warehouse_count(&game)<7)assert(pp_test_hatch(&game)==PP_OK);
    for(unsigned i=0;i<game.count;i++){game.pets[i].energy=100;game.pets[i].food=90;game.pets[i].health=90;}
    assert(pp_travel(&game,3)==PP_OK);pp_training swap_job;pp_training_init(&swap_job,123);unsigned swap_picks[]={0,1,2};
    assert(pp_training_job_start(&swap_job,&game,0,swap_picks,3)==PP_OK);pp_training_close(&swap_job);
    pp_identity protected_identity[4];memcpy(protected_identity,game.identity,sizeof(protected_identity));
    pp_stored_pet swap_in,swap_out;assert(pp_warehouse_get(&game,0,&swap_in));pp_identity idle_identity=game.identity[4];uint64_t protected_trip=game.travel_end;
    pp_ui_create(&game);pp_ui_open(16);pump(100);tap(in,258,185,80);assert(pp_ui_page()==26);tap(in,248,276,80);assert(pp_ui_page()==29);
    tap(in,180,120,80);assert(pp_ui_page()==29&&visible_labels(lv_layer_top(),"伙伴正在历练")==1);
    tap(in,141,198,80);assert(pp_ui_page()==29&&visible_labels(lv_layer_top(),"伙伴正在旅行")==1);
    if(frames_dir){char path[512];snprintf(path,sizeof(path),"%s/warehouse-training-before.ppm",frames_dir);screenshot(path);}
    tap(in,219,198,80);assert(pp_ui_page()==0&&game.training.status==1&&game.traveler==3&&game.travel_end==protected_trip&&pp_valid(&game));
    assert(!memcmp(protected_identity,game.identity,sizeof(protected_identity))&&!memcmp(&game.identity[4],&swap_in.identity,sizeof(pp_identity)));
    assert(pp_warehouse_get(&game,0,&swap_out)&&!memcmp(&swap_out.identity,&idle_identity,sizeof(pp_identity)));
    if(frames_dir){char path[512];snprintf(path,sizeof(path),"%s/warehouse-training-after.ppm",frames_dir);screenshot(path);}
    /* Display sorting must not change true slots; pause fixture jobs during exhaustive navigation. */
    memset(&game.training,0,sizeof(game.training));game.traveler=-1;
    while(game.count+pp_warehouse_count(&game)<22)assert(pp_test_hatch(&game)==PP_OK);
    for(unsigned i=0;i<pp_warehouse_count(&game);i++){
      if(i<game.reserve_count)game.reserve_growth[i].level=1+(i*17)%60;
      else game.overflow[i-game.reserve_count].growth.level=1+(i*17)%60;
    }
    pp_ui_create(&game);pp_ui_open(16);pump(100);
    uint32_t sort_checksum=pp_checksum(&game);
    for(unsigned mode=0;mode<4;mode++){
      unsigned actual_mode=mode%3;check_safe_buttons(lv_screen_active());
      pump(3200); /* Dismiss the transient mode receipt before captures. */
      assert(visible_labels(lv_screen_active(),"22只    1 / 3")==1);
      if(frames_dir&&mode<3){char path[512];snprintf(path,sizeof(path),"%s/warehouse-sort-%u.ppm",frames_dir,mode);screenshot(path);}
      for(unsigned pg=0;pg<3;pg++){
        unsigned slots[9];assert(pp_warehouse_sorted_page(&game,actual_mode,pg,9,slots));
        unsigned size=pg==2?4:9;
        for(unsigned i=0;i<size;i++){
          unsigned n=slots[i];pp_stored_pet pet;
          if(n<game.count)pet=(pp_stored_pet){.pet=game.pets[n],.growth=game.growth[n],.identity=game.identity[n]};
          else assert(pp_warehouse_get(&game,n-game.count,&pet));
          char name[64],level[24];snprintf(name,sizeof(name),"%s",pp_stored_name(&pet));snprintf(level,sizeof(level),"Lv.%u",pet.growth.level);
          unsigned row_count=size-(i/3)*3;if(row_count>3)row_count=3;
          tap(in,(360-(int)row_count*78+6)/2+36+(i%3)*78,117+(i/3)*68,80);
          assert(pp_ui_page()==26&&visible_labels(lv_screen_active(),name)==1&&visible_labels(lv_screen_active(),level)==1);
          tap(in,180,312,80);assert(pp_ui_page()==16);
        }
        if(pg<2)tap(in,320,180,80);
      }
      assert(pp_checksum(&game)==sort_checksum);
      tap(in,230,310,80); /* Cycle from last page must return to first. */
    }
    /* Mode is level now. Release the visible sorted item and verify real compaction. */
    unsigned sorted_slots[9];assert(pp_warehouse_sorted_page(&game,1,0,9,sorted_slots));
    unsigned raw=sorted_slots[5]-game.count;pp_stored_pet released;assert(pp_warehouse_get(&game,raw,&released));
    tap(in,258,185,80);tap(in,180,276,80);assert(pp_ui_page()==28);tap(in,180,266,80);
    assert(pp_ui_page()==16&&pp_warehouse_count(&game)==16);
    if(raw<pp_warehouse_count(&game)){pp_stored_pet next;assert(pp_warehouse_get(&game,raw,&next));assert(memcmp(&next.identity,&released.identity,sizeof(pp_identity)));}
    assert(!memcmp(protected_identity,game.identity,sizeof(protected_identity)));
    for(unsigned i=0;i<game.count;i++){game.pets[i].energy=100;game.pets[i].food=90;game.pets[i].health=90;}
    assert(pp_travel(&game,3)==PP_OK);protected_trip=game.travel_end;
    pp_training_init(&swap_job,124);assert(pp_training_job_start(&swap_job,&game,0,swap_picks,3)==PP_OK);pp_training_close(&swap_job);
    pp_ui_create(&game);pp_ui_open(16);pump(100);tap(in,230,310,80);
    assert(pp_warehouse_sorted_page(&game,1,0,9,sorted_slots));
    assert(pp_warehouse_get(&game,sorted_slots[5]-game.count,&swap_in));idle_identity=game.identity[4];
    tap(in,258,185,80);tap(in,248,276,80);assert(pp_ui_page()==29);
    tap(in,180,120,80);assert(pp_ui_page()==29&&visible_labels(lv_layer_top(),"伙伴正在历练")==1);
    tap(in,141,198,80);assert(pp_ui_page()==29&&visible_labels(lv_layer_top(),"伙伴正在旅行")==1);
    tap(in,219,198,80);assert(pp_ui_page()==0&&game.training.status==1&&game.traveler==3&&game.travel_end==protected_trip);
    assert(!memcmp(protected_identity,game.identity,sizeof(protected_identity))&&!memcmp(&game.identity[4],&swap_in.identity,sizeof(pp_identity)));
    check_safe_buttons(lv_screen_active());
    puts("PASS: all 22 real card targets under all sort modes, 5 scene slots pinned, cycle resets page, detail-return retains mode, sorted release resolves true slot, ongoing training/travel intact");
    puts("PASS: actual warehouse pointers allow idle replacement during three-partner training and unrelated travel; participating slots remain protected");
    puts("PASS: nine warehouse cards, all card targets including third row, 9/10/18/22 paging and last-page release clamp, sole courtyard pet, active highlight frames, protected last pet, store/field round-trip, lowercase-only naming, release cancel/confirm/history, explicit replacement during another pet trip, ball shop, once-only capture");pp_dispose(&game);lv_indev_delete(in);return 0;
  }
  if(settings_smoke) {
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(200);
    assert(pp_ui_display_options()==7);
    lv_obj_t *initial_switches[5];assert(collect_switches(lv_screen_active(),initial_switches,0)==0);
    assert(visible_labels(lv_screen_active(),"显示设置")==1&&visible_labels(lv_screen_active(),"场景设置")==1&&visible_labels(lv_screen_active(),"历练设置")==1);
    check_safe_buttons(lv_screen_active());screenshot("build/settings-menu-root.ppm");tap(in,180,122,80);pump(100);
    assert(visible_labels(lv_screen_active(),"显示设置")==1);screenshot("build/settings-menu-display.ppm");
    uint32_t checksum=pp_checksum(&game),reads=pp_image_probe().file_reads;
    static const unsigned states[]={7,6,4,5,1,0,2,3,7};
    for(unsigned n=0;n<9;n++) {
      if(n){unsigned changed=states[n]^states[n-1],row=changed==1?0:changed==2?1:2;tap(in,n%2?110:250,125+row*48,80);pump(180);}
      unsigned flags=states[n];assert(pp_ui_display_options()==flags);
      lv_obj_t *switches[5];assert(collect_switches(lv_screen_active(),switches,0)==3);
      for(unsigned bit=0;bit<3;bit++)assert(lv_obj_has_state(switches[bit],LV_STATE_CHECKED)==!!(flags&(1u<<bit)));
      check_safe_buttons(lv_screen_active());
      for(unsigned i=0;i<3;i++){lv_area_t a;lv_obj_get_coords(switches[i],&a);assert(a.x1>=66 && a.x2<=294 && a.y1>=80 && a.y2<=266);}
      pp_ui_open(0);pp_ui_clock("11:11");pump(80);
      assert(visible_labels(lv_screen_active(),"11:11")==((flags&PP_UI_SHOW_TIME)?2u:0u));
      assert(visible_labels(lv_screen_active(),"1234")==((flags&PP_UI_SHOW_COINS)?2u:0u));
      assert(visible_labels(lv_screen_active(),"72%")==((flags&PP_UI_SHOW_BATTERY)?1u:0u));
      assert(pp_checksum(&game)==checksum);
      if(frames_dir && n<8){char path[640];snprintf(path,sizeof(path),"%s/home-%u.ppm",frames_dir,flags);screenshot(path);}
      pp_ui_display_load(flags);pp_ui_open(11);pump(100);tap(in,180,122,80);pump(100);
    }
    tap(in,180,320,80);pump(100);assert(collect_switches(lv_screen_active(),initial_switches,0)==0);
    tap(in,180,234,80);pump(100);assert(visible_labels(lv_screen_active(),"历练设置")==1&&collect_switches(lv_screen_active(),initial_switches,0)==1);
    assert(!pp_ui_auto_supply());tap(in,250,125,80);pump(180);assert(pp_ui_auto_supply());
    screenshot("build/settings-menu-training.ppm");assert(pp_checksum(&game)==checksum);
    game.revision++;pp_ui_refresh();pump(100);assert(visible_labels(lv_screen_active(),"历练设置")==1&&pp_ui_auto_supply());
    pp_ui_open(0);pp_ui_open(11);pump(100);assert(pp_ui_auto_supply());assert(collect_switches(lv_screen_active(),initial_switches,0)==0);
    tap(in,180,234,80);tap(in,110,125,80);pump(180);assert(!pp_ui_auto_supply());
    pp_ui_auto_supply_load(99);assert(!pp_ui_auto_supply());pp_ui_auto_supply_load(1);assert(pp_ui_auto_supply());pp_ui_auto_supply_load(0);
    tap(in,180,320,80);tap(in,180,178,80);pump(100);assert(visible_labels(lv_screen_active(),"场景设置")==1&&collect_switches(lv_screen_active(),initial_switches,0)==1);
    screenshot("build/settings-menu-scene.ppm");tap(in,250,125,80);pump(180);assert(game.random_scene);
    tap(in,110,125,80);pump(180);assert(!game.random_scene);
    tap(in,180,320,80);pump(80);assert(pp_ui_page()==11&&collect_switches(lv_screen_active(),initial_switches,0)==0);
    tap(in,180,320,80);pump(80);assert(pp_ui_page()==1);
    pp_ui_open(11);tap(in,180,178,80);pp_ui_boot_hold();pump(80);assert(pp_ui_page()==0);pp_ui_open(11);pump(80);assert(collect_switches(lv_screen_active(),initial_switches,0)==0);
    pp_ui_display_load(99);assert(pp_ui_display_options()==7);
    pp_ui_open(0);pp_ui_battery((pp_battery_reading){.percent=-1});pump(80);assert(visible_labels(lv_screen_active(),"--")==1);
    pp_ui_battery((pp_battery_reading){.valid=true,.percent=0});pump(80);assert(visible_labels(lv_screen_active(),"0%")==1);
    pp_ui_battery((pp_battery_reading){.valid=true,.percent=100,.charging=true});pump(80);assert(visible_labels(lv_screen_active(),"100%")==1);
    pp_ui_open(1);pp_ui_battery((pp_battery_reading){.valid=true,.percent=15});pp_ui_open(0);pump(80);assert(visible_labels(lv_screen_active(),"15%")==1);
    uint32_t battery_checksum=pp_checksum(&game);
    pp_ui_battery((pp_battery_reading){.valid=true,.percent=72,.charging=true});pump(50);
    lv_obj_t *battery=label_parent(lv_screen_active(),"72%");unsigned widths=0;
    uint16_t battery_partial[18*90];
    for(unsigned n=0;n<44;n++){
      pump(50);lv_refr_now(lv_display_get_default());unsigned width=battery_fill_width(battery);assert(width<=10);widths|=1u<<width;
      assert(battery_bolt_pixels(battery)==15);
      assert(visible_labels(lv_screen_active(),"72%")==1);
      if(n%2==0){
        for(unsigned y=0;y<18;y++)memcpy(battery_partial+y*90,export_frame+(331+y)*360+135,90*2);
        lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());
        for(unsigned y=0;y<18;y++)assert(!memcmp(battery_partial+y*90,export_frame+(331+y)*360+135,90*2));
        if(frames_dir && n<36){char path[640];snprintf(path,sizeof(path),"%s/charge-%02u.ppm",frames_dir,n/2);screenshot(path);}
      }
    }
    assert(widths==((1u<<0)|(1u<<2)|(1u<<4)|(1u<<6)|(1u<<8)|(1u<<10)));
    unsigned paused=battery_fill_width(battery);pp_ui_screen(false);pump(700);assert(battery_fill_width(battery)==paused);pp_ui_screen(true);
    pp_ui_display_load(3);pump(60);paused=battery_fill_width(battery);pump(700);assert(!lv_obj_is_visible(battery)&&battery_fill_width(battery)==paused);pp_ui_display_load(7);pump(60);assert(lv_obj_is_visible(battery));
    pp_ui_battery((pp_battery_reading){.valid=true,.percent=72});pump(60);assert(battery_fill_width(battery)==7);assert(battery_bolt_pixels(battery)==0);pump(700);assert(battery_fill_width(battery)==7);
    pp_ui_battery((pp_battery_reading){.valid=true,.percent=100,.charging=true});pump(60);battery=label_parent(lv_screen_active(),"100%");assert(battery_fill_width(battery)==10);assert(battery_bolt_pixels(battery)==0);pump(1900);assert(battery_fill_width(battery)==10);
    pp_ui_battery((pp_battery_reading){.valid=true,.percent=0,.charging=true});pump(500);battery=label_parent(lv_screen_active(),"0%");assert(battery_fill_width(battery)>0);assert(battery_bolt_pixels(battery)==15);
    pp_ui_battery((pp_battery_reading){.percent=-1});pump(60);battery=label_parent(lv_screen_active(),"--");assert(battery_fill_width(battery)==0);assert(battery_bolt_pixels(battery)==0);
    pp_ui_open(1);pp_ui_battery((pp_battery_reading){.valid=true,.percent=15,.charging=true});pump(1900);pp_ui_open(0);pump(50);battery=label_parent(lv_screen_active(),"15%");assert(battery_fill_width(battery)==0);pump(350);assert(battery_fill_width(battery)>0);
    assert(pp_checksum(&game)==battery_checksum);
    puts("PASS: charging six-step fill/cycle, actual SOC unchanged, partial/full repaint equality, screen-off/hidden pause, unplug/full/zero/unknown/menu transitions and no game writes");
    assert(pp_image_probe().file_reads==reads);
    lv_indev_delete(in);puts("PASS: three category menus, parent/root/APP/BOOT navigation, five retained switches, row/knob touch, all eight time/coins/battery states, restore, unknown/zero/full/menu battery updates, random scene and zero extra image reads");return 0;
  }

  if(legend_phase && (!strcmp(legend_phase,"bag") || !strcmp(legend_phase,"apps") || !strcmp(legend_phase,"detail") || !strcmp(legend_phase,"locked-detail") || !strcmp(legend_phase,"party"))){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(100);
    if(!strcmp(legend_phase,"bag"))tap(in,320,180,80);
    else if(!strcmp(legend_phase,"apps"))tap(in,320,180,80);
    else {int x=legend_index<3?94+legend_index*86:137+(legend_index-3)*86,y=legend_index<3?130:224;tap(in,x,y,80);}
    lv_indev_delete(in);
  }
  if(menu_state && strcmp(menu_state,"active")){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    pump(100);tap(in,40,180,80);assert(pp_ui_scene_selected()==scene);lv_indev_delete(in);
  }
  if(battle_types_smoke){
    lv_tick_set_cb(NULL);if(mouse)lv_indev_enable(mouse,false);
    for(unsigned art=0;art<PP_ART_COUNT;art++){
      pp_ui_open(1);pp_battle_encounter_close(&game);pp_dispose(&game);pp_init(&game,123);assert(pp_choose_starter(&game,1)==PP_OK);
      const pp_dex_entry *ally=NULL,*foe=NULL;
      for(unsigned n=0;n<PP_DEX_COUNT;n++){if(pp_pokedex[n].art==art)ally=&pp_pokedex[n];if(pp_pokedex[n].art==(art+77)%PP_ART_COUNT)foe=&pp_pokedex[n];}
      assert(ally&&foe);game.pets[0].species=ally->species;game.growth[0]=(pp_growth){.level=30,.stage=ally->stage};
      game.wild=(pp_wild_state){.phase=PP_WILD_BATTLE,.dex=foe-pp_pokedex,.level=30};game.battle_hp=game.battle_max_hp=game.enemy_hp=game.enemy_max_hp=1000;game.battling=1;game.battle_next=game.seconds+PP_BATTLE_TURN_SECONDS;
      pp_ui_open(6);pump(100);check_battle_types();unsigned reads=pp_image_probe().file_reads,checksum=pp_checksum(&game);
      pump(1100);check_battle_types();assert(pp_image_probe().file_reads==reads&&pp_checksum(&game)==checksum);
    }
    pp_ui_open(1);pp_dispose(&game);pp_init(&game,123);assert(pp_choose_starter(&game,4)==PP_OK);game.count=2;
    game.pets[1]=game.pets[0];game.growth[0]=(pp_growth){.level=36,.stage=2};game.growth[1]=(pp_growth){.level=10,.stage=0};game.pets[0].energy=game.pets[1].energy=100;
    pp_gym_enemy enemies[2]={{.name="Articuno",.art=pp_pokedex[143].art,.level=30,.hp=1000,.attack=50,.defense=50},{.name="Mewtwo",.art=pp_pokedex[149].art,.level=30,.hp=1000,.attack=50,.defense=50}};
    pp_gym_def def={.title="Relay",.team=enemies};assert(pp_battle_encounter_start(&game,&def,2)==PP_OK);
    pp_ui_open(6);pump(1100);check_battle_types();unsigned reads=pp_image_probe().file_reads;
    game.battle_hp=game.gym.hp[0]=0;game.enemy_hp=game.gym.enemy_hp[0]=0;
    for(unsigned n=0;n<3&&(game.gym.ally!=1||game.gym.enemy!=1);n++){pp_tick(&game,3,0);pp_ui_refresh();pump(1100);check_battle_types();}
    assert(game.gym.ally==1&&game.gym.enemy==1);assert(pp_image_probe().file_reads==reads);
    pp_ui_open(1);pp_battle_encounter_close(&game);
    puts("PASS: all151 forms both sides, exact visible type-icon pixels/positions, entry and settled, actual dual-to-single relay clears both stale badges, no extra reads/writes");return 0;
  }
  if(immunity_smoke){
    lv_tick_set_cb(NULL);if(mouse)lv_indev_enable(mouse,false);
    lv_font_glyph_dsc_t glyph;assert(lv_font_get_glyph_dsc(&pp_font_18,&glyph,0x514d,0)&&lv_font_get_glyph_dsc(&pp_font_18,&glyph,0x75ab,0));
    const unsigned pairs[][3]={{25,50,PP_ELECTRIC},{19,92,PP_NORMAL},{50,16,PP_GROUND},{56,92,PP_FIGHTING},{92,19,PP_GHOST},{23,81,PP_POISON},{147,35,PP_DRAGON},{27,16,PP_GROUND},{4,7,PP_FIRE},{4,1,PP_FIRE},{4,46,PP_FIRE},{56,142,PP_FIGHTING}};
    for(unsigned mode=0;mode<2;mode++)for(unsigned c=0;c<12;c++)for(unsigned attacking=0;attacking<2;attacking++){
      pp_ui_open(1);pp_battle_encounter_close(&game);pp_dispose(&game);pp_init(&game,123);assert(pp_choose_starter(&game,1)==PP_OK);
      const pp_dex_entry *caster=&pp_pokedex[pairs[c][0]-1],*defender=&pp_pokedex[pairs[c][1]-1];
      const pp_dex_entry *ally=attacking?defender:caster,*foe=attacking?caster:defender;
      game.pets[0].species=ally->species;game.growth[0]=(pp_growth){.level=30,.stage=ally->stage};game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;
      pp_gym_enemy enemy={.name="test",.art=foe->art,.level=30,.hp=10000,.attack=100,.defense=100};
      pp_gym_def def={.title="test",.team=&enemy};
      if(mode){assert(pp_battle_encounter_start(&game,&def,1)==PP_OK);game.battle_hp=game.battle_max_hp=game.gym.hp[0]=game.gym.max_hp[0]=10000;}
      else {game.wild=(pp_wild_state){.phase=PP_WILD_BATTLE,.dex=foe-pp_pokedex,.level=30};game.battle_hp=game.battle_max_hp=game.enemy_hp=game.enemy_max_hp=10000;game.battling=1;game.battle_next=game.seconds+PP_BATTLE_TURN_SECONDS;}
      unsigned slot=0;while(slot<4 && (pp_battle_move(&game,attacking,slot)->type!=pairs[c][2] || !pp_battle_move_power(&game,attacking,slot)))slot++;
      if(c==7)slot=1;
      bool immune=c<7,damage=c>=8;assert(slot<4);assert((pp_gym_effectiveness(&game,attacking,slot)==0)==(c<8));assert(c!=7||!pp_battle_move_power(&game,attacking,slot));game.battle_round=slot;
      pp_ui_open(6);pump(1100);assert(!pp_battle_view_probe().entrance_mask);
      int before=attacking?game.battle_hp:game.enemy_hp;pp_tick(&game,PP_BATTLE_TURN_SECONDS,day_now());pp_ui_refresh();
      assert(((attacking?game.battle_hp:game.enemy_hp)<before)==damage);
      unsigned unchanged=pp_checksum(&game),reads=pp_image_probe().file_reads;
      unsigned before_hit=attacking?(mode?550:830):(mode?150:230);
      pump(before_hit);pp_battle_view_tick();assert(!visible_labels(lv_screen_active(),"免疫"));
      pump(20);pp_battle_view_tick();assert(!visible_labels(lv_screen_active(),"免疫")&&!visible_labels(lv_screen_active(),"VS"));
      pp_battle_view_state state=pp_battle_view_probe();assert(state.casting&&state.side==attacking&&state.damage_visible==damage&&state.immunity_visible==immune);
      if(immune)assert(!state.impact_text[0]&&state.hp[1-attacking]==before);
      unsigned factor=pp_gym_effectiveness(&game,attacking,slot),power=pp_battle_move_power(&game,attacking,slot);
      assert(state.relation_factor[attacking]==factor&&state.relation_kind[attacking]==pp_relation_kind(factor,power));
      assert(state.relation_level[attacking]==(power?255:0));
      assert(c!=8||factor==65);assert(c!=9||factor==150);assert(c!=10||factor==200);assert(c!=11||factor==100);
      if(!immune&&!damage)assert(!state.impact_text[0]);
      if(shot&&(c==0||c>=8)){char path[768];snprintf(path,sizeof(path),"%s-%u-%u-%u.ppm",shot,mode,attacking,c);screenshot(path);}
      pump(1400);pp_battle_view_tick();assert(!visible_labels(lv_screen_active(),"免疫")&&pp_image_probe().file_reads==reads&&pp_checksum(&game)==unchanged);
      printf("PASS: impact mode=%u side=%u type=%u immune=%d damage=%d timing/HP/no reads/mutations\n",mode,attacking,pairs[c][2],immune,damage);
    }
    pp_ui_open(1);pp_battle_encounter_close(&game);return 0;
  }
  if(completion_smoke){
    lv_tick_set_cb(NULL);if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pp_ui_bind_input(in);
    for(unsigned scenario=0;scenario<64;scenario++){
      pp_ui_open(1);pp_dispose(&game);pp_init(&game,123);assert(pp_choose_starter(&game,1)==PP_OK);
      game.count=4;for(unsigned i=1;i<4;i++){game.pets[i]=game.pets[0];game.identity[i]=game.identity[0];}
      /* A trainer on an evolution boundary and a separate traveler. */
      game.pets[1].species=2;game.growth[0]=game.growth[1]=(pp_growth){.level=scenario&32?60:15,.xp=scenario&32?0:299};
      game.growth[2]=(pp_growth){.level=1};game.pets[2].species=0;
      game.pets[3].species=3;game.growth[3]=(pp_growth){.level=scenario&8?16:1};
      game.coins=500;assert(pp_travel(&game,2)==PP_OK);
      pp_training job;pp_training_init(&job,123);unsigned picks[]={0,1};
      assert(pp_training_job_start(&job,&game,0,picks,2)==PP_OK);
      pp_ui_screen(false);pp_ui_open(scenario&1?33:0);
      pp_training_job_advance(&job,&game,PP_TRAINING_DURATION_MS);pp_training_close(&job);
      pp_tick(&game,PP_TRAVEL_SECONDS,day_now());
      assert(pp_training_growth_pending(&game)==!(scenario&32) && pp_travel_describe(&game).ready);
      if(scenario&2){unsigned bytes=pp_save_size(&game);void *blob=malloc(bytes);assert(blob&&pp_save_write(&game,blob,bytes,123));pp_dispose(&game);assert(pp_save_read(&game,blob,bytes,NULL));free(blob);}
      unsigned unsettled_coins=game.coins;
      pump(200);assert(!pp_ui_evolving());pp_ui_screen(true);pump(100);
      assert((scenario&32)?pp_ui_travel_probe().confirmation:pp_ui_evolution_probe().level);
      if((scenario&4) && !(scenario&32)){pp_ui_open(33);pump(100);assert(pp_ui_evolution_probe().level);pp_ui_boot_hold();pump(100);assert(pp_ui_evolution_probe().level);}
      for(unsigned i=0;!(scenario&32)&&i<2;i++){
        assert(pp_ui_evolution_probe().level);tap(in,180,290,80);pump(100);
        assert(pp_ui_evolution_probe().choice);
        if(scenario&16){tap(in,180,260,80);pump(2400);assert(pp_ui_evolution_probe().ready);tap(in,180,310,80);pump(100);}
        else {tap(in,100,68,80);pump(100);}
      }
      assert(!pp_training_growth_pending(&game)&&!pp_ui_evolving());
      pp_ui_refresh();pump(100);
      assert(pp_ui_travel_probe().confirmation&&pp_ui_travel_probe().claimable&&!pp_ui_evolving());
      assert(game.traveler==2&&game.coins==unsettled_coins);
      check_safe_buttons(lv_screen_active());
      /* Explicit BOOT escape from the travel panel still postpones only that
       * panel; revisiting travel must restore a working confirmation. */
      if(scenario==0){pp_ui_boot_hold();pump(100);assert(pp_ui_page()==0&&game.traveler==2);pp_ui_open(5);pump(100);assert(pp_ui_travel_probe().confirmation);}
      unsigned coins=game.coins,collection=pp_collected(&game),xp=pp_travel_xp(&game);
      pp_growth trainer[2]={game.growth[0],game.growth[1]};
      if(shot&&scenario==0)screenshot(shot);
      tap(in,180,276,80);pump(1600);pp_travel_view_state state=pp_ui_travel_probe();
      assert(state.receipt&&state.ready&&state.coins==80&&state.xp==xp&&game.traveler==-1);
      assert(game.coins==coins+80&&pp_collected(&game)==collection+1&&!memcmp(trainer,game.growth,sizeof(trainer)));
      check_safe_buttons(lv_screen_active());
      if(shot&&scenario==0){char path[768];snprintf(path,sizeof(path),"%s-receipt.ppm",shot);screenshot(path);}
      if(scenario==0){pp_ui_boot_hold();pump(100);assert(pp_ui_page()==0);pp_ui_open(5);pump(1600);assert(pp_ui_travel_probe().receipt&&game.coins==coins+80);}
      uint32_t settled=pp_checksum(&game);
      for(unsigned i=0;i<3;i++){pp_ui_screen(false);pp_ui_screen(true);pp_ui_refresh();pp_ui_open(5);pump(1600);assert(pp_checksum(&game)==settled);}
      tap(in,180,301,80);pump(100);pp_ui_open(5);pump(100);assert(!pp_ui_travel_probe().active&&game.coins==coins+80&&pp_valid(&game));
      unsigned bytes=pp_save_size(&game);void *blob=malloc(bytes);assert(blob&&pp_save_write(&game,blob,bytes,123));pp_dispose(&game);assert(pp_save_read(&game,blob,bytes,NULL));free(blob);
      pp_ui_open(5);pump(100);assert(game.traveler==-1&&game.coins==coins+80&&!pp_ui_travel_probe().active&&!pp_training_growth_pending(&game));
      printf("PASS: simultaneous completion scenario %u\n",scenario);
    }
    lv_indev_delete(in);return 0;
  }
  if(training_growth_smoke){
    pp_ui_open(1);pp_dispose(&game);pp_init(&game,123);assert(pp_choose_starter(&game,1)==PP_OK);
    game.count=3;game.pets[1]=game.pets[2]=game.pets[0];game.pets[1].species=2;game.pets[2].species=67;
    game.growth[0]=game.growth[1]=game.growth[2]=(pp_growth){.level=15,.xp=299};game.identity[1]=game.identity[2]=game.identity[0];game.pats_today[2]=9;
    game.reserve_count=1;game.reserves[0]=game.pets[0];game.reserves[0].species=67;game.reserve_growth[0]=(pp_growth){.level=15};game.reserve_identity[0]=game.identity[0];game.reserve_pats_today[0]=9;
    game.coins=500;pp_training job;pp_training_init(&job,123);unsigned picks[]={0,1,2};
    assert(pp_training_job_start(&job,&game,0,picks,3)==PP_OK);pp_training_job_advance(&job,&game,PP_TRAINING_DURATION_MS);pp_training_close(&job);
    assert(pp_valid(&game)&&pp_training_growth_pending(&game));
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pp_ui_bind_input(in);
    pp_ui_screen(false);pp_ui_open(0);pump(200);assert(!pp_ui_evolving());pp_ui_screen(true);pump(100);
    assert(pp_ui_evolution_probe().level&&visible_labels(lv_screen_active(),"历练升级")==1);check_safe_buttons(lv_screen_active());
    if(shot)screenshot(shot);
    /* BOOT interruption and a real save/load leave the unacknowledged first notice pending. */
    pp_ui_boot_hold();pp_ui_open(1);unsigned bytes=pp_save_size(&game);void *blob=malloc(bytes);assert(blob&&pp_save_write(&game,blob,bytes,123));
    pp_dispose(&game);assert(pp_save_read(&game,blob,bytes,NULL));free(blob);pp_ui_open(0);pump(100);assert(pp_ui_evolution_probe().level&&!game.training.growth_seen);
    tap(in,180,290,80);pump(100);assert(pp_ui_evolution_probe().choice&&game.training.growth_seen==1);
    tap(in,180,260,80);pump(2400);assert(pp_ui_evolution_probe().ready&&game.growth[0].stage==1);
    tap(in,180,310,80);pump(100);
    /* A multi-level gain can qualify for the following stage; postpone it. */
    if(pp_ui_evolution_probe().choice){assert(game.growth[0].level>=32);tap(in,100,68,80);pump(100);}
    assert(pp_ui_evolution_probe().level&&game.training.evolution_seen==1);
    tap(in,180,290,80);pump(100);assert(pp_ui_evolution_probe().choice&&game.training.growth_seen==3);
    tap(in,180,302,80);pump(2400);assert(pp_ui_evolution_probe().ready&&pp_evolution_locked(&game,1));
    tap(in,180,310,80);pump(100);assert(pp_ui_evolution_probe().level&&game.training.evolution_seen==3);
    tap(in,180,290,80);pump(100);assert(pp_ui_evolution_probe().choice&&game.training.growth_seen==7);
    unsigned coins=game.coins;tap(in,180,241,80);assert(pp_ui_evolution_probe().choice);tap(in,100,68,80);assert(pp_ui_evolution_probe().choice);tap(in,180,241,80);tap(in,180,260,80);pump(2400);
    assert(pp_ui_evolution_probe().ready&&game.growth[2].stage==2&&game.coins==coins-200&&game.count==3);
    tap(in,180,310,80);pump(500);assert(!pp_ui_evolving()&&!pp_training_growth_pending(&game)&&game.training.evolution_seen==7&&pp_valid(&game));
    pp_ui_open(1);bytes=pp_save_size(&game);blob=malloc(bytes);assert(blob&&pp_save_write(&game,blob,bytes,123));pp_dispose(&game);assert(pp_save_read(&game,blob,bytes,NULL));free(blob);
    pp_ui_open(0);pump(1200);assert(!pp_ui_evolving());pp_ui_open(33);pump(100);assert(pp_training_view_mode()==4&&!pp_ui_evolving());
    puts("PASS: training growth: real one-hour XP, screen-off deferral, level before evolution, BOOT/save/load recovery, three partners, chained evolution/postpone, permanent refusal, courtyard Eevee branch/animation/coins/identity, no repeated notices after reload");
    lv_indev_delete(in);return 0;
  }
  if(training_smoke){
    /* Compare actual RGB565 panorama rows at odd/even offsets and the wrap seam. */
    lv_obj_t *sample=lv_image_create(lv_screen_active());char map_path[640];
    snprintf(map_path,sizeof(map_path),"%s%s/training/%u.bin",root?"S:":"",root?root:"S:/assets",training_scene);
    pp_scene_view_panorama_set(sample,map_path,0);
    const lv_image_dsc_t *map=lv_image_get_src(sample);assert(map->header.cf==LV_COLOR_FORMAT_RGB565);
    const uint16_t *pixels=(const uint16_t *)map->data;
    uint16_t *initial=malloc(360*120*sizeof(uint16_t));assert(initial);memcpy(initial,pixels+108*360,360*120*sizeof(uint16_t));
    unsigned map_reads=pp_image_probe().file_reads;
    const unsigned times[]={24,25,33,49,50,66,99,1000,8975,8999,9000,9001};
    for(unsigned t=0;t<sizeof(times)/sizeof(times[0]);t++){
      pp_scene_view_panorama_tick(sample,times[t]);unsigned shift=times[t]/25%360;
      for(unsigned y=0;y<120;y++)for(unsigned x=0;x<360;x++)assert(pixels[(108+y)*360+x]==initial[y*360+(x+shift)%360]);
    }
    assert(pp_image_probe().file_reads==map_reads);free(initial);lv_obj_delete(sample);pp_ui_open(1);
    puts("PASS: actual RGB565 panorama single-pixel shifts, 40px/s, odd/even rows, 9-second wrap seam and zero frame reads");
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pp_ui_bind_input(in);pump(100);
    tap_app(in,1);for(unsigned n=0;n<training_scene;n++)tap(in,320,180,80);
    tap(in,180,276,80);tap(in,104,110,80);if(training_party>1)tap(in,256,110,80);if(training_party>2)tap(in,180,110,80);tap(in,180,276,80);
    assert(game.training.status==1&&game.training.count==training_party&&pp_training_busy(&game,0));
    /* Render at travel cadence while the persisted simulation still advances in 40ms steps. */
    unsigned visual=0,reads_before=pp_image_probe().file_reads;
    for(unsigned frame=0;frame<30;frame++){
      for(unsigned ms=0;ms<PP_TRAINING_VIEW_FRAME_MS;ms++)pump(1);
      pp_training_actor actor=pp_training_view_actor(0);
      assert(actor.walk_ms==game.training.walk_ms+game.training.pending_ms);
      if(frame)assert(actor.walk_ms-visual==PP_TRAINING_VIEW_FRAME_MS);
      visual=actor.walk_ms;check_training_actors(root,pp_training_view_probe());check_partial_frame();
    }
    assert(pp_image_probe().file_reads==reads_before);check_safe_buttons(lv_screen_active());
    printf("PASS: 30 travel-cadence 33ms visual updates, no repeated visual clock, 40ms save logic retained, exact partial/full redraw and zero frame reads\n");
    unsigned elapsed=game.training.elapsed_ms;tap(in,180,310,80);assert(pp_ui_page()==1&&game.training.status==1);
    pp_ui_open(0);pump(100);assert(!pp_ui_home_probe(0).visible);pump(3000);assert(game.training.elapsed_ms>elapsed+3000);
    elapsed=game.training.elapsed_ms;pp_ui_screen(false);pump(3000);assert(game.training.elapsed_ms>=elapsed+2960);pp_ui_screen(true);
    pp_ui_open(33);assert(pp_training_view_mode()>=2&&game.training.scene==training_scene);
    for(unsigned n=0;n<600&&pp_training_view_probe()->phase!=PP_TRAINING_FIGHT;n++)pump(33);
    const pp_training *t=pp_training_view_probe();assert(t->phase==PP_TRAINING_FIGHT);pump(720);assert(!pp_battle_view_probe().entrance_mask);pump(560);assert(!t->battle.battle_round);pump(80);assert(t->battle.battle_round==1);
    unsigned reads=pp_image_probe().file_reads;
    if(training_profile){
      /* The device advances its coarse game clock at an unrelated second phase.
       * Every visible reply and HP tween must finish before the next round. */
      unsigned start=game.training.elapsed_ms+game.training.pending_ms;
      uint64_t seconds=game.training.anchor_seconds+start/1000;
      unsigned prior_round=t->battle.battle_round,prior_ally=t->battle.gym.ally,prior_enemy=t->battle.gym.enemy;
      int target[2]={t->battle.battle_hp,t->battle.enemy_hp};unsigned sides=0,damage_mask=0,checked=0,replies=0,knockouts=0,last_side=0;bool completed=false;
      pp_battle_view_state previous=pp_battle_view_probe();
      unsigned next_frame=0,frame_index=0;const unsigned frame_gaps[]={33,66,99,132};
      for(unsigned elapsed=0;elapsed<30000;elapsed+=33){
        game.seconds=seconds+(elapsed+990)/1000;
        /* Hardware reports every 50ms; rendering can stall across several frames. */
        for(unsigned ms=0;ms<33;ms++){
          lv_tick_inc(1);if((elapsed+ms)%50==0)pp_ui_screen(true);
          if(elapsed+ms>=next_frame){lv_timer_handler();next_frame+=frame_gaps[frame_index++%4];}
        }
        check_partial_frame();
        unsigned progressed=game.training.elapsed_ms+game.training.pending_ms-start;
        if(progressed>elapsed+73)fprintf(stderr,"coarse clock jumped: wall=%u job=%u\n",elapsed+33,progressed);
        assert(progressed<=elapsed+73);
        if(t->phase!=PP_TRAINING_FIGHT&&t->phase!=PP_TRAINING_RESULT){prior_round=0;sides=0;completed=false;continue;}
        unsigned round=t->battle.battle_round;
        bool swapped=prior_ally!=t->battle.gym.ally||prior_enemy!=t->battle.gym.enemy;
        if(round!=prior_round||swapped){
          assert(!prior_round||completed);
          pp_battle_view_state p=pp_battle_view_probe();
          for(unsigned side=0;side<2;side++)if(previous.actor[side]!=p.actor[side]){
            assert(previous.hp[side]==0);assert(damage_mask&(1u<<side));knockouts++;
          }
          prior_round=round;prior_ally=t->battle.gym.ally;prior_enemy=t->battle.gym.enemy;
          target[0]=t->battle.battle_hp;target[1]=t->battle.enemy_hp;sides=damage_mask=0;last_side=0;completed=swapped;
        }
        pp_battle_view_state p=pp_battle_view_probe();
        if(p.casting){assert(p.side>=last_side);last_side=p.side;sides|=1u<<p.side;}
        if(p.damage_visible){assert(p.casting&&p.damage_side==1-p.side&&p.loss[p.damage_side]>0);damage_mask|=1u<<p.damage_side;}
        for(unsigned side=0;side<2;side++)if(previous.actor[side]==p.actor[side]&&p.hp[side]<previous.hp[side])
          assert(p.damage_visible&&p.damage_side==side);
        previous=p;
        if(round&&!p.casting&&!p.entrance_mask&&!completed){
          assert(p.hp[0]==target[0]&&p.hp[1]==target[1]);assert(sides&1);
          if(target[1]>0){assert(sides&2);replies++;}
          for(unsigned side=0;side<2;side++)if(p.loss[side]>0)assert(damage_mask&(1u<<side));
          checked++;completed=true;
        }
      }
      assert(checked>=1&&replies>=1);
      game.seconds+=60;pump(33);unsigned caught=game.training.elapsed_ms+game.training.pending_ms;
      pump(990);unsigned continued=game.training.elapsed_ms+game.training.pending_ms;
      assert(continued>=caught+950&&continued<=caught+1030);
      printf("PASS: training device screen notifications / 33-132ms frames: %u complete HP rounds, %u enemy replies with hit numbers, %u KO relays after zero HP, alternating attacks\n",checked,replies,knockouts);
    }else for(unsigned frame=0;frame<80;frame++){pump(33);check_partial_frame();}
    assert(pp_image_probe().file_reads==reads);
    tap(in,180,310,80);assert(pp_ui_page()==1);elapsed=game.training.elapsed_ms;pump(61000);assert(game.training.elapsed_ms>elapsed+60900);
    assert(game.training.loot_checks>=1);pp_ui_open(33);check_safe_buttons(lv_screen_active());
    /* The persisted game clock supplies off-screen/offline catch-up. */
    game.seconds+=PP_TRAINING_DURATION_MS/1000;for(unsigned i=0;i<181&&game.training.status==1;i++)pump(33);
    assert(game.training.status==2&&game.training.elapsed_ms<=PP_TRAINING_DURATION_MS&&!pp_training_busy(&game,0));game.training.growth_seen=game.training.evolution_seen=7;pp_ui_open(33);pump(33);assert(pp_training_view_mode()==4);
    tap(in,180,320,80);pp_ui_open(0);pump(100);assert(pp_ui_home_probe(0).visible);
    /* Real pointer exits from exploration, warning, live combat and settled combat. */
    const unsigned exit_phases[]={PP_TRAINING_SEARCH,PP_TRAINING_APPROACH,PP_TRAINING_ALERT,PP_TRAINING_FIGHT,PP_TRAINING_RESULT};
    for(unsigned n=0;n<5;n++){
      for(unsigned i=0;i<game.count;i++){game.pets[i].energy=100;game.pets[i].food=90;game.pets[i].health=100;game.pets[i].sleeping=0;}
      pp_ui_open(33);tap(in,180,276,80);tap(in,180,276,80);tap(in,104,110,80);tap(in,180,276,80);
      for(unsigned frame=0;pp_training_view_probe()->phase!=exit_phases[n]&&frame<4000;frame++)pump(33);
      if(pp_training_view_probe()->phase!=exit_phases[n]||game.training.status!=1)fprintf(stderr,"exit n=%u mode=%u status=%u phase=%u count=%u sleep=%u energy=%u\n",n,pp_training_view_mode(),game.training.status,pp_training_view_probe()->phase,pp_training_view_probe()->count,game.pets[0].sleeping,game.pets[0].energy);assert(pp_training_view_probe()->phase==exit_phases[n]&&game.training.status==1);check_safe_buttons(lv_screen_active());assert(visible_labels(lv_screen_active(),"退出")==0);
      unsigned coins=game.coins,rounds=game.training.rounds,checks=game.training.loot_checks;
      if(training_scene==0&&n==3)screenshot("build/training-exit-active.ppm");
      tap(in,230,310,80);assert(game.training.status==2&&pp_training_view_mode()==4&&!pp_training_busy(&game,0)&&!pp_training_view_probe()->battle.battling&&pp_valid(&game));
      assert(game.coins==coins&&game.training.rounds==rounds&&game.training.loot_checks==checks&&visible_labels(lv_screen_active(),"退出")==0);
      if(training_scene==0&&n==3)screenshot("build/training-exit-complete.ppm");
      unsigned elapsed=game.training.elapsed_ms;pump(1000);assert(game.training.elapsed_ms==elapsed&&game.coins==coins);
      game.training.growth_seen=game.training.evolution_seen=7;pp_ui_open(0);pump(33);assert(pp_ui_home_probe(0).visible);
    }
    if(training_scene==0){
      pp_ui_open(11);tap(in,180,234,80);tap(in,250,125,80);pump(80);assert(pp_ui_auto_supply());
      for(unsigned i=0;i<game.count;i++){game.pets[i].energy=100;game.pets[i].food=90;game.pets[i].health=100;game.pets[i].clean=90;game.pets[i].mood=90;game.pets[i].sleeping=0;}
      pp_ui_open(33);tap(in,180,276,80);tap(in,180,276,80);tap(in,104,110,80);tap(in,180,276,80);
      assert(game.training.status==1&&pp_training_view_probe()->phase==PP_TRAINING_SEARCH);
      unsigned pick=game.training.picks[0];game.pets[pick].food=19;game.inventory[0]=2;
      pump(80);assert(game.pets[pick].food==59&&game.inventory[0]==1);
      pp_ui_open(0);game.pets[pick].energy=19;game.pets[pick].sleeping=1;game.inventory[4]=2;
      pp_ui_screen(false);pump(80);assert(game.pets[pick].energy==69&&!game.pets[pick].sleeping&&!game.inventory[4]);pp_ui_screen(true);
      pp_training_view_bind(&game);pump(80);assert(game.inventory[0]==1&&!game.inventory[4]);
      pp_ui_open(11);tap(in,180,234,80);tap(in,250,125,80);pump(80);assert(!pp_ui_auto_supply());
      game.pets[pick].food=19;pump(80);assert(game.pets[pick].food==19&&game.inventory[0]==1);
      tap(in,250,125,80);pump(80);assert(pp_ui_auto_supply()&&game.pets[pick].food==59&&!game.inventory[0]);
      pp_ui_open(33);tap(in,230,310,80);assert(game.training.status==2);pp_ui_auto_supply_load(0);
      puts("PASS: actual settings switch controls live/background/screen-off supply, two-biscuit waking, session restore, toggle off/on, once-only stock and exit");
    }
    printf("PASS: training job scene %u party %u: background/back/resume, hidden courtyard pet, screen-off progression, faster first round, exact partial redraw, no frame reads, loot, 1-hour expiry and return\n",training_scene,training_party);
    lv_indev_delete(in);return 0;
  }
  if(training_smoke || training_phase){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pp_ui_bind_input(in);pump(100);
    uint32_t original_rng=game.rng,coins=game.coins;uint64_t badges=game.gym.badges;
    if(training_smoke){tap_app(in,1);assert(pp_ui_page()==33);}
    for(unsigned n=0;n<training_scene;n++)tap(in,320,180,80);
    assert(pp_training_view_mode()==0);check_safe_buttons(lv_screen_active());
    if(training_smoke){for(unsigned n=0;n<6;n++){tap(in,320,180,80);check_safe_buttons(lv_screen_active());}assert(pp_training_view_probe()->phase==PP_TRAINING_SELECT);}
    if(training_smoke || strcmp(training_phase,"scene")){
      tap(in,180,276,80);assert(pp_training_view_mode()==1);check_safe_buttons(lv_screen_active());
      tap(in,104,110,80);if(training_party>1)tap(in,256,110,80);if(training_party>2)tap(in,180,110,80); /* Select only partners placed in the courtyard. */
      if(training_smoke || strcmp(training_phase,"team")){
        tap(in,180,276,80);assert(pp_training_view_mode()==2);const pp_training *t=pp_training_view_probe();assert(t->count==training_party&&t->picks[0]==0&&(training_party==1||t->picks[1]==2));check_safe_buttons(lv_screen_active());
        for(unsigned i=0;i<t->count;i++){pp_stored_pet p;assert(pp_training_pet(&game,t->picks[i],&p));char level[20];snprintf(level,sizeof(level),"Lv.%u",p.growth.level);assert(visible_labels(lv_screen_active(),level)>0);}char stats[96];snprintf(stats,sizeof(stats),"战斗 %u 次 · 经验 +%u XP",t->rounds,t->total_xp);assert(visible_labels(lv_screen_active(),stats)==1);
        bool saw_left=false,saw_right=true;
        if(training_smoke){
          unsigned reads=pp_image_probe().file_reads;uint16_t *partial=malloc(sizeof(export_frame));assert(partial);
          for(unsigned f=0;f<75;f++){unsigned walk_before=pp_training_view_actor(0).walk_ms;pump(PP_BATTLE_VIEW_FRAME_MS);assert(pp_training_view_actor(0).walk_ms-walk_before==PP_BATTLE_VIEW_FRAME_MS);bool left=check_training_actors(root,t);saw_left|=left;saw_right|=!left;lv_refr_now(lv_display_get_default());memcpy(partial,export_frame,sizeof(export_frame));lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());if(memcmp(partial,export_frame,sizeof(export_frame))){unsigned minx=360,miny=360,maxx=0,maxy=0;for(unsigned px=0;px<360*360;px++)if(partial[px]!=export_frame[px]){unsigned x=px%360,y=px/360;if(x<minx)minx=x;if(y<miny)miny=y;if(x>maxx)maxx=x;if(y>maxy)maxy=y;}fprintf(stderr,"training scene %u frame %u redraw differs %u,%u to %u,%u\n",training_scene,f,minx,miny,maxx,maxy);screenshot("build/training2-full.ppm");memcpy(export_frame,partial,sizeof(export_frame));screenshot("build/training2-partial.ppm");abort();}}
          free(partial);assert(pp_image_probe().file_reads==reads);
          unsigned elapsed=t->elapsed;pp_ui_screen(false);pump(30000);assert(t->elapsed==elapsed);pp_ui_screen(true);game.revision++;pp_ui_refresh();assert(pp_training_view_mode()==2);}
        if(training_smoke){
          for(unsigned n=0;n<400&&t->phase==PP_TRAINING_SEARCH;n++)pump(33);
          assert(t->phase==PP_TRAINING_APPROACH);unsigned clock=t->clock_ms;
          uint32_t rng=t->rng;unsigned art=t->foes[0].art;pp_training_actor moving=pp_training_view_actor(0);
          pp_ui_screen(false);pump(30000);assert(t->clock_ms==clock&&pp_training_view_actor(0).walk_ms==moving.walk_ms);pp_ui_screen(true);
          assert(t->rng==rng&&t->foes[0].art==art);
        }
        if(training_smoke || strcmp(training_phase,"search")){
          for(unsigned n=0;n<400&&(t->phase==PP_TRAINING_SEARCH||t->phase==PP_TRAINING_APPROACH);n++){pump(33);bool left=check_training_actors(root,t);saw_left|=left;saw_right|=!left;}
          assert(t->phase==PP_TRAINING_ALERT&&pp_training_view_mode()==2);
          if(training_smoke){
            assert(!saw_left&&saw_right&&t->clock_ms==0);unsigned revision=game.revision,reads=pp_image_probe().file_reads;
            pp_training_actor stopped=pp_training_view_actor(0);uint32_t rng=t->rng;
            for(unsigned n=0;n<49;n++){
              pump(33);check_training_actors(root,t);pp_training_actor a=pp_training_view_actor(0);
              assert(a.x==stopped.x&&a.y==stopped.y&&a.left==stopped.left&&a.walk_ms==stopped.walk_ms&&a.alert&&pp_training_view_mode()==2);
              if(n==9){unsigned clock=t->clock_ms;pp_ui_screen(false);pump(30000);assert(t->clock_ms==clock);pp_ui_screen(true);}
            }
            assert(t->clock_ms>=1900&&t->clock_ms<2000&&t->rng==rng&&game.revision==revision&&pp_image_probe().file_reads==reads);
          }
          if(training_smoke||strcmp(training_phase,"alert")){
          for(unsigned n=0;n<70&&pp_training_view_mode()!=3;n++)pump(33);
          assert(pp_training_view_mode()==3);assert(pp_battle_team_count(&t->battle)==training_party);check_safe_buttons(lv_screen_active());
          if(training_smoke){
            memset(&scene_metrics,0,sizeof(scene_metrics));scene_measuring=training_profile;
            pp_battle_view_state entry=pp_battle_view_probe();assert(entry.entrance_mask==3&&!entry.casting&&entry.pet_opacity[0]==0&&entry.pet_opacity[1]==0);
            unsigned round=t->battle.battle_round;int hp=t->battle.battle_hp,foe=t->battle.enemy_hp;unsigned reads=pp_image_probe().file_reads;
            pump(250);assert(pp_battle_view_probe().entrance_mask==3&&pp_battle_view_probe().pet_opacity[0]==0);
            unsigned clock=pp_battle_view_probe().entrance_ms;pp_ui_screen(false);pump(30000);assert(pp_battle_view_probe().entrance_ms==clock);pp_ui_screen(true);
            pump(250);entry=pp_battle_view_probe();assert(entry.entrance_mask==3&&entry.pet_opacity[0]>0&&entry.pet_opacity[0]<255&&!entry.casting);
            pump(600);entry=pp_battle_view_probe();assert(!entry.entrance_mask&&entry.pet_opacity[0]==255&&entry.pet_opacity[1]==255);
            assert(t->battle.battle_round==round&&t->battle.battle_hp==hp&&t->battle.enemy_hp==foe&&pp_image_probe().file_reads==reads);
            scene_measuring=false;
            if(training_profile)printf("ENTRY_PROFILE {\"frames\":%llu,\"pixels\":%llu,\"flushes\":%llu,\"fullFrames\":%llu}\n",(unsigned long long)scene_metrics.frames,(unsigned long long)scene_metrics.pixels,(unsigned long long)scene_metrics.flushes,(unsigned long long)scene_metrics.full);
          }
          if(training_smoke){
            uint16_t *partial=malloc(sizeof(export_frame));assert(partial);unsigned reads=pp_image_probe().file_reads;
            for(unsigned f=0;f<80;f++){pump(33);lv_refr_now(lv_display_get_default());memcpy(partial,export_frame,sizeof(export_frame));lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());assert(!memcmp(partial,export_frame,sizeof(export_frame)));}
            assert(pp_image_probe().file_reads==reads);free(partial);
          }
          if(training_smoke){uint64_t seconds=t->battle.seconds;pp_ui_screen(false);pump(30000);assert(t->battle.seconds==seconds);pp_ui_screen(true);}
          if(training_smoke || !strcmp(training_phase,"result")){
            memset(&scene_metrics,0,sizeof(scene_metrics));scene_measuring=training_profile;uint64_t profile_start=SDL_GetPerformanceCounter();
            for(unsigned n=0;n<10000&&t->phase!=PP_TRAINING_RESULT;n++)pump(33);
            scene_measuring=false;
            if(training_profile)printf("TRAINING_PROFILE {\"frames\":%llu,\"pixels\":%llu,\"flushes\":%llu,\"fullFrames\":%llu,\"hostMs\":%.3f}\n",(unsigned long long)scene_metrics.frames,(unsigned long long)scene_metrics.pixels,(unsigned long long)scene_metrics.flushes,(unsigned long long)scene_metrics.full,(double)(SDL_GetPerformanceCounter()-profile_start)*1000/SDL_GetPerformanceFrequency());
            assert(t->phase==PP_TRAINING_RESULT&&t->awarded&&t->last_xp>0&&t->rounds==1);
            unsigned revision=game.revision;pump(500);assert(game.revision==revision);
            if(training_smoke){for(unsigned n=0;n<500&&pp_training_view_mode()!=2;n++)pump(33);assert(pp_training_view_mode()==2&&t->phase==PP_TRAINING_SEARCH);for(unsigned i=0;i<t->count;i++){pp_stored_pet p;assert(pp_training_pet(&game,t->picks[i],&p));char level[20];snprintf(level,sizeof(level),"Lv.%u",p.growth.level);assert(visible_labels(lv_screen_active(),level)>0);}char stats[96];snprintf(stats,sizeof(stats),"战斗 %u 次 · 经验 +%u XP",t->rounds,t->total_xp);assert(visible_labels(lv_screen_active(),stats)==1);}
          }
          }
        }
      }
    }
    if(training_smoke){
      assert(game.rng==original_rng&&game.coins==coins&&game.gym.badges==badges);
      for(unsigned n=0;n<400&&(pp_training_view_probe()->phase==PP_TRAINING_SEARCH||pp_training_view_probe()->phase==PP_TRAINING_APPROACH);n++)pump(33);
      assert(pp_training_view_probe()->phase==PP_TRAINING_ALERT);unsigned revision=game.revision;
      unsigned reads=pp_image_probe().file_reads;tap(in,180,320,80);assert(pp_training_view_mode()==0&&pp_training_view_probe()->phase==PP_TRAINING_SELECT&&game.revision==revision);
      if(training_scene==0&&training_party==3){
        tap(in,180,276,80);tap(in,180,276,80);assert(pp_training_view_mode()==2);
        for(unsigned n=0;n<500&&pp_training_view_mode()!=3;n++)pump(33);
        assert(pp_training_view_mode()==3&&pp_battle_view_probe().entrance_mask==3);revision=game.revision;
        tap(in,180,320,80);assert(pp_training_view_mode()==0&&!pp_battle_view_probe().active&&game.revision==revision);
      }
      tap(in,180,320,80);assert(pp_ui_page()==1);tap_app(in,13);assert(pp_ui_page()==8);tap(in,180,320,80);assert(pp_ui_page()==1);tap_app(in,1);assert(pp_training_view_mode()==0);tap(in,180,52,1150);assert(pp_ui_page()==33);pp_ui_boot_hold();assert(pp_ui_page()==0);assert(pp_image_probe().file_reads==reads);
      printf("PASS: training scene %u, %u partners, initial two-ball release/pause/reveal before attacks, side-scroll formation/live levels, visible foe approach/pause, frozen 2000ms collision warning, exact battle partial redraw, warning pause/cancel, equal XP once, automatic resume, pause, retained collection, touch hold ignored/BOOT-home cleanup and zero new file reads\n",training_scene,training_party);lv_indev_delete(in);return 0;
    }
    if(training_phase&&(!strcmp(training_phase,"complete")||!strcmp(training_phase,"complete-all")||!strcmp(training_phase,"complete-empty"))){game.seconds+=PP_TRAINING_DURATION_MS/1000;for(unsigned i=0;i<181&&game.training.status==1;i++)pump(33);assert(game.training.status==2);if(strcmp(training_phase,"complete"))for(unsigned i=0;i<PP_SHOP_ITEMS;i++)game.training.items[i]=!strcmp(training_phase,"complete-all")?i+1:0;game.training.growth_seen=game.training.evolution_seen=7;pp_ui_open(33);pump(33);assert(pp_training_view_mode()==4);}
    lv_indev_delete(in);
  }
  if(hospital_smoke || hospital_phase){
    lv_tick_set_cb(NULL);if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(100);
    pp_pet hospital_before[PP_PETS];memcpy(hospital_before,game.pets,sizeof(hospital_before));
    uint32_t before=pp_checksum(&game),coins=game.coins;
    if(hospital_smoke){tap_app(in,8);assert(pp_ui_page()==32);check_safe_buttons(lv_screen_active());}
    assert(pp_hospital_view_probe().phase==PP_HOSPITAL_READY);
    if(hospital_smoke){
      unsigned reads=pp_image_probe().file_reads,rng=game.rng;
      pump(10000);assert(pp_checksum(&game)==before&&pp_image_probe().file_reads==reads&&game.rng==rng);
      uint16_t *partial=malloc(sizeof(export_frame));assert(partial);
      for(unsigned frame=0;frame<100;frame++){
        pump(33);lv_refr_now(lv_display_get_default());memcpy(partial,export_frame,sizeof(export_frame));
        lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());assert(!memcmp(partial,export_frame,sizeof(export_frame)));
      }
      pp_ui_screen(false);lv_refr_now(lv_display_get_default());memcpy(partial,export_frame,sizeof(export_frame));
      pump(10000);lv_refr_now(lv_display_get_default());assert(!memcmp(partial,export_frame,sizeof(export_frame)));
      pp_ui_screen(true);bool resumed_motion=false;for(unsigned frame=0;frame<30;frame++){pump(33);lv_refr_now(lv_display_get_default());resumed_motion|=memcmp(partial,export_frame,sizeof(export_frame))!=0;}assert(resumed_motion);
      free(partial);assert(pp_checksum(&game)==before&&pp_image_probe().file_reads==reads&&game.rng==rng);
      puts("PASS: clinic particles/pets 100 partial/full frames at 33ms match, screen pause/resume and zero ambient rewards/RNG/frame reads");
    }
    if(hospital_smoke || strcmp(hospital_phase,"ready")){
      tap(in,126,276,80);assert(pp_hospital_view_probe().phase==PP_HOSPITAL_PLAY);check_safe_buttons(lv_screen_active());
      if(hospital_smoke){
        uint32_t elapsed=pp_hospital_view_probe().elapsed;pp_ui_screen(false);pump(30000);assert(pp_hospital_view_probe().elapsed==elapsed&&pp_checksum(&game)==before);pp_ui_screen(true);
        game.revision++;pp_ui_refresh();assert(pp_hospital_view_probe().phase==PP_HOSPITAL_PLAY);
        uint16_t *partial=malloc(sizeof(export_frame));assert(partial);
        unsigned reads=pp_image_probe().file_reads,rng=game.rng;uint32_t play_before=pp_checksum(&game);
        for(unsigned frame=0;frame<20;frame++){
          uint32_t previous=pp_hospital_view_probe().elapsed;pump(33);
          assert(pp_hospital_view_probe().elapsed-previous==33);
          lv_refr_now(lv_display_get_default());memcpy(partial,export_frame,sizeof(export_frame));
          lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());assert(!memcmp(partial,export_frame,sizeof(export_frame)));
        }
        free(partial);assert(pp_checksum(&game)==play_before&&pp_image_probe().file_reads==reads&&game.rng==rng);
        puts("PASS: hospital drop 20 real 33ms updates, partial/full pixels match, zero extra reads/RNG/rewards");
      }
      if(hospital_phase&&!strcmp(hospital_phase,"play"))pump(500);
      else {
        bool miss=hospital_phase&&!strcmp(hospital_phase,"miss");
        while(pp_hospital_view_probe().phase==PP_HOSPITAL_PLAY){
          pp_hospital h=pp_hospital_view_probe();unsigned lane=pp_hospital_cloud(&h)?(miss?h.lane:(h.lane+1)%3):(miss?(h.lane+1)%3:h.lane);
          tap(in,104+(int)lane*76,200,30);h=pp_hospital_view_probe();
          unsigned age=h.elapsed-h.slot*PP_HOSPITAL_SLOT_MS;pump(PP_HOSPITAL_SLOT_MS-age+5);
        }
        pp_hospital h=pp_hospital_view_probe();assert(h.phase==(miss?PP_HOSPITAL_MISSED:PP_HOSPITAL_WON));check_safe_buttons(lv_screen_active());
        if(miss)assert(pp_checksum(&game)==before);
        else {assert(h.claimed&&h.treated==game.count&&game.coins==coins);for(unsigned n=0;n<game.count;n++){pp_pet p=game.pets[n];assert(p.health==(hospital_before[n].health<60?60:hospital_before[n].health)&&p.food==hospital_before[n].food&&p.clean==hospital_before[n].clean&&p.mood==hospital_before[n].mood&&p.energy==hospital_before[n].energy&&p.sleeping==hospital_before[n].sleeping);}}
      }
    }
    if(hospital_smoke){
      unsigned rev=game.revision,reads=pp_image_probe().file_reads;pump(5000);pp_ui_refresh();assert(game.revision==rev&&pp_image_probe().file_reads==reads);
      tap(in,180,276,80);assert(pp_ui_page()==1);tap_app(in,8);assert(pp_hospital_view_probe().phase==PP_HOSPITAL_READY&&game.revision==rev);
      tap(in,180,312,80);assert(pp_ui_page()==1);tap_app(in,12);assert(pp_ui_page()==15);lv_indev_delete(in);
      puts("PASS: hospital real pointer catch/recovery once, screen pause, safe controls, clean re-entry, retained encyclopedia, zero idle reads");return 0;
    }
    lv_indev_delete(in);
  }
  if(fishing_smoke || fishing_phase){
    lv_tick_set_cb(NULL);if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(100);
    if(fishing_smoke){tap_app(in,5);assert(pp_ui_page()==31);check_safe_buttons(lv_screen_active());}
    if(fishing_smoke&&shot){
      uint16_t *partial=malloc(sizeof(export_frame));assert(partial);
      unsigned reads=pp_image_probe().file_reads;uint32_t rng=game.rng,revision=game.revision;
      for(unsigned frame=0;frame<100;frame++){
        pump(33);lv_refr_now(lv_display_get_default());memcpy(partial,export_frame,sizeof(export_frame));
        lv_obj_invalidate(lv_screen_active());lv_refr_now(lv_display_get_default());
        assert(!memcmp(partial,export_frame,sizeof(export_frame)));
      }
      assert(pp_image_probe().file_reads==reads&&game.rng==rng&&game.revision==revision);free(partial);
      puts("PASS: 100 fishing scenery partial/full frames match; stable game RNG/revision and zero frame reads");
    }
    if(fishing_smoke){
      tap(in,320,158,80);assert(pp_fishing_view_mode()==1);check_safe_buttons(lv_screen_active());
      tap(in,320,180,80);assert(pp_fishing_view_mode()==1);tap(in,141,122,80);assert(pp_fishing_view_mode()==0&&game.fishing_partner==7);
      tap(in,320,202,80);assert(pp_fishing_view_mode()==2);for(unsigned i=0;i<4;i++)tap(in,320,180,80);check_safe_buttons(lv_screen_active());
      tap(in,258,126,80);assert(pp_fishing_view_mode()==3);tap(in,180,312,80);assert(pp_fishing_view_mode()==2);tap(in,180,312,80);assert(pp_fishing_view_mode()==0);
    }
    unsigned coins=game.coins,reads=pp_image_probe().file_reads;unsigned caught_reward=0;
    bool menu_phase=fishing_phase&&(!strcmp(fishing_phase,"partner")||!strcmp(fishing_phase,"dex")||!strcmp(fishing_phase,"detail")||!strcmp(fishing_phase,"detail-last")||!strncmp(fishing_phase,"dex-",4));
    if(menu_phase){tap(in,320,!strcmp(fishing_phase,"partner")?158:202,80);if(!strncmp(fishing_phase,"dex-",4))for(unsigned i=1;i<(unsigned)atoi(fishing_phase+4);i++)tap(in,320,180,80);if(!strcmp(fishing_phase,"detail-last")){for(unsigned i=0;i<4;i++)tap(in,320,180,80);tap(in,258,126,80);}else if(!strcmp(fishing_phase,"detail"))tap(in,102,120,80);}
    if(!menu_phase&&(fishing_smoke || strcmp(fishing_phase,"ready"))) {
      tap(in,180,276,80);pp_fishing f=pp_fishing_view_probe();assert(f.phase==PP_FISH_WAIT);
      if(fishing_smoke){
        game.revision++;pp_ui_refresh();pump(100);assert(pp_fishing_view_probe().started==f.started);
        pp_fishing_view_screen(false);pump(30000);assert(pp_fishing_view_probe().phase==PP_FISH_WAIT && game.coins==coins);
        pp_fishing_view_screen(true);f=pp_fishing_view_probe();
        tap(in,320,202,80);assert(pp_fishing_view_mode()==2);pp_fishing frozen=pp_fishing_view_probe();
        pump(9000);pp_fishing_view_screen(false);pump(9000);pp_fishing_view_screen(true);pump(9000);
        assert(pp_fishing_view_probe().started==frozen.started && pp_fishing_view_probe().phase==PP_FISH_WAIT);
        tap(in,180,312,80);assert(pp_fishing_view_mode()==0);f=pp_fishing_view_probe();
      }
      if(fishing_smoke || strcmp(fishing_phase,"wait")) {
        uint32_t reel_at=f.started+f.wait_ms;
        uint32_t aim=reel_at+f.target*10;
        if(fishing_phase && !strcmp(fishing_phase,"reel"))pump(reel_at+200-lv_tick_get());
        else if(fishing_phase && !strcmp(fishing_phase,"miss"))pump(reel_at+PP_FISH_REEL_MS+160-lv_tick_get());
        else {pump(aim-80-lv_tick_get());tap(in,180,276,80);caught_reward=pp_fishing_view_probe().reward;assert(pp_fishing_view_probe().caught && caught_reward>0 && game.coins==coins+caught_reward);}
      }
    }
    if(fishing_smoke){
      assert(pp_valid(&game));tap(in,180,276,80);assert(pp_fishing_view_probe().phase==PP_FISH_WAIT && game.coins==coins+caught_reward);
      pp_fishing f=pp_fishing_view_probe();pump(f.started+f.wait_ms+PP_FISH_REEL_MS+160-lv_tick_get());
      assert(pp_fishing_view_probe().phase==PP_FISH_RESULT && !pp_fishing_view_probe().reward && game.coins==coins+caught_reward);
      tap(in,180,312,80);assert(pp_ui_page()==1 && pp_ui_app_page()==0);
      tap_app(in,5);assert(pp_fishing_view_probe().phase==PP_FISH_READY && game.coins==coins+caught_reward);
      tap(in,180,312,80);assert(pp_ui_page()==1 && pp_image_probe().file_reads==reads);
      lv_indev_delete(in);puts("PASS: APP fishing entry/return, real cast/reel weighted fish once, timeout, wake pause, clean re-entry, round-safe controls and zero extra reads");return 0;
    }
    lv_indev_delete(in);
  }
  if(menu_smoke){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(100);
    uint32_t before=pp_checksum(&game),reads=pp_image_probe().file_reads,rng=game.rng;
    assert(pp_ui_app_page()==0);check_safe_buttons(lv_screen_active());
    tap(in,320,180,80);assert(pp_ui_app_page()==1);check_safe_buttons(lv_screen_active());
    tap(in,40,180,80);assert(pp_ui_app_page()==0);
    const unsigned targets[]={5,33,17,16,9,31,14,10,32,20,24,11,15,8};
    for(unsigned i=0;i<14;i++){
      tap_app(in,i);
      assert(pp_ui_page()==targets[i]);check_safe_buttons(lv_screen_active());
      tap(in,180,312,80);assert(pp_ui_page()==1);check_safe_buttons(lv_screen_active());
    }
    assert(pp_checksum(&game)==before && !pp_legend_unlocked(&game));
    reads=pp_image_probe().file_reads;pump(45000);assert(pp_image_probe().file_reads==reads && game.rng==rng);
    pp_ui_open(9);pump(100);assert(pp_ui_scene_selected()==game.scene);check_safe_buttons(lv_screen_active());
    for(unsigned i=0;i<PP_SCENES;i++){
      tap(in,320,180,80);unsigned selected=(i+1)%PP_SCENES;
      assert(pp_ui_scene_selected()==selected);check_safe_buttons(lv_screen_active());
      char path[640];snprintf(path,sizeof(path),"%s%s/scenes/%u.bin",root?"S:":"",root?root:"S:/assets",selected);
      lv_obj_t *im=visible_image(lv_screen_active(),pp_image_descriptor(path));assert(im);
      lv_obj_t *window=lv_obj_get_parent(im);lv_area_t area;lv_obj_get_coords(window,&area);
      assert(area.x1==0 && area.y1==108 && area.x2==359 && area.y2==251);
      assert(lv_obj_get_x(im)==0 && lv_obj_get_y(im)==-108 && lv_obj_get_width(im)==360 && lv_obj_get_height(im)==360);
    }
    for(unsigned i=0;i<PP_SCENES;i++){tap(in,40,180,80);assert(pp_ui_scene_selected()==PP_SCENES-1-i);}
    assert(pp_checksum(&game)==before);
    tap(in,180,278,80);assert(pp_checksum(&game)==before); /* active disabled */
    tap(in,320,180,80);game.coins=0;before=pp_checksum(&game);
    tap(in,180,278,80);assert(pp_checksum(&game)==before && game.scene==0 && !(game.scenes&2));
    game.coins=2000;
    for(unsigned d=0;pp_prosperity(&game)<pp_scene_required(1);d++){assert(d<PP_DEX_COUNT);game.dex_history[d/8]|=1u<<(d%8);}
    pp_tick(&game,0,0);pp_ui_open(9);tap(in,180,278,80);assert(pp_ui_page()==0 && game.scene==1 && (game.scenes&2) && game.coins==2000);
    pp_ui_open(9);assert(pp_ui_scene_selected()==1);
    before=pp_checksum(&game);tap(in,180,278,80);assert(pp_checksum(&game)==before&&pp_ui_page()==9);
    tap(in,40,180,80);tap(in,180,278,80);assert(pp_ui_page()==0&&game.scene==0&&game.coins==2000);
    pp_ui_open(9);tap(in,320,180,80);tap(in,180,278,80);assert(pp_ui_page()==0&&game.scene==1&&game.coins==2000);
    game.scene=0;game.scenes=1;game.coins=10000;
    for(unsigned d=0;pp_prosperity(&game)<pp_scene_required(11)-1;d++){assert(d<PP_DEX_COUNT);game.dex_history[d/8]|=1u<<(d%8);}
    pp_tick(&game,0,0);pp_ui_open(1);pp_ui_open(9);pump(100);
    tap(in,40,180,80);assert(pp_ui_scene_selected()==PP_SCENE_REWARD);before=pp_checksum(&game);
    tap(in,180,278,80);assert(pp_checksum(&game)==before&&pp_ui_page()==9&&!(game.scenes&(1u<<PP_SCENE_REWARD)));
    tap(in,40,180,80);assert(pp_ui_scene_selected()==11);tap(in,180,278,80);assert(pp_ui_page()==9&&game.scene==0);
    for(unsigned d=0;pp_prosperity(&game)<pp_scene_required(11);d++){assert(d<PP_DEX_COUNT);game.dex_history[d/8]|=1u<<(d%8);}
    pp_tick(&game,0,0);pp_ui_open(9);tap(in,180,278,80);assert(pp_ui_page()==0&&game.coins==10000&&game.scene==11&&game.scenes==PP_SCENE_REQUIRED_MASK);
    pp_ui_open(9);tap(in,320,180,80);before=pp_checksum(&game);tap(in,180,278,80);assert(pp_ui_page()==9&&pp_checksum(&game)==before);
    for(unsigned d=0;pp_prosperity(&game)<pp_scene_required(PP_SCENE_REWARD);d++){assert(d<PP_DEX_COUNT);game.dex_history[d/8]|=1u<<(d%8);}
    pp_tick(&game,0,0);pp_ui_open(9);tap(in,180,278,80);assert(pp_ui_page()==0&&game.scene==PP_SCENE_REWARD&&game.coins==10000);
    reads=pp_image_probe().file_reads;pump(45000);assert(pp_image_probe().file_reads==reads && game.rng==rng && pp_valid(&game));
    lv_indev_delete(in);puts("PASS: scene13 prosperity threshold, locked below 125, free unlock/use; APP fourteen ordered actual card entries, both pagination directions, all returns, legendary lock; scene wrap/current/owned/locked, prosperity unlock without spending coins, reopen selection, circular-safe controls, idle zero image reads/RNG changes");return 0;
  }
  if(gift_smoke){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    pump(100);assert(pp_ui_gift_pending());check_safe_buttons(lv_screen_active());
    unsigned total=pp_collected(&game),item=game.gift.item;uint32_t reads=pp_image_probe().file_reads;
    tap(in,110,290,80);assert(pp_ui_page()==0 && game.gift.pending && pp_ui_gift_pending());
    pp_ui_open(1);pump(100);assert(!pp_ui_gift_pending() && game.gift.pending);
    pp_ui_open(0);pump(100);assert(pp_ui_gift_pending());
    pp_tick(&game,86400,0);pump(100);assert(pp_ui_gift_pending() && pp_collected(&game)==total);
    pump(2000);assert(pp_image_probe().file_reads==reads);
    tap(in,180,312,80);assert(!pp_ui_gift_pending() && !game.gift.pending && pp_has(&game,item) && pp_collected(&game)==total);
    tap(in,180,312,80);assert(pp_collected(&game)==total && pp_valid(&game));
    lv_indev_delete(in);puts("PASS: gift modal pointer blocking, circular safety, page return, persistent receipt, no expiry/reward replay or frame image reads");return 0;
  }
  if(legend_smoke){
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    unsigned reads=0;
    game.gym.badges=127;pp_ui_open(24);pump(100);check_safe_buttons(lv_screen_active());tap(in,94,130,80);assert(pp_ui_page()==25);
    uint32_t before=pp_checksum(&game);tap(in,180,271,80);assert(!game.battling && pp_checksum(&game)==before);check_safe_buttons(lv_screen_active());
    game.gym.badges=255;pp_ui_open(1);pump(100);if(pp_ui_app_page())tap(in,40,180,80);check_safe_buttons(lv_screen_active());tap_app(in,10);assert(pp_ui_page()==24);
    tap(in,94,130,80);assert(pp_ui_page()==25);tap(in,180,271,80);assert(pp_ui_page()==6 && game.legend.active==1 && game.battling);check_safe_buttons(lv_screen_active());check_battle_release();reads=pp_image_probe().file_reads;
    tap(in,180,312,80);assert(pp_ui_page()==0);tap(in,180,120,80);assert(pp_ui_page()==6 && game.battling);
    game.enemy_hp=1;pp_tick(&game,3,0);pp_ui_refresh();pump(18000);assert(pp_ui_page()==23 && game.legend.eggs[0]==1 && game.count==1);check_safe_buttons(lv_screen_active());
    tap(in,180,312,80);assert(pp_ui_page()==24 && !game.legend.active && game.legend.eggs[0]==1);
    pp_ui_open(2);pump(100);check_safe_buttons(lv_screen_active());tap_bag_item(in,9,80);assert(pp_ui_page()==0 && game.legend.hatching==1 && !game.legend.eggs[0]);
    unsigned duration=(unsigned)(game.egg_end-game.seconds);pp_tick(&game,duration,0);assert(game.count==1);assert(pp_shake(&game)==PP_OK);pp_tick(&game,1,0);pp_ui_refresh();pump(100);assert(game.count==2 && game.pets[1].species==pp_pokedex[pp_legend_dex(0)].species && !game.legend.hatching && pp_valid(&game));
    assert(pp_image_probe().file_reads==reads);lv_indev_delete(in);
    puts("PASS: legendary APP pagination, seven-badge lock, target/partner confirmation, battle return/resume, final-impact egg receipt, once-only bag award, bag selection to courtyard exact hatch, circular safety and no new image reads");
    in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    for(unsigned i=0;i<PP_LEGEND_COUNT;i++) {
      pp_init(&game,42);pp_choose_starter(&game,1);game.gym.badges=255;game.legend.defeated=31;
      for(unsigned p=1;p<5;p++){game.pets[p]=game.pets[0];game.pets[p].species=p+1;game.growth[p]=(pp_growth){.level=60};}game.count=5;
      pp_ui_open(24);pump(100);check_safe_buttons(lv_screen_active());
      tap(in,i<3?88+i*92:134+(i-3)*92,i<3?135:230,80);assert(pp_ui_page()==25);check_safe_buttons(lv_screen_active());
      tap(in,178,218,80);assert(pp_ui_page()==25);check_safe_buttons(lv_screen_active());
      tap(in,180,276,80);assert(game.battling && game.battle_pet==2 && game.legend.active==i+1 && pp_valid(&game));check_battle_release();
    }
    lv_indev_delete(in);puts("PASS: all five themed cards, victory marks, full five-partner picker, actual target and partner selection, ball release/pause/reveal and circular-safe controls");return 0;
  }
  if(wild_smoke) {
    wild_fixture("alert");pp_ui_open(0);pump(100);
    pp_tick(&game,59,0);pp_ui_refresh();pump(100);assert(pp_ui_wild_alert() && !pp_ui_wild_pending());
    pp_tick(&game,1,0);pp_ui_refresh();pump(100);assert(!game.wild.phase && !pp_ui_wild_alert() && !pp_ui_wild_pending());
    wild_fixture("alert");pp_ui_open(0);pump(100);
    unsigned alert_reads=pp_image_probe().file_reads;
    assert(pp_ui_wild_alert() && !pp_ui_wild_pending() && !pp_wild_opened(&game));
    uint32_t before_alert=pp_checksum(&game);assert(pp_wild_start(&game,0)==PP_NOT_READY && pp_checksum(&game)==before_alert);
    pp_tick(&game,2,0);pump(2000);assert(pp_ui_wild_alert() && !pp_ui_wild_pending());
    pp_ui_open(1);pump(100);assert(!pp_ui_wild_alert() && !pp_ui_wild_pending());
    pp_ui_open(0);pump(100);assert(pp_ui_wild_alert() && pp_wild_alert_remaining(&game)==58);
    pp_tick(&game,1,0);pump(1000);assert(pp_ui_wild_alert() && !pp_ui_wild_pending());
    lv_indev_t *held=lv_indev_create();lv_indev_set_type(held,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(held,pointer);
    point=(lv_point_t){226,257};pressed=true;lv_indev_read(held);pump(100);
    pp_tick(&game,56,0);pp_ui_refresh();pump(100);
    pressed=false;lv_indev_read(held);pump(100);
    assert(pp_ui_wild_alert() && !pp_ui_wild_pending());
    /* The spawn uses the emptiest of three fixed spots. Tap its top marker. */
    for(unsigned x=96;x<=260 && !pp_ui_wild_pending();x+=80)tap(held,x,166,80);
    lv_indev_delete(held);
    assert(!game.battling && !pp_ui_wild_alert() && pp_ui_wild_pending() && pp_wild_opened(&game));
    assert(pp_image_probe().file_reads==alert_reads);
    puts("PASS: encountered sprite with pixel alert, click-only modal after prolonged notice, no automatic countdown, menu cleanup/return and zero extra reads");
    wild_fixture("pending");pp_ui_open(0);pump(100);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    assert(pp_ui_wild_pending() && !game.battling && pp_wild_opened(&game));
    assert(check_wild_ball_controls(lv_screen_active())==2);
    unsigned reads=pp_image_probe().file_reads;pp_home_state before=pp_ui_home_probe(0);
    pp_tick(&game,14,0);pp_ui_refresh();pump(100);assert(pp_ui_wild_pending() && pp_wild_opened(&game));
    tap(in,180,300,80);assert(pp_ui_page()==0 && !game.battling); /* modal blocks underlying BAG */
    pp_tick(&game,120,0);pp_ui_refresh();pump(100);assert(pp_ui_wild_pending() && pp_ui_page()==0 && !game.battling);
    assert(pp_image_probe().file_reads==reads);pp_home_state after=pp_ui_home_probe(0);assert(abs(after.x-before.x)<60 && abs(after.y-before.y)<60);
    wild_fixture("pending");pp_ui_open(0);pump(100);tap(in,136,270,80);assert(!pp_ui_wild_pending() && !game.battling && pp_ui_page()==0);
    wild_fixture("pending");game.count=2;game.pets[1]=game.pets[0];game.pets[1].species=2;game.growth[1].level=30;
    pp_ui_open(0);pump(100);assert(pp_ui_wild_fighter()==1);tap(in,320,180,80);assert(pp_ui_wild_fighter()==1 && pp_ui_wild_pending());
    game.pets[1].sleeping=1;pp_ui_refresh();pump(100);assert(pp_ui_wild_fighter()==0);
    game.pets[0].sleeping=1;pp_ui_refresh();pump(100);assert(pp_ui_wild_fighter()==PP_PETS && visible_labels(lv_screen_active(),"暂无可出战伙伴")==1);
    tap(in,224,270,80);assert(pp_ui_wild_pending() && !game.battling);
    game.pets[0].sleeping=game.pets[1].sleeping=0;pp_ui_refresh();pump(100);assert(pp_ui_wild_fighter()==1);
    tap(in,224,270,80);assert(pp_ui_page()==6 && game.battling && game.wild.phase==PP_WILD_BATTLE && game.battle_pet==pp_ui_wild_fighter());check_battle_release();
    tap(in,180,312,80);assert(pp_ui_page()==0 && game.battling);tap(in,180,120,80);assert(pp_ui_page()==6 && game.battling);
    for(unsigned n=0;n<65 && game.battling;n++){pp_tick(&game,3,0);pp_ui_refresh();pump(3000);}
    pump(16000);assert(pp_ui_page()==23 && game.wild.phase==PP_WILD_WON && game.wild.outcome==PP_WILD_CAPTURE_READY);
    game.balls=1;pp_ui_open(23);tap(in,220,302,80);
    assert(game.count==3 && pp_art_id(&game,2)==3);check_safe_buttons(lv_screen_active());
    assert(pp_ui_wild_capturing());
    uint32_t captured=pp_checksum(&game);unsigned capture_reads=pp_image_probe().file_reads;
    unsigned capture_objects=lv_obj_get_child_count(lv_screen_active());
    tap(in,220,302,80);tap(in,180,312,80);
    assert(pp_ui_page()==23 && pp_checksum(&game)==captured && game.balls==0);
    pump(1500);assert(pp_ui_wild_capturing());check_safe_buttons(lv_screen_active());
    assert(lv_obj_get_child_count(lv_screen_active())==capture_objects);
    pump(1500);assert(!pp_ui_wild_capturing() && pp_ui_page()==23);
    assert(pp_checksum(&game)==captured && pp_image_probe().file_reads==capture_reads);
    assert(visible_labels(lv_screen_active(),"捕捉成功")==1);
    pp_ui_open(0);pp_ui_open(23);pump(100);assert(!pp_ui_wild_capturing() && pp_checksum(&game)==captured);
    check_safe_buttons(lv_screen_active());
    uint32_t coins=game.coins;tap(in,180,312,80);assert(pp_ui_page()==0 && !game.wild.phase && game.coins==coins);pp_tick(&game,30,0);pump(100);assert(game.coins==coins);
    wild_fixture("duplicate");pp_ui_open(23);pump(100);assert(game.wild.outcome==PP_WILD_CAPTURE_READY && game.wild.reward_coins==25 && game.wild.reward_xp==PP_WILD_BASE_XP+8u*(game.wild.level-1));check_safe_buttons(lv_screen_active());
    tap(in,126,302,80);assert(!game.wild.phase && pp_ui_page()==0 && pp_valid(&game));
    lv_indev_delete(in);puts("PASS: wild real pointer modal blocking, untimed opened dialog, flee/confirm within ball, highest-level eligible fighter with no switch arrows, ball release/pause/reveal, battle return/resume, final-impact settlement, capture/family duplicate, once-only rewards, round safe buttons and no extra image reads");return 0;
  }
  if(notice_smoke) {
    pp_choose_starter(&game,1);pp_ui_onboarding(true);pp_ui_create(&game);pp_ui_open(0);pump(100);
    unsigned objects=lv_obj_get_child_count(lv_layer_top());assert(objects==1);
    lv_obj_t *t=lv_obj_get_child(lv_layer_top(),0);
    assert(lv_obj_has_flag(t,LV_OBJ_FLAG_HIDDEN));
    pp_ui_notice("金币不足");pump(250);
    int short_width=lv_obj_get_width(t),start_y=lv_obj_get_y(t);
    assert(short_width<120 && lv_obj_get_style_text_opa(t,0)==255);
    pp_ui_open(2);pump(1000);
    assert(lv_obj_is_visible(t) && lv_obj_get_y(t)<start_y);
    pp_ui_notice("保存失败，请检查设备存储");pump(250);
    assert(lv_obj_get_width(t)>short_width && lv_obj_get_y(t)>=start_y-1);
    pump(2100);assert(lv_obj_get_style_text_opa(t,0)<255);
    pp_ui_notice("旅行藏品已集齐，后续旅行仅获得金币和宠物经验");
    pump(250);assert(lv_obj_get_width(t)<=244 && lv_obj_get_height(t)>33);
    for(unsigned ms=0;ms<3000;ms+=33) {
      if(lv_obj_is_visible(t)) {
        lv_obj_update_layout(t);lv_area_t a;lv_obj_get_coords(t,&a);
        assert(a.x2==319);
        int xs[]={a.x1,a.x2},ys[]={a.y1,a.y2};
        for(unsigned x=0;x<2;x++)for(unsigned y=0;y<2;y++)
          assert((xs[x]-180)*(xs[x]-180)+(ys[y]-180)*(ys[y]-180)<=168*168);
      }
      pump(33);
    }
    assert(lv_obj_has_flag(t,LV_OBJ_FLAG_HIDDEN));
    /* Timer expiry from the previous notice must never hide its replacement. */
    pp_ui_notice("金币不足");pump(3000);pp_ui_notice("伙伴已换上场");pump(300);
    assert(lv_obj_is_visible(t) && !strcmp(lv_label_get_text(t),"伙伴已换上场"));
    pump(3000);assert(lv_obj_has_flag(t,LV_OBJ_FLAG_HIDDEN));
    pp_ui_onboarding(false);pp_init(&game,42);pp_ui_create(&game);pp_ui_open(22);
    assert(pp_ui_page()==22);pp_ui_notice("请检查资源包");pump(3300);
    assert(lv_obj_has_flag(t,LV_OBJ_FLAG_HIDDEN));
    assert(lv_obj_get_child_count(lv_layer_top())==objects);
    puts("Notice: adaptive width/wrap, right circular safe area, rise/fade, page and intro lifecycle, rapid replacement, one persistent overlay: PASS");
    return 0;
  }
  if(notice_text)pp_ui_notice(notice_text);

  if (activity && (!strcmp(activity,"travel-receipt") || !strcmp(activity,"travel-complete"))) {
    assert(pp_ui_travel_probe().confirmation);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    pump(50);tap(in,180,276,80);lv_indev_delete(in);
    assert(pp_ui_travel_probe().receipt);
  }
  if(activity && !strcmp(gym_stage,"cleared")) {
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(100);
    unsigned wanted=viridian?7:cinnabar?6:saffron?5:fuchsia?4:celadon?3:vermilion?2:cerulean?1:0;
    if(pp_ui_gym_map_probe().gym!=wanted)tap(in,36,180,80);
    assert(pp_ui_gym_map_probe().gym==wanted && pp_ui_gym_map_probe().cleared && pp_valid(&game));lv_indev_delete(in);
  }
  if(activity && !strcmp(gym_stage,"locked")) {
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(100);tap(in,320,180,80);lv_indev_delete(in);
  }
  if(activity && !strncmp(gym_stage,"badges",6)) {
    unsigned selected=!strcmp(gym_stage,"badges-pewter")?0:!strcmp(gym_stage,"badges-cerulean")?1:!strcmp(gym_stage,"badges-vermilion")?2:!strcmp(gym_stage,"badges-celadon")?3:!strcmp(gym_stage,"badges-fuchsia")?4:!strcmp(gym_stage,"badges-saffron")?5:!strcmp(gym_stage,"badges-cinnabar")?6:viridian?7:cinnabar?6:saffron?5:fuchsia?4:celadon?3:vermilion?2:cerulean?1:0;
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(100);tap(in,90+(selected%4)*60,110+(selected/4)*78,80);lv_indev_delete(in);
  }
  if(activity && !strcmp(gym_stage,"team")) {
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    pump(100);tap(in,180,115,80);tap(in,250,115,80);tap(in,110,115,80);lv_indev_delete(in);
  }
  if(gym_map_smoke) {
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    unsigned reads=pp_image_probe().file_reads,before=pp_checksum(&game);
    assert(pp_ui_gym_map_probe().active && !pp_ui_gym_map_probe().cleared && pp_ui_gym_map_probe().welcoming);
    for(unsigned i=0;i<PP_GYM_COUNT*2;i++) {
      tap(in,320,180,80);assert(pp_ui_gym_map_probe().gym==(i+1)%PP_GYM_COUNT);
      assert(!pp_ui_gym_map_probe().cleared && pp_ui_gym_map_probe().welcoming);check_safe_buttons(lv_screen_active());
      unsigned target=pp_ui_gym_map_probe().gym;
      tap(in,180,215,80);
      if(pp_gym_unlocked(&game,target)) {
        assert(pp_ui_page()==18 && !pp_ui_gym_map_probe().welcoming);
        pp_ui_open(17);pump(100);assert(pp_ui_gym_map_probe().welcoming);
      } else assert(pp_ui_page()==17 && pp_ui_gym_map_probe().welcoming);
    }
    for(unsigned i=0;i<PP_GYM_COUNT;i++) {
      tap(in,36,180,80);assert(pp_ui_gym_map_probe().gym==PP_GYM_COUNT-1-i);
      check_safe_buttons(lv_screen_active());
    }
    assert(pp_checksum(&game)==before);
    for(unsigned i=0;i<PP_GYM_COUNT;i++) {
      game.gym.badges=(1u<<(i+1))-1;game.revision++;pp_ui_refresh();pump(200);
      assert(pp_ui_gym_map_probe().gym==i && pp_ui_gym_map_probe().cleared && !pp_ui_gym_map_probe().welcoming);
      pp_ui_open(18);pump(100);assert(!pp_ui_gym_map_probe().active && !pp_ui_gym_map_probe().cleared && !pp_ui_gym_map_probe().welcoming);
      pp_ui_open(17);pump(100);assert(pp_ui_gym_map_probe().cleared);
      before=pp_checksum(&game);tap(in,320,180,80);assert(pp_ui_gym_map_probe().gym==(i+1)%PP_GYM_COUNT);
      assert(pp_ui_gym_map_probe().cleared==(i+1==PP_GYM_COUNT));assert(pp_ui_gym_map_probe().welcoming==(i+1!=PP_GYM_COUNT));assert(pp_checksum(&game)==before);
    }
    game.gym.badges=0;game.revision++;pp_ui_refresh();pump(200);assert(!pp_ui_gym_map_probe().cleared && pp_ui_gym_map_probe().welcoming);
    assert(pp_image_probe().file_reads==reads);lv_indev_delete(in);
    puts("PASS: real side arrows wrap all gyms in both directions; per-gym badge stamp/door leader and three pets mutually exclusive, reopen/detail exclusion, no save mutation, zero new reads and circular-safe buttons");return 0;
  }
  if(gym_smoke) {
    unsigned target=viridian_smoke?7:cinnabar_smoke?6:saffron_smoke?5:fuchsia_smoke?4:celadon_smoke?3:vermilion_smoke?2:cerulean_smoke?1:0;
    const pp_gym_def *def=&pp_gyms[target];
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    pp_ui_open(1);pump(100);tap_app(in,2);assert(pp_ui_page()==17);
    if(target){
      tap(in,320,180,80);assert(pp_ui_page()==17);unsigned before=pp_checksum(&game);
      tap(in,180,180,80);assert(pp_ui_page()==17 && pp_checksum(&game)==before);
      game.gym.badges=(1u<<target)-1;pp_ui_refresh();pump(4000);
    }
    tap(in,180,180,80);assert(pp_ui_page()==18);tap(in,180,277,80);assert(pp_ui_page()==19);
    tap(in,110,115,80);tap(in,180,115,80);tap(in,250,115,80);
    if(!target){
      /* Keep the selected team while resolving an opened wild encounter. */
      game.wild=(pp_wild_state){.phase=PP_WILD_PENDING,.dex=0,.level=1,.scene=0,.escape_at=UINT64_MAX};
      assert(pp_valid(&game));
      tap(in,180,270,80);assert(!game.battling && pp_ui_page()==0 && pp_wild_opened(&game));
      tap(in,136,270,80);assert(pp_ui_page()==19 && !game.wild.phase);
      /* An unclaimed catch is preserved until an explicit release. */
      game.wild=(pp_wild_state){.phase=PP_WILD_WON,.dex=0,.level=1,.outcome=PP_WILD_CAPTURE_READY,.reward_coins=25,.reward_xp=30};
      assert(pp_valid(&game));unsigned coins=game.coins;
      tap(in,180,270,80);assert(pp_ui_page()==23 && game.wild.outcome==PP_WILD_CAPTURE_READY && !game.battling);
      tap(in,126,302,80);assert(pp_ui_page()==19 && !game.wild.phase && game.coins==coins);
      /* A settled receipt cannot block the next actual gym battle. */
      game.wild=(pp_wild_state){.phase=PP_WILD_LOST,.dex=0,.level=1};
      assert(pp_valid(&game));
    }
    tap(in,180,270,80);assert(game.battling && game.gym.mode && pp_ui_page()==6);
    assert(game.gym.team[0]==0&&game.gym.team[1]==1&&game.gym.team[2]==2);
    assert(game.gym.mode==target+1);
    assert(pp_battle_view_probe().entrance_mask==3&&!pp_battle_view_probe().casting);pump(1100);assert(!pp_battle_view_probe().entrance_mask);
    check_battle_types();unsigned reads=pp_image_probe().file_reads;
    if(!fuchsia_smoke && !saffron_smoke && !cinnabar_smoke && !viridian_smoke)game.battle_hp=game.gym.hp[0]=1;
    if(fuchsia_smoke || saffron_smoke || cinnabar_smoke || viridian_smoke) {
      /* Late gyms need all three partners; observe a natural faint. */
      for(unsigned i=0;i<100 && game.battle_hp>0;i++){pp_tick(&game,3,0);pp_ui_refresh();pump(1900);}
    }
    if(vermilion_smoke)game.battle_round=1;
    if(!fuchsia_smoke && !saffron_smoke && !cinnabar_smoke && !viridian_smoke){pp_tick(&game,3,0);pp_ui_refresh();pump(1900);}
    assert(game.gym.hp[0]==0 && pp_battle_view_probe().hp[0]==0 && pp_battle_view_probe().alive_mask[0]==6);
    check_safe_buttons(lv_screen_active());
    pp_tick(&game,3,0);pp_ui_refresh();pump(100);
    check_battle_types();assert(game.gym.ally==1&&game.battle_pet==1&&pp_battle_view_probe().hp[0]==game.battle_hp&&pp_battle_view_probe().art[0]==pp_art_id(&game,1));
    assert(pp_battle_view_probe().entrance_mask==1&&pp_battle_view_probe().pet_opacity[0]==0&&pp_battle_view_probe().pet_opacity[1]==255);pump(1100);assert(!pp_battle_view_probe().entrance_mask);
    if(target==0){
      game.enemy_hp=game.gym.enemy_hp[game.gym.enemy]=0;pp_tick(&game,3,0);pp_ui_refresh();pump(80);
      assert(pp_battle_view_probe().entrance_mask==2&&pp_battle_view_probe().pet_opacity[0]==255&&pp_battle_view_probe().pet_opacity[1]==0);
      pump(1100);assert(!pp_battle_view_probe().entrance_mask);
    }
    assert(pp_image_probe().file_reads==reads);
    tap(in,180,310,80);assert(pp_ui_page()==1);tap_app(in,2);assert(pp_ui_page()==6);
    for(unsigned i=0;i<500&&game.battling;i++){pp_tick(&game,3,0);pp_ui_refresh();pump(3000);}
    pump(12000);assert(pp_ui_page()==21 && game.gym.result);check_safe_buttons(lv_screen_active());
    unsigned coins=game.coins;tap(in,180,310,80);assert(pp_ui_page()==18);tap(in,180,310,80);assert(pp_ui_page()==17);
    pp_ui_open(21);pump(100);assert(game.coins==coins);pp_ui_open(20);pump(100);check_safe_buttons(lv_screen_active());
    assert(pp_image_probe().file_reads==reads);
    if(target){
      assert(game.gym.result==1 && game.gym.badges==((1u<<(target+1))-1) && game.gym.reward_coins==def->first_coins && game.gym.reward_xp==def->win_xp);
      tap(in,90+(target%4)*60,110+(target/4)*78,80);check_safe_buttons(lv_screen_active());
      tap(in,180,276,80);assert(pp_ui_page()==18);
      tap(in,180,276,80);assert(pp_ui_page()==19);
      tap(in,110,115,80);tap(in,180,115,80);tap(in,250,115,80);tap(in,180,270,80);
      for(unsigned i=0;i<600&&game.battling;i++){pp_tick(&game,3,0);pp_ui_refresh();pump(3000);}
      pump(12000);assert(game.gym.result==1 && game.gym.reward_coins==def->rematch_coins && !game.gym.first_clear && game.gym.badges==((1u<<(target+1))-1));
      assert(pp_image_probe().file_reads==reads);
    }
    lv_indev_delete(in);puts("PASS: gym real pointer map/leader/ordered-three/relay/reentry/reward/gallery; zero new frame reads; circular-safe targets");return 0;
  }
  if(home_smoke) {
    if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(in,queued_pointer);
    pump(7300);
    pp_home_state before[PP_PETS];
    uint32_t checksum=pp_checksum(&game),rng=game.rng;
    const unsigned pages[]={2,3,1,11,4,8,5,9,10,14,15};
    for(unsigned pass=0;pass<sizeof(pages)/sizeof(pages[0]);pass++) {
      for(unsigned i=0;i<game.count;i++)before[i]=pp_ui_home_probe(i);
      pp_ui_open(pages[pass]);pump(1730+pass*30);pp_ui_open(0);
      for(unsigned i=0;i<game.count;i++) {
        pp_home_state after=pp_ui_home_probe(i);
        if(after.x!=before[i].x || after.y!=before[i].y || after.left!=before[i].left)fprintf(stderr,"home return page %u pet %u: %d,%d,%d -> %d,%d,%d\n",pages[pass],i,before[i].x,before[i].y,before[i].left,after.x,after.y,after.left);
        assert(after.x==before[i].x && after.y==before[i].y && after.left==before[i].left);
        assert(after.pose==before[i].pose && after.frame==before[i].frame && after.sleeping==before[i].sleeping);
      }
      assert(pp_checksum(&game)==checksum && game.rng==rng);
    }
    /* Sleeping/waking changes action in place; a scene change does not respawn. */
    before[0]=pp_ui_home_probe(0);game.pets[0].sleeping=1;pp_ui_refresh();pump(3000);
    pp_home_state state=pp_ui_home_probe(0);
    assert(state.x==before[0].x && state.y==before[0].y && state.pose==PP_POSE_SLEEP);
    game.scene=1;pp_ui_open(0);state=pp_ui_home_probe(0);
    assert(state.x==before[0].x && state.y==before[0].y);
    game.pets[0].sleeping=0;pp_ui_refresh();pump(2000);
    /* Item starts at the actual last garden position, including all bystanders. */
    for(unsigned i=0;i<game.count;i++)before[i]=pp_ui_home_probe(i);
    if(frames_dir){char p[512];snprintf(p,sizeof(p),"%s/00-before-bag.ppm",frames_dir);screenshot(p);}
    unsigned stock=game.inventory[0],food=game.pets[0].food;
    pp_ui_open(2);pump(4000);queue_tap(113,132);lv_indev_read(in);
    assert(pp_ui_page()==0 && game.drop_active && game.drop_target==0 && game.inventory[0]==stock-1);
    for(unsigned i=0;i<game.count;i++) {
      state=pp_ui_home_probe(i);assert(state.x==before[i].x && state.y==before[i].y);
    }
    if(frames_dir){char p[512];snprintf(p,sizeof(p),"%s/01-item-start.ppm",frames_dir);screenshot(p);}
    pump(700);before[0]=pp_ui_home_probe(0);
    pp_ui_open(3);pump(1200);pp_ui_open(0);state=pp_ui_home_probe(0);
    assert(state.x==before[0].x && state.y==before[0].y && state.frame==before[0].frame && state.drop_visible);
    pp_image_stats images=pp_image_probe();
    for(unsigned n=0;n<19;n++) {
      before[0]=pp_ui_home_probe(0);pump(100);state=pp_ui_home_probe(0);
      assert(abs(state.x-before[0].x)<=12 && abs(state.y-before[0].y)<=12);
      if(frames_dir){char p[512];snprintf(p,sizeof(p),"%s/02-walk-%02u.ppm",frames_dir,n);screenshot(p);}
    }
    int dx,dy;pp_drop_position(&game,&dx,&dy);
    dx-=8;dy-=16;pp_home_ground_project(game.scene,&dx,&dy);dx+=8;dy+=16;
    assert(state.x==dx-8 && state.y==dy-16 && state.pose==PP_POSE_EAT);
    pp_tick(&game,4,game.day);pp_ui_refresh();state=pp_ui_home_probe(0);
    assert(!game.drop_active && !state.drop_visible && state.x==dx-8 && state.y==dy-16);
    assert(game.pets[0].food>=food && game.inventory[0]==stock-1);
    if(frames_dir){char p[512];snprintf(p,sizeof(p),"%s/03-item-finished.ppm",frames_dir);screenshot(p);}
    before[0]=state;pump(100);state=pp_ui_home_probe(0);
    assert(abs(state.x-before[0].x)<=3 && abs(state.y-before[0].y)<=3);
    /* If core game time finishes the next item in a menu, resume at the last
     * visible point and apply recovery once, rather than restarting movement. */
    pp_ui_open(2);pump(100);queue_tap(113,132);lv_indev_read(in);pump(700);
    before[0]=pp_ui_home_probe(0);pp_ui_open(3);pp_tick(&game,4,game.day);pump(5100);
    pp_ui_open(0);state=pp_ui_home_probe(0);
    assert(state.x==before[0].x && state.y==before[0].y && !state.drop_visible && !game.drop_active);
    assert(game.inventory[0]==stock-2);
    assert(pp_image_probe().file_reads==images.file_reads);
    /* The last viewed pet is travelling; the bag must pick the lowest food
     * among the remaining courtyard pets rather than reject that stale slot. */
    game.inventory[0]=1;game.pets[0].food=1;game.pets[1].food=20;game.pets[2].food=5;
    assert(pp_travel(&game,0)==PP_OK);
    pp_ui_open(2);pump(100);queue_tap(113,132);lv_indev_read(in);pump(100);
    assert(pp_ui_page()==0 && game.drop_active && game.drop_target==2 && !game.inventory[0]);
    pp_tick(&game,4,game.day);game.traveler=-1;game.travel_end=0;
    /* Core wake transitions must reach the real garden renderer immediately,
     * including the 4s biscuit completion and old high-energy sleep saves. */
    game.pets[0].sleeping=1;game.pets[0].energy=19;game.care_remainder=0;
    pp_ui_refresh();pump(100);assert(pp_ui_home_probe(0).pose==PP_POSE_SLEEP);
    if(frames_dir){char p[512];snprintf(p,sizeof(p),"%s/04-sleep.ppm",frames_dir);screenshot(p);}
    pp_tick(&game,1799,game.day);pp_ui_refresh();assert(pp_ui_home_probe(0).pose==PP_POSE_SLEEP);
    pp_tick(&game,1,game.day);pp_ui_refresh();pump(100);
    assert(!game.pets[0].sleeping && pp_ui_home_probe(0).pose!=PP_POSE_SLEEP);
    if(frames_dir){char p[512];snprintf(p,sizeof(p),"%s/05-wake.ppm",frames_dir);screenshot(p);}
    game.pets[0].energy=40;game.pets[0].sleeping=1;game.inventory[4]=1;
    for(unsigned i=1;i<game.count;i++)game.pets[i].energy=85;
    pp_ui_refresh();assert(pp_use_auto(&game,4)==PP_OK && game.drop_target==0);
    pp_tick(&game,4,game.day);pp_ui_refresh();pump(100);
    assert(!game.pets[0].sleeping && game.pets[0].energy==70 && pp_ui_home_probe(0).pose!=PP_POSE_SLEEP);
    assert(pp_image_probe().file_reads==images.file_reads);
    lv_indev_delete(in);
    puts("PASS: menu returns preserve all positions/directions/frames/sleep, scene changes do not respawn; item walks from remembered position, resumes across menus, finishes in place, applies once, no animation reads");return 0;
  }
  if (touch_smoke) {
    lv_tick_set_cb(NULL);if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(in,queued_pointer);
    lv_indev_set_long_press_time(in,1000);
    lv_timer_set_period(lv_indev_get_read_timer(in),PP_TOUCH_POLL_MS);
    pump(100);assert(pp_ui_page()==13);
    queue_tap(178,170);lv_indev_read(in);
    assert(pp_needs_starter(&game));
    queue_tap(178,308);lv_indev_read(in);pump(100);
    assert(!pp_needs_starter(&game) && game.pets[0].species==2 && pp_ui_page()==0);
    unsigned before_pat=game.coins;
    pp_touch_sample(&queued_input,true,109,149);lv_indev_read(in);pump(1200);
    assert(pp_ui_page()==4);
    pp_touch_sample(&queued_input,false,0,0);lv_indev_read(in);pump(100);
    assert(game.coins==before_pat); /* long hold must not also pat */
    unsigned info_checksum=pp_checksum(&game);
    queue_tap(125,304);lv_indev_read(in);pump(100);assert(pp_ui_page()==16);
    assert(pp_checksum(&game)==info_checksum);
    queue_tap(180,310);lv_indev_read(in);pump(100);assert(pp_ui_page()==4);
    assert(pp_checksum(&game)==info_checksum);
    /* A held finger sliding onto the new change button must not activate it. */
    pp_ui_open(0);pump(100);pp_home_state pet=pp_ui_home_probe(0);
    pp_touch_sample(&queued_input,true,pet.x+24,pet.y+24);lv_indev_read(in);pump(1200);
    assert(pp_ui_page()==4);
    pp_touch_sample(&queued_input,true,125,304);lv_indev_read(in);pump(100);
    pp_touch_sample(&queued_input,false,0,0);lv_indev_read(in);pump(100);
    assert(pp_ui_page()==4 && pp_checksum(&game)==info_checksum);
    puts("PASS: long press opens pet information; release/slide never changes pets; explicit change opens reserves; back preserves pet state");
    game.coins=500;pp_ui_open(3);pump(100);
    unsigned stock=game.inventory[0];
    /* Six buffered down/up pairs must purchase exactly six items. */
    for(int i=0;i<6;i++)queue_tap(113,132);
    lv_indev_read(in);pump(100);
    assert(game.coins==500-6*pp_items[0].price && game.inventory[0]==stock+6);
    pump(500);assert(game.coins==500-6*pp_items[0].price); /* released, no duplicate */
    queue_tap(178,322);lv_indev_read(in);pump(100);assert(pp_ui_page()==0);
    queue_tap(128,300);lv_indev_read(in);pump(100);assert(pp_ui_page()==1);
    /* Sleep/wake discards a held contact, then accepts the next full tap. */
    pp_touch_reset(&queued_input);pp_touch_sample(&queued_input,true,180,312);
    lv_indev_read(in);pump(100);assert(pp_ui_page()==1);
    pp_touch_sample(&queued_input,false,0,0);queue_tap(180,312);
    lv_indev_read(in);pump(100);assert(pp_ui_page()==0);
    lv_indev_delete(in);
    puts("PASS: queued short taps select/confirm/navigate, six taps purchase exactly six times, 1s hold without pat, no replay after release or wake");
    return 0;
  }
  if(hatch_smoke){
    lv_tick_set_cb(NULL);if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    pp_game original=game;
    char egg_path[640];snprintf(egg_path,sizeof(egg_path),"%s%s/icons/5.bin",root?"S:":"",root?root:"S:/assets");
    const void *egg_source=pp_image_descriptor(egg_path);assert(egg_source);
    for(unsigned s=0;s<PP_SCENES;s++) {
      game.scene=s;game.egg_end=game.seconds+7200;game.egg_shakes=0;
      pp_ui_open(0);pump(100);
      char rim_path[640];snprintf(rim_path,sizeof(rim_path),"%s%s/nest-fronts/%u.bin",root?"S:":"",root?root:"S:/assets",s);
      const void *rim_source=pp_image_descriptor(rim_path);assert(rim_source);
      unsigned reads=pp_image_probe().file_reads;uint32_t rng=game.rng;
      for(unsigned t=0;t<1200;t+=100) {
        pump(100);int x,y;pp_egg_position(&game,&x,&y);
        lv_obj_t *egg=visible_image(lv_screen_active(),egg_source),*rim=visible_image(lv_screen_active(),rim_source);
        assert(egg && rim && abs(lv_obj_get_x(egg)-x)<=3 && lv_obj_get_y(egg)==y);
        assert(lv_obj_get_x(rim)==pp_scene_nest_positions[s][0] && lv_obj_get_y(rim)==pp_scene_nest_positions[s][1]+PP_SCENE_NEST_FRONT_TOP);
        assert(lv_obj_get_index(rim)>lv_obj_get_index(egg));
      }
      assert(game.rng==rng && pp_image_probe().file_reads==reads);
      pp_ui_open(1);pump(100);pp_ui_open(0);pump(100);
      int x,y;pp_egg_position(&game,&x,&y);lv_obj_t *egg=visible_image(lv_screen_active(),egg_source);
      assert(egg && abs(lv_obj_get_x(egg)-x)<=3 && lv_obj_get_y(egg)==y && game.egg_end==game.seconds+7200);
      uint32_t checksum=pp_checksum(&game);
      tap(in,x+16,y+16,80);
      assert(visible_labels(lv_layer_top(),"距离孵化还有\n02:00:00")==1);
      assert(pp_ui_page()==0 && pp_checksum(&game)==checksum && pp_image_probe().file_reads==reads);
      if(frames_dir){pump(200);char path[640];snprintf(path,sizeof(path),"%s/egg-time-%02u.ppm",frames_dir,s);screenshot(path);}
    }
    const int remaining[]={1,59,60,3599,3600,14400,0,-1};
    const char *expected[]={"00:00:01","00:00:59","00:01:00","00:59:59","01:00:00","04:00:00"};
    for(unsigned legendary=0;legendary<2;legendary++)for(unsigned n=0;n<8;n++) {
      game.legend.hatching=legendary;game.seconds=100;game.egg_end=100+remaining[n];game.egg_shakes=0;
      pp_ui_open(0);pump(100);int x,y;pp_egg_position(&game,&x,&y);
      uint32_t checksum=pp_checksum(&game);tap(in,x+16,y+16,80);
      char message[96];if(remaining[n]>0)snprintf(message,sizeof(message),"距离孵化还有\n%s",expected[n]);
      else snprintf(message,sizeof(message),"孵化时间已到");
      assert(visible_labels(lv_layer_top(),message)==1 && pp_checksum(&game)==checksum);
    }
    game=original;
    puts("PASS: egg pointer notices in all13 scenes, normal/legendary time boundaries; no game mutation or extra image reads");
    puts("PASS: all13 actual egg/rim positions, seated layering,1200ms wobble/menu return, unchanged deadline/RNG and zero animation reads");
    pp_ui_open(3);pump(100);tap(in,100,265,80);
    assert(pp_ui_page()==3 && game.coins==880 && game.hatch_boosts==1 && game.count==1 && !game.egg_end);
    pp_ui_open(2);pump(100);tap_bag_item(in,6,80);
    assert(pp_ui_page()==2 && game.hatch_boosts==1 && game.count==1);
    pp_ui_open(3);pump(100);tap(in,260,199,80);
    assert(game.inventory[5]==1 && game.coins==700);
    pp_ui_open(2);pump(100);tap_bag_item(in,5,80);
    assert(pp_ui_page()==0 && game.egg_end && !game.egg_shakes);
    pp_ui_open(2);pump(100);tap_bag_item(in,6,80);
    assert(pp_ui_page()==30 && !game.egg_end && game.count==2 && !game.hatch_boosts && game.coins==700 && pp_valid(&game));
    assert(visible_labels(lv_screen_active(),"放入仓库")==1 && visible_labels(lv_screen_active(),"替换伙伴")==1);
    check_safe_buttons(lv_screen_active());
    unsigned hatch_reads=pp_image_probe().file_reads;uint32_t hatch_checksum=pp_checksum(&game);
    pump(2000);pp_ui_refresh();assert(pp_ui_page()==30 && pp_image_probe().file_reads==hatch_reads && pp_checksum(&game)==hatch_checksum);
    if(frames_dir){char path[640];snprintf(path,sizeof(path),"%s/hatch-result.ppm",frames_dir);screenshot(path);}
    tap(in,252,302,80);assert(pp_ui_page()==29);tap(in,180-(int)((game.count<3?game.count:3)-1)*39,120,80);
    assert(pp_ui_page()==0 && game.count==1 && game.reserve_count==1 && pp_valid(&game));
    pp_ui_open(2);pump(100);tap_bag_item(in,6,80);assert(game.count==1);
    game.coins=10000;
    while(game.count<PP_PETS)assert(pp_test_hatch(&game)==PP_OK);
    assert(pp_test_hatch(&game)==PP_OK && game.reserve_count==2);
    assert(pp_reserve_pet(&game,4)==PP_OK && game.count==4 && game.reserve_count==3);
    assert(pp_buy(&game,5)==PP_OK && pp_use(&game,5,0)==PP_OK && pp_buy(&game,PP_HATCH_BOOST)==PP_OK);
    pp_ui_open(2);pump(100);tap_bag_item(in,6,80);
    assert(pp_ui_page()==30 && game.count==5 && game.reserve_count==3 && !game.egg_end);
    tap(in,252,302,80);assert(pp_ui_page()==29);tap(in,180-(int)((game.count<3?game.count:3)-1)*39,120,80);
    assert(pp_ui_page()==0 && game.count==4 && game.reserve_count==4 && pp_valid(&game));
    assert(pp_buy(&game,5)==PP_OK && pp_use(&game,5,0)==PP_OK && pp_buy(&game,PP_HATCH_BOOST)==PP_OK);
    pp_ui_open(2);pump(100);tap_bag_item(in,6,80);
    assert(pp_ui_page()==30 && game.count==5 && game.reserve_count==4 && !game.egg_end && pp_valid(&game));
    tap(in,108,302,80);assert(pp_ui_page()==16);assert(visible_labels(lv_screen_active(),"9只    1 / 1")==1);
    /* Timed hatches do not pass through the bag callback: they must still
     * present the same reveal receipt on the next UI refresh. */
    pp_ui_open(0);game.egg_end=game.seconds+1;game.egg_shakes=1;pp_ui_refresh();
    pp_tick(&game,1,game.day);pp_ui_refresh();
    assert(pp_ui_page()==30 && game.count==5 && !game.egg_end && pp_valid(&game));
    check_safe_buttons(lv_screen_active());pump(100);
    if(frames_dir){char path[640];snprintf(path,sizeof(path),"%s/hatch-result-stored.ppm",frames_dir);screenshot(path);}
    lv_indev_delete(in);puts("PASS: real shop/bag pointer purchase, absent egg keeps stock, existing egg completion once; courtyard and reserve results route correctly");return 0;
  }
  if (intro_smoke) {
    lv_tick_set_cb(NULL);if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    assert(pp_ui_page()==22 && !pp_ui_intro_seen());
    uint32_t checksum=pp_checksum(&game),rng=game.rng;
    pp_image_stats images=pp_image_probe();
    pump(1200);pp_intro_state a=pp_intro_view_probe();
    assert(a.pets[0].opacity>0 && a.pets[1].opacity==0);
    pump(2800);pp_intro_state b=pp_intro_view_probe();
    assert(b.pets[0].x!=a.pets[0].x && b.pets[2].opacity>0);
    pump(5000);b=pp_intro_view_probe();
    assert(b.icons==255 && b.caption==255);
    assert(pp_checksum(&game)==checksum && game.rng==rng && pp_needs_starter(&game));
    assert(pp_image_probe().file_reads==images.file_reads);
    pump(2400);assert(pp_ui_page()==13 && pp_ui_intro_seen());
    pp_ui_onboarding(false);pp_ui_open(0);pump(100);assert(pp_ui_page()==22);
    tap(in,180,309,80);assert(pp_ui_page()==13 && pp_ui_intro_seen());
    assert(pp_checksum(&game)==checksum);
    pp_ui_open(0);assert(pp_ui_page()==13); /* seen but still awaiting choice */
    pump(12000);assert(pp_ui_page()==13); /* no stale callback */
    tap(in,178,170,80);tap(in,178,308,80);assert(pp_ui_page()==0 && !pp_needs_starter(&game));
    pp_ui_onboarding(false);pp_ui_create(&game);assert(pp_ui_page()==0); /* existing owner */
    lv_indev_delete(in);
    puts("PASS: intro entrance/timeline/automatic selection/skip/once-only/existing owner; no game mutation or animation file reads");return 0;
  }
  if (travel_smoke) {
    lv_tick_set_cb(NULL);
    if(mouse)lv_indev_enable(mouse,false);
    lv_indev_t *in=lv_indev_create();
    lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    /* Actual return button, complete panel coverage and exact clean repaint. */
    for(unsigned i=0;i<10;i++) {pp_ui_open(5);pump(100);check_travel_return(in,180);}
    pp_ui_open(5);pump(100);tap(in,180,276,80);pump(200);
    assert(game.traveler==0);
    pp_travel_view_state state=pp_ui_travel_probe();
    assert(state.active && !state.claimable && !state.receipt);
    assert(state.x==0 && state.y==108 && state.width==360 && state.height==112);
    assert(strstr(state.activity,"皮卡丘"));
    /* Exact mirror seams / 1px shifts, then the real page including ambient FX. */
    uint64_t original_seconds=game.seconds,original_end=game.travel_end;
    for(unsigned destination=0;destination<PP_TRAVEL_DESTINATIONS;destination++){
      while(pp_travel_destination(&game)!=destination){game.seconds++;game.travel_end++;}
      pp_ui_open(1);
      char strip_path[640];snprintf(strip_path,sizeof(strip_path),"%s%s/travel-scenes/%u.bin",root?"S:":"",root?root:"S:/assets",destination);
      const lv_image_dsc_t *original=pp_image_descriptor(strip_path);assert(original);
      lv_obj_t *sample=lv_image_create(lv_screen_active());assert(pp_scene_view_strip_set(sample,original,0));
      const lv_image_dsc_t *output=lv_image_get_src(sample);assert(output->header.cf==LV_COLOR_FORMAT_RGB565);
      unsigned reads=pp_image_probe().file_reads;
      const unsigned times[]={0,24,25,33,49,50,66,8975,8999,9000,9001,17999,18000,18001};
      for(unsigned n=0;n<sizeof(times)/sizeof(times[0]);n++){
        pp_scene_view_strip_tick(sample,times[n]);const uint16_t *pixels=(const uint16_t *)output->data;
        for(unsigned y=0;y<112;y++)for(unsigned x=0;x<360;x++){
          unsigned at=(x+times[n]/25)%720,sx=at<360?at:719-at;
          uint32_t c=pp_image_indexed_pixel(original,sx,y);
          uint16_t expected=(uint16_t)(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));
          assert(pixels[y*360+x]==expected);
        }
      }
      assert(pp_image_probe().file_reads==reads);lv_obj_delete(sample);
      pp_ui_open(5);pump(100);pp_travel_view_state start=pp_ui_travel_probe();
      uint32_t checksum=pp_checksum(&game);reads=pp_image_probe().file_reads;
      for(unsigned frame=0;frame<60;frame++){
        for(unsigned ms=0;ms<33;ms++)pump(1);
        pp_travel_view_state moving=pp_ui_travel_probe();assert(moving.motion_ms>start.motion_ms&&moving.effects);
        check_partial_frame();
      }
      state=pp_ui_travel_probe();assert(state.motion_ms-start.motion_ms>=1947&&state.motion_ms-start.motion_ms<=2013);
      assert(pp_checksum(&game)==checksum&&pp_image_probe().file_reads==reads);
      unsigned stopped=state.motion_ms;pp_ui_screen(false);pump(30000);assert(pp_ui_travel_probe().motion_ms==stopped);
      pp_ui_screen(true);pump(66);assert(pp_ui_travel_probe().motion_ms<=stopped+66);
    }
    game.seconds=original_seconds;game.travel_end=original_end;pp_ui_open(5);
    puts("PASS: 10 travel destinations: exact 1px mirror-tiled scrolling / both seams / 18s wrap, 600 animated partial/full frames, ambient FX, zero reads/RNG/rewards, screen pause and resume");
    for(unsigned i=0;i<10;i++) {check_travel_return(in,180);pp_ui_open(5);pump(100);}
    pp_image_stats images=pp_image_probe();uint32_t rng=game.rng;
    pump(8000);
    assert(pp_image_probe().file_reads==images.file_reads && game.rng==rng);
    pp_tick(&game,1800,day_now());pp_ui_refresh();pump(100);
    state=pp_ui_travel_probe();assert(strstr(state.event,"+5金币") && strstr(state.reward,"55金币"));
    assert(!state.claimable);
    uint32_t coins=game.coins;unsigned count=pp_collected(&game);
    pp_tick(&game,PP_TRAVEL_SECONDS-1800,day_now());pp_ui_refresh();pump(100);
    state=pp_ui_travel_probe();
    assert(state.confirmation && state.claimable && !state.receipt && game.traveler==0 && game.coins==coins && pp_collected(&game)==count);
    uint32_t waiting=pp_checksum(&game);pump(30000);pp_ui_refresh();pp_ui_open(1);pump(100);
    assert(pp_ui_page()==5 && pp_ui_travel_probe().confirmation && pp_checksum(&game)==waiting);
    tap(in,180,276,80);state=pp_ui_travel_probe();
    assert(state.receipt && !state.claimable && !state.ready && game.traveler==-1);
    assert(state.coins==80 && state.xp==4 && state.before_level==1 && state.after_level==1);
    assert(game.coins==coins+80 && pp_collected(&game)==count+1 && strstr(state.event,"新收藏"));
    tap(in,180,301,40);assert(pp_ui_page()==5); /* Complete button awaits animation. */
    images=pp_image_probe();pump(1500);assert(pp_ui_travel_probe().ready);
    assert(pp_image_probe().file_reads==images.file_reads);
    for(unsigned i=0;i<10;i++){pp_ui_refresh();pp_ui_open(5);pump(100);assert(game.coins==coins+80);}
    pump(1500);memset(export_rows,0,sizeof(export_rows));tap(in,180,301,80);assert(pp_ui_page()==0);
    for(unsigned y=0;y<360;y++)assert(export_rows[y]);
    pp_ui_open(5);pump(100);assert(!pp_ui_travel_probe().active);
    /* Completed pool and Lv60 retain only actual rewards, without RNG use. */
    for(unsigned id=0;id<PP_TRAVEL_COLLECTION;id++)game.collected[id/8]|=1u<<(id%8);
    game.collected[PP_PURE_HEART/8]&=~(1u<<(PP_PURE_HEART%8));
    game.growth[0]=(pp_growth){.level=60};
    game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;tap(in,180,276,80);assert(game.traveler==0);
    pp_ui_open(1);coins=game.coins;
    pp_tick(&game,PP_TRAVEL_SECONDS,day_now());rng=game.rng;pp_ui_refresh();pump(100);
    assert(pp_ui_page()==5 && pp_ui_travel_probe().confirmation && game.traveler==0 && game.coins==coins);
    tap(in,180,276,80);pump(1500);state=pp_ui_travel_probe();
    assert(pp_ui_page()==5 && state.receipt && state.ready && state.coins==80 && state.xp==0);
    assert(strstr(state.event,"旅行藏品已集齐") && game.coins==coins+80);
    assert(pp_collected(&game)==583 && !pp_has(&game,PP_PURE_HEART) && game.rng==rng);
    tap(in,180,301,80);assert(pp_ui_page()==0 && game.coins==coins+80);
    /* Busy operations defer settlement until the scene is available. */
    for(unsigned busy=0;busy<2;busy++) {
      game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;
      game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;assert(pp_travel(&game,0)==PP_OK);pp_tick(&game,PP_TRAVEL_SECONDS,day_now());coins=game.coins;
      if(busy==0)game.battling=1;
      if(busy==1)game.drop_active=1;
      pp_ui_open(0);pp_ui_refresh();assert(game.traveler==0 && game.coins==coins);
      pp_ui_open(5);pump(100);state=pp_ui_travel_probe();
      assert(!state.claimable && !state.receipt && strstr(state.activity,busy?"道具":"战斗"));
      uint32_t unchanged=pp_checksum(&game);tap(in,224,304,80);
      assert(pp_checksum(&game)==unchanged && !visible_labels(lv_layer_top(),"还没到时间哦"));
      game.battling=0;game.wild.phase=0;game.drop_active=0;
      pp_ui_refresh();pump(100);assert(pp_ui_travel_probe().confirmation && game.coins==coins);
      tap(in,180,276,80);pump(1500);assert(pp_ui_travel_probe().receipt && game.coins==coins+80);
      tap(in,180,301,80);assert(pp_ui_page()==0);
    }
    /* Completion interrupts safe menus without paying anything before tap. */
    const unsigned menu_pages[]={1,2,3,9,11,14,15,16,17,20,24,26};
    for(unsigned n=0;n<sizeof(menu_pages)/sizeof(menu_pages[0]);n++) {
      game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;assert(pp_travel(&game,0)==PP_OK);pp_ui_open(menu_pages[n]);pump(40);coins=game.coins;
      pp_tick(&game,PP_TRAVEL_SECONDS,day_now());pp_ui_refresh();pump(100);
      assert(pp_ui_page()==5 && pp_ui_travel_probe().confirmation && game.coins==coins && game.traveler==0);
      check_safe_buttons(lv_screen_active());tap(in,180,276,80);assert(game.coins==coins+80);
      tap(in,180,276,80);assert(game.coins==coins+80);pump(1500);tap(in,180,312,80);assert(pp_ui_page()==0);
    }
    /* No popup behind a sleeping screen. Cold reload of the unclaimed trip
     * still requires confirmation and retains the original deadline. */
    game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;assert(pp_travel(&game,0)==PP_OK);pp_ui_open(1);pp_ui_screen(false);coins=game.coins;
    pp_tick(&game,PP_TRAVEL_SECONDS,day_now());pp_ui_refresh();pump(100);
    assert(pp_ui_page()==1 && game.traveler==0 && game.coins==coins);
    pp_ui_screen(true);pp_ui_refresh();pump(100);assert(pp_ui_travel_probe().confirmation && game.coins==coins);
    unsigned saved_bytes=pp_save_size(&game);void *pending_blob=malloc(saved_bytes);assert(pending_blob);int64_t pending_wall;
    uint64_t deadline=game.travel_end;assert(pp_save_write(&game,pending_blob,saved_bytes,123456) && pp_save_read(&game,pending_blob,saved_bytes,&pending_wall));free(pending_blob);
    pp_ui_create(&game);pump(100);assert(pp_ui_travel_probe().confirmation && game.coins==coins && game.travel_end==deadline);
    tap(in,180,276,80);pump(1500);assert(game.coins==coins+80);tap(in,180,312,80);assert(pp_ui_page()==0);
    for(unsigned deferred=0;deferred<2;deferred++) {
      game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;assert(pp_travel(&game,0)==PP_OK);pp_ui_open(deferred?31:27);pump(40);coins=game.coins;
      pp_tick(&game,PP_TRAVEL_SECONDS,day_now());pp_ui_refresh();pump(100);
      assert(pp_ui_page()==(deferred?31:27) && game.coins==coins && game.traveler==0);
      pp_ui_open(1);pump(100);assert(pp_ui_travel_probe().confirmation && game.coins==coins);
      tap(in,180,276,80);pump(1500);tap(in,180,312,80);assert(pp_ui_page()==0);
    }
    /* Passive saved encounters and receipts must never strand a returned pet.
     * Reproduce the old visible-claim / NOT_READY loop, including save reload. */
    for(unsigned pending=0;pending<7;pending++) {
      pp_ui_open(1);game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;assert(pp_travel(&game,0)==PP_OK);pp_tick(&game,PP_TRAVEL_SECONDS,day_now());
      if(pending<2)game.wild=(pp_wild_state){.phase=PP_WILD_PENDING,.dex=0,.level=1,
        .next_at=pending?0:PP_WILD_STEP_MARK,.escape_at=UINT64_MAX};
      if(pending>=2 && pending<5)game.wild=(pp_wild_state){.phase=PP_WILD_WON,.dex=0,.level=1,
        .outcome=pending==2?PP_WILD_CAUGHT:pending==3?PP_WILD_DUPLICATE:PP_WILD_CAPTURE_READY,
        .reward_coins=8,.reward_xp=2};
      if(pending==5)game.wild=(pp_wild_state){.phase=PP_WILD_LOST,.dex=0,.level=1};
      if(pending==6){game.gift.pending=1;game.gift.item=0;game.gift.dex=0;}
      assert(pp_valid(&game));
      unsigned bytes=pp_save_size(&game);void *blob=malloc(bytes);assert(blob);int64_t wall;
      assert(pp_save_write(&game,blob,bytes,123456) && pp_save_read(&game,blob,bytes,&wall));free(blob);
      pp_wild_state wild=game.wild;unsigned gift=game.gift.pending;coins=game.coins;
      pp_ui_create(&game);pp_ui_onboarding(true);pp_ui_open(5);pump(100);
      assert(pp_ui_travel_probe().confirmation && game.coins==coins);tap(in,180,276,80);pump(1600);state=pp_ui_travel_probe();
      assert(state.receipt && state.ready && !state.claimable && game.traveler==-1 && game.coins==coins+80);
      assert(!memcmp(&wild,&game.wild,sizeof(wild)) && game.gift.pending==gift && pp_valid(&game));
      uint32_t settled=pp_checksum(&game);
      for(unsigned i=0;i<3;i++){pp_ui_refresh();pp_ui_open(5);pump(1300);assert(pp_checksum(&game)==settled);}
      tap(in,180,301,80);assert(pp_ui_page()==0);
      assert(!memcmp(&wild,&game.wild,sizeof(wild)) && game.gift.pending==gift);
      game.wild=(pp_wild_state){0};game.gift.pending=0;pp_ui_open(0);pump(100);
    }
    /* A modal already being answered is not interrupted; dismissing it releases
     * the finished trip immediately, with no remaining-time error or lost catch. */
    game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;assert(pp_travel(&game,0)==PP_OK);
    game.wild=(pp_wild_state){.phase=PP_WILD_PENDING,.dex=0,.level=1,.escape_at=UINT64_MAX};
    pp_ui_open(0);pump(100);assert(pp_ui_wild_pending());coins=game.coins;
    pp_tick(&game,PP_TRAVEL_SECONDS,day_now());pp_ui_refresh();pump(100);
    assert(pp_ui_wild_pending() && game.traveler==0 && game.coins==coins);
    tap(in,136,270,80);pump(100);assert(pp_ui_travel_probe().confirmation && game.coins==coins);
    tap(in,180,276,80);pump(1600);assert(pp_ui_travel_probe().receipt && game.coins==coins+80);
    tap(in,180,301,80);assert(pp_ui_page()==0);
    /* A timed hatch during either travel panel must retain its own reveal.
     * Travel confirmation/receipt stays visible until its explicit finish. */
    for(unsigned phase=0;phase<2;phase++) {
      game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;assert(pp_travel(&game,0)==PP_OK);pp_tick(&game,PP_TRAVEL_SECONDS,day_now());
      pp_ui_refresh();pump(100);assert(pp_ui_travel_probe().confirmation);
      if(phase)tap(in,180,276,80);
      coins=game.coins;unsigned pets_before=game.count+pp_warehouse_count(&game);
      game.egg_end=game.seconds+1;game.egg_shakes=1;pp_ui_refresh();
      pp_tick(&game,2,day_now());pp_ui_refresh();pump(100);
      assert(pp_ui_page()==5 && !game.egg_end && game.count+pp_warehouse_count(&game)==pets_before+1);
      assert(game.coins==coins);
      if(!phase){assert(pp_ui_travel_probe().confirmation);tap(in,180,276,80);}
      pump(1600);tap(in,180,312,80);assert(pp_ui_page()==30);
      check_safe_buttons(lv_screen_active());tap(in,130,299,80);assert(pp_ui_page()==16);
      pp_ui_open(0);pump(100);
    }
    /* BOOT may leave a hatch reveal or its replacement picker unfinished.
     * That hidden receipt must not prevent another pet from returning. */
    for(unsigned exit=0;exit<5;exit++) {
      pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);
      game.count=exit==3?2:PP_PETS;
      for(unsigned p=1;p<game.count;p++) {
        game.pets[p]=game.pets[0];game.growth[p]=game.growth[0];
        game.identity[p]=game.identity[0];game.pets[p].species=0;
      }
      pp_ui_create(&game);assert(pp_travel(&game,0)==PP_OK);pp_ui_open(5);
      unsigned pets_before=game.count+pp_warehouse_count(&game);
      game.egg_end=game.seconds+1;game.egg_shakes=1;pp_ui_refresh();
      pp_tick(&game,2,day_now());pp_ui_refresh();pump(100);
      assert(pp_ui_page()==30 && game.count+pp_warehouse_count(&game)==pets_before+1);
      if(frames_dir && exit==3){char p[640];snprintf(p,sizeof(p),"%s/hatch-waiting.ppm",frames_dir);screenshot(p);}
      if(exit==1 || exit==2 || exit==4){tap(in,224,310,80);assert(pp_ui_page()==29);}
      coins=game.coins;
      if(exit>=3) {
        if(exit==3) {
          uint32_t waiting=pp_checksum(&game);
          tap(in,130,310,80);assert(pp_ui_page()==30 && pp_checksum(&game)==waiting);
        }
        pp_tick(&game,PP_TRAVEL_SECONDS,day_now());pp_ui_refresh();pump(100);
        assert(pp_ui_page()==5 && pp_ui_travel_probe().confirmation && game.coins==coins && game.traveler==0);
      }
      if(exit<3) {
        if(exit==2){tap(in,180,312,80);assert(pp_ui_page()==30);}
        pp_ui_boot_hold();pump(100);
        assert(pp_ui_page()==0);pp_ui_open(exit==1?5:exit==2?27:1);
        pp_tick(&game,PP_TRAVEL_SECONDS,day_now());
        if(exit==2)pp_ui_open(5);else pp_ui_refresh();
        pump(100);
      }
      assert(pp_ui_page()==5 && pp_ui_travel_probe().confirmation && game.coins==coins && game.traveler==0);
      if(frames_dir && exit==3){char p[640];snprintf(p,sizeof(p),"%s/travel-confirmation.ppm",frames_dir);screenshot(p);}
      tap(in,180,276,80);pump(1600);
      assert(pp_ui_travel_probe().receipt && game.traveler==-1 && game.coins==coins+80);
      tap(in,180,276,80);assert(game.coins==coins+80);
      if(frames_dir && exit==3){char p[640];snprintf(p,sizeof(p),"%s/travel-receipt.ppm",frames_dir);screenshot(p);}
      tap(in,180,312,80);
      if(exit>=3){
        assert(pp_ui_page()==30);
        if(frames_dir && exit==3){pump(100);char p[640];snprintf(p,sizeof(p),"%s/hatch-resumed.ppm",frames_dir);screenshot(p);}
        tap(in,130,310,80);assert(pp_ui_page()==16);
      }
      else assert(pp_ui_page()==0);
      pump(100);
      assert(game.count+pp_warehouse_count(&game)==pets_before+1);
      assert(game.coins==coins+80 && !pp_ui_travel_probe().receipt && pp_valid(&game));
      /* Dismissing the previous reveal must also allow a later natural hatch. */
      pets_before=game.count+pp_warehouse_count(&game);
      game.egg_end=game.seconds+1;game.egg_shakes=1;pp_ui_refresh();
      pp_tick(&game,2,day_now());pp_ui_refresh();pump(100);
      assert(pp_ui_page()==30 && game.count+pp_warehouse_count(&game)==pets_before+1);
      tap(in,130,310,80);assert(pp_ui_page()==16);
      pp_ui_open(0);
    }
    puts("PASS: 5 hatch/travel scenarios: BOOT/back dismissal, automatic/manual return, visible placement deadlock and replacement picker; newborn retained, later hatch allowed and rewards paid once");
    /* Cross an evolution threshold: finish travel first, then ask evolution. */
    game.pets[0].species=1;game.growth[0]=(pp_growth){.level=15,.xp=299};
    game.pets[0].food=game.pets[0].health=game.pets[0].energy=100;assert(pp_travel(&game,0)==PP_OK);pp_tick(&game,PP_TRAVEL_SECONDS,day_now());
    pp_ui_refresh();pump(100);assert(pp_ui_travel_probe().confirmation && !pp_ui_evolving());
    tap(in,180,276,80);pump(1600);assert(pp_ui_travel_probe().ready && !pp_ui_evolving());
    assert(game.growth[0].level>=16);tap(in,180,301,80);pump(100);assert(pp_ui_evolving());
    puts("Travel: automatic persistent completion notice, explicit confirmation before any reward, once-only 80 coins/scaled XP, animation/cap, saved encounters/receipts/gift preserved, busy/modal deferral, evolution ordering and full return repaint: PASS");
    return 0;
  }
  if (evolution_smoke) {
    lv_tick_set_cb(NULL);
    if (mouse) lv_indev_enable(mouse, false);
    lv_indev_t *in=lv_indev_create();
    lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(in,pointer);
    const unsigned colors[]={0xf4ce58,0x80cb83,0xf69b68,0x74c7ed,0xb4cb65,0xb4cb65,0xc4b29a,0xc4b29a,0xc4b29a,0xc294df,0xd9b67c,0xc294df,0xc294df,0xf4a8c5,0xf69b68,0xc4b29a,0xc294df,0x80cb83,0xb4cb65,0xc294df,0xd9b67c,0xc4b29a,0x74c7ed,0xe6a083,0xf69b68,0x74c7ed,0xc294df,0xe6a083,0x80cb83,0x74c7ed,0xc4b29a,0xf69b68,0xf3a7bc,0xc5d3dc,0xd9b67c,0xd9b67c,0xcde7ef,0xc294df,0xc294df,0x9f87dc,0xc4b29a,0xd695ed,0xf4ad7a,0xef9d9d,0xe9b4cb,0xd8bb98,0xd4b591,0xe9ad8c,0xefb1cb,0xb8a1e8,0xaeb5c5,0xf2bbd0,0x79b9ee,0xd5b28b,0x8bd4ed,0xf0b889,0xd7b07b,0xe8bed9,0xa0cf87,0xd6acde,0xf1d16c,0xeab77f,0xc8ae8d,0xceac7f,0xef9f69,0x8ecde8,0xcba9dc,0xc69d72,0xe7b1d7,0x88b9ce,0xb9a47b,0xb5a6cd,0x85aca2,0x91cfed,0xf3d66d,0xf2a46f,0x91bde8,0xc0a4e2,0xf0b4ce};
    unsigned transitions=0;
    for(unsigned i=0;i<PP_EVOLUTIONS;i++)for(unsigned decline=0;decline<2;decline++) {
      const pp_evolution_template *t=&pp_evolutions[i];
      pp_init(&game,42);game.count=2;game.pets[0].species=t->species;game.pets[1].species=1;
      game.growth[0].stage=t->from_stage;
      game.growth[0].level=t->level?t->level:form_level(t->species,t->from_stage);
      game.coins=(!t->level && decline)?0:500;
      unsigned old_art=pp_art_id(&game,0),coins=game.coins;
      pp_ui_open(t->level?0:4);pump(140);
      if(!t->level)tap(in,180,276,80);
      pp_evolution_view_state e=pp_ui_evolution_probe();
      assert(e.active && e.choice && !e.ready && e.before_art==old_art && e.current_art==old_art);
      assert(game.growth[0].stage==t->from_stage && game.coins==coins && !pp_has(&game,PP_PURE_HEART));
      assert(e.tint==colors[t->species]);
      uint32_t unchanged=pp_checksum(&game);
      check_safe_buttons(lv_screen_active());
      tap(in,100,68,80);assert(!pp_ui_evolving() && pp_checksum(&game)==unchanged);
      pump(1200);assert(!pp_ui_evolving() && pp_checksum(&game)==unchanged);
      /* Return is temporary: reopening from status still offers both choices. */
      pp_ui_open(4);pump(140);tap(in,180,276,80);
      assert(pp_ui_evolution_probe().choice && pp_checksum(&game)==unchanged);
      if(!t->level && decline) {
        uint32_t before=pp_checksum(&game);
        tap(in,180,260,80);
        assert(pp_ui_evolution_probe().choice && pp_checksum(&game)==before);
      }
      tap(in,180,decline?302:260,80);assert(!pp_ui_evolution_probe().choice);
      pp_image_stats cache=pp_image_probe();
      if(decline) {
        assert(pp_evolution_locked(&game,0) && pp_has(&game,PP_PURE_HEART));
        assert(game.coins==coins && pp_art_id(&game,0)==old_art && pp_valid(&game));
        pump(2400);e=pp_ui_evolution_probe();
        assert(e.declined && e.ready && e.current_art==old_art && !e.silhouette);
      } else {
        assert(game.growth[0].stage==t->from_stage+1 && game.coins==coins-(t->level?0:200));
        assert(pp_art_id(&game,0)==t->art && pp_valid(&game));
        pump(700);e=pp_ui_evolution_probe();assert(e.silhouette>150 && e.current_art==old_art);
        pump(600);e=pp_ui_evolution_probe();assert(e.current_art==t->art && !e.ready);
        pump(1000);e=pp_ui_evolution_probe();assert(e.ready && !e.silhouette);
        transitions++;
      }
      assert(pp_image_probe().file_reads==cache.file_reads);
      tap(in,180,310,80);assert(!pp_ui_evolving() && pp_ui_page()==4);
      if(decline) {
        pp_game saved=game;pp_init(&game,1);memcpy(&game,&saved,sizeof(game));
        pp_ui_open(0);pump(6000);assert(!pp_ui_evolving() && pp_evolution_locked(&game,0));
      }
    }
    for(unsigned branch=1;branch<=3;branch++)for(unsigned decline=0;decline<2;decline++){
      pp_init(&game,42);assert(pp_choose_starter(&game,1)==PP_OK);game.count=2;game.pets[0].species=67;game.pets[1].species=1;game.coins=decline?0:500;
      uint32_t before=pp_checksum(&game);pp_ui_open(4);pump(140);tap(in,180,276,80);
      assert(pp_ui_evolution_probe().choice && pp_checksum(&game)==before);
      tap(in,180,205+(branch-1)*36,80);assert(pp_ui_evolution_probe().choice && pp_checksum(&game)==before);
      tap(in,100,68,80);assert(pp_ui_evolution_probe().choice && pp_checksum(&game)==before);
      tap(in,100,68,80);assert(!pp_ui_evolving() && pp_ui_page()==4 && pp_checksum(&game)==before);
      pump(1200);assert(!pp_ui_evolving());
      tap(in,180,276,80);tap(in,180,205+(branch-1)*36,80);
      assert(pp_ui_evolution_probe().choice && pp_checksum(&game)==before);
      if(decline){tap(in,180,260,80);assert(pp_checksum(&game)==before);}
      tap(in,180,decline?302:260,80);assert(!pp_ui_evolution_probe().choice);
      pp_image_stats stats=pp_image_probe();pump(2400);pp_evolution_view_state e=pp_ui_evolution_probe();
      assert(e.ready && e.current_art==(decline?132:132+branch) && game.coins==(decline?0:300) && pp_valid(&game));
      assert(pp_evolution_locked(&game,0)==(bool)decline && pp_has(&game,PP_PURE_HEART)==(bool)decline);
      assert(pp_image_probe().file_reads==stats.file_reads);tap(in,180,310,80);assert(!pp_ui_evolving());
    }
    puts("Eevee: three real pointer branch/confirmation flows, no early mutation, insufficient coins/free permanent refusal, actual form reveal, zero frame reads: PASS");
    printf("Evolution choices: all %u transitions, temporary return/reopen and Eevee branch back, no mutation before decision, per-individual permanent refusal/unique gift, save restoration: PASS\n",transitions);
    printf("Evolution: %u transitions, typed palettes, exact old/new forms, reveal/ready timing, atomic level/stone commit, no image reads and completion pointer: PASS\n",transitions);
    return 0;
  }
  if (story_smoke) {
    lv_tick_set_cb(NULL);
    game.pets[0].species = 0;
    assert(pp_battle(&game, 0) == PP_OK);
    pp_ui_open(6);pump(1100);
    pp_tick(&game, 2, day_now()); pp_ui_refresh();
    pp_battle_view_state state = pp_battle_view_probe();
    assert(!strcmp(state.narration, "皮卡丘使用了电击"));
    assert(state.narration_opacity == 255);
    pump(800);
    state = pp_battle_view_probe();
    assert(!strcmp(state.narration, "皮卡丘使用了电击"));
    assert(state.narration_y < 176 && state.narration_opacity < 255);
    pump(240);
    state = pp_battle_view_probe();
    assert(!strcmp(state.narration, "对方的妙蛙种子使用了藤鞭"));
    assert(state.narration_y == 176 && state.narration_opacity == 255);
    pump(1080);
    assert(!pp_battle_view_probe().narration[0]);
    game.enemy_hp = 1;
    pp_ui_open(6);pump(1100);
    pp_tick(&game, 2, day_now()); pp_ui_refresh();
    assert(!game.battling && !pp_battle_view_probe().settled);
    pump(1080);
    assert(strstr(pp_battle_view_probe().narration, "妙蛙种子倒下了"));
    pump(1020);
    assert(strstr(pp_battle_view_probe().narration, "挑战成功"));
    pump(1080);
    assert(pp_battle_view_probe().settled);
    pump(1000);
    assert(!pp_battle_view_probe().narration[0]);
    pp_init(&game, 42);
    game.pets[0].species = 1;
    game.growth[0].stage = 1;
    assert(pp_battle(&game, 0) == PP_OK);
    pp_ui_open(6);pump(1100);
    pp_tick(&game, 2, day_now()); pp_ui_refresh();
    assert(!strcmp(pp_battle_view_probe().narration, "妙蛙草使用了藤鞭"));
    pp_init(&game, 42);
    game.pets[0].species = 1;
    assert(pp_battle(&game, 0) == PP_OK);
    game.battle_hp = 1;
    pp_ui_open(6);pump(1100);
    pp_tick(&game, 2, day_now()); pp_ui_refresh();
    assert(!game.battling && game.battle_hp == 0);
    pump(1080);
    assert(strstr(pp_battle_view_probe().narration, "对方的小火龙使用了火花"));
    pump(1020);
    assert(!strcmp(pp_battle_view_probe().narration, "妙蛙种子倒下了"));
    pump(1020);
    assert(strstr(pp_battle_view_probe().narration, "伙伴需要休息"));
    pump(1080);
    assert(pp_battle_view_probe().settled);
    pp_ui_open(0);
    assert(!pp_battle_view_probe().active);
    puts("Battle narration: exact actor/move, rise/fade/reset, queued reply/defeat/result, settlement and page cleanup: PASS");
    return 0;
  }
  if (ambient_smoke) {
    lv_tick_set_cb(NULL);
    if (mouse) lv_indev_enable(mouse, false);
    lv_indev_t *in = lv_indev_create();
    lv_indev_set_type(in, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(in, pointer);
    pump(100);
    pp_image_stats images = pp_image_probe();
    uint32_t rng = game.rng;
    pump(45000);
    assert(game.rng == rng);
    assert(pp_image_probe().file_reads == images.file_reads);
    pp_home_state pet = pp_ui_home_probe(0);
    uint32_t coins = game.coins;
    tap(in, pet.x + 24, pet.y + 24, 80);
    assert(game.coins == coins + 1);
    tap(in, 180, 300, 80);
    assert(pp_ui_page() == 2);
    tap(in, 180, 310, 80);
    assert(pp_ui_page() == 0);
    tap(in, 232, 300, 80);
    assert(pp_ui_page() == 3);
    tap(in, 180, 310, 80);
    assert(pp_ui_page() == 0);
    tap(in, 128, 300, 80);
    assert(pp_ui_page() == 1);
    puts("Ambient particles: 45s no image reads/RNG changes, pet tap and bag/shop/menu pointers: PASS");
    return 0;
  }
  if(activity && (!strcmp(activity,"evolution-stone") || !strcmp(activity,"evolution") ||
      !strcmp(activity,"evolution-decline") || !strcmp(activity,"evolution-choice"))) {
    lv_tick_set_cb(NULL);lv_indev_t *evo_input=lv_indev_create();
    lv_indev_set_type(evo_input,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(evo_input,pointer);
    pump(100);
    if(!strcmp(activity,"evolution-stone") || pp_ui_page()==4) tap(evo_input,180,276,80);
    assert(pp_ui_evolution_probe().choice);
    if(species==67 && evo_branch_choice>=1 && evo_branch_choice<=3)tap(evo_input,180,205+(evo_branch_choice-1)*36,80);
    if(strcmp(activity,"evolution-choice")) tap(evo_input,180,!strcmp(activity,"evolution-decline")?302:260,80);
    lv_indev_delete(evo_input);
    assert(pp_ui_evolving());
  }
  if(activity && !strcmp(activity,"heart-detail")) {
    lv_tick_set_cb(NULL);lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(in,pointer);pump(100);tap(in,91,121,80);lv_indev_delete(in);assert(pp_ui_page()==12);
  }
  if(activity && (!strcmp(activity,"achievement-crown") || !strcmp(activity,"achievement-tome"))) {
    lv_tick_set_cb(NULL);lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);
    pump(100);tap(in,274,93,80);tap(in,!strcmp(activity,"achievement-crown")?116:141,121,80);
    assert(pp_ui_page()==12);lv_indev_delete(in);
  }
  if(page==8 && (collection_quality!=PP_QUALITY_ALL || (activity && !strcmp(activity,"collection-detail")))) {
    lv_tick_set_cb(NULL);lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(in,pointer);pump(100);
    if(activity && !strcmp(activity,"collection-detail")) collection_quality=4;
    tap(in,88+(collection_quality==PP_QUALITY_ALL?0:collection_quality+1)*31,93,80);
    assert(pp_ui_collection_quality()==collection_quality);
    if(activity && !strcmp(activity,"collection-detail")) {tap(in,91,121,80);assert(pp_ui_page()==12);}
    lv_indev_delete(in);
  }
  if (smoke) {
    lv_tick_set_cb(NULL);
    lv_indev_enable(mouse, false);
    lv_indev_t *in = lv_indev_create();
    lv_indev_set_type(in, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(in, pointer);
    lv_indev_set_long_press_time(in, 1000);
    pump(100);
    if (pp_ui_page() != 13) return 1;
    tap(in, 178, 170, 80);
    if (game.pets[0].species != 0) return 1; /* Preview is not confirmation. */
    tap(in, 178, 308, 80);
    if (game.pets[0].species != 2 || pp_ui_page() != 0 || pp_needs_starter(&game)) return 1;
    pp_ui_open(2); pump(100); tap(in, 178, 322, 80);
    if (pp_ui_page() != 0) return 1;
    pp_ui_open(3); pump(100); tap(in, 178, 322, 80);
    if (pp_ui_page() != 0) return 1;
    pp_ui_open(0); pump(100);
    uint32_t coins = game.coins;
    tap(in, 109, 149, 80);
    if (game.coins != coins + 1) {
      fprintf(stderr, "Pet pointer tap failed\n");
      return 1;
    }
    tap(in, 109, 149, 1200);
    if (pp_ui_page() != 4) {
      fprintf(stderr, "Long press status did not open\n");
      return 1;
    }
    if (game.coins != coins + 1) {
      fprintf(stderr, "Long press also patted\n");
      return 1;
    }
    pp_ui_open(0);
    pump(600);
    tap(in, 232, 313, 80);
    pump(600);
    if (pp_ui_page() != 3) {
      fprintf(stderr, "Shop did not open, current=%u\n", pp_ui_page());
      return 1;
    }
    tap(in, 113, 132, 80);
    if (game.coins != coins + 1 - 10 || game.inventory[0] != 4) {
      fprintf(stderr,
              "Shop pointer purchase failed coins=%u expected=%u stock=%u\n",
              game.coins, coins + 1 - 10, game.inventory[0]);
      return 1;
    }
    game.coins=600;
    tap(in,100,265,80);
    assert(game.count==1 && game.coins==450 && game.hatch_boosts==1 && pp_ui_page()==3);
    tap(in,260,199,80);assert(game.inventory[5]==1 && game.coins==270);
    pp_ui_open(2);pump(100);tap_bag_item(in,5,80);assert(game.egg_end && pp_ui_page()==0);
    pp_ui_open(2);pump(100);tap_bag_item(in,6,80);
    assert(game.count==2 && game.coins==270 && !game.hatch_boosts && pp_ui_page()==30 && !game.egg_end);
    tap(in,252,302,80);assert(pp_ui_page()==29);tap(in,180-(int)((game.count<3?game.count:3)-1)*39,120,80);
    assert(pp_ui_page()==0 && game.count==1 && game.reserve_count==1);
    pp_ui_open(2);pump(100);tap(in,113,132,80);
    assert(pp_ui_page()==0 && game.drop_active && game.drop_target==0);
    int dx,dy;pp_drop_position(&game,&dx,&dy);
    int tx=dx-8,ty=dy-16;pp_home_ground_project(game.scene,&tx,&ty);dx=tx+8;dy=ty+16; /* Visible drop follows the same safe ground projection as the pet. */
    pump(2600);
    pp_home_state fed=pp_ui_home_probe(0);
    assert(fed.drop_visible && fed.drop_x==dx+4 && fed.drop_y==dy+8);
    assert(fed.x==dx-8 && fed.y==dy-16);
    pp_tick(&game,4,game.day);pump(100);
    assert(!pp_ui_home_probe(0).drop_visible && !game.drop_active);
    puts("Shop hatch boost and bag random drop: pointer purchase, home return, pet arrival and consumption: PASS");
    pp_init(&game,42); pp_tick(&game,0,day_now()); fixture();
    pp_ui_open(5);pump(100);tap(in,100,151,80);
    pp_ui_open(1); pump(100); tap_app(in,6);
    assert(pp_ui_page()==14);
    pp_record_steps(&game,100); pump(200);
    assert(pp_ui_page()==14 && game.steps_today==100 && game.steps_coins==0);
    tap(in,180,312,80); assert(pp_ui_page()==1);
    tap_app(in,12);assert(pp_ui_page()==15 && pp_ui_pokedex_page()==0);
    for(unsigned dex_page=0;dex_page<(PP_DEX_COUNT+9)/10;dex_page++) {
      for(unsigned entry=dex_page*10;entry<PP_DEX_COUNT && entry<(dex_page+1)*10;entry++) {
        char number[16];snprintf(number,sizeof(number),"%03u",pp_pokedex[entry].national_id);
        assert(visible_labels(lv_screen_active(),pp_dex_owned(&game,entry)?pp_pokedex[entry].name:number)==1);
      }
      if(dex_page+1<(PP_DEX_COUNT+9)/10)tap(in,320,180,80);
    }
    tap(in,320,180,80);assert(pp_ui_pokedex_page()==0);
    for(unsigned dex_page=1;dex_page<(PP_DEX_COUNT+9)/10;dex_page++) {
      tap(in,320,180,80);assert(pp_ui_page()==15 && pp_ui_pokedex_page()==dex_page);
    }
    for(unsigned dex_page=(PP_DEX_COUNT+9)/10-1;dex_page>0;dex_page--) {
      tap(in,40,180,80);assert(pp_ui_pokedex_page()==dex_page-1);
    }
    tap(in,180,312,80);assert(pp_ui_page()==1);
    printf("Pet Pokedex APP entry, %u forms in national order regardless of ownership, all pages and APP return: PASS\n",PP_DEX_COUNT);
    pp_ui_open(8);pump(100);pp_image_stats collection_cache=pp_image_probe();uint32_t saved_rng=game.rng;
    for(unsigned q=0;q<7;q++) {
      tap(in,88+(q==PP_QUALITY_ALL?0:q+1)*31,93,80);
      assert(pp_ui_collection_quality()==q && pp_ui_collection_page()==0);
      unsigned pages=(pp_collection_total(q)+63)/64;
      for(unsigned p=1;p<pages;p++){tap(in,320,180,80);assert(pp_ui_collection_page()==p);}
      if(pages>1){tap(in,320,180,80);assert(pp_ui_collection_page()==0);}
      tap(in,91,121,80);assert(pp_ui_page()==12);
      tap(in,180,310,80);assert(pp_ui_page()==8 && pp_ui_collection_quality()==q);
    }
    assert(pp_image_probe().file_reads==collection_cache.file_reads && game.rng==saved_rng);
    puts("Collection: seven quality tabs, all pages/wrap, actual item detail, return retains filter, zero new image reads or RNG changes: PASS");

    pp_ui_open(5);
    pump(600);
    tap(in, 180, 276, 80);
    pump(600);
    if (game.traveler != 0 || pp_ui_page() != 5) {
      fprintf(stderr, "Travel pointer departure failed\n");
      return 1;
    }
    pp_ui_open(6);
    pump(600);
    tap(in, 153, 151, 80);
    pump(600);
    tap(in, 180, 264, 80);
    pump(600);
    if (!game.battling || pp_ui_page() != 6) {
      fprintf(stderr, "Battle pointer start failed\n");
      return 1;
    }
    for (unsigned move_id = 0; move_id < PP_MOVE_COUNT; move_id++) {
      unsigned species_id=PP_SPECIES,move_slot=PP_MOVE_SLOTS;
      for(unsigned family=0;family<PP_SPECIES;family++)for(unsigned slot=0;slot<PP_MOVE_SLOTS;slot++)
        if(pp_move_at(family,slot)==&pp_moves[move_id]){species_id=family;move_slot=slot;}
      assert(species_id<PP_SPECIES && move_slot<PP_MOVE_SLOTS);
      pp_init(&game, 42);
      pp_tick(&game, 0, day_now());
      game.pets[0].species = species_id;
      assert(pp_battle(&game, 0) == PP_OK);
      game.battle_round = move_slot;
      pp_ui_open(6);
      pump(1100); /* Complete the opening release before the forced test turn. */
      int own = game.battle_hp, enemy = game.enemy_hp;
      pp_tick(&game, 2, day_now());
      pp_ui_refresh();
      pp_battle_view_state state = pp_battle_view_probe();
      assert(state.active && state.casting && !state.settled);
      assert(state.slot==move_slot);
      assert(state.hp[0] == own && state.hp[1] == enemy);
      pump(140);
      state = pp_battle_view_probe();
      assert(state.particles > 0 && state.species == species_id &&
             state.side == 0);
      assert(state.hp[0] == own && state.hp[1] == enemy);
      pump(160);
      state = pp_battle_view_probe();
      assert((state.hp[1] < enemy)==(pp_battle_move_power(&game,0,move_slot)>0) && state.hp[0] == own);
      pump(440);
      state = pp_battle_view_probe();
      assert(state.side == 1 && state.species == (species_id + 1) % PP_SPECIES);
      assert(state.hp[1] == game.enemy_hp && state.hp[0] == own);
      pump(240);
      state = pp_battle_view_probe();
      assert((state.hp[0] < own)==(pp_battle_move_power(&game,1,move_slot)>0));
      pump(280);
      assert(!pp_battle_view_probe().casting);
      game.enemy_hp = 1;
      while(!pp_battle_move_power(&game,0,game.battle_round%4))game.battle_round++;
      pp_ui_open(6);
      pp_tick(&game, 2, day_now());
      pp_ui_refresh();
      assert(!game.battling && !pp_battle_view_probe().settled);
      pump(5000); /* Narration drains before settlement/evolution/retry. */
      state = pp_battle_view_probe();
      assert(state.settled && state.hp[1] == 0);
      pp_ui_open(0);
      pump(80);
      assert(!pp_battle_view_probe().active);
    }
    pp_init(&game, 42); pp_tick(&game, 0, day_now()); game.pets[0].species = 1;
    /* A new game needs fresh home motion after the all-move fixture. */
    pp_ui_create(&game); pump(100);
    pp_image_stats cache = pp_image_probe();
    int min_x = 999, max_x = -999, min_y = 999, max_y = -999;
    bool saw_left = false, saw_right = false, saw_bubble = false;
    unsigned visible_ms = 0, burst_ms=0, longest_burst=0;
    for (unsigned ms = 0; ms < 45000; ms += 100) {
      pump(100);
      pp_home_state h = pp_ui_home_probe(0);
      if (h.x < min_x) min_x = h.x;
      if (h.x > max_x) max_x = h.x;
      if (h.y < min_y) min_y = h.y;
      if (h.y > max_y) max_y = h.y;
      saw_left |= h.left; saw_right |= !h.left;
      if (h.x < 48 || h.x > 272 || h.y < 115 || h.y > 220) {fprintf(stderr,"Patrol bounds at %ums: x=%d y=%d\n",ms,h.x,h.y);return 1;}
      burst_ms=h.bubble_visible?burst_ms+100:0;
      if(burst_ms>longest_burst)longest_burst=burst_ms;
      if (h.bubble_visible) {
        saw_bubble = true; visible_ms += 100;
        int padding=h.bubble_icon>=0?48:16;lv_point_t measured;
        lv_text_get_size(&measured,h.bubble_text,&pp_font_12,0,0,220-padding,LV_TEXT_FLAG_NONE);
        assert(h.bubble_width==measured.x+padding&&h.bubble_height>=44);
        /* The 33ms speech placement may trail the motion sample by up to 2px. */
        int expected_x = h.x + 24 - h.bubble_width/2;
        if (expected_x < 36) expected_x = 36;
        if (expected_x > 324-h.bubble_width) expected_x = 324-h.bubble_width;
        int expected_y = h.y - 48; if (expected_y < 99) expected_y = 99;
        if (h.speaker != 0 || abs(h.bubble_x - expected_x) > 2 || abs(h.bubble_y - expected_y) > 2) {fprintf(stderr,"Bubble attachment at %ums: %d/%d %d/%d speaker=%u\n",ms,h.bubble_x,expected_x,h.bubble_y,expected_y,h.speaker);return 1;}
      }
    }
    if (max_x - min_x < 80 || max_y - min_y < 60 || !saw_left || !saw_right ||
        !saw_bubble || longest_burst > 3400 || pp_image_probe().file_reads != cache.file_reads) {
      fprintf(stderr, "Patrol/bubble/cache regression: x=%d y=%d speech=%u reads=%u/%u\n",
              max_x-min_x,max_y-min_y,visible_ms,cache.file_reads,pp_image_probe().file_reads);
      return 1;
    }
    puts("Starter confirm, bag/shop home return, 2D patrol, both directions, attached 2.8s speech and zero animation file reads: PASS");
    printf("All %u move effects, alternating casts, impact-synchronized HP, "
           "final-hit settlement and exit cleanup: PASS\n", PP_MOVE_COUNT);
    pp_ui_open(7);pump(100);assert(pp_ui_page()==1); /* retired route */
    pp_ui_open(11);pump(100);tap(in,180,178,80);tap(in,180,125,80);
    assert(game.random_scene);tap(in,180,125,80);assert(!game.random_scene);
    pp_ui_open(0);unsigned pats=game.coins;pp_ui_pat_all();
    assert(game.coins==pats+game.count-(game.traveler>=0?1:0));
    game.traveler=-1;game.battling=0;game.drop_active=0;
    game.coins=1000;assert(pp_test_hatch(&game)==PP_OK);
    assert(pp_reserve_pet(&game,1)==PP_OK);
    unsigned reserve_species=game.reserves[0].species;
    pp_ui_open(4);pump(100);tap(in,125,304,80);assert(pp_ui_page()==16);
    unsigned target_slot=game.count<PP_PETS?game.count:0;unsigned row=game.count/3,col=game.count%3,columns=game.count+pp_warehouse_count(&game);if(columns>9)columns=9;columns-=row*3;if(columns>3)columns=3;tap(in,(360-columns*78+6)/2+col*78+36,117+row*68,80);assert(pp_ui_page()==26);
    tap(in,248,276,80);if(game.count==PP_PETS){assert(pp_ui_page()==29);tap(in,180-(int)((game.count<3?game.count:3)-1)*39,120,80);}
    assert(pp_ui_page()==0 && game.pets[target_slot].species==reserve_species);
    puts("Bulk petting, reserve swap, daily scene switch and removed digging route: PASS");
    pp_ui_open(10);
    pump(600);
    uint32_t before = game.coins;
    tap(in, 180, 276, 80);
    pump(600);
    tap(in, 180, 276, 80);
    pump(600);
    if (game.coins != before + 10) {
      fprintf(stderr, "Tarot daily cap failed\n");
      return 1;
    }
    fixture();
    game.pets[0].species = 2;
    game.growth[0].level = 15;
    game.growth[0].xp = 299;
    pp_ui_open(0);
    pp_gain_xp(&game, 0, 1);
    pump(140);
    assert(pp_ui_evolution_probe().choice && game.growth[0].stage == 0);
    tap(in,180,260,80);
    assert(pp_ui_evolving() && game.growth[0].stage == 1);
    pump(2400);
    assert(game.growth[0].stage == 1);
    tap(in, 180, 310, 80);
    assert(!pp_ui_evolving() && pp_ui_page() == 4);
    pump(200);
    assert(!pp_ui_evolving());
    game.growth[0].level = 36;
    assert(pp_battle(&game, 0) == PP_OK);
    pp_ui_open(6);
    pump(140);
    assert(!pp_ui_evolving());
    game.enemy_hp = 1;
    pp_tick(&game, 2, day_now());
    pp_ui_refresh();
    pump(400);
    assert(!pp_ui_evolving());
    pump(5000); /* Finish the queued defeat/result before evolution. */
    assert(pp_ui_evolution_probe().choice && game.growth[0].stage == 1);
    tap(in,180,260,80);
    assert(pp_ui_evolving() && game.growth[0].stage == 2);
    pump(2400);
    tap(in, 180, 310, 80);
    assert(!pp_ui_evolving() && game.growth[0].stage == 2);
    puts("Automatic evolution, one-time commit, completion pointer and battle "
         "FX deferral: PASS");
    for (unsigned family=4;family<7;family++) {
      pp_init(&game,42);pp_tick(&game,0,day_now());game.pets[0].species=family;
      for(unsigned stage=0;stage<2;stage++) {
        unsigned level=pp_evolution_level(&game,0);
        game.growth[0].level=level-1;game.growth[0].xp=pp_xp_needed(level-1)-1;
        pp_ui_open(0);pp_gain_xp(&game,0,1);pump(140);
        assert(pp_ui_evolution_probe().choice && game.growth[0].stage==stage);
        tap(in,180,260,80);
        assert(pp_ui_evolving() && game.growth[0].stage==stage+1);
        assert(pp_art_id(&game,0)==10+(family-4)*3+stage+1);
        pump(2400);tap(in,180,310,80);
        assert(!pp_ui_evolving() && pp_ui_page()==4 && pp_valid(&game));
      }
    }
    puts("010-018: all six automatic evolution presentations, completion pointers and exact new forms: PASS");
    for(unsigned family=7;family<=9;family++) {
      pp_init(&game,42);pp_tick(&game,0,day_now());game.pets[0].species=family;
      unsigned gate=pp_evolution_level(&game,0);game.growth[0].level=gate;
      pp_ui_open(0);pump(140);assert(pp_ui_evolution_probe().choice && !game.growth[0].stage);
      tap(in,180,260,80);assert(pp_ui_evolving() && game.growth[0].stage==1);
      pump(2400);tap(in,180,310,80);assert(!pp_ui_evolving() && pp_valid(&game));
    }
    pp_init(&game,42);pp_tick(&game,0,day_now());game.count=2;game.pets[1].species=1;game.coins=500;
    pp_ui_open(4);pump(140);tap(in,180,276,80);
    assert(pp_ui_evolution_probe().choice && game.coins==500);tap(in,180,260,80);
    assert(pp_ui_evolving()&&game.growth[0].stage==1&&game.coins==300);
    pump(2400);tap(in,180,310,80);assert(!pp_ui_evolving() && pp_art_id(&game,0)==25 && pp_valid(&game));
    puts("019-027: three level evolutions and stone purchase/use by real pointer, atomic cost and exact Raichu: PASS");
    for(unsigned entry=0;entry<PP_DEX_COUNT;entry++) {
      const pp_dex_entry *form=&pp_pokedex[entry];
      pp_init(&game,42);pp_tick(&game,0,day_now());game.count=2;game.pets[1].species=1;
      game.pets[0].species=form->species;game.growth[0].stage=form->stage;
      game.growth[0].level=form_level(form->species,form->stage);
      assert(pp_valid(&game));pp_ui_open(0);pump(100);pp_image_stats before_images=pp_image_probe();
      for(unsigned ms=0;ms<45000;ms+=100) pump(100);
      assert(pp_image_probe().file_reads==before_images.file_reads);
      game.pets[0].sleeping=1;pump(2200);game.pets[0].sleeping=0;
      assert(pp_battle(&game,0)==PP_OK);pp_ui_open(6);pump(1100);pp_ui_open(0);assert(!pp_ui_evolving());
    }
    printf("All %u forms: lossless indexed images, 45s cache residency, sleep and battle rendering: PASS\n",PP_DEX_COUNT);
    puts("Real pointer tap, 1s hold, purchase, travel, battle, roster and tarot "
         "daily cap: PASS");
    return 0;
  }
  if (frames_dir || shot) {
    lv_tick_set_cb(NULL);
    unsigned elapsed = 0, seconds = 0;
    unsigned wait = activity && !strcmp(gym_stage,"faint") ? 1900 : at_ms                                     ? at_ms
                    : activity && !strcmp(activity, "battle") ? 2450
                                                              : 200;
    while (elapsed < wait) {
      unsigned step = wait - elapsed < 10 ? wait - elapsed : 10;
      pump(step);
      elapsed += step;
      if (elapsed / 1000 > seconds) {
        pp_tick(&game, elapsed / 1000 - seconds, day_now());
        seconds = elapsed / 1000;
        pp_ui_refresh();
      }
    }
    if (frames_dir) {
      /* Preview-only continuous recording: exercise real pointer input without
       * skipping simulation time or changing firmware UI behavior. */
      lv_indev_t *film_input=NULL;
      if(warehouse_film){
        assert(activity && !strcmp(activity,"warehouse") && pp_ui_page()==16);
        film_input=lv_indev_create();
        lv_indev_set_type(film_input,LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(film_input,pointer);
      }
      if (activity && (!strcmp(activity,"evolution") || !strcmp(activity,"evolution-stone")))
        assert(pp_ui_evolving()); /* Never export home motion as an evolution GIF. */
      for (unsigned i = 0; i < frame_count; i++) {
        if(film_input){
          if(i==30 || i==60 || i==94){
            point=(lv_point_t){i==30?320:180,i==30?180:i==60?117:312};
            pressed=true;lv_indev_read(film_input);
          }else if(i==32 || i==62 || i==96){
            pressed=false;lv_indev_read(film_input);
          }
          if(i==64)assert(pp_ui_page()==26);
          if(i==98)assert(pp_ui_page()==16);
        }
        char path[640];
        snprintf(path, sizeof(path), "%s/%03u.ppm", frames_dir, i);
        screenshot(path);
        for (unsigned t = 0; t < frame_step; t += 5) {
          unsigned capture_step=frame_step-t<5?frame_step-t:5;
          pump(capture_step);
          elapsed += capture_step;
          if (elapsed / 1000 > seconds) {
            pp_tick(&game, elapsed / 1000 - seconds, day_now());
            seconds = elapsed / 1000;
            pp_ui_refresh();
          }
        }
      }
      if(film_input)lv_indev_delete(film_input);
    } else
      screenshot(shot);
    return 0;
  }
  uint32_t last = SDL_GetTicks();
  for (;;) {
    if(shell && !pp_shell_poll())break;
    lv_timer_handler();
    if(shell)pp_shell_render();
    uint32_t now = SDL_GetTicks();
    if (now - last >= 1000) {
      pp_tick(&game, (now - last) / 1000, day_now());
      pp_gift_scene(&game,pp_ui_page()==0 && !pp_ui_evolving(),(now-last)/1000);
      pp_wild_scene(&game,pp_ui_page()==0 && !pp_ui_evolving());
      last += ((now - last) / 1000) * 1000;
      clock_now();
      pp_ui_refresh();
    }
    SDL_Delay(5);
  }
  if(shell)pp_shell_destroy();
  pp_dispose(&game);
  return 0;
}
