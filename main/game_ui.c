#include "shop_reel.h"
#include "warehouse_sort.h"
#include "game_ui.h"
#include "nav_controls.h"
#include "battle_view.h"
#include "intro_view.h"
#include "fishing_view.h"
#include "hospital_view.h"
#include "training_view.h"
#include "moves.h"
#include "pet_animation.h"
#include "image_assets.h"
#include "gym_door_layout.h"
_Static_assert(PP_GYM_DOOR_COUNT == PP_GYM_COUNT, "gym door layout count");
#include "scene_fx.h"
#include "scene_layout.h"
#include "scene_nests.h"
#include "home_navigation.h"
#include "egg_glow.h"
#include "item_drop.h"
#include "home_behavior.h"
#include "home_conflict.h"
#include "pet_dialogue.h"
_Static_assert(PP_HOME_ACTORS==PP_PETS,"home behavior actor capacity");
#include "scene_view.h"
#include "travel_scenery.h"
#include "scene_effect_view.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
LV_FONT_DECLARE(pp_font_12);
LV_FONT_DECLARE(pp_font_shop_10);
LV_FONT_DECLARE(pp_font_14);
LV_FONT_DECLARE(pp_font_18);
LV_FONT_DECLARE(pp_font_title_22);
LV_FONT_DECLARE(pp_font_nav_14);
LV_FONT_DECLARE(pp_clock_38);
LV_FONT_DECLARE(pp_clock_36);
static pp_game *g;
static pp_battery_reading battery_reading={.percent=-1};
static uint32_t battery_charge_elapsed, battery_charge_tick;
static int battery_shown_width=-1;
static lv_obj_t *battery_group, *battery_outline, *battery_cap, *battery_fill, *battery_value;
static unsigned display_options=PP_UI_DISPLAY_DEFAULT;
static bool training_auto_supply;
static unsigned settings_section;
static const char *const settings_titles[]={"设置","显示设置","场景设置","历练设置"};
void pp_ui_auto_supply_load(unsigned enabled){training_auto_supply=enabled==1;}
bool pp_ui_auto_supply(void){return training_auto_supply;}
void pp_ui_display_load(unsigned flags) {display_options=flags<=7?flags:PP_UI_DISPLAY_DEFAULT;}
unsigned pp_ui_display_options(void) {return display_options;}

static lv_obj_t *screen, *body, *title, *subtitle, *coin, *clock_label, *toast,
    *pet_img[PP_PETS], *bubble, *drop, *clock_shadow;
static lv_obj_t *bubble_box,*bubble_icon,*bubble_badge;
static int talk_width=176,talk_height=44;
static uint16_t talk_cursors[PP_PETS][PP_TALK_TOPICS],ambient_talk_seq[PP_PETS];
static int talk_icon=-1;
static bool talk_care;
static char talk_text[96];
static void home_say(unsigned pet,unsigned topic,unsigned duration);
static void home_speech(unsigned pet,const char *words,int icon,unsigned duration);
static lv_obj_t *live_status, *live_log, *again, *egg_icon, *egg_glow, *egg_rim, *drop_icon,
    *coin_shadow, *coin_icon, *back_button, *step_number, *step_rewards, *step_rate, *step_bar, *step_total;
static unsigned current, selected, dex_page, scene_page, frame_count,
    last_scene = 999;
static bool battle_layout, travel_layout, starter_mode, intro_seen;
void pp_ui_onboarding(bool seen) { intro_seen=seen; }
bool pp_ui_intro_seen(void) { return intro_seen; }
static void intro_finish(void) { intro_seen=true; pp_ui_open(13); }
static lv_obj_t *gift_overlay;
static lv_obj_t *wild_overlay, *wild_fighter, *wild_fight_action, *wild_resume;
static unsigned wild_selected, legend_selected, app_page;
static bool gym_waiting_wild;
static unsigned bag_page;
static struct {bool active,awarded;unsigned item,elapsed;uint32_t tick;} shop_reel;
static lv_obj_t *shop_cards[9];
static int shop_lit=-1,shop_glow=-1;

enum { BAG_PAGE_SIZE=12 };
/* Owned ordinary stock and eggs share a compact list in catalog order. */
static unsigned bag_entries(unsigned *entries) {
  unsigned count=0;
  for(unsigned i=0;i<PP_SHOP_ITEMS;i++)if(pp_item_enabled(i)&&pp_item_count(g,i)){if(entries)entries[count]=i;count++;}
  for(unsigned i=0;i<PP_LEGEND_COUNT;i++)if(g->legend.eggs[i]){if(entries)entries[count]=PP_SHOP_ITEMS+i;count++;}
  return count;
}
static lv_obj_t *wild_alert, *wild_alert_pet, *wild_alert_mark;
static const void *wild_alert_src;
/* 3px blocks keep the alert crisp without a font or another Flash image. */
static const char *const wild_mark_pixels[]={
  "01110","12221","12221","12221","12221","01110","00000","01110","12221","01110"
};
static void wild_overlay_tick(void);
/* Presentation only: capture is committed atomically by the core before this
 * timeline starts. Reopening a saved receipt never consumes another ball. */
static struct {
  lv_obj_t *pet, *ball, *halo, *shadow, *spark[12];
  const void *source;
  pp_animation_clock clock;
  uint32_t started;
  bool active;
} wild_result;
static void wild_result_tick(void);
static unsigned gym_team[3], gym_team_count, gym_selected;
static char gym_art_paths[PP_GYM_COUNT][5][320];
static const char *const gym_files[]={"brock","boulder","pewter","badge-icon","enemy-74","enemy-75","enemy-95","arena","map","badge-display"};
static char gym_paths[10][320], gym_defeated_paths[PP_GYM_COUNT][320];
static char gym_guard_paths[PP_GYM_COUNT][320];
static lv_obj_t *gym_clear_stamp, *gym_guards;
static lv_obj_t *travel_activity, *travel_reward_label, *travel_claim;
static lv_obj_t *travel_backdrop;
static struct {
  lv_obj_t *effects;
  uint32_t elapsed,last;
  unsigned destination,particles;
} travel_motion;
static bool travel_notice_active, travel_home_deferred, ui_screen_on=true;
static struct {
  bool active;unsigned art,coins,xp,destination;
  char name[40],item[96];pp_travel_reward reward;
} travel_receipt;
static struct {
  lv_obj_t *pet,*coins,*xp,*halo,*spark[8];
  const void *source;pp_animation_clock clock;uint32_t started;
} travel_result;
static bool travel_settle(void);
static bool travel_present(void);
static bool travel_page_safe(unsigned page);
static void travel_notice_view(void);
static bool travel_settlement_blocked(void);
static const char *travel_wait_message(void);
static void travel_result_view(void);
static void travel_result_tick(void);
enum { PETDEX_PAGE_SIZE = 10 };
static unsigned starter_choice = 1, petdex_page;
static unsigned collection_quality = PP_QUALITY_ALL;
static unsigned collection_pages(void) { return (pp_collection_total(collection_quality)+63)/64; }
static lv_obj_t *starter_card[3], *starter_name;
static uint32_t home_epoch, next_talk, talk_until;
/* Garden presentation time pauses in menus; game time and rewards do not. */
static uint32_t home_time, home_real_tick;
static bool home_initialized;
static unsigned home_fx_scene;
typedef struct {
  int x,y,origin_x,origin_y;
  uint32_t started,behavior_start;
  bool initialized,sleeping,interacting,tired;
} home_motion_state;
static home_motion_state home_motion[PP_PETS];
static bool facing_left[PP_PETS];
static pp_home_behavior_state home_behavior;
static pp_home_output home_outputs[PP_PETS];
static pp_home_conflict home_conflict;
static int home_conflict_origin[2][2];
static unsigned home_conflict_particles;
static void home_conflict_begin(void);
static void home_conflict_release(uint32_t now);
static void home_conflict_update(uint32_t now,pp_home_actor actors[PP_PETS]);
static lv_obj_t *home_symbols[PP_PETS];
static unsigned home_symbol_kind[PP_PETS];
static uint32_t home_left_at,home_welcomed_at;
static bool home_away,home_has_welcomed,home_welcome_pending;
static unsigned pet_ground_layer[PP_PETS];
static void home_inputs(pp_home_actor in[PP_PETS]) {
 for(unsigned i=0;i<PP_PETS;i++)in[i]=(pp_home_actor){.x=home_motion[i].x,.y=home_motion[i].y,
  .left=facing_left[i],.mood=g->pets[i].mood,.energy=g->pets[i].energy,
  .available=i<g->count && !g->pets[i].sleeping && g->pets[i].health && g->traveler!=(int)i && !pp_training_busy(g,i) &&
   !(g->drop_active&&g->drop_target==i) && !g->battling && !gift_overlay && !wild_overlay &&
   (!pet_img[i] || !lv_obj_has_state(pet_img[i],LV_STATE_PRESSED))};
}
static void home_react(unsigned p) {
 pp_home_actor in[PP_PETS];home_inputs(in);
 pp_home_behavior_request(&home_behavior,home_time,PP_HOME_PAT,p,0,0,0,in,g->count);
 home_say(p,PP_TALK_PAT,3200);
}
static void home_ground_event(lv_event_t *e) {
 if(current!=0 || gift_overlay || wild_overlay || g->battling || g->wild.phase || g->drop_active)return;
 lv_indev_t *indev=lv_event_get_indev(e);if(!indev)return;
 lv_point_t point;lv_indev_get_point(indev,&point);
 if(g->egg_end){int x,y;pp_egg_position(g,&x,&y);if(point.x>=x-8&&point.x<x+40&&point.y>=y-8&&point.y<y+42)return;}
 pp_home_actor in[PP_PETS];home_inputs(in);
 int pet=pp_home_behavior_call(&home_behavior,home_time,point.x,point.y,in,g->count);
 if(pet>=0)home_say((unsigned)pet,PP_TALK_CALL,3200);
}
static void home_symbol_draw(lv_event_t *e) {
 unsigned i=(unsigned)(uintptr_t)lv_event_get_user_data(e),kind=home_symbol_kind[i];
 static const char *pixels[4][7]={
  {"0110110","1111111","1111111","0111110","0011100","0001000","0000000"},
  {"0001110","0001010","0001000","0001000","0111000","1111000","0110000"},
  {"0011100","0100010","0000100","0001000","0001000","0000000","0001000"},
  {"0100010","0110110","0000000","1100011","0000000","0110110","0100010"}};
 if(!kind || kind>4)return;
 lv_area_t area;lv_obj_get_coords(lv_event_get_target_obj(e),&area);
 lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_opa=LV_OPA_COVER;
 d.bg_color=lv_color_hex(kind==1?0xf6aabb:kind==2?0xffdf8a:kind==4?0xf18b7a:0xc9e7dc);
 for(unsigned y=0;y<7;y++)for(unsigned x=0;x<7;x++)if(pixels[kind-1][y][x]=='1'){
  lv_area_t pixel={area.x1+(int)x*2,area.y1+(int)y*2,area.x1+(int)x*2+1,area.y1+(int)y*2+1};
  lv_draw_rect(lv_event_get_layer(e),&d,&pixel);
 }
}
/* The upper shell emits small fading dots. Draw into one persistent object. */
static uint32_t egg_glow_elapsed;
static void egg_glow_draw(lv_event_t *e) {
  pp_egg_glow_dot dots[PP_EGG_GLOW_DOTS];
  unsigned count=pp_egg_glow_sample(egg_glow_elapsed,g->legend.hatching!=0,dots);
  lv_area_t area;lv_obj_get_coords(lv_event_get_target_obj(e),&area);
  lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);
  for(unsigned i=0;i<count;i++) {
    const pp_egg_glow_dot *dot=&dots[i];
    d.bg_color=lv_color_hex(dot->color);d.bg_opa=dot->opacity;
    int x=area.x1+PP_EGG_GLOW_OFFSET_X+dot->x,y=area.y1+PP_EGG_GLOW_OFFSET_Y+dot->y;
    lv_area_t pixel={x,y,x+dot->size-1,y+dot->size-1};
    lv_draw_rect(lv_event_get_layer(e),&d,&pixel);
  }
}
static lv_obj_t *home_tree_front;
static char lake_front_path[320];
static uint32_t ambient_seed, ambient_visits;
static bool ambient_preview, ambient_enabled=true;
static const lv_image_dsc_t *tumbleweed_frames[3][8];
static void ambient_update(uint32_t elapsed) {
  pp_scene_particle particles[PP_SCENE_FX_MAX];
  unsigned count=ambient_enabled?pp_scene_fx_sample(g->scene,ambient_seed,elapsed,ambient_preview,particles):0;
  pp_scene_particle skill[PP_HOME_CONFLICT_PARTICLES];
  home_conflict_particles=pp_home_conflict_sample(&home_conflict,home_time,
   home_conflict_origin[0][0]+24,home_conflict_origin[0][1]+24,
   home_conflict_origin[1][0]+24,home_conflict_origin[1][1]+24,skill);
  if(count+home_conflict_particles>PP_SCENE_FX_MAX)count=PP_SCENE_FX_MAX-home_conflict_particles;
  memcpy(particles+count,skill,home_conflict_particles*sizeof(*skill));count+=home_conflict_particles;
  pp_scene_effect_view_update(particles,count);
}
static unsigned talk_pet;

static const void *left_paths[PP_ART_COUNT][PP_FRAME_COUNT];
static pp_animation_clock home_clocks[PP_PETS], activity_clocks[2], evolution_clock;
static char starter_paths[3][320], type_paths[3][320];
static lv_obj_t *evolution, *evo_pet, *evo_ring[3], *evo_glow[5], *evo_spark[32], *evo_hint, *evo_name,
    *evo_done, *evo_gift;
static bool evo_choosing, evo_stone, evo_declined;
static uint32_t evo_started;
static unsigned evo_branch,evo_pick,training_notice_index,training_notice_page;
static bool training_level,evo_training;
static pp_game evo_preview;
static pp_game *evo_state(void){return evo_pick>=PP_PETS?&evo_preview:g;}
static bool training_notice_tick(void);
static uint8_t evo_postponed_level[PP_PETS];
static unsigned evo_postponed_art[PP_PETS];
static unsigned evo_return_page;
static unsigned evo_pet_id, evo_before_art, evo_current_art, evo_tint;
static const void *evo_sprite_source;
static uint32_t evo_pixels[96 * 96];
static lv_image_dsc_t evo_descriptor;
/* Primary types, independent of whichever attack occupies slot zero. */
static const unsigned evo_colors[] = {
  0xf4ce58, 0x80cb83, 0xf69b68, 0x74c7ed, 0xb4cb65, 0xb4cb65,
  0xc4b29a, 0xc4b29a, 0xc4b29a, 0xc294df, 0xd9b67c,
  0xc294df, 0xc294df, 0xf4a8c5,
  0xf69b68, 0xc4b29a, 0xc294df, 0x80cb83, 0xb4cb65,
  0xc294df,0xd9b67c,0xc4b29a,0x74c7ed,0xe6a083,
  0xf69b68,0x74c7ed,0xc294df,0xe6a083,0x80cb83,0x74c7ed,0xc4b29a,0xf69b68,0xf3a7bc,0xc5d3dc,0xd9b67c,0xd9b67c,0xcde7ef,0xc294df,0xc294df,0x9f87dc,0xc4b29a,0xd695ed,0xf4ad7a,0xef9d9d,0xe9b4cb,0xd8bb98,0xd4b591,0xe9ad8c,0xefb1cb,0xb8a1e8,0xaeb5c5,0xf2bbd0,0x79b9ee,0xd5b28b,0x8bd4ed,0xf0b889,0xd7b07b,0xe8bed9,0xa0cf87,0xd6acde,0xf1d16c,0xeab77f,0xc8ae8d,0xceac7f,0xef9f69,0x8ecde8,0xcba9dc,0xc69d72,0xe7b1d7,0x88b9ce,0xb9a47b,0xb5a6cd,0x85aca2,0x91cfed,0xf3d66d,0xf2a46f,0x91bde8,0xc0a4e2,0xf0b4ce
};
static void evolution_tick(void);
static void evolution_open(unsigned pet, bool stone);
static void evolution_choose(bool decline);
static void evo_sprite(unsigned art, unsigned pose, uint32_t now);
static lv_obj_t *stairs[6], *lives;
static uint32_t last_revision, happy_until[PP_PETS], drop_started;
static bool drop_was_active;
static uint64_t egg_drop_deadline;
static uint32_t egg_drop_started;
static bool egg_drop_playing;
static int drop_from_x, drop_from_y;
static const void *pet_src[PP_PETS], *activity_src[2];
static lv_obj_t *activity_pet[2];
static char asset_root[240] = "S:/assets",

            scene_paths[PP_SCENES][320], thumb_paths[PP_SCENES][320],
            icon_paths[16][320],
            large_paths[16][320], avatar_paths[PP_ART_COUNT][320],
            small_paths[16][320], nav_paths[3][320], hud_coin_path[320], hud_coin_large_path[320], step_icon_path[320], pokedex_icon_path[320], hatch_boost_icon_path[320], capture_ball_icon_path[320], tree_essence_icon_path[320], tree_essence_small_path[320], gym_icon_path[320], training_icon_path[320], warehouse_icon_path[320], scene_icon_path[320], pure_heart_path[320], pure_heart_small_path[320];
static const void *paths[PP_ART_COUNT][PP_FRAME_COUNT];
static char menu_background_path[320];
static const void *shop_item_sources[PP_SHOP_ITEMS], *shop_lottery_source, *shop_coin_source;
static char fishing_icon_path[320],hospital_icon_path[320];
static const void *travel_sources[PP_TRAVEL_DESTINATIONS];
static const void *nest_front_sources[PP_SCENES];
static void collection_path(char *out, size_t size, unsigned id, bool small) {
  if (id==PP_PURE_HEART) snprintf(out,size,"%s/collection/pure-heart%s.bin",asset_root,small?"-small":"");
  else snprintf(out,size,"%s/collection/%03u%s.bin",asset_root,id,small?"-small":"");
}

static bool assets_ok, imu_ok, refresh_pending;
static const int px[] = {89, 238, 163, 189, 65},
                 py[] = {128, 179, 227, 80, 213};
static const char *const nav[] = {
    "庭院", "APP",   "BAG",   "SHOP",     "宠物信息", "旅行",  "挑战",
    "挖宝", "收藏品", "换场景", "每日塔罗", "设置",     "收藏物", "选择伙伴", "计步器", "宠物图鉴", "宠物仓库", "关都地图", "尼比道馆", "出战编队", "徽章展馆", "挑战收获", "片头", "野外收获", "神兽挑战", "神兽挑战", "个体信息", "命名", "放生", "替换伙伴", "孵化成功"};
static lv_color_t color(unsigned c) { return lv_color_hex(c); }
static void text(lv_obj_t *o, const char *s) {
  if (strcmp(lv_label_get_text(o), s))
    lv_label_set_text(o, s);
}
/* Measure only when speech changes or the courtyard objects are created. */
static void home_bubble_layout(void) {
  bool icon=talk_icon>=0;int padding=icon?48:16;
  lv_point_t size;lv_text_get_size(&size,talk_text,&pp_font_12,0,0,220-padding,LV_TEXT_FLAG_NONE);
  int width=size.x>0?size.x:1;
  talk_width=width+padding;talk_height=size.y+16;if(talk_height<44)talk_height=44;
  lv_obj_set_size(bubble_box,talk_width,talk_height);lv_obj_set_width(bubble,width);
  lv_obj_set_pos(bubble,icon?40:8,(talk_height-size.y)/2);
  lv_obj_set_y(bubble_badge,(talk_height-28)/2);lv_obj_set_y(bubble_icon,(talk_height-24)/2);
}
static void home_speech(unsigned pet,const char *words,int icon,unsigned duration) {
  if(pet>=g->count)return;
  talk_pet=pet;talk_icon=icon;talk_until=home_time+duration;
  talk_care=false;
  snprintf(talk_text,sizeof(talk_text),"%s",words);
  if(current==0 && bubble){text(bubble,talk_text);home_bubble_layout();}
}
static void home_say(unsigned pet,unsigned topic,unsigned duration) {
  if(pet>=g->count)return;
  pp_talk_line line=pp_talk_pick(talk_cursors[pet],topic,pp_art_id(g,pet)*7+pet*3);
  home_speech(pet,line.text,line.icon,duration);
}
static lv_obj_t *label(lv_obj_t *parent, const char *s, int x, int y, int w,
                       unsigned c) {
  lv_obj_t *o = lv_label_create(parent);
  lv_label_set_text(o, s);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_width(o, w);
  lv_obj_set_style_text_color(o, color(c), 0);
  lv_obj_set_style_text_align(o, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_font(o, &pp_font_14, 0);
  return o;
}
static lv_obj_t *icon(lv_obj_t *parent, unsigned id, int x, int y) {
  lv_obj_t *im = lv_image_create(parent);
  pp_image_set_src(im, icon_paths[id]);
  lv_obj_set_pos(im, x, y);
  return im;
}
static void settings_changed(lv_event_t *e) {
  unsigned flag=(unsigned)(uintptr_t)lv_event_get_user_data(e);
  bool checked=lv_obj_has_state(lv_event_get_target_obj(e),LV_STATE_CHECKED);
  if(flag==8)training_auto_supply=checked;
  else if(flag)display_options=checked?display_options|flag:display_options&~flag;
  else {pp_random_scene(g,checked);last_revision=g->revision;}
}
static void settings_row_pressed(lv_event_t *e) {
  lv_obj_t *sw=lv_event_get_user_data(e);
  if(lv_obj_has_state(sw,LV_STATE_CHECKED))lv_obj_remove_state(sw,LV_STATE_CHECKED);
  else lv_obj_add_state(sw,LV_STATE_CHECKED);
  lv_obj_send_event(sw,LV_EVENT_VALUE_CHANGED,NULL);
}
static void settings_row(lv_obj_t *parent,const char *name,int y,unsigned flag,bool checked) {
  lv_obj_t *row=lv_obj_create(parent);lv_obj_remove_style_all(row);
  lv_obj_set_pos(row,66,y);lv_obj_set_size(row,228,42);
  lv_obj_set_style_radius(row,14,0);lv_obj_set_style_bg_color(row,color(0x132d37),0);
  lv_obj_set_style_bg_opa(row,LV_OPA_COVER,0);
  lv_obj_remove_flag(row,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t *l=label(row,name,16,11,140,0xd4e4e3);
  lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_LEFT,0);
  lv_obj_t *sw=lv_switch_create(row);lv_obj_remove_style_all(sw);
  lv_obj_set_pos(sw,160,8);lv_obj_set_size(sw,48,26);
  lv_obj_set_ext_click_area(sw,8);
  lv_obj_set_style_radius(sw,LV_RADIUS_CIRCLE,LV_PART_MAIN);
  lv_obj_set_style_bg_color(sw,color(0x344c59),LV_PART_MAIN);
  lv_obj_set_style_bg_opa(sw,LV_OPA_COVER,LV_PART_MAIN);
  lv_obj_set_style_radius(sw,LV_RADIUS_CIRCLE,LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(sw,color(0x344c59),LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(sw,color(0x68b99b),LV_PART_INDICATOR|LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(sw,LV_OPA_COVER,LV_PART_INDICATOR);
  lv_obj_set_style_radius(sw,LV_RADIUS_CIRCLE,LV_PART_KNOB);
  lv_obj_set_style_bg_color(sw,color(0xf0f6ef),LV_PART_KNOB);
  lv_obj_set_style_bg_opa(sw,LV_OPA_COVER,LV_PART_KNOB);
  lv_obj_set_style_pad_all(sw,-3,LV_PART_KNOB);
  lv_obj_set_style_anim_duration(sw,160,0);
  if(checked)lv_obj_add_state(sw,LV_STATE_CHECKED);
  lv_obj_add_event_cb(sw,settings_changed,LV_EVENT_VALUE_CHANGED,(void *)(uintptr_t)flag);
  lv_obj_add_flag(row,LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(row,settings_row_pressed,LV_EVENT_SHORT_CLICKED,sw);
}
static void action(lv_event_t *e);
static lv_obj_t *button(lv_obj_t *parent, const char *s, int x, int y, int w,
                        int h, int command) {
  lv_obj_t *b = lv_button_create(parent);
  lv_obj_set_pos(b, x, y);
  lv_obj_set_size(b, w, h);
  lv_obj_set_style_radius(b, 13, 0);
  lv_obj_set_style_bg_color(b, color(0x1c3c4e), 0);
  lv_obj_set_style_bg_grad_color(b, color(0x142d3e), 0);
  lv_obj_set_style_bg_grad_dir(b, LV_GRAD_DIR_VER, 0);
  lv_obj_set_style_bg_color(b, color(0x34576a), LV_STATE_PRESSED);
  lv_obj_set_style_border_width(b, 1, 0);
  lv_obj_set_style_border_color(b, color(0x355568), 0);
  lv_obj_set_style_shadow_width(b, 0, 0);
  lv_obj_set_style_pad_all(b, 0, 0);
  lv_obj_t *l = label(b, s, 0, 0, w, 0xe4eee9);
  lv_obj_center(l);
  int icon = -1;
  if (strcmp(s, "返回")) {
    switch (command) {
    case 1:
      icon = 14;
      break;
    case 2:
      icon = 6;
      break;
    case 3:
      icon = 7;
      break;
    case 5:
      icon = 8;
      break;
    case 8:
      icon = 10;
      break;
    case 6:
      icon = 9;
      break;
    case 7:
      icon = 10;
      break;
    case 9:
      icon = 11;
      break;
    case 10:
      icon = 12;
      break;
    case 15:
      icon = 10;
      break;
    case 14:
      icon = 8;
      break;
    case 11:
      icon = 13;
      break;
    default:
      break;
    }
  }
  if (command >= 100 && command < 100 + PP_SHOP_ITEMS)
    icon = command - 100;
  if (command >= 120 && command < 120 + PP_SHOP_ITEMS)
    icon = command - 120;
  if (!strcmp(s, "返回")) {
    lv_label_set_text(l, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_14, 0);
  }
  if (icon >= 0) {
    bool ball=command==100+PP_BALL||command==120+PP_BALL;
    lv_obj_t *im = lv_image_create(b);
    pp_image_set_src(im, (command==100+PP_TREE_ESSENCE || command==120+PP_TREE_ESSENCE) ? tree_essence_icon_path : ball ? capture_ball_icon_path :
                         (command == 100 + PP_HATCH_BOOST || command == 120 + PP_HATCH_BOOST)
                         ? hatch_boost_icon_path
                         : command < 100 ? small_paths[icon] : icon_paths[icon]);
    if (command >= 100) {
      lv_obj_set_pos(im, 5, (h - 32) / 2);
      lv_obj_set_width(l, w - 40);
      lv_obj_align(l, LV_ALIGN_RIGHT_MID, -1, 0);
    } else {

      lv_obj_set_pos(im, (w - 24) / 2, s[0] ? 3 : (h - 24) / 2);
      lv_obj_align(l, LV_ALIGN_BOTTOM_MID, 0, -5);
    }
  }
  lv_obj_add_event_cb(b, action, LV_EVENT_CLICKED, (void *)(intptr_t)command);
  return b;
}
/* All previous/next controls share one visual and touch target. */
static lv_obj_t *switch_arrow(lv_obj_t *parent,int parent_y,int command,bool next) {
  lv_obj_t *b=button(parent,next?">":"<",next?298:18,158-parent_y,44,44,command);
  pp_nav_apply(b,next);
  return b;
}
static void pet_swap_button(lv_obj_t *parent, int x, int y) {
  lv_obj_t *b=button(parent,"",x,y,36,36,180);
  pp_footer_text_style(b);
  /* Two opposing pixel arrows; shared geometry with the Web information page. */
  static const uint8_t parts[][4]={
    {0,4,18,2},{12,0,2,2},{14,2,2,2},{14,6,2,2},{12,8,2,2},
    {0,14,18,2},{4,10,2,2},{2,12,2,2},{2,16,2,2},{4,18,2,2}
  };
  for(unsigned i=0;i<sizeof(parts)/sizeof(parts[0]);i++) {
    lv_obj_t *p=lv_obj_create(b);lv_obj_remove_style_all(p);
    lv_obj_set_pos(p,9+parts[i][0],8+parts[i][1]);
    lv_obj_set_size(p,parts[i][2],parts[i][3]);
    lv_obj_set_style_bg_color(p,color(0xb9e8d4),0);
    lv_obj_set_style_bg_opa(p,LV_OPA_COVER,0);
    lv_obj_remove_flag(p,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  }
}
/* One overlay survives page replacement. Animation only changes position/alpha;
 * text storage and the LVGL object are reused for every notification. */
static lv_timer_t *notice_timer;
static char notice_message[256];
static uint32_t notice_started;
static int notice_y;
enum { NOTICE_MS = 3200, NOTICE_RISE = 36 };
static void notice_animate(lv_timer_t *timer) {
  uint32_t age = lv_tick_get() - notice_started;
  if (age >= NOTICE_MS) {
    lv_obj_add_flag(toast, LV_OBJ_FLAG_HIDDEN);
    lv_timer_pause(timer);
    return;
  }
  unsigned opacity = age < 180 ? age * 255 / 180
                     : age > 2240 ? (NOTICE_MS - age) * 255 / 960 : 255;
  lv_obj_set_y(toast, notice_y - (int)(age * NOTICE_RISE / NOTICE_MS));
  lv_obj_set_style_bg_opa(toast, opacity * 245 / 255, 0);
  lv_obj_set_style_text_opa(toast, opacity, 0);
}
static void notice(const char *message) {
  if (!toast || !message || !*message) return;
  snprintf(notice_message, sizeof(notice_message), "%s", message);
  lv_point_t size;
  lv_text_get_size(&size, notice_message, &pp_font_14, 0, 2, 220, LV_TEXT_FLAG_NONE);
  int width = size.x + 24;
  if (width < 40) width = 40;
  if (width > 244) width = 244;
  lv_obj_set_width(toast, width);
  lv_label_set_text_static(toast, notice_message);
  lv_obj_update_layout(toast);
  /* Keep both short and wrapped messages inside the circular display. */
  notice_y = 210 - lv_obj_get_height(toast) / 2;
  lv_obj_set_pos(toast, 320 - width, notice_y);
  notice_started = lv_tick_get();
  lv_obj_remove_flag(toast, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(toast);
  notice_animate(notice_timer);
  lv_timer_reset(notice_timer);
  lv_timer_resume(notice_timer);
}
static void notice_create(void) {
  if (!toast) {
    toast = label(lv_layer_top(), "", 0, 0, 40, 0xf8f4db);
    lv_label_set_long_mode(toast, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_style_text_align(toast, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_line_space(toast, 2, 0);
    lv_obj_set_style_bg_color(toast, color(0x1e433d), 0);
    lv_obj_set_style_pad_hor(toast, 12, 0);
    lv_obj_set_style_pad_ver(toast, 8, 0);
    lv_obj_set_style_radius(toast, 10, 0);
    lv_obj_remove_flag(toast, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    notice_timer = lv_timer_create(notice_animate, 33, NULL);
  }
  lv_obj_add_flag(toast, LV_OBJ_FLAG_HIDDEN);
  lv_timer_pause(notice_timer);
}
void pp_ui_notice(const char *s) { notice(s); }
static void shop_reel_paint(void) {
  if(current!=3 || !shop_reel.awarded)return;
  pp_shop_reel_view view=pp_shop_reel_at(shop_reel.item,shop_reel.elapsed);
  unsigned lit=view.slot;int glow=view.visible?1:0;
  if(shop_lit==(int)lit && shop_glow==glow)return;
  for(unsigned i=0;i<9;i++)if(shop_cards[i]){
    bool hot=i==lit && view.visible;
    lv_obj_set_style_border_width(shop_cards[i],hot?2:i==4?1:0,0);
    lv_obj_set_style_bg_opa(shop_cards[i],hot?110:i==4?LV_OPA_COVER:LV_OPA_TRANSP,0);
    lv_obj_set_style_border_color(shop_cards[i],color(hot?0xffdb83:i==4?0xcda962:0x386264),0);
    lv_obj_set_style_bg_color(shop_cards[i],hot?lv_color_mix(color(0xb49a50),color(0x173b40),200):color(i==4?0x544127:0x173b40),0);
  }
  shop_lit=(int)lit;shop_glow=glow;
}
static void shop_reel_tick(void) {
  uint32_t now=lv_tick_get(),delta=now-shop_reel.tick;shop_reel.tick=now;
  if(!shop_reel.active || current!=3 || !ui_screen_on)return;
  shop_reel.elapsed=pp_shop_reel_advance(shop_reel.item,shop_reel.elapsed,delta);
  if(pp_shop_reel_at(shop_reel.item,shop_reel.elapsed).done){
    shop_reel.active=false;
    char message[64];snprintf(message,sizeof(message),"抽中%sx1",pp_items[shop_reel.item].name);notice(message);
    if(back_button)lv_obj_remove_state(back_button,LV_STATE_DISABLED);
  }
  shop_reel_paint();
}

static void egg_notice(lv_event_t *e) {
  (void)e;
  if (current != 0 || !g->egg_end) return;
  uint64_t remaining = g->egg_end > g->seconds ? g->egg_end - g->seconds : 0;
  char message[96];
  if (remaining) {
    snprintf(message, sizeof(message), "距离孵化还有\n%02llu:%02u:%02u",
             (unsigned long long)(remaining / 3600),
             (unsigned)(remaining / 60 % 60), (unsigned)(remaining % 60));
    notice(message);
  } else notice("孵化时间已到");
}
void pp_ui_assets(const char *root) {
  snprintf(asset_root, sizeof(asset_root), "%s", root);
  for(unsigned i=0;i<6;i++){char path[320];snprintf(path,sizeof(path),"%s/training/%u.bin",root,i);pp_image_preload(path);}
  snprintf(hospital_icon_path,sizeof(hospital_icon_path),"%s/hospital/icon.bin",root);pp_image_preload(hospital_icon_path);
  {char path[320];snprintf(path,sizeof(path),"%s/hospital/room.bin",root);pp_image_preload(path);
   snprintf(path,sizeof(path),"%s/hospital/joy.bin",root);pp_image_preload(path);}
  snprintf(fishing_icon_path,sizeof(fishing_icon_path),"%s/fishing/icon.bin",root);
  pp_image_preload(fishing_icon_path);
  {char path[320];snprintf(path,sizeof(path),"%s/fishing/pond.bin",root);pp_image_preload(path);
   snprintf(path,sizeof(path),"%s/fishing/partner-icon.bin",root);pp_image_preload(path);
   snprintf(path,sizeof(path),"%s/fishing/dex-icon.bin",root);pp_image_preload(path);}
  for(unsigned i=0;i<PP_FISH_COUNT;i++){
    char path[320];snprintf(path,sizeof(path),"%s/fishing/fish-%u.bin",root,i);pp_image_preload(path);
    snprintf(path,sizeof(path),"%s/fishing/thumb-%u.bin",root,i);pp_image_preload(path);
  }
  for(unsigned i=0;i<PP_TRAVEL_DESTINATIONS;i++) {
    char path[320];snprintf(path,sizeof(path),"%s/travel-scenes/%u.bin",root,i);
    travel_sources[i]=pp_image_descriptor(path);
    if(!travel_sources[i]) abort();
  }
  const char *const stamps[PP_GYM_COUNT]={"defeated-rock","defeated-water","defeated-electric","defeated-grass","defeated-poison","defeated-psychic","defeated-fire","defeated-ground"};
  for(unsigned i=0;i<PP_GYM_COUNT;i++) {
    snprintf(gym_defeated_paths[i],sizeof(gym_defeated_paths[i]),"%s/gyms/%s.bin",root,stamps[i]);pp_image_preload(gym_defeated_paths[i]);
    snprintf(gym_guard_paths[i],sizeof(gym_guard_paths[i]),"%s/gyms/door-team-%u.bin",root,i);pp_image_preload(gym_guard_paths[i]);
  }
  for(unsigned i=0;i<10;i++) {snprintf(gym_paths[i],sizeof(gym_paths[i]),"%s/gyms/%s.bin",root,gym_files[i]);pp_image_preload(gym_paths[i]);}
  for(unsigned i=0;i<PP_GYM_COUNT;i++) {
    const pp_gym_def *d=&pp_gyms[i];
    const char *files[]={d->leader_art,d->badge_art,d->display_art,d->map_art,d->arena_art};
    for(unsigned j=0;j<5;j++){snprintf(gym_art_paths[i][j],sizeof(gym_art_paths[i][j]),"%s/gyms/%s.bin",root,files[j]);pp_image_preload(gym_art_paths[i][j]);}
  }
  snprintf(pure_heart_path,sizeof(pure_heart_path),"%s/collection/pure-heart.bin",root);
  pp_image_preload(pure_heart_path);
  snprintf(pure_heart_small_path,sizeof(pure_heart_small_path),"%s/collection/pure-heart-small.bin",root);
  pp_image_preload(pure_heart_small_path);
  for (unsigned id=0; id<PP_COLLECTION; id++) {
    if(id==PP_PURE_HEART)continue;
    char path[320];
    collection_path(path,sizeof(path),id,false);pp_image_preload(path);
    collection_path(path,sizeof(path),id,true);pp_image_preload(path);
  }
  snprintf(step_icon_path,sizeof(step_icon_path),"%s/hud/steps.bin",root);
  for(unsigned size=0;size<3;size++)for(unsigned frame=0;frame<8;frame++) {
    char path[320];snprintf(path,sizeof(path),"%s/effects/tumble-%u-%u.bin",root,32+size*4,frame);
    tumbleweed_frames[size][frame]=pp_image_descriptor(path);
  }
  pp_image_preload(step_icon_path);
  snprintf(pokedex_icon_path,sizeof(pokedex_icon_path),"%s/hud/pokedex.bin",root);
  pp_image_preload(pokedex_icon_path);
  snprintf(hatch_boost_icon_path,sizeof(hatch_boost_icon_path),"%s/items/hatch-boost.bin",root);
  pp_image_preload(hatch_boost_icon_path);
  snprintf(capture_ball_icon_path,sizeof(capture_ball_icon_path),"%s/items/capture-ball.bin",root);
  pp_image_preload(capture_ball_icon_path);
  snprintf(tree_essence_icon_path,sizeof(tree_essence_icon_path),"%s/items/tree-essence.bin",root);
  pp_image_preload(tree_essence_icon_path);
  snprintf(tree_essence_small_path,sizeof(tree_essence_small_path),"%s/items/tree-essence-small.bin",root);
  pp_image_preload(tree_essence_small_path);
  /* Keep resolved descriptors, rather than eleven persistent path buffers. */
  {
    char path[320];
    for(unsigned i=0;i<PP_SHOP_ITEMS;i++)if(pp_item_enabled(i)){
      snprintf(path,sizeof(path),"%s/shop/%u.bin",root,i);shop_item_sources[i]=pp_image_descriptor(path);
    }
    snprintf(path,sizeof(path),"%s/shop/lottery.bin",root);shop_lottery_source=pp_image_descriptor(path);
    snprintf(path,sizeof(path),"%s/shop/coin.bin",root);shop_coin_source=pp_image_descriptor(path);
  }
  snprintf(menu_background_path,sizeof(menu_background_path),"%s/ui/menu-bg.bin",root);
  pp_image_preload(menu_background_path);
  snprintf(gym_icon_path,sizeof(gym_icon_path),"%s/hud/gym.bin",root);
  pp_image_preload(gym_icon_path);
  snprintf(training_icon_path,sizeof(training_icon_path),"%s/hud/training.bin",root);
  pp_image_preload(training_icon_path);
  snprintf(warehouse_icon_path,sizeof(warehouse_icon_path),"%s/hud/warehouse.bin",root);
  pp_image_preload(warehouse_icon_path);
  snprintf(scene_icon_path,sizeof(scene_icon_path),"%s/hud/scene.bin",root);
  pp_image_preload(scene_icon_path);
  snprintf(lake_front_path,sizeof(lake_front_path),"%s/scene-fronts/12.bin",asset_root);
  snprintf(hud_coin_path, sizeof(hud_coin_path), "%s/hud/coin.bin", asset_root);
  for (unsigned i = 0; i < 16; i++) {
    snprintf(icon_paths[i], sizeof(icon_paths[i]), "%s/icons/%u.bin",
             asset_root, i);
    snprintf(small_paths[i], sizeof(small_paths[i]), "%s/icons-small/%u.bin",
             asset_root, i);
    snprintf(large_paths[i], sizeof(large_paths[i]), "%s/icons-large/%u.bin",
             asset_root, i);
  }
  for (unsigned n = 0; n < 3; n++)
    snprintf(nav_paths[n], sizeof(nav_paths[n]), "%s/nav/%u.bin", asset_root,
             n);
  for (unsigned p = 0; p < PP_ART_COUNT; p++)
    snprintf(avatar_paths[p], sizeof(avatar_paths[p]), "%s/avatars/%u-0.bin",
             asset_root, p);
  for (unsigned p = 0; p < PP_ART_COUNT; p++) {
    pp_image_preload(avatar_paths[p]);
    for (unsigned f = 0; f < pp_pet_frame_count(p); f++) {
      char path[320];
      snprintf(path,sizeof(path),"%s/pets/%u-%u.bin",asset_root,p,f);
      paths[p][f]=pp_image_descriptor(path);
      snprintf(path,sizeof(path),"%s/pets-left/%u-%u.bin",asset_root,p,f);
      left_paths[p][f]=pp_image_descriptor(path);
      if (!paths[p][f] || !left_paths[p][f]) {fprintf(stderr,"Missing pet descriptor art=%u frame=%u path=%s\n",p,f,path);abort();}
      /* Sources are stable descriptors; no duplicate per-frame path strings. */
    }
  }
  for (unsigned i = 0; i < 3; i++)
    snprintf(starter_paths[i], sizeof(starter_paths[i]), "%s/starters-large/%03u.bin", asset_root, 1 + i * 3);
  static const char *const types[] = {"grass", "fire", "water"};
  for (unsigned i = 0; i < 3; i++) {
    snprintf(type_paths[i],sizeof(type_paths[i]),"%s/types-large/%s.bin",asset_root,types[i]);
    pp_image_preload(type_paths[i]);
  }
  snprintf(hud_coin_large_path,sizeof(hud_coin_large_path),"%s/hud/coin-large.bin",asset_root);
  pp_image_preload(hud_coin_large_path);
  for(unsigned type=0;type<4;type++)for(unsigned phase=0;phase<4;phase++) {
    char path[320];snprintf(path,sizeof(path),"%s/effects/%u-%u.bin",root,type,phase);pp_image_preload(path);
  }
  pp_image_preload(hud_coin_path);
  for (unsigned i = 0; i < 16; i++) {
    pp_image_preload(icon_paths[i]);
    pp_image_preload(small_paths[i]);
    if (i==10 || i==12) pp_image_preload(large_paths[i]);
  }
  for (unsigned i = 0; i < 3; i++) {
    pp_image_preload(nav_paths[i]);
    pp_image_preload(starter_paths[i]);
  }
  for (unsigned i = 0; i < PP_SCENES; i++) {
    char front_path[320];snprintf(front_path,sizeof(front_path),"%s/nest-fronts/%u.bin",asset_root,i);
    nest_front_sources[i]=pp_image_descriptor(front_path);
    snprintf(scene_paths[i], sizeof(scene_paths[i]), "%s/scenes/%u.bin",
             asset_root, i);
    snprintf(thumb_paths[i], sizeof(thumb_paths[i]), "%s/thumbs/%u.bin",
             asset_root, i);
  }
}
static unsigned pet_art(unsigned p) {
  unsigned art = pp_art_id(g, p);
  return art < PP_ART_COUNT ? art : pp_base_art(g->pets[p].species);
}
static unsigned reserve_page, replacement_slot, warehouse_selected;
enum { WAREHOUSE_PAGE_SIZE = 9 };
static unsigned warehouse_sort_mode,warehouse_visible_slots[WAREHOUSE_PAGE_SIZE];
static const char *const warehouse_sort_labels[]={"原序","等级","属性"};
static bool info_warehouse, info_from_warehouse, warehouse_swap;
/* A hatch is committed by the core first; this receipt only decides where the
 * newborn should live after its reveal, so it remains safe across UI redraws. */
static struct {
  bool active, in_warehouse;
  unsigned slot;
  pp_stored_pet pet;
} hatch_result;
static struct {
  lv_obj_t *pet;
  const void *source;
  pp_animation_clock clock;
} hatch_reveal;
static void hatch_result_view(void);
static void hatch_result_tick(void);
static bool hatch_replacing, egg_present_seen;
static unsigned roster_count_seen, warehouse_count_seen;
static unsigned warehouse_view_count(void){return g->count+pp_warehouse_count(g);}
static bool warehouse_view_get(unsigned n,pp_stored_pet *pet){
 if(n>=g->count)return pp_warehouse_get(g,n-g->count,pet);
 *pet=(pp_stored_pet){.pet=g->pets[n],.growth=g->growth[n],.identity=g->identity[n]};return true;
}
static void hatch_result_capture(bool in_warehouse,unsigned slot) {
  pp_stored_pet pet={0};
  bool valid=in_warehouse?pp_warehouse_get(g,slot,&pet):warehouse_view_get(slot,&pet);
  if(!valid)return;
  hatch_result.active=true;hatch_result.in_warehouse=in_warehouse;
  hatch_result.slot=slot;hatch_result.pet=pet;
  hatch_replacing=false;
  roster_count_seen=g->count;warehouse_count_seen=pp_warehouse_count(g);
  egg_present_seen=g->egg_end!=0;
}
static char name_draft[PP_NAME_BYTES];
static lv_obj_t *name_value;
static void name_update(void){text(name_value,name_draft[0]?name_draft:"—");}
static bool name_append(char c){
 size_t n=strlen(name_draft);if(n>=8)return false;
 name_draft[n]=c;name_draft[n+1]=0;return true;
}
static void name_backspace(void){
 size_t n=strlen(name_draft);if(!n)return;
 do{n--;}while(n&&(((unsigned char)name_draft[n]&0xc0)==0x80));name_draft[n]=0;
}

static void pet_event(lv_event_t *e) {
  unsigned p = (unsigned)(uintptr_t)lv_event_get_user_data(e);
  selected = p;
  if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED) {
    lv_indev_t *indev = lv_event_get_indev(e);
    if (indev) lv_indev_wait_release(indev);
    pp_ui_open(4);
  } else if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
    pp_result r = pp_pat(g, p);
    if (r == PP_OK)
      happy_until[p] = home_time + 2000;
    if(r==PP_OK)home_react(p);
    if(r!=PP_DAILY_LIMIT)notice(r == PP_OK ? "♥ +2     +1" : pp_message(r));
  }
}
void pp_ui_pat_all(void) {
  if(current!=0) return;
  unsigned n=0;
  for(unsigned i=0;i<g->count;i++)if(pp_pat(g,i)==PP_OK){happy_until[i]=home_time+2000;home_react(i);n++;}
  if(!n)return;
  char message[64];snprintf(message,sizeof(message),"摸摸 %u 位伙伴 ♥",n);notice(message);
  pp_ui_refresh();
}
static void home_reset_pet(unsigned i) {
  pp_home_behavior_clear(&home_behavior);
  memset(&home_motion[i],0,sizeof(home_motion[i]));
  memset(&home_clocks[i],0,sizeof(home_clocks[i]));
  happy_until[i]=0;facing_left[i]=false;
  evo_postponed_level[i]=0;
  memset(talk_cursors[i],0,sizeof(talk_cursors[i]));ambient_talk_seq[i]=0;
  if(talk_pet==i)talk_until=0;
}
static void home_remove_pet(unsigned slot) {
  for(unsigned i=slot;i<g->count;i++) {
    home_motion[i]=home_motion[i+1];home_clocks[i]=home_clocks[i+1];
    happy_until[i]=happy_until[i+1];facing_left[i]=facing_left[i+1];
    evo_postponed_level[i]=evo_postponed_level[i+1];evo_postponed_art[i]=evo_postponed_art[i+1];
    memcpy(talk_cursors[i],talk_cursors[i+1],sizeof(talk_cursors[i]));ambient_talk_seq[i]=ambient_talk_seq[i+1];
  }
  home_reset_pet(g->count);
  talk_until=0;
}
/* A tiny outlined bolt stays inside the battery, above its animated fill. */
static void home_battery_bolt_draw(lv_event_t *e) {
  if(!battery_reading.valid||!battery_reading.charging||battery_reading.percent>=100)return;
  static const char rows[7][6]={"00010","00110","01100","11111","00110","01100","01000"};
  lv_area_t area;lv_obj_get_coords(lv_event_get_target_obj(e),&area);
  lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_opa=LV_OPA_COVER;
  for(unsigned pass=0;pass<2;pass++) {
    d.bg_color=lv_color_hex(pass?0xfff0ab:0x0a2530);
    for(int y=0;y<7;y++) {
      int first=0,last=4;while(rows[y][first]!='1')first++;while(rows[y][last]!='1')last--;
      int edge=pass?0:1;
      lv_area_t pixels={area.x1+9+first-edge,area.y1+5+y,area.x1+9+last+edge,area.y1+5+y};
      lv_draw_rect(lv_event_get_layer(e),&d,&pixels);
    }
  }
}
/* Only this existing 10x5 fill changes during charging; the percentage stays
 * the gauge reading. Six pixel steps repeat every 1.8s, with no new timer. */
static void home_battery_fill_update(void) {
  if(!battery_group)return;
  unsigned width=0;
  if(battery_reading.valid){
    if(battery_reading.charging && battery_reading.percent<100)width=(battery_charge_elapsed/300)*2;
    else if(battery_reading.percent>0)width=1+battery_reading.percent*9/100;
  }
  if(battery_shown_width==(int)width)return;
  battery_shown_width=(int)width;
  if(width){lv_obj_set_width(battery_fill,width);lv_obj_remove_flag(battery_fill,LV_OBJ_FLAG_HIDDEN);}
  else lv_obj_add_flag(battery_fill,LV_OBJ_FLAG_HIDDEN);
}
static void home_battery_animate(void) {
  uint32_t now=lv_tick_get(),delta=now-battery_charge_tick;battery_charge_tick=now;
  if(!ui_screen_on || !(display_options&PP_UI_SHOW_BATTERY) || !battery_reading.valid ||
     !battery_reading.charging || battery_reading.percent>=100)return;
  battery_charge_elapsed=(battery_charge_elapsed+delta%1800)%1800;
  home_battery_fill_update();
}
static void home_battery_update(void) {
  if(!battery_group)return;
  char value[12];
  if(battery_reading.valid)snprintf(value,sizeof(value),"%d%%",battery_reading.percent);
  else snprintf(value,sizeof(value),"--");
  text(battery_value,value);
  unsigned tint=!battery_reading.valid?0x89979e:battery_reading.charging && battery_reading.percent<100?0xe8d492:battery_reading.percent<=20?0xf29a8e:0xb2dfbe;
  lv_obj_set_style_text_color(battery_value,color(tint),0);
  lv_obj_set_style_border_color(battery_outline,color(tint),0);
  lv_obj_set_style_bg_color(battery_cap,color(tint),0);
  lv_obj_set_style_bg_color(battery_fill,color(tint),0);
  home_battery_fill_update();
  lv_point_t size;lv_text_get_size(&size,value,&pp_font_12,0,0,LV_COORD_MAX,LV_TEXT_FLAG_NONE);
  lv_obj_set_width(battery_value,size.x);lv_obj_set_width(battery_group,30+size.x);
  lv_obj_set_x(battery_group,(360-30-size.x)/2);
  if(display_options&PP_UI_SHOW_BATTERY)lv_obj_remove_flag(battery_group,LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(battery_group,LV_OBJ_FLAG_HIDDEN);
  lv_obj_invalidate(battery_group);
}
void pp_ui_battery(pp_battery_reading reading) {
  if(!reading.valid || reading.percent<0 || reading.percent>100)reading=(pp_battery_reading){.percent=-1};
  if(reading.valid==battery_reading.valid && reading.percent==battery_reading.percent && reading.charging==battery_reading.charging)return;
  if(reading.charging!=battery_reading.charging || reading.valid!=battery_reading.valid ||
     (reading.percent>=100)!=(battery_reading.percent>=100))battery_charge_elapsed=0;
  battery_charge_tick=lv_tick_get();battery_reading=reading;
  if(current==0)home_battery_update();
}
static void home_conflict_begin(void) {
  const unsigned pets[]={home_conflict.attacker,home_conflict.target};
  for(unsigned i=0;i<2;i++){
    unsigned p=pets[i];home_conflict_origin[i][0]=home_motion[p].x;home_conflict_origin[i][1]=home_motion[p].y;
    pp_home_behavior_cancel(&home_behavior,p);happy_until[p]=0;memset(&home_clocks[p],0,sizeof(home_clocks[p]));
  }
  home_speech(home_conflict.attacker,pp_move_at(g->pets[home_conflict.attacker].species,home_conflict.slot)->name,-1,PP_HOME_CONFLICT_IMPACT_MS);
}
static void home_conflict_release(uint32_t now) {
  unsigned participants[]={home_conflict.attacker,home_conflict.target};
  for(unsigned i=0;i<2;i++){
    home_motion_state *m=&home_motion[participants[i]];
    m->origin_x=m->x;m->origin_y=m->y;m->started=now;
    if(g->scene==PP_MIRROR_LAKE_SCENE)m->started-=pp_lake_nearest_phase(m->x,m->y);
  }
}
static void home_conflict_update(uint32_t now,pp_home_actor actors[PP_PETS]) {
  unsigned mask=0;
  for(unsigned i=0;i<g->count;i++)if(actors[i].available && !home_behavior.pets[i].kind)mask|=1u<<i;
  pp_conflict_event event=pp_home_conflict_tick(&home_conflict,g,now,
    ui_screen_on && !evolution && !starter_mode && !gift_overlay && !wild_overlay,mask);
  if(event==PP_CONFLICT_BEGIN)home_conflict_begin();
  if(event==PP_CONFLICT_HIT){
    char message[64];snprintf(message,sizeof(message),"健康-%u 心情-%u",home_conflict.health_loss,home_conflict.mood_loss);
    home_speech(home_conflict.target,message,-1,1400);
  }
  if(event==PP_CONFLICT_END || event==PP_CONFLICT_CANCEL)home_conflict_release(now);
  if(home_conflict.active){actors[home_conflict.attacker].available=false;actors[home_conflict.target].available=false;}
}
static void update_home(void) {
  uint32_t real=lv_tick_get(), delta=real-home_real_tick;
  /* Item entrances pause with the screen even while background care continues. */
  if(!ui_screen_on){if(drop_was_active)drop_started+=delta;if(egg_drop_playing)egg_drop_started+=delta;}
  home_time+=delta;home_real_tick=real;
  uint32_t now=home_time;
  home_battery_animate();
  for(unsigned i=0;i<g->count;i++) if(!home_motion[i].initialized) {
    int x=px[i],y=py[i];pp_home_ground_project(g->scene,&x,&y);
    home_motion[i]=(home_motion_state){.x=x,.y=y,.origin_x=x,.origin_y=y,
      .started=now-i*2900,.initialized=true,.sleeping=g->pets[i].sleeping,.tired=g->pets[i].energy<30};
    if(!g->pets[i].sleeping)pp_home_scene_patrol(g->scene,i*2900,i,g->pets[i].energy<30,x,y,
      facing_left[i],&home_motion[i].x,&home_motion[i].y,&facing_left[i]);
  }
  char s[64];
  snprintf(s, sizeof(s), "%lu", (unsigned long)g->coins);
  text(coin, s);
  text(coin_shadow, s);
  lv_point_t size;
  lv_text_get_size(&size,s,&pp_font_18,0,0,LV_COORD_MAX,LV_TEXT_FLAG_NONE);
  int money_x=(360-24-4-size.x)/2;
  lv_obj_set_x(coin_icon,money_x);
  lv_obj_set_x(coin,money_x+28);
  lv_obj_set_x(coin_shadow,money_x+29);
  if (last_scene != g->scene) {
    pp_scene_view_set(body, scene_paths[g->scene], g->scene, home_time-home_epoch);
    last_scene = g->scene;
  }
  lv_obj_t *clock_objects[]={clock_label,clock_shadow};
  lv_obj_t *coin_objects[]={coin,coin_shadow,coin_icon};
  for(unsigned i=0;i<2;i++) {
    if(display_options&PP_UI_SHOW_TIME)lv_obj_remove_flag(clock_objects[i],LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(clock_objects[i],LV_OBJ_FLAG_HIDDEN);
  }
  for(unsigned i=0;i<3;i++) {
    if(display_options&PP_UI_SHOW_COINS)lv_obj_remove_flag(coin_objects[i],LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(coin_objects[i],LV_OBJ_FLAG_HIDDEN);
  }
  if(display_options&PP_UI_SHOW_BATTERY)lv_obj_remove_flag(battery_group,LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(battery_group,LV_OBJ_FLAG_HIDDEN);
  pp_scene_view_tick(body,g->scene,now-home_epoch);
  int drop_x, drop_y;
  pp_drop_position(g, &drop_x, &drop_y);
  int target_x=drop_x-8,target_y=drop_y-16;
  pp_home_ground_project(g->scene,&target_x,&target_y);drop_x=target_x+8;drop_y=target_y+16;
  if (g->drop_active && !drop_was_active) {
    talk_until=0;
    drop_started = now;
    unsigned p = g->drop_target;
    drop_from_x = home_motion[p].x;
    drop_from_y = home_motion[p].y;
  }
  if (!g->drop_active && drop_was_active) {
    home_motion_state *m=&home_motion[g->drop_target];
    /* Resume from the last visible item-use position, even if game time
     * finished the item in a menu. Never jump back onto the old route. */
    m->origin_x=m->x;m->origin_y=m->y;m->started=now;
    if(g->scene==PP_MIRROR_LAKE_SCENE)m->started-=pp_lake_nearest_phase(m->x,m->y);
    happy_until[g->drop_target] = now + 1400;
    home_say(g->drop_target,pp_talk_item(g->drop_item),3600);
    talk_care=true;
  }
  drop_was_active = g->drop_active;
  if(g->egg_end!=egg_drop_deadline){
    egg_drop_deadline=g->egg_end;egg_drop_started=now;egg_drop_playing=g->egg_end!=0;
  }
  if(egg_drop_playing && now-egg_drop_started>=PP_ITEM_DROP_MS)egg_drop_playing=false;

  pp_home_actor actors[PP_PETS];home_inputs(actors);
  if(home_behavior.scene!=g->scene)pp_home_behavior_init(&home_behavior,home_behavior.rng,now,g->scene);
  home_conflict_update(now,actors);
  if(home_welcome_pending && !g->wild.phase && !g->gift.pending && !g->drop_active) {
   unsigned nearest=PP_PETS;int best=1000000;
   for(unsigned i=0;i<g->count;i++)if(actors[i].available&&actors[i].energy>=30){int d=abs(actors[i].x-156)+abs(actors[i].y-198);if(d<best){best=d;nearest=i;}}
   if(nearest<PP_PETS){if(pp_home_behavior_request(&home_behavior,now,PP_HOME_WELCOME,nearest,0,156,198,actors,g->count))home_say(nearest,PP_TALK_WELCOME,3200);home_welcome_pending=false;home_welcomed_at=real;home_has_welcomed=true;}
  }
  pp_home_behavior_tick(&home_behavior,now,actors,g->count,!g->wild.phase&&!g->gift.pending&&!g->drop_active&&!g->battling,home_outputs);
  for (unsigned i = 0; i < PP_PETS; i++) {
    if (i >= g->count || g->traveler == (int)i || pp_training_busy(g,i)) {
      lv_obj_add_flag(pet_img[i], LV_OBJ_FLAG_HIDDEN);
      if(home_symbols[i])lv_obj_add_flag(home_symbols[i],LV_OBJ_FLAG_HIDDEN);
      continue;
    }
    lv_obj_remove_flag(pet_img[i], LV_OBJ_FLAG_HIDDEN);
    home_motion_state *m=&home_motion[i];
    if(m->sleeping!=(bool)g->pets[i].sleeping || m->tired!=(g->pets[i].energy<30)) {
      m->sleeping=g->pets[i].sleeping;m->tired=g->pets[i].energy<30;
      m->origin_x=m->x;m->origin_y=m->y;m->started=now;
    if(g->scene==PP_MIRROR_LAKE_SCENE)m->started-=pp_lake_nearest_phase(m->x,m->y);
    }
    if(lv_obj_has_state(pet_img[i],LV_STATE_PRESSED))m->started+=delta;
    int x=m->x,y=m->y;bool direction=facing_left[i];
    unsigned action=PP_POSE_IDLE;
    if(g->pets[i].sleeping)action=PP_POSE_SLEEP;
    else if(!lv_obj_has_state(pet_img[i],LV_STATE_PRESSED)) {
      action=pp_home_scene_patrol(g->scene,now-m->started,i,m->tired,m->origin_x,m->origin_y,
                            facing_left[i],&x,&y,&direction);
      facing_left[i]=direction;
    }
    if (!g->pets[i].sleeping && (int32_t)(happy_until[i] - now) > 0)
      action=PP_POSE_HAPPY;
    if(g->scene==PP_MIRROR_LAKE_SCENE) {
      bool direction=false;
      pp_lake_patrol(now-m->started,&x,&y,&direction);
      facing_left[i]=direction;
      if(g->pets[i].sleeping || lv_obj_has_state(pet_img[i],LV_STATE_PRESSED)) {
        x=m->x;y=m->y;

      } else if((int32_t)(happy_until[i]-now)<=0) action=PP_POSE_WALK;
    }
    if (x < (g->scene==PP_MIRROR_LAKE_SCENE?40:48)) x = g->scene==PP_MIRROR_LAKE_SCENE?40:48;
    if (x > 272) x = 272;
    if (y < 115) y = 115;
    if (y > (g->scene==PP_MIRROR_LAKE_SCENE?PP_LAKE_FRONT_TOP:208)) y = g->scene==PP_MIRROR_LAKE_SCENE?PP_LAKE_FRONT_TOP:208;
    if (g->drop_active && g->drop_target == i) {
      unsigned elapsed = now - drop_started;
      if (elapsed > 2400)
        elapsed = 2400;
      pp_home_route_sample(g->scene,drop_from_x,drop_from_y,drop_x-8,drop_y-16,elapsed,2400,&x,&y,&facing_left[i]);
      action = elapsed < 2400 ? PP_POSE_WALK : PP_POSE_EAT;
    }
    pp_home_output *interaction=&home_outputs[i];
    if(interaction->active && (!m->interacting || m->behavior_start!=home_behavior.pets[i].start)) {
      memset(&home_clocks[i],0,sizeof(home_clocks[i]));m->behavior_start=home_behavior.pets[i].start;
    }
    if(m->interacting&&!interaction->active&&!interaction->release) {
      interaction->release=true;interaction->x=m->x;interaction->y=m->y;interaction->left=facing_left[i];
    }
    m->interacting=interaction->active;
    if(interaction->active || interaction->release) {
      x=interaction->x;y=interaction->y;facing_left[i]=interaction->left;
      if(interaction->active)action=interaction->pose;
      m->origin_x=x;m->origin_y=y;m->started=now;
      if(g->scene==PP_MIRROR_LAKE_SCENE && !interaction->active)m->started-=pp_lake_nearest_phase(x,y);
    } else if(!g->pets[i].sleeping && (int32_t)(happy_until[i]-now)>0 && !(g->drop_active&&g->drop_target==i)) {
      x=m->x;y=m->y;m->origin_x=x;m->origin_y=y;m->started=now;
      if(g->scene==PP_MIRROR_LAKE_SCENE)m->started-=pp_lake_nearest_phase(x,y);
    }
    if(g->scene==PP_MIRROR_LAKE_SCENE && !interaction->active &&
       !(g->drop_active&&g->drop_target==i) && !g->pets[i].sleeping) {
      int dx=x-m->x,dy=y-m->y,span=abs(dx)>abs(dy)?abs(dx):abs(dy);
      int limit=(int)((delta*40+999)/1000);if(limit<1)limit=1;
      if(span>limit){x=m->x+dx*limit/span;y=m->y+dy*limit/span;}
    }
    bool conflict_pet=home_conflict.active && (i==home_conflict.attacker || i==home_conflict.target);
    unsigned conflict_age=now-home_conflict.started;
    if(conflict_pet){
      unsigned side=i==home_conflict.target;
      x=home_conflict_origin[side][0];y=home_conflict_origin[side][1];
      facing_left[i]=home_conflict_origin[side][0]>home_conflict_origin[1-side][0];
      action=side?PP_POSE_IDLE:PP_POSE_ATTACK;
      if(side && home_conflict.hit && conflict_age<1050)x+=(conflict_age/65%2?2:-2);
    }
    pp_home_ground_advance(g->scene,m->x,m->y,&x,&y,(delta*40+999)/1000+1,&facing_left[i]);
    bool hurt=conflict_pet && i==home_conflict.target && home_conflict.hit && conflict_age<1100;
    lv_obj_set_style_image_recolor(pet_img[i],color(0xff8b86),0);
    lv_obj_set_style_image_recolor_opa(pet_img[i],hurt&&conflict_age/80%2?100:0,0);
    if(home_symbols[i]) {
      unsigned symbol=interaction->active?interaction->symbol:0;
      if(conflict_pet && i==home_conflict.attacker && conflict_age<PP_HOME_CONFLICT_IMPACT_MS)symbol=4;
      if(home_symbol_kind[i]!=symbol){home_symbol_kind[i]=symbol;lv_obj_invalidate(home_symbols[i]);}
      if(symbol){lv_obj_set_pos(home_symbols[i],x+17,y-16-interaction->jump);lv_obj_remove_flag(home_symbols[i],LV_OBJ_FLAG_HIDDEN);}
      else lv_obj_add_flag(home_symbols[i],LV_OBJ_FLAG_HIDDEN);
    }
    unsigned layer=home_tree_front&&!pp_lake_in_front(y)?0:y>=pp_scene_nest_positions[g->scene][1]?2:1;
    if(pet_ground_layer[i]!=layer+1){
      lv_obj_t *anchor=layer==0?home_tree_front:layer==1?egg_glow:egg_rim;
      int from=lv_obj_get_index(pet_img[i]),at=lv_obj_get_index(anchor);
      /* Adjust for removing the old sibling before inserting at its depth. */
      lv_obj_move_to_index(pet_img[i],at-(from<at)+(layer==2));
      pet_ground_layer[i]=layer+1;
    }
    unsigned pose = pp_animation_update(&home_clocks[i], pet_art(i), action, now);
    const void *src = facing_left[i] ? left_paths[pet_art(i)][pose] : paths[pet_art(i)][pose];
    if (pet_src[i] != src) {
      pp_image_set_src(pet_img[i], src);
      pet_src[i] = src;
    }
    if (!lv_obj_has_state(pet_img[i], LV_STATE_PRESSED)) {
      m->x=x;m->y=y;
      if(lv_obj_get_x(pet_img[i])!=x || lv_obj_get_y(pet_img[i])!=y-interaction->jump)
        lv_obj_set_pos(pet_img[i],x,y-interaction->jump);
    }
  }
  ambient_update(now-home_epoch);
  if (g->egg_end) {
    int ex, ey; pp_egg_position(g, &ex, &ey);
    /* A 1.2s pixel wobble; final shake progress makes it a little stronger,
     * but presentation never changes the saved hatch timing or state. */
    static const int8_t wobble[]={0,1,2,3,2,1,0,-1,-2,-3,-2,-1};
    unsigned wobble_at=(now/100)%12;
    int offset=wobble[wobble_at]*(g->egg_shakes?1:2)/2;
    pp_item_drop_state falling_egg=pp_item_drop(egg_drop_playing?now-egg_drop_started:PP_ITEM_DROP_MS,ex,ey,32);
    ey=falling_egg.y;
    lv_obj_set_style_opa(egg_icon,falling_egg.opacity,0);
    lv_obj_set_style_opa(egg_glow,falling_egg.opacity,0);
    egg_glow_elapsed=now;
    lv_obj_set_pos(egg_glow,ex-PP_EGG_GLOW_OFFSET_X+offset,ey-PP_EGG_GLOW_OFFSET_Y);
    lv_obj_set_pos(egg_icon, ex+offset, ey);
    lv_obj_set_style_image_recolor(egg_icon,color(0xbc98ef),0);lv_obj_set_style_image_recolor_opa(egg_icon,g->legend.hatching?100:0,0);
    lv_obj_invalidate(egg_glow);
    lv_obj_remove_flag(egg_glow, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(egg_icon, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(egg_glow, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(egg_icon, LV_OBJ_FLAG_HIDDEN);
  }
  if(nest_front_sources[g->scene]&&lv_image_get_src(egg_rim)!=nest_front_sources[g->scene])lv_image_set_src(egg_rim,nest_front_sources[g->scene]);
  lv_obj_set_pos(egg_rim,pp_scene_nest_positions[g->scene][0],pp_scene_nest_positions[g->scene][1]+PP_SCENE_NEST_FRONT_TOP);
  lv_obj_remove_flag(egg_rim,LV_OBJ_FLAG_HIDDEN);
  if (g->drop_active) {
    lv_obj_remove_flag(drop, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(drop_icon, LV_OBJ_FLAG_HIDDEN);
    const char *drop_path=g->drop_item==PP_TREE_ESSENCE?tree_essence_small_path:small_paths[g->drop_item];
    if (lv_image_get_src(drop_icon) != drop_path)
      pp_image_set_src(drop_icon, drop_path);
    pp_item_drop_state falling=pp_item_drop(now-drop_started,drop_x,drop_y,24);
    lv_obj_set_pos(drop_icon, falling.x, falling.y);
    lv_obj_set_style_opa(drop_icon,falling.opacity,0);
    lv_obj_set_pos(drop,drop_x+7,drop_y+28);
    lv_obj_set_style_bg_opa(drop,falling.y>=drop_y-8?50:25,0);
  } else {
    lv_obj_add_flag(drop, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(drop_icon, LV_OBJ_FLAG_HIDDEN);
  }
  if (!home_conflict.active && !g->drop_active && (int32_t)(now-talk_until)>=0 && (int32_t)(now - next_talk) >= 0) {
    next_talk = now + 32000 + (g->rng % 11000);
    talk_pet = (talk_pet + 1) % g->count;
    if (g->traveler != (int)talk_pet && !pp_training_busy(g,talk_pet) && !g->pets[talk_pet].sleeping) {
      const pp_pet *p=&g->pets[talk_pet];
      home_say(talk_pet,pp_talk_context(g->scene,p->food,p->clean,p->health,p->mood,p->energy,ambient_talk_seq[talk_pet]++),3200);
    }
  }
  bool speaking = (int32_t)(talk_until - now) > 0 &&
                  talk_pet < g->count && g->traveler != (int)talk_pet && !pp_training_busy(g,talk_pet) &&
                  (!g->pets[talk_pet].sleeping || talk_care);
  if (!speaking)
    lv_obj_add_flag(bubble_box, LV_OBJ_FLAG_HIDDEN);
  else {
    int x = lv_obj_get_x(pet_img[talk_pet]) + PP_SPRITE_SIZE / 2 - talk_width/2;
    int y = lv_obj_get_y(pet_img[talk_pet]) - 48;
    if (x < 36) x = 36;
    if (x > 324-talk_width) x = 324-talk_width;
    if (y < 99) y = 99;
    bool icon=talk_icon>=0;
    if(icon){const char *path=talk_icon==8?tree_essence_small_path:small_paths[talk_icon];if(lv_image_get_src(bubble_icon)!=path)pp_image_set_src(bubble_icon,path);}
    if(icon){lv_obj_remove_flag(bubble_icon,LV_OBJ_FLAG_HIDDEN);lv_obj_remove_flag(bubble_badge,LV_OBJ_FLAG_HIDDEN);}
    else {lv_obj_add_flag(bubble_icon,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(bubble_badge,LV_OBJ_FLAG_HIDDEN);}
    lv_obj_set_pos(bubble_box, x, y);
    lv_obj_remove_flag(bubble_box, LV_OBJ_FLAG_HIDDEN);
  }
}
static void live(void) {
  char s[160];
  if (current == 5) {
    if(travel_receipt.active || travel_notice_active){travel_result_tick();return;}
    bool active = g->traveler >= 0 || travel_receipt.active;
    if (travel_layout != active) {
      refresh_pending = true;
      return;
    }
    if (!active) {
      text(live_status, "三小时的小小冒险");
      return;
    }
    pp_travel_status trip = pp_travel_describe(g);
    const char *name = travel_receipt.active ? travel_receipt.name : pp_form_name(g, g->traveler);
    if (travel_receipt.active || trip.ready)
      snprintf(s, sizeof(s), "%s · 已返程", name);
    else snprintf(s, sizeof(s), "%s · %02u:%02u:%02u", name,
      trip.remaining / 3600, trip.remaining / 60 % 60, trip.remaining % 60);
    text(live_status, s);
    if (travel_receipt.active) {
      text(travel_activity, "带回了旅途中的收获");
      text(live_log, travel_receipt.item);
      snprintf(s, sizeof(s), "金币 +%u · 经验 +%u", travel_receipt.coins,travel_receipt.xp);
    } else {
      if(trip.ready)snprintf(s,sizeof(s),"%s",travel_wait_message());
      else snprintf(s, sizeof(s), "%s%s", name, trip.activity);
      text(travel_activity, s);
      if (trip.event) snprintf(s, sizeof(s), "奇遇 · %s · +%u金币", trip.event,PP_TRAVEL_EVENT_COINS);
      else snprintf(s, sizeof(s), "新的奇遇还在路上");
      text(live_log, s);
      snprintf(s, sizeof(s), "返程可领 %u金币 · %u经验", trip.pending_coins,pp_travel_xp(g));
    }
    text(travel_reward_label, s);
    bool claimable = !travel_receipt.active && trip.ready && !travel_settlement_blocked();
    if (claimable) lv_obj_remove_flag(travel_claim, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(travel_claim, LV_OBJ_FLAG_HIDDEN);
    /* The return target stays centered even while a claim is available. */
  }
  if (current == 6) {
    bool layout = g->battling || g->battle_hp > 0 || g->enemy_hp > 0;
    if (battle_layout != layout) {
      refresh_pending = true;
      return;
    }
    if (battle_layout && ui_screen_on)
      pp_battle_view_tick();
  }

  if (current == 7) {
    snprintf(s, sizeof(s), "%u/6    ↓%u    +%lu", g->digs_today, g->dig_depth,
             (unsigned long)g->dig_reward);
    text(live_status, s);
    const char *hearts[] = {"♡ ♡ ♡", "♥ ♡ ♡", "♥ ♥ ♡", "♥ ♥ ♥"};
    text(lives, hearts[g->dig_lives]);
    for (unsigned i = 0; i < 6; i++)
      lv_obj_set_style_bg_color(
          stairs[i], color(i <= g->dig_depth % 6 ? 0xa78958 : 0x203e4c), 0);
  }
}
static lv_obj_t *wild_shape(lv_obj_t *parent,int x,int y,int w,int h,unsigned c,int radius) {
  lv_obj_t *o=lv_obj_create(parent);lv_obj_remove_style_all(o);
  lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
  lv_obj_set_style_bg_color(o,color(c),0);lv_obj_set_style_bg_opa(o,255,0);
  lv_obj_set_style_radius(o,radius,0);
  lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);return o;
}
/* Evolution and wild receipts are mutually exclusive. Reuse their bounded
 * 96px pixel canvas; native indexed Flash descriptors remain unchanged. */
static void large_portrait(lv_obj_t *target,const void *source,const void **shown) {
  if(source==*shown)return;
  *shown=source;
  const lv_image_dsc_t *im=pp_image_descriptor(source);
  if(!im)return;
  for(unsigned y=0;y<96;y++)for(unsigned x=0;x<96;x++)
    evo_pixels[y*96+x]=pp_image_indexed_pixel(im,x/2,y/2);
  evo_descriptor=(lv_image_dsc_t){.header={.magic=LV_IMAGE_HEADER_MAGIC,
    .cf=LV_COLOR_FORMAT_ARGB8888,.flags=LV_IMAGE_FLAGS_MODIFIABLE,
    .w=96,.h=96,.stride=96*4},.data_size=sizeof(evo_pixels),.data=(const uint8_t *)evo_pixels};
  lv_image_cache_drop(&evo_descriptor);lv_image_set_src(target,&evo_descriptor);
  lv_obj_invalidate(target);
}
static void wild_portrait(const void *source) {large_portrait(wild_result.pet,source,&wild_result.source);}
static void hatch_result_tick(void) {
  if(!hatch_reveal.pet || !hatch_result.active)return;
  unsigned art=pp_stored_art(&hatch_result.pet);
  unsigned frame=pp_animation_update(&hatch_reveal.clock,art,PP_POSE_HAPPY,lv_tick_get());
  large_portrait(hatch_reveal.pet,paths[art][frame],&hatch_reveal.source);
}
static void hatch_result_view(void) {
  notice_create();
  const pp_stored_pet *p=&hatch_result.pet;
  unsigned accent=evo_colors[p->pet.species%(sizeof(evo_colors)/sizeof(evo_colors[0]))];
  lv_obj_add_flag(body,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(subtitle,LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(back_button,LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_color(screen,color(0x0c1c30),0);lv_obj_set_style_bg_grad_dir(screen,LV_GRAD_DIR_NONE,0);
  text(title,"孵化成功");lv_obj_set_y(title,43);lv_obj_set_style_text_color(title,color(0xf1dca7),0);
  for(unsigned i=0;i<2;i++) {
    lv_obj_t *ring=wild_shape(screen,116-(int)i*11,73-(int)i*11,128+i*22,128+i*22,accent,LV_RADIUS_CIRCLE);
    lv_obj_set_style_bg_opa(ring,i?0:10,0);lv_obj_set_style_border_width(ring,1,0);
    lv_obj_set_style_border_color(ring,color(accent),0);lv_obj_set_style_border_opa(ring,i?28:55,0);
  }
  static const int spark[][2]={{108,104},{248,104},{102,158},{254,158}};
  for(unsigned i=0;i<4;i++) {
    wild_shape(screen,spark[i][0]-2,spark[i][1]-5,4,10,accent,0);
    wild_shape(screen,spark[i][0]-5,spark[i][1]-2,10,4,accent,0);
  }
  hatch_reveal.pet=lv_image_create(screen);lv_obj_set_pos(hatch_reveal.pet,132,89);hatch_result_tick();
  lv_obj_t *l=label(screen,pp_stored_name(p),68,190,224,0xf4ead4);
  lv_obj_set_style_text_font(l,&pp_font_18,0);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_set_height(l,23);
  char s[48];snprintf(s,sizeof(s),"Lv.%u",p->growth.level);
  lv_obj_t *pill=wild_shape(screen,90,217,68,20,0x203c4c,10);
  l=label(pill,s,0,3,68,0xd3e6db);lv_obj_set_style_text_font(l,&pp_font_12,0);
  pill=wild_shape(screen,166,217,104,20,0x203c4c,10);
  l=label(pill,pp_nature_names[p->identity.nature],0,3,104,0xd3e6db);lv_obj_set_style_text_font(l,&pp_font_12,0);
  lv_obj_t *stats=wild_shape(screen,82,242,196,34,0x142d3e,9);
  static const char *iv_names[]={"HP","攻","防","特攻","特防","速"};
  for(unsigned i=0;i<6;i++) {
    snprintf(s,sizeof(s),"%s %u",iv_names[i],p->identity.iv[i]);
    l=label(stats,s,2+(int)(i%3)*64,1+(int)(i/3)*16,64,0xa4cbbd);lv_obj_set_style_text_font(l,&pp_font_12,0);
  }
  lv_obj_t *store=button(screen,"放入仓库",100,296,72,32,780);
  lv_obj_t *replace=button(screen,"替换伙伴",188,296,72,32,781);
  pp_footer_text_style(store);pp_footer_text_style(replace);
}
/* Travel is a separate page from the reveal views. Reuse their bounded canvas
 * to scale the full48px logical sprite, including omitted transparent margins.
 * Transforming the cropped indexed descriptor can read outside its rows. */
static void travel_paint(void *data,int x,int y,int w,int h,unsigned tint,unsigned opacity){
  lv_area_t origin;lv_obj_get_coords(travel_motion.effects,&origin);
  lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=color(tint);d.bg_opa=opacity;
  lv_area_t area={origin.x1+x,origin.y1+y,origin.x1+x+w-1,origin.y1+y+h-1};
  lv_draw_rect(data,&d,&area);travel_motion.particles++;
}
static void travel_scenery_draw(lv_event_t *event){
  travel_motion.particles=0;
  pp_travel_scenery(travel_motion.destination,travel_motion.elapsed,travel_paint,lv_event_get_layer(event));
}
static void travel_portrait(lv_obj_t *target,const void *source) {
  const lv_image_dsc_t *im=pp_image_descriptor(source);
  if(!im)return;
  for(unsigned y=0;y<72;y++)for(unsigned x=0;x<72;x++)
    evo_pixels[y*72+x]=pp_image_indexed_pixel(im,x*2/3,y*2/3);
  evo_descriptor=(lv_image_dsc_t){.header={.magic=LV_IMAGE_HEADER_MAGIC,
    .cf=LV_COLOR_FORMAT_ARGB8888,.flags=LV_IMAGE_FLAGS_MODIFIABLE,
    .w=72,.h=72,.stride=72*4},.data_size=72*72*4,.data=(const uint8_t *)evo_pixels};
  lv_image_cache_drop(&evo_descriptor);lv_image_set_src(target,&evo_descriptor);
  lv_obj_invalidate(target);
}
/* Completion is inferred from the saved traveler/deadline. Only the explicit
 * confirmation calls claim; presenting a notice never mutates the save. */
static bool travel_settlement_blocked(void) {
  /* Saved encounters/receipts are independent rewards, not active operations.
   * Keep them intact and defer only for combat, care or a visible home modal. */
  return evolution || starter_mode || g->battling || g->wild.phase==PP_WILD_BATTLE ||
    g->drop_active || (current==0 && (wild_overlay || gift_overlay));
}
static bool travel_page_safe(unsigned page) {
  /* A finished training page is a receipt, not an active operation. Drain its
   * saved growth choices first, then allow the next independent completion. */
  if(page==33)return g->training.status!=1 && !pp_training_growth_pending(g);
  /* Placing a newborn may require the traveler to return first (reserving
   * a roster slot is locked during travel). Keep its reveal queued while
   * travel is claimed, then resume it after the receipt. */
  if(page==30 || (page==29 && hatch_replacing))return hatch_result.active;
  return page<=5 || (page>=8 && page<=12) || (page>=14 && page<=20) ||
    page==24 || page==25 || page==26;
}
static bool travel_present(void) {
  if(!ui_screen_on || travel_receipt.active || travel_settlement_blocked() || !pp_travel_describe(g).ready)return false;
  travel_notice_active=true;return true;
}
static const char *travel_wait_message(void) {
  if(g->battling || g->wild.phase==PP_WILD_BATTLE)return "战斗结束后提醒领取";
  if(g->drop_active)return "道具使用完成后提醒领取";
  return "等待确认领取";
}
static bool travel_settle(void) {
  if(travel_receipt.active || travel_settlement_blocked() || !pp_travel_describe(g).ready)return false;
  pp_travel_reward reward;
  if(pp_claim_travel(g,&reward)!=PP_OK)return false;
  travel_receipt.active=true;travel_receipt.reward=reward;
  travel_receipt.art=pet_art(reward.pet);travel_receipt.destination=reward.destination;
  travel_receipt.coins=reward.coins;travel_receipt.xp=reward.xp;
  snprintf(travel_receipt.name,sizeof(travel_receipt.name),"%s",pp_pet_name(g,reward.pet));
  snprintf(travel_receipt.item,sizeof(travel_receipt.item),"%s",g->log[3]);
  return true;
}
static void travel_notice_view(void) {
  notice_create();
  unsigned art=pet_art(g->traveler);
  travel_layout=true;
  lv_obj_add_flag(body,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(subtitle,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(back_button,LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_color(screen,color(0x0c1c30),0);lv_obj_set_style_bg_grad_dir(screen,LV_GRAD_DIR_NONE,0);
  text(title,"旅行完成");lv_obj_set_y(title,47);lv_obj_set_style_text_color(title,color(0xf1dca7),0);
  lv_obj_t *l=label(screen,pp_travel_name(pp_travel_destination(g)),60,79,240,0x93b9c8);lv_obj_set_style_text_font(l,&pp_font_12,0);
  lv_obj_t *halo=wild_shape(screen,116,102,128,128,0x4a8c96,LV_RADIUS_CIRCLE);lv_obj_set_style_bg_opa(halo,15,0);
  travel_result.pet=lv_image_create(screen);lv_obj_set_pos(travel_result.pet,132,118);
  large_portrait(travel_result.pet,paths[art][pp_animation_frame(art,PP_POSE_HAPPY,0)],&travel_result.source);
  l=label(screen,pp_pet_name(g,g->traveler),60,226,240,0xddebcf);lv_obj_set_style_text_font(l,&pp_font_18,0);
  travel_claim=button(screen,"确认领取",116,262,128,28,161);
}
static void travel_result_view(void) {
  notice_create();
  const pp_travel_reward *r=&travel_receipt.reward;
  unsigned accent=evo_colors[g->pets[r->pet].species%(sizeof(evo_colors)/sizeof(evo_colors[0]))];
  travel_layout=true;travel_result.started=lv_tick_get();
  lv_obj_add_flag(body,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(subtitle,LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_color(screen,color(0x0c1c30),0);lv_obj_set_style_bg_grad_dir(screen,LV_GRAD_DIR_NONE,0);
  text(title,"旅行完成");lv_obj_set_y(title,47);lv_obj_set_style_text_color(title,color(accent),0);
  if(r->item<PP_TRAVEL_COLLECTION) {
    const pp_collection_entry *item=&pp_collectibles[r->item];
    char path[320];collection_path(path,sizeof(path),r->item,true);
    char s[96];snprintf(s,sizeof(s),"新收藏 · %s",item->name);
    lv_point_t size;lv_text_get_size(&size,s,&pp_font_12,0,0,220,LV_TEXT_FLAG_NONE);
    int x=(360-26-size.x)/2;
    lv_obj_t *im=lv_image_create(screen);pp_image_set_src(im,path);lv_obj_set_pos(im,x,78);
    lv_obj_t *l=label(screen,s,x+26,81,size.x,pp_qualities[item->quality].color);
    lv_obj_set_style_text_font(l,&pp_font_12,0);
  } else {
    lv_obj_t *l=label(screen,"旅行藏品已集齐",60,81,240,0x91b8c8);
    lv_obj_set_style_text_font(l,&pp_font_12,0);
  }
  travel_result.halo=wild_shape(screen,120,106,120,120,accent,LV_RADIUS_CIRCLE);
  lv_obj_set_style_bg_opa(travel_result.halo,8,0);
  for(unsigned i=0;i<8;i++)travel_result.spark[i]=wild_shape(screen,0,0,2+i%2,2+i%2,accent,0);
  travel_result.pet=lv_image_create(screen);lv_obj_set_pos(travel_result.pet,132,118);
  char s[112];
  if(r->after.level>r->before.level)snprintf(s,sizeof(s),"%s  Lv.%u → %u",travel_receipt.name,r->before.level,r->after.level);
  else snprintf(s,sizeof(s),"%s  Lv.%u",travel_receipt.name,r->after.level);
  lv_obj_t *l=label(screen,s,50,227,260,accent);lv_obj_set_style_text_font(l,&pp_font_18,0);
  lv_obj_t *im=lv_image_create(screen);pp_image_set_src(im,hud_coin_path);lv_obj_set_pos(im,87,257);
  travel_result.coins=label(screen,"",108,254,64,0xf2cf8b);
  lv_obj_set_style_text_font(travel_result.coins,&pp_font_14,0);
  travel_result.xp=label(screen,"",184,254,98,0xa5dbce);
  lv_obj_set_style_text_font(travel_result.xp,&pp_font_14,0);
  lv_obj_set_pos(back_button,125,282);lv_obj_set_size(back_button,110,38);
  lv_obj_set_style_radius(back_button,13,0);lv_obj_set_style_bg_color(back_button,color(0x1c3c4e),0);
  lv_obj_set_style_border_color(back_button,color(0x355568),0);
  lv_obj_set_style_text_font(lv_obj_get_child(back_button,0),&pp_font_14,0);
  text(lv_obj_get_child(back_button,0),"完成");lv_obj_center(lv_obj_get_child(back_button,0));
  lv_obj_remove_event_cb(back_button,action);lv_obj_add_event_cb(back_button,action,LV_EVENT_CLICKED,(void *)(intptr_t)0);
  lv_obj_add_flag(back_button,LV_OBJ_FLAG_HIDDEN);travel_result_tick();
}
static void travel_result_tick(void) {
  if(!travel_result.pet || (!travel_receipt.active && !travel_notice_active))return;
  unsigned age=lv_tick_get()-travel_result.started,t=age<1000?age:1000;
  unsigned art=travel_receipt.active?travel_receipt.art:pet_art(g->traveler);
  unsigned frame=pp_animation_update(&travel_result.clock,art,PP_POSE_HAPPY,lv_tick_get());
  large_portrait(travel_result.pet,paths[art][frame],&travel_result.source);
  if(!travel_receipt.active)return;
  char s[48];snprintf(s,sizeof(s),"+%u",travel_receipt.coins*t/1000);text(travel_result.coins,s);
  if(travel_receipt.reward.before.level>=PP_MAX_LEVEL)snprintf(s,sizeof(s),"已满级");
  else snprintf(s,sizeof(s),"XP +%u",travel_receipt.xp*t/1000);
  text(travel_result.xp,s);
  lv_obj_set_style_bg_opa(travel_result.halo,age<1200?12*(1200-age)/1200:0,0);
  for(unsigned i=0;i<8;i++) {
    if(age>=1200){lv_obj_add_flag(travel_result.spark[i],LV_OBJ_FLAG_HIDDEN);continue;}
    float angle=i*6.283185f/8+age*0.0003f;int radius=40+age*24/1200;
    lv_obj_set_pos(travel_result.spark[i],179+(int)(cosf(angle)*radius),165+(int)(sinf(angle)*radius));
    lv_obj_set_style_bg_opa(travel_result.spark[i],160*(1200-age)/1200,0);
  }
  if(age>=1200)lv_obj_remove_flag(back_button,LV_OBJ_FLAG_HIDDEN);
}
static void wild_result_view(void) {
  const pp_wild_state *w=&g->wild;
  const pp_dex_entry *e=&pp_pokedex[w->dex];
  bool won=w->phase==PP_WILD_WON,caught=won&&w->outcome==PP_WILD_CAUGHT;
  bool ready=won&&w->outcome==PP_WILD_CAPTURE_READY;
  char s[96];
  lv_obj_add_flag(body,LV_OBJ_FLAG_HIDDEN);
  /* Match evolution: quiet navy, centered portrait, sparse typed glow. */
  unsigned accent=evo_colors[e->species%(sizeof(evo_colors)/sizeof(evo_colors[0]))];
  lv_obj_set_style_bg_color(screen,color(0x0c1c30),0);
  lv_obj_set_style_bg_grad_dir(screen,LV_GRAD_DIR_NONE,0);
  text(title,caught?"捕捉成功":won?"对战获胜":"伙伴逃走了");
  lv_obj_set_style_text_color(title,color(accent),0);lv_obj_set_y(title,47);
  lv_obj_add_flag(subtitle,LV_OBJ_FLAG_HIDDEN);
  wild_result.halo=wild_shape(screen,120,106,120,120,accent,LV_RADIUS_CIRCLE);
  lv_obj_set_style_bg_opa(wild_result.halo,ready?8:0,0);
  lv_obj_set_style_border_width(wild_result.halo,1,0);
  lv_obj_set_style_border_color(wild_result.halo,color(accent),0);
  lv_obj_set_style_border_opa(wild_result.halo,ready?20:0,0);
  wild_result.shadow=wild_shape(screen,148,205,64,7,0x0c1c30,LV_RADIUS_CIRCLE);
  wild_result.pet=lv_image_create(screen);wild_portrait(paths[e->art][0]);
  lv_obj_set_pos(wild_result.pet,132,118);
  wild_result.ball=lv_image_create(screen);pp_image_set_src(wild_result.ball,capture_ball_icon_path);
  lv_obj_set_pos(wild_result.ball,164,150);lv_obj_add_flag(wild_result.ball,LV_OBJ_FLAG_HIDDEN);
  for(unsigned i=0;i<12;i++) {
    wild_result.spark[i]=wild_shape(screen,0,0,i%3?3:5,i%3?3:5,i%2?accent:0xe8f4e7,0);
    lv_obj_add_flag(wild_result.spark[i],LV_OBJ_FLAG_HIDDEN);
  }
  snprintf(s,sizeof(s),"%s  Lv.%u",e->name,w->level);
  lv_obj_t *name=label(screen,s,60,227,240,accent);
  lv_obj_set_style_text_font(name,&pp_font_18,0);
  if(won)for(unsigned i=0;i<2;i++) {
    lv_obj_t *reward=wild_shape(screen,78+(int)i*106,254,98,22,0x0c1c30,0);
    lv_obj_set_style_bg_opa(reward,0,0);
    if(!i){lv_obj_t *im=lv_image_create(reward);pp_image_set_src(im,hud_coin_path);lv_obj_set_pos(im,10,3);}
    snprintf(s,sizeof(s),i?"XP +%u":"+%u",i?w->reward_xp:w->reward_coins);
    lv_obj_t *l=label(reward,s,i?0:27,2,i?98:62,i?0xa5dbce:0xf2cf8b);
    lv_obj_set_style_text_font(l,&pp_font_14,0);
  }
  if(ready) {
    lv_obj_add_flag(back_button,LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *release=button(screen,"放走",100,296,72,32,654);
    lv_obj_set_style_bg_opa(release,0,0);lv_obj_set_style_border_color(release,color(0x57766e),0);
    snprintf(s,sizeof(s),g->balls?"捕捉 ×%u":"购买咕噜球",g->balls);
    lv_obj_t *capture=button(screen,s,188,296,72,32,g->balls?656:657);
    lv_obj_set_style_bg_color(capture,color(0x23435a),0);
    lv_obj_set_style_bg_grad_dir(capture,LV_GRAD_DIR_NONE,0);
    lv_obj_set_style_border_color(capture,color(0x47758d),0);
    lv_obj_set_style_text_font(lv_obj_get_child(capture,0),&pp_font_12,0);
  } else {
    lv_obj_move_foreground(back_button);
    lv_obj_set_pos(back_button,125,282);lv_obj_set_size(back_button,110,38);
    lv_obj_set_style_radius(back_button,13,0);
    lv_obj_set_style_bg_color(back_button,color(0x1c3c4e),0);
    lv_obj_set_style_border_color(back_button,color(0x355568),0);
    text(lv_obj_get_child(back_button,0),"完成");lv_obj_set_width(lv_obj_get_child(back_button,0),110);
    lv_obj_set_style_text_font(lv_obj_get_child(back_button,0),&pp_font_14,0);
    lv_obj_center(lv_obj_get_child(back_button,0));
  }
}
static void wild_capture_begin(void) {
  wild_result.started=lv_tick_get();wild_result.active=true;
  text(title,"捕捉中");lv_obj_add_flag(back_button,LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(wild_result.ball,LV_OBJ_FLAG_HIDDEN);
  wild_result_tick();
}
static void wild_result_tick(void) {
  if(!wild_result.pet || current!=23 || g->legend.active)return;
  unsigned age=lv_tick_get()-wild_result.started;
  unsigned art=pp_pokedex[g->wild.dex].art;
  if(!wild_result.active) {
    unsigned frame=pp_animation_update(&wild_result.clock,art,g->wild.outcome==PP_WILD_CAUGHT?PP_POSE_HAPPY:PP_POSE_IDLE,lv_tick_get());
    wild_portrait(paths[art][frame]);
    return;
  }
  /* Native ball; pet uses exact 2x pixel sampling in the shared fixed canvas. */
  if(age<500) {
    int t=(int)age;
    lv_obj_set_pos(wild_result.ball,80+84*t/500,214-192*t*(500-t)/250000-64*t/500);
  } else if(age<1050) {
    unsigned t=age-500;
    lv_obj_set_pos(wild_result.ball,164,150);
    lv_obj_set_y(wild_result.pet,118+(int)t*28/550);
    lv_obj_set_style_image_recolor(wild_result.pet,color(0xc9ffe4),0);
    lv_obj_set_style_image_recolor_opa(wild_result.pet,t*255/550,0);
    lv_obj_set_style_image_opa(wild_result.pet,255-t*255/550,0);
    lv_obj_set_style_bg_opa(wild_result.halo,24+t*90/550,0);
  } else if(age<2150) {
    lv_obj_add_flag(wild_result.pet,LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wild_result.shadow,LV_OBJ_FLAG_HIDDEN);
    unsigned t=age-1050,cycle=t%500;
    int wobble=t<1000?(int)lround(sin(cycle*6.2831853/500)*5):0;
    lv_obj_set_pos(wild_result.ball,164+wobble,150-abs(wobble)/2);
    lv_obj_set_style_bg_opa(wild_result.halo,24,0);
  } else if(age<2850) {
    unsigned t=age-2150;
    lv_obj_set_pos(wild_result.ball,164,150);
    text(title,"捕捉成功");
    for(unsigned i=0;i<12;i++) {
      double a=i*6.2831853/12;
      int radius=30+(int)t*44/700;
      lv_obj_t *spark=wild_result.spark[i];
      lv_obj_set_pos(spark,178+(int)lround(cos(a)*radius),166+(int)lround(sin(a)*radius*.65));
      lv_obj_set_style_bg_opa(spark,255-t*255/700,0);
      lv_obj_remove_flag(spark,LV_OBJ_FLAG_HIDDEN);
    }
  } else {
    wild_result.active=false;
    for(unsigned i=0;i<12;i++)lv_obj_add_flag(wild_result.spark[i],LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wild_result.ball,LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(wild_result.pet,LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(wild_result.shadow,LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(wild_result.pet,132,118);
    lv_obj_set_style_image_opa(wild_result.pet,255,0);
    lv_obj_set_style_image_recolor_opa(wild_result.pet,0,0);
    unsigned species=pp_pokedex[g->wild.dex].species;
    lv_obj_set_style_text_color(title,color(evo_colors[species%(sizeof(evo_colors)/sizeof(evo_colors[0]))]),0);
    lv_obj_set_style_bg_opa(wild_result.halo,0,0);
    lv_obj_set_style_border_opa(wild_result.halo,0,0);
    lv_obj_remove_flag(back_button,LV_OBJ_FLAG_HIDDEN);
    memset(&wild_result.clock,0,sizeof(wild_result.clock));
  }
}
static void gift_overlay_tick(void) {
  bool pending=current==0 && g->gift.pending && !g->wild.phase && !g->battling && !pp_ui_evolving();
  if(!pending && gift_overlay){lv_obj_delete(gift_overlay);gift_overlay=NULL;}
  if(!pending || gift_overlay)return;
  for(lv_indev_t *in=lv_indev_get_next(NULL);in;in=lv_indev_get_next(in))lv_indev_wait_release(in);
  gift_overlay=lv_obj_create(screen);lv_obj_remove_style_all(gift_overlay);
  lv_obj_set_size(gift_overlay,360,360);lv_obj_add_flag(gift_overlay,LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(gift_overlay,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(gift_overlay,color(0x071923),0);lv_obj_set_style_bg_opa(gift_overlay,210,0);
  /* Every component shares x=142 inside a ball centered on the 360px screen. */
  lv_obj_t *ball=wild_shape(gift_overlay,38,38,284,284,0xf5f0df,LV_RADIUS_CIRCLE);
  lv_obj_set_style_clip_corner(ball,true,0);
  lv_obj_set_style_shadow_width(ball,18,0);lv_obj_set_style_shadow_opa(ball,120,0);
  lv_obj_t *cap=wild_shape(ball,0,0,284,142,0xdf9987,0);
  lv_obj_set_style_bg_grad_color(cap,color(0xba5667),0);lv_obj_set_style_bg_grad_dir(cap,LV_GRAD_DIR_VER,0);
  lv_obj_t *l=label(ball,"发现藏品",42,18,200,0xffefd4);lv_obj_set_style_text_font(l,&pp_font_18,0);
  const pp_dex_entry *sender=&pp_pokedex[g->gift.dex];
  lv_obj_t *im=lv_image_create(ball);pp_image_set_src(im,paths[sender->art][0]);lv_obj_set_pos(im,118,44);
  char found[96];snprintf(found,sizeof(found),"%s找到了藏品",sender->name);
  l=label(ball,found,32,94,220,0xfff3d9);lv_obj_set_style_text_font(l,&pp_font_12,0);
  for(unsigned side=0;side<2;side++) {
    int cx=side?200:84;
    wild_shape(ball,cx-1,62,2,14,0xffdba4,0);
    wild_shape(ball,cx-7,68,14,2,0xffdba4,0);
    wild_shape(ball,side?217:65,84,2,2,0xf5bd9e,0);
    wild_shape(ball,side?190:92,52,2,2,0xf5bd9e,0);
  }
  /* The horizontal seam and concentric clasp share the ball's true center. */
  wild_shape(ball,0,135,284,2,0xf5bba2,0);
  wild_shape(ball,0,137,284,10,0x243e4b,0);
  wild_shape(ball,0,147,284,2,0xd5ddcf,0);
  wild_shape(ball,112,112,60,60,0x243e4b,LV_RADIUS_CIRCLE);
  wild_shape(ball,116,116,52,52,0xfff8e4,LV_RADIUS_CIRCLE);
  wild_shape(ball,120,120,44,44,0xe1e7d9,LV_RADIUS_CIRCLE);
  char path[320];collection_path(path,sizeof(path),g->gift.item,false);
  im=lv_image_create(ball);pp_image_set_src(im,path);lv_obj_set_pos(im,126,126);
  const pp_collection_entry *item=&pp_collectibles[g->gift.item];
  unsigned tint=pp_qualities[item->quality].color;
  l=label(ball,item->name,22,183,240,0x294b57);lv_obj_set_style_text_font(l,&pp_font_14,0);
  for(unsigned side=0;side<2;side++) {
    lv_obj_t *line=wild_shape(ball,side?190:60,214,34,1,0x8da99b,0);
    lv_obj_set_style_bg_opa(line,100,0);
  }
  lv_obj_t *quality=wild_shape(ball,104,205,76,18,0x243e4b,9);
  l=label(quality,pp_qualities[item->quality].name,0,2,76,tint);lv_obj_set_style_text_font(l,&pp_font_12,0);
  lv_obj_t *accept=button(gift_overlay,"收下藏品",116,294,128,36,724);
  lv_obj_set_style_radius(accept,16,0);lv_obj_set_style_bg_color(accept,color(0x327c70),0);
  lv_obj_set_style_bg_grad_color(accept,color(0x205950),0);lv_obj_set_style_bg_grad_dir(accept,LV_GRAD_DIR_VER,0);
  lv_obj_set_style_border_color(accept,color(0x76b39a),0);
  lv_obj_set_style_text_color(lv_obj_get_child(accept,0),color(0xfff0c8),0);
  pp_footer_done(accept);
  lv_obj_t *rim=wild_shape(ball,0,0,284,284,0x203c49,LV_RADIUS_CIRCLE);
  lv_obj_set_style_bg_opa(rim,0,0);lv_obj_set_style_border_width(rim,3,0);
  lv_obj_set_style_border_color(rim,color(0x203c49),0);
  rim=wild_shape(ball,4,4,276,276,0xfbe4c6,LV_RADIUS_CIRCLE);
  lv_obj_set_style_bg_opa(rim,0,0);lv_obj_set_style_border_width(rim,1,0);
  lv_obj_set_style_border_color(rim,color(0xfbe4c6),0);lv_obj_set_style_border_opa(rim,120,0);
}
static void wild_fighter_update(void) {
  if(!wild_fighter)return;
  wild_selected=pp_wild_default_pet(g);
  char s[80];
  if(wild_selected<g->count)snprintf(s,sizeof(s),"%s Lv.%u",pp_form_name(g,wild_selected),g->growth[wild_selected].level);
  else snprintf(s,sizeof(s),"暂无可出战伙伴");
  text(wild_fighter,s);
  if(wild_fight_action){
    if(wild_selected<g->count)lv_obj_remove_state(wild_fight_action,LV_STATE_DISABLED);
    else lv_obj_add_state(wild_fight_action,LV_STATE_DISABLED);
  }
}
static void wild_overlay_tick(void) {
  bool alert=current==0 && pp_wild_alert_remaining(g);
  if(!alert && wild_alert){lv_obj_delete(wild_alert);wild_alert=wild_alert_pet=wild_alert_mark=NULL;wild_alert_src=NULL;}
  if(alert) {
    if(!wild_alert) {
      wild_alert=lv_obj_create(screen);lv_obj_remove_style_all(wild_alert);
      int spawn_x=236,best=-1;const int spots[]={72,156,236};
      for(unsigned slot=0;slot<3;slot++) {
        int nearest=1000000;
        for(unsigned pet=0;pet<g->count;pet++)if(pet_img[pet] && !lv_obj_has_flag(pet_img[pet],LV_OBJ_FLAG_HIDDEN)) {
          int dx=spots[slot]-lv_obj_get_x(pet_img[pet]),dy=186-lv_obj_get_y(pet_img[pet]);
          int distance=dx*dx+dy*dy;if(distance<nearest)nearest=distance;
        }
        if(nearest>best){best=nearest;spawn_x=spots[slot];}
      }
      lv_obj_set_pos(wild_alert,spawn_x,150);lv_obj_set_size(wild_alert,48,84);
      lv_obj_add_flag(wild_alert,LV_OBJ_FLAG_CLICKABLE);
      lv_obj_remove_flag(wild_alert,LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_add_event_cb(wild_alert,action,LV_EVENT_CLICKED,(void *)(intptr_t)658);
      for(lv_indev_t *in=lv_indev_get_next(NULL);in;in=lv_indev_get_next(in))lv_indev_wait_release(in);
      wild_alert_mark=lv_obj_create(wild_alert);lv_obj_remove_style_all(wild_alert_mark);
      lv_obj_set_size(wild_alert_mark,15,30);lv_obj_remove_flag(wild_alert_mark,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
      for(unsigned row=0;row<10;row++)for(unsigned col=0;col<5;col++) {
        char pixel=wild_mark_pixels[row][col];
        if(pixel!='0')wild_shape(wild_alert_mark,col*3,row*3,3,3,pixel=='1'?0x203c49:0xfff0b0,0);
      }
      wild_alert_pet=lv_image_create(wild_alert);lv_obj_set_pos(wild_alert_pet,0,36);
      lv_obj_remove_flag(wild_alert_pet,LV_OBJ_FLAG_CLICKABLE);
    }
    unsigned art=pp_pokedex[g->wild.dex].art;
    const void *src=paths[art][pp_animation_frame(art,PP_POSE_IDLE,lv_tick_get())];
    if(src!=wild_alert_src){pp_image_set_src(wild_alert_pet,src);wild_alert_src=src;}
    lv_obj_set_pos(wild_alert_mark,17,(lv_tick_get()/330)%2?0:3);
    lv_obj_move_foreground(wild_alert);
  }
  bool pending=current==0 && pp_wild_opened(g);
  if(!pending && wild_overlay){lv_obj_delete(wild_overlay);wild_overlay=wild_fighter=wild_fight_action=NULL;}
  if(pending && !wild_overlay) {
    /* Swallow a touch already held when the encounter appears. */
    for(lv_indev_t *in=lv_indev_get_next(NULL);in;in=lv_indev_get_next(in))lv_indev_wait_release(in);
    wild_overlay=lv_obj_create(screen);lv_obj_remove_style_all(wild_overlay);
    lv_obj_set_size(wild_overlay,360,360);lv_obj_add_flag(wild_overlay,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(wild_overlay,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(wild_overlay,color(0x071923),0);lv_obj_set_style_bg_opa(wild_overlay,210,0);
    /* The modal owns its geometry; the garden below keeps its existing motion. */
    lv_obj_t *ball=wild_shape(wild_overlay,38,34,284,284,0xeef1e8,LV_RADIUS_CIRCLE);
    lv_obj_set_style_clip_corner(ball,true,0);
    lv_obj_set_style_shadow_width(ball,18,0);lv_obj_set_style_shadow_color(ball,color(0x020a12),0);
    lv_obj_set_style_shadow_opa(ball,150,0);lv_obj_set_style_shadow_offset_y(ball,5,0);
    lv_obj_t *cap=wild_shape(ball,0,0,284,126,0xe18277,0);
    lv_obj_set_style_bg_grad_color(cap,color(0xb7475b),0);lv_obj_set_style_bg_grad_dir(cap,LV_GRAD_DIR_VER,0);
    lv_obj_t *glint=wild_shape(ball,43,25,27,5,0xffe5c4,3);lv_obj_set_style_bg_opa(glint,75,0);
    glint=wild_shape(ball,38,34,12,4,0xffe5c4,2);lv_obj_set_style_bg_opa(glint,40,0);
    lv_obj_t *eyebrow=wild_shape(ball,94,18,96,20,0x7b3147,10);lv_obj_set_style_bg_opa(eyebrow,75,0);
    lv_obj_t *l=label(eyebrow,"野生相遇",0,3,96,0xffe5c8);lv_obj_set_style_text_font(l,&pp_font_12,0);
    const pp_dex_entry *e=&pp_pokedex[g->wild.dex];char s[80];
    l=label(ball,e->name,32,49,220,0xfff5df);lv_obj_set_style_text_font(l,&pp_font_18,0);
    wild_shape(ball,0,119,284,2,0xf1b597,0);
    wild_shape(ball,0,121,284,11,0x243e4b,0);
    wild_shape(ball,0,132,284,2,0xc5d3ca,0);
    /* The central ball clasp doubles as the pet's small display window. */
    wild_shape(ball,98,84,88,88,0x243e4b,LV_RADIUS_CIRCLE);
    wild_shape(ball,102,88,80,80,0xf7f1df,LV_RADIUS_CIRCLE);
    lv_obj_t *well=wild_shape(ball,107,93,70,70,0xdce9df,LV_RADIUS_CIRCLE);
    lv_obj_set_style_bg_grad_color(well,color(0xf4f1df),0);lv_obj_set_style_bg_grad_dir(well,LV_GRAD_DIR_VER,0);
    lv_obj_t *shadow=wild_shape(ball,126,142,32,6,0x5b8b83,LV_RADIUS_CIRCLE);lv_obj_set_style_bg_opa(shadow,60,0);
    lv_obj_t *im=lv_image_create(ball);pp_image_set_src(im,paths[e->art][0]);lv_obj_set_pos(im,118,103);
    lv_obj_t *level=wild_shape(ball,174,91,44,20,0x294b57,10);
    snprintf(s,sizeof(s),"Lv.%u",g->wild.level);
    l=label(level,s,0,3,44,0xf5edcf);lv_obj_set_style_text_font(l,&pp_font_12,0);
    lv_obj_t *fighter=wild_shape(ball,74,192,136,24,0xdfe8df,12);
    wild_fighter=label(fighter,"",0,5,136,0x42636b);lv_obj_set_style_text_font(wild_fighter,&pp_font_12,0);
    lv_obj_t *flee=button(ball,"放走",62,220,72,32,651);
    lv_obj_set_style_radius(flee,16,0);lv_obj_set_style_bg_color(flee,color(0xe1e8df),0);
    lv_obj_set_style_bg_grad_dir(flee,LV_GRAD_DIR_NONE,0);
    lv_obj_set_style_border_color(flee,color(0xb8ccc1),0);
    lv_obj_set_style_text_color(lv_obj_get_child(flee,0),color(0x526c6c),0);
    lv_obj_t *fight=button(ball,"对战",150,220,72,32,650);wild_fight_action=fight;
    lv_obj_set_style_radius(fight,16,0);lv_obj_set_style_bg_color(fight,color(0x327c70),0);
    lv_obj_set_style_bg_grad_color(fight,color(0x205950),0);lv_obj_set_style_bg_grad_dir(fight,LV_GRAD_DIR_VER,0);
    lv_obj_set_style_border_color(fight,color(0x76b39a),0);
    lv_obj_set_style_text_color(lv_obj_get_child(fight,0),color(0xfff0c8),0);
    lv_obj_t *rim=wild_shape(ball,0,0,284,284,0x203c49,LV_RADIUS_CIRCLE);
    lv_obj_set_style_bg_opa(rim,0,0);lv_obj_set_style_border_width(rim,3,0);
    lv_obj_set_style_border_color(rim,color(0x203c49),0);
    lv_obj_t *inner_rim=wild_shape(ball,4,4,276,276,0xf9ddb2,LV_RADIUS_CIRCLE);
    lv_obj_set_style_bg_opa(inner_rim,0,0);lv_obj_set_style_border_width(inner_rim,1,0);
    lv_obj_set_style_border_color(inner_rim,color(0xf8dfb6),0);lv_obj_set_style_border_opa(inner_rim,145,0);
    pp_footer_text_style(flee);pp_footer_text_style(fight);
  }
  if(pending)wild_fighter_update();

  bool resumable=current==0 && g->wild.phase>=PP_WILD_BATTLE;
  if(!resumable && wild_resume){lv_obj_delete(wild_resume);wild_resume=NULL;}
  if(resumable && !wild_resume)wild_resume=button(screen,g->legend.active?(g->wild.phase==PP_WILD_BATTLE?"神兽挑战 · 继续":"神兽挑战 · 收获"):g->wild.phase==PP_WILD_BATTLE?"野外对战 · 继续":"野外对战 · 收获",110,106,140,30,655);
}
static void update_steps(void) {
  char s[128];
  snprintf(s,sizeof(s),"%lu",(unsigned long)g->steps_today); text(step_number,s);
  snprintf(s,sizeof(s),"+%lu              +%lu XP",(unsigned long)g->steps_coins,(unsigned long)g->steps_xp); text(step_rewards,s);
  snprintf(s,sizeof(s),"累计 %llu%s",(unsigned long long)g->total_steps,
           !imu_ok?" · IMU—":!g->clock_ready?" · 待校时":""); text(step_total,s);
  snprintf(s,sizeof(s),"每千步 +%u金币 · 经验%u%%",pp_step_coin_rate(g),pp_step_xp_rate(g));
  text(step_rate,s);
  lv_bar_set_value(step_bar,g->steps_today%100,LV_ANIM_OFF);
}
void pp_ui_refresh(void) {
  if(shop_reel.active){last_revision=g->revision;return;}
  if (evolution)
    return;
  unsigned warehouse_now=pp_warehouse_count(g);
  /* Natural hatches arrive from pp_tick rather than a button callback. Only
   * an egg-to-no-egg transition may open this reveal, never a wild capture. */
  if(!hatch_result.active && egg_present_seen && !g->egg_end &&
     (g->count>roster_count_seen || warehouse_now>warehouse_count_seen)) {
    if(g->count>roster_count_seen)hatch_result_capture(false,g->count-1);
    else hatch_result_capture(true,warehouse_now-1);
    pp_ui_open(30);return;
  }
  egg_present_seen=g->egg_end!=0;
  roster_count_seen=g->count;warehouse_count_seen=warehouse_now;
  if (current == 13 || current == 22 || current==23)
    return;
  if (!travel_home_deferred && !travel_notice_active && travel_page_safe(current) && travel_present()) {pp_ui_open(5);return;}
  if(current==30 && hatch_result.active){last_revision=g->revision;return;}
  /* Self-updating views still yield to a queued completion when safe. */
  if(current==31 || current==32 || current==33){last_revision=g->revision;return;}
  if (current == 0) {
    update_home();wild_overlay_tick();gift_overlay_tick();
  }
  else if (current == 14)
    update_steps();
  else if (current == 5 || current == 6 || current == 7)
    live();
  else if (last_revision != g->revision)
    refresh_pending = true;
  last_revision = g->revision;
}
static void animate(lv_timer_t *t) {
  (void)t;
  frame_count++;shop_reel_tick();
  lv_timer_set_period(t, (current == 0 || current == 22 || current == 23 || current==30 || (current==3 && shop_reel.active) || (current==5 && travel_layout)) ? 33 : PP_FRAME_MS);
  if (current == 22) { pp_intro_view_tick(); return; }
  if (evolution) {
    if(ui_screen_on)evolution_tick();
    return;
  }
  if(shop_reel.active)return;
  if(training_notice_tick())return;
  if (refresh_pending) {
    refresh_pending = false;
    pp_ui_open(current);
  }
  pp_ui_refresh();
  if(current==30){hatch_result_tick();return;}
  if(current==5 && (travel_receipt.active || travel_notice_active))return;
  if(current==23)wild_result_tick();
  if(current==6 && g->wild.phase>=PP_WILD_WON && pp_battle_view_probe().settled) {pp_ui_open(23);return;}
  if(current==6 && g->gym.mode && g->gym.result && pp_battle_view_probe().settled) {pp_ui_open(21);return;}
  if (ui_screen_on && current<17 && !g->wild.phase && !g->battling && !g->drop_active &&
      (current != 6 || !battle_layout || pp_battle_view_probe().settled)) {
    for (unsigned p = 0; p < g->count; p++) {
      if(pp_training_growth_pending(g))continue;
      bool training_seen=false;
      for(unsigned i=0;g->training.status==2&&i<g->training.count;i++)
        if(g->training.picks[i]==p && (g->training.evolution_seen&(1u<<i)) && g->training.end_levels[i]==g->growth[p].level)training_seen=true;
      if(training_seen)continue;
      unsigned gate = pp_evolution_level(g, p);
      if (gate && g->growth[p].level >= gate && g->traveler != (int)p && !pp_training_busy(g,p) &&
          !(evo_postponed_level[p]==g->growth[p].level && evo_postponed_art[p]==pet_art(p))) {
        evolution_open(p, false);
        return;
      }
    }
  }
  if (!refresh_pending && ui_screen_on && current == 5 && activity_pet[0]) {
    pp_travel_status trip = pp_travel_describe(g);
    uint32_t now=lv_tick_get();
    if(!trip.ready){
      travel_motion.elapsed+=now-travel_motion.last;
      pp_scene_view_strip_tick(travel_backdrop,travel_motion.elapsed);
      if(travel_motion.effects)lv_obj_invalidate(travel_motion.effects);
    }
    travel_motion.last=now;
    unsigned art = travel_receipt.active ? travel_receipt.art : pet_art(g->traveler);
    unsigned pose = travel_receipt.active || trip.ready ? PP_POSE_HAPPY
                    : trip.walking ? PP_POSE_WALK : PP_POSE_IDLE;
    unsigned frame = pp_animation_update(&activity_clocks[0], art, pose, travel_motion.elapsed);
    const void *src = paths[art][frame];
    if (src != activity_src[0]) {
      travel_portrait(activity_pet[0], src);
      activity_src[0] = src;
    }
  }

}
static void replacement_failed(pp_result result) {
  if(result!=PP_BUSY)notice(pp_message(result));
  else if(g->battling || g->wild.phase==PP_WILD_BATTLE)notice("战斗中");
  else if(g->traveler==(int)replacement_slot)notice(pp_travel_describe(g).ready?"请先领取旅行奖励":"伙伴正在旅行");
  else if(g->drop_active && g->drop_target==replacement_slot)notice("正在使用道具");
  else if(g->training.status==1)notice(pp_training_busy(g,replacement_slot)?"伙伴正在历练":"历练中不能增减伙伴");
  else if(pp_training_growth_pending(g))notice("请先查看历练升级提示");
  else notice(pp_message(result));
}
static void action(lv_event_t *e) {
  int cmd = (int)(intptr_t)lv_event_get_user_data(e);
  if(current==5 && travel_notice_active && cmd!=161)return;
  if(current==5 && travel_receipt.active) {
    if(cmd!=0 || lv_obj_has_flag(back_button,LV_OBJ_FLAG_HIDDEN))return;
    travel_receipt.active=false;pp_ui_open(hatch_result.active?30:0);return;
  }
  if(current==23 && wild_result.active)return;
  if(shop_reel.active)return;
  pp_result r = PP_OK;
  if(current==11 && cmd>=800 && cmd<=803){settings_section=cmd-800;pp_ui_open(11);return;}
  if(cmd==790){
    unsigned item=0;r=pp_shop_draw(g,&item);
    if(r!=PP_OK){notice(pp_message(r));return;}
    memset(&shop_reel,0,sizeof(shop_reel));shop_reel.active=shop_reel.awarded=true;shop_reel.item=item;shop_reel.tick=lv_tick_get();
    pp_ui_open(3);last_revision=g->revision;refresh_pending=false;
    lv_obj_add_flag(toast,LV_OBJ_FLAG_HIDDEN);lv_timer_pause(notice_timer);
    if(back_button)lv_obj_add_state(back_button,LV_STATE_DISABLED);
    shop_reel.tick=lv_tick_get();shop_reel_paint();return;
  }
  unsigned previous_count = g->count;
  if(cmd==658){if(pp_wild_reveal(g)==PP_OK)wild_overlay_tick();return;}
  if(cmd>=3000&&cmd<3000+WAREHOUSE_PAGE_SIZE){
    unsigned n=warehouse_visible_slots[cmd-3000];if(n>=warehouse_view_count())return;
    info_from_warehouse=true;info_warehouse=n>=g->count;
    if(info_warehouse)warehouse_selected=n-g->count;else selected=n;
    pp_ui_open(26);return;
  }
  if(cmd==772){pp_ui_open(16);return;}
  if(cmd==773){warehouse_sort_mode=(warehouse_sort_mode+1)%PP_WAREHOUSE_SORT_COUNT;reserve_page=0;pp_ui_open(16);notice(warehouse_sort_labels[warehouse_sort_mode]);return;}
  if(cmd==780 && hatch_result.active) {
    if(!hatch_result.in_warehouse) {
      r=pp_reserve_pet(g,hatch_result.slot);
      if(r!=PP_OK){notice(pp_message(r));return;}
      hatch_result.in_warehouse=true;hatch_result.slot=pp_warehouse_count(g)-1;
    }
    hatch_result.active=false;hatch_replacing=false;
    roster_count_seen=g->count;warehouse_count_seen=pp_warehouse_count(g);
    reserve_page=(warehouse_view_count()-1)/WAREHOUSE_PAGE_SIZE;pp_ui_open(16);notice("已放入仓库");return;
  }
  if(cmd==781 && hatch_result.active) {
    if(!hatch_result.in_warehouse) {
      r=pp_reserve_pet(g,hatch_result.slot);
      if(r!=PP_OK){notice(pp_message(r));return;}
      hatch_result.in_warehouse=true;hatch_result.slot=pp_warehouse_count(g)-1;
    }
    roster_count_seen=g->count;warehouse_count_seen=pp_warehouse_count(g);
    warehouse_selected=hatch_result.slot;hatch_replacing=true;pp_ui_open(29);return;
  }
  if(cmd==760){info_from_warehouse=false;info_warehouse=false;pp_ui_open(26);return;}
  if(cmd==761){
    const char *name;if(info_warehouse){pp_stored_pet p;pp_warehouse_get(g,warehouse_selected,&p);name=p.identity.nickname;snprintf(name_draft,sizeof(name_draft),"%s",name);}else snprintf(name_draft,sizeof(name_draft),"%s",g->identity[selected].nickname);
    for(char *c=name_draft;*c;c++)if(*c>='A'&&*c<='Z')*c+='a'-'A';
    pp_ui_open(27);return;
  }
  if(cmd==762){pp_ui_open(28);return;}
  if(cmd==763){if(g->fishing_partner==warehouse_selected+1){notice("请先更换陪钓伙伴");return;}r=pp_release(g,warehouse_selected);if(r==PP_OK){unsigned pages=(warehouse_view_count()+WAREHOUSE_PAGE_SIZE-1)/WAREHOUSE_PAGE_SIZE;if(reserve_page>=pages)reserve_page=pages?pages-1:0;pp_ui_open(16);}else notice(pp_message(r));return;}
  if(cmd==764){
    if(g->fishing_partner==warehouse_selected+1){notice("请先更换陪钓伙伴");return;}
    if(g->count==PP_PETS){pp_ui_open(29);return;}
    replacement_slot=g->count;
    r=pp_replace_pet(g,replacement_slot,warehouse_selected);if(r==PP_OK){home_reset_pet(replacement_slot);selected=replacement_slot;pp_ui_open(0);}else replacement_failed(r);return;
  }
  if(cmd>=4200&&cmd<4200+PP_PETS){
    replacement_slot=cmd-4200;
    r=pp_replace_pet(g,replacement_slot,warehouse_selected);if(r==PP_OK){home_reset_pet(replacement_slot);selected=replacement_slot;hatch_result.active=false;hatch_replacing=false;pp_ui_open(0);}else replacement_failed(r);return;
  }
  if(cmd==765){r=pp_rename(g,info_warehouse,info_warehouse?warehouse_selected:selected,name_draft);if(r==PP_OK)pp_ui_open(26);else notice("请用8个小写字母");return;}
  if(cmd==766){name_draft[0]=0;name_update();return;}
  if(cmd==767){name_backspace();name_update();return;}
  if(cmd>=4000&&cmd<4026){if(!name_append('a'+cmd-4000))notice("最多8个字母");name_update();return;}
  if(cmd==656){r=pp_wild_capture(g);if(r==PP_OK){pp_ui_open(23);wild_capture_begin();}else notice(pp_message(r));return;}
  if(cmd==657){pp_ui_open(3);return;}
  if(cmd==16){replacement_slot=g->count<PP_PETS?g->count:selected;reserve_page=0;warehouse_swap=false;pp_ui_open(16);return;}
  if(cmd==724){pp_gift_ack(g);gift_overlay_tick();notice("已放入收藏");return;}
  if(cmd==650) {wild_fighter_update();r=pp_wild_start(g,wild_selected);if(r==PP_OK)pp_ui_open(6);else {wild_overlay_tick();notice("需清醒、健康30、精力3");}return;}
  if(cmd==651){pp_wild_dismiss(g);if(gym_waiting_wild){gym_waiting_wild=false;pp_ui_open(19);}else wild_overlay_tick();return;}
  if(cmd==654){bool legendary=g->legend.active!=0,return_gym=gym_waiting_wild;gym_waiting_wild=false;pp_wild_clear_result(g);pp_ui_open(return_gym?19:legendary?24:0);return;}
  if(cmd==24 && g->legend.active){pp_ui_open(g->wild.phase>=PP_WILD_WON?23:6);return;}
  if(cmd>=700 && cmd<700+PP_LEGEND_COUNT){legend_selected=cmd-700;pp_ui_open(25);return;}
  if(cmd==710){r=pp_legend_start(g,legend_selected,selected);if(r==PP_OK)pp_ui_open(6);else notice(!pp_legend_unlocked(g)?"先集齐八枚徽章":r==PP_NOT_READY?"需清醒、健康30、精力3":pp_message(r));return;}
  if(cmd==720 || cmd==721){app_page^=1;pp_ui_open(1);return;}
  if(cmd==722 || cmd==723){unsigned pages=(bag_entries(NULL)+BAG_PAGE_SIZE-1)/BAG_PAGE_SIZE;if(!pages)pages=1;bag_page=(bag_page+(cmd==723?1:pages-1))%pages;pp_ui_open(2);return;}
  if(cmd>=730 && cmd<730+PP_LEGEND_COUNT){r=pp_legend_egg_use(g,cmd-730);if(r==PP_OK)pp_ui_open(0);notice(r==PP_OK?g->log[3]:pp_message(r));return;}
  if(cmd==655 || (cmd==6 && g->wild.phase>=PP_WILD_BATTLE)){pp_ui_open(g->wild.phase>=PP_WILD_WON?23:6);return;}

  if (cmd >= 201 && cmd <= 203) {
    starter_choice = (unsigned)(cmd - 200);
    for (unsigned i = 0; i < 3; i++) {
      bool active = i + 1 == starter_choice;
      lv_obj_set_style_border_color(starter_card[i], color(active ? 0xa7e6ac : 0x355568), 0);
      lv_obj_set_style_bg_color(starter_card[i], color(active ? 0x294a50 : 0x142d3e), 0);
    }
    text(starter_name, pp_pet_names[starter_choice]);
    return;
  }
  if (cmd == 210) {
    if (pp_choose_starter(g, 1 + (starter_choice - 1) * 3) == PP_OK) {
      starter_mode = false;
      pp_ui_open(0);
    }
    return;
  }
  if (cmd == 1 && current == 12) {pp_ui_open(8);return;}
  if(cmd==180 || cmd==184) {replacement_slot=cmd==184?g->count:selected;warehouse_swap=true;reserve_page=0;pp_ui_open(16);return;}
  if(cmd==181) {bool return_list=current==26&&info_from_warehouse;r=pp_reserve_pet(g,selected);if(r==PP_OK){home_remove_pet(selected);selected=0;pp_ui_open(return_list?16:0);}else notice(pp_message(r));return;}
  if(cmd==182 || cmd==183) {unsigned pages=(warehouse_view_count()+WAREHOUSE_PAGE_SIZE-1)/WAREHOUSE_PAGE_SIZE;if(pages)reserve_page=(reserve_page+(cmd==183?1:pages-1))%pages;pp_ui_open(16);return;}
  if(cmd>=400 && cmd<400+PP_RESERVES) {
    r=pp_replace_pet(g,replacement_slot,cmd-400);
    if(r==PP_OK){home_reset_pet(replacement_slot);selected=replacement_slot;pp_ui_open(0);}else replacement_failed(r);return;
  }
  if(cmd==185){pp_random_scene(g,!g->random_scene);pp_ui_open(11);return;}
  if(cmd==186){pp_ui_open(4);return;}
  if(cmd==6 && !g->battling) {pp_ui_open(17);return;}
  if(cmd==512) {gym_team_count=0;pp_ui_open(19);return;}
  if(cmd>=500 && cmd<500+PP_PETS) {
    unsigned p=cmd-500,i=0;
    if(p>=g->count)return;
    if(g->traveler==(int)p) {
      if(pp_travel_describe(g).ready && travel_present()) {pp_ui_open(5);return;}
      notice(pp_travel_describe(g).ready?"旅行结算中":"正在旅行");return;
    }
    if(g->drop_active && g->drop_target==p) {notice("正在使用道具");return;}
    while(i<gym_team_count && gym_team[i]!=p)i++;
    if(i<gym_team_count){for(unsigned j=i;j+1<gym_team_count;j++)gym_team[j]=gym_team[j+1];gym_team_count--;}
    else if(gym_team_count<3)gym_team[gym_team_count++]=p;
    else {notice("先移除一位，再选择伙伴");return;}
    pp_ui_open(19);return;
  }
  if(cmd==510) {
    if(pp_travel_describe(g).ready && travel_present()) {pp_ui_open(5);return;}
    r=gym_team_count==3?pp_gym_start_at(g,gym_selected,gym_team):PP_NEED_THREE;
    if(r==PP_OK){gym_waiting_wild=false;pp_ui_open(6);}
    else if(r==PP_BUSY && pp_wild_opened(g)){gym_waiting_wild=true;pp_ui_open(0);notice("请先处理野生伙伴");}
    else if(r==PP_BUSY && g->wild.phase>=PP_WILD_WON){gym_waiting_wild=true;pp_ui_open(23);notice("请先确认野外收获");}
    else if(r==PP_BUSY && g->battling){pp_ui_open(6);notice("继续未完成的战斗");}
    else if(r==PP_BUSY){bool trip=false;for(unsigned i=0;i<gym_team_count;i++)trip|=g->traveler==(int)gym_team[i];notice(trip?"伙伴正在旅行":"伙伴正在使用道具");}
    else notice(r==PP_NOT_READY?"需清醒、健康30、精力5":pp_message(r));
    return;
  }
  if(cmd>=540 && cmd<540+PP_GYM_COUNT) {gym_selected=cmd-540;pp_ui_open(17);return;}
  if(cmd==522) {if(!pp_gym_unlocked(g,gym_selected)){notice(gym_selected==7?"先获得深红徽章，再挑战坂木":gym_selected==6?"先获得金色徽章，再挑战夏伯":gym_selected==5?"先获得粉红徽章，再挑战娜姿":gym_selected==4?"先获得彩虹徽章，再挑战阿桔":gym_selected==3?"先获得橙色徽章，再挑战莉佳":gym_selected==2?"先获得蓝色徽章，再挑战马志士":"先获得灰色徽章，再挑战小霞");return;}pp_ui_open(18);return;}
  if(cmd>=530 && cmd<530+PP_GYM_COUNT) {gym_selected=cmd-530;pp_ui_open(20);return;}
  if(cmd==511) {pp_gym_leave(g);pp_ui_open(18);return;}
  if (cmd < 100) {
    if (cmd == 15) petdex_page = 0;
    if (cmd == 8) { dex_page=0; collection_quality=PP_QUALITY_ALL; }
    pp_ui_open(cmd);
    return;
  }
  if (cmd >= 100 && cmd < 110) {
    r = pp_buy(g, cmd - 100);
  } else if (cmd >= 120 && cmd < 130) {
    r = pp_use_auto(g, cmd - 120);
  } else if (cmd >= 140 && cmd < 150) {
    selected = cmd - 140;
    pp_ui_open(current);
    return;
  } else if (cmd == 160)
    r = pp_travel(g, selected);
  else if (cmd == 161) {
    if(travel_settle()){travel_notice_active=false;pp_ui_open(5);return;}
    pp_travel_status trip=pp_travel_describe(g);
    if(!trip.active){pp_ui_open(5);return;}
    if(trip.ready){notice(travel_wait_message());return;}
    r=PP_NOT_READY;
  }
  else if (cmd == 162) {
    if(g->gym.mode) {pp_ui_open(18);return;}
    r = pp_battle(g, selected);
    pp_ui_open(6);
  } else if (cmd == 163)
    r = PP_INVALID;
  else if (cmd == 164)
    r = pp_shake(g);
  else if (cmd == 165)
    r = pp_dig_stop(g);
  else if (cmd == 166)
    r = pp_tarot(g);
  else if(cmd==191){
    if(!training_level)return;
    g->training.growth_seen|=1u<<training_notice_index;g->revision++;
    pp_ui_open(training_notice_page);return;
  }
  else if (cmd == 167) {
    evolution_open(selected, false);
    return;
  } else if (cmd == 177) {
    evo_branch=0;
    evolution_open(selected, true);
    return;
  } else if (cmd>=187 && cmd<=189) {
    if(evolution && evo_choosing && evo_state()->pets[evo_pet_id].species==67){
      unsigned pet=evo_pick;lv_obj_delete(evolution);evolution=NULL;
      evo_branch=cmd-186;evolution_open(pet,true);
    }
    return;
  } else if (cmd == 190) {
    if(!evolution || !evo_choosing)return;
    /* A branch confirmation returns to the branch picker first. */
    if(evo_branch && evo_state()->pets[evo_pet_id].species==67) {
      unsigned pet=evo_pick;lv_obj_delete(evolution);evolution=NULL;
      evo_branch=0;evolution_open(pet,true);return;
    }
    if(evo_training){if(evo_pick<PP_PETS){evo_postponed_level[evo_pick]=g->growth[evo_pick].level;evo_postponed_art[evo_pick]=pet_art(evo_pick);}g->training.evolution_seen|=1u<<training_notice_index;g->revision++;pp_ui_open(training_notice_page);return;}
    selected=evo_pet_id;
    evo_postponed_level[selected]=g->growth[selected].level;
    evo_postponed_art[selected]=pet_art(selected);
    pp_ui_open(evo_return_page);
    return;
  } else if (cmd == 178 || cmd == 179) {
    evolution_choose(cmd == 179);
    return;
  } else if (cmd == 168) {
    if (training_level || !evolution || evo_choosing || !evo_done || lv_obj_has_flag(evo_done, LV_OBJ_FLAG_HIDDEN)) return;
    if(evo_training){
      bool available=pp_roster_evolution_preview(g,evo_pick,&evo_preview)&&pp_evolution_offer(&evo_preview,0)==PP_OK;
      if(!available)g->training.evolution_seen|=1u<<training_notice_index;
      g->revision++;pp_ui_open(training_notice_page);return;
    }
    selected = evo_pet_id;
    pp_ui_open(4);
    return;
  } else if (cmd == 170 || cmd == 171) {
    unsigned pages=collection_pages();
    dex_page = (dex_page + (cmd == 171 ? 1 : pages-1)) % pages;
    pp_ui_open(8);
    return;
  } else if (cmd >= 300 && cmd <= 300 + PP_QUALITY_COUNT) {
    collection_quality=cmd-300;dex_page=0;pp_ui_open(8);return;
  } else if (cmd == 172 || cmd == 173) {
    scene_page = (scene_page + (cmd == 173 ? 1 : PP_SCENES-1)) % PP_SCENES;
    pp_ui_open(9);
    return;
  } else if (cmd == 175 || cmd == 176) {
    unsigned pages=(PP_DEX_COUNT+PETDEX_PAGE_SIZE-1)/PETDEX_PAGE_SIZE;
    petdex_page=(petdex_page+(cmd==176?1:pages-1))%pages;
    pp_ui_open(15);
    return;
  } else if (cmd == 174) {
    r = pp_scene(g, scene_page);
    if(r==PP_OK){pp_ui_open(0);return;}
  } else if (cmd >= 1000 && cmd < 1000 + PP_COLLECTION) {
    selected = cmd - 1000;
    pp_ui_open(12);
    return;
  }
  if (r == PP_OK && cmd == 120 + PP_HATCH_BOOST) {
    if(g->count==previous_count)hatch_result_capture(true,pp_warehouse_count(g)-1);
    else hatch_result_capture(false,g->count-1);
    pp_ui_open(30);
    return;
  }
  pp_ui_refresh();
  if (r == PP_OK && (cmd >= 120 && cmd < 130)) {
    pp_ui_open(0);
  } else if (current != 0 && current != 5 && current != 6 && current != 7)
    pp_ui_open(current);
  if (r == PP_OK && cmd >= 100 && cmd < 100 + PP_SHOP_ITEMS) {
    char message[64];
    snprintf(message,sizeof(message),"购买%sx1",pp_items[cmd-100].name);
    notice(message);
  } else if (r == PP_OK && cmd == 161) notice(g->log[3]);
  else if (r == PP_BUSY && cmd>=120 && cmd<125)
    notice(g->drop_active?"正在使用道具":g->battling?"战斗中":"伙伴正在旅行");
  else if (r != PP_OK || cmd != 162) notice(pp_message(r));
}
static void pet_picker(int y) {
  int start = 180 - (int)g->count * 25;
  for (unsigned i = 0; i < g->count; i++) {
    lv_obj_t *b = button(body, selected == i ? "●" : "", start + i * 50, y, 46,
                         64, 140 + i);
    if (selected == i)
      lv_obj_set_style_border_color(b, color(0x9cdc98), 0);
    lv_obj_t *im = lv_image_create(b);
    pp_image_set_src(im, avatar_paths[pet_art(i)]);
    lv_obj_set_pos(im, 3, 2);
    lv_obj_t *l = lv_obj_get_child(b, 0);
    lv_obj_align(l, LV_ALIGN_BOTTOM_MID, 0, -5);
  }
  label(body, pp_form_name(g, selected), 65, y + 76, 230, 0x9cbeb2);
}


/* Legendary menus use existing sprites and static, bounded LVGL geometry. */
static const unsigned legend_tints[PP_LEGEND_COUNT]={0x8cdbef,0xf2cf70,0xf2a173,0xbda5ed,0xf0b0d1};
static const unsigned legend_fills[PP_LEGEND_COUNT]={0x182c3b,0x302b28,0x33252d,0x29243d,0x30243a};
static const char *const legend_types[PP_LEGEND_COUNT]={"冰 / 飞行","电 / 飞行","火 / 飞行","超能","超能 · 幻之宝可梦"};
static lv_obj_t *legend_shape(lv_obj_t *parent,int x,int y,int w,int h,unsigned tint,int radius) {
  lv_obj_t *o=lv_obj_create(parent);lv_obj_remove_style_all(o);
  lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
  lv_obj_set_style_bg_color(o,color(tint),0);lv_obj_set_style_bg_opa(o,LV_OPA_COVER,0);
  lv_obj_set_style_radius(o,radius,0);
  lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  return o;
}
static lv_obj_t *legend_ring(lv_obj_t *parent,int x,int y,int size,unsigned tint) {
  lv_obj_t *o=legend_shape(parent,x,y,size,size,tint,LV_RADIUS_CIRCLE);
  lv_obj_set_style_bg_opa(o,LV_OPA_TRANSP,0);lv_obj_set_style_border_width(o,1,0);
  lv_obj_set_style_border_color(o,color(tint),0);return o;
}
static void legend_star(lv_obj_t *parent,int x,int y,unsigned tint) {
  legend_shape(parent,x+2,y,2,6,tint,0);legend_shape(parent,x,y+2,6,2,tint,0);
}
static void legend_lock(lv_obj_t *parent,int x,int y,unsigned tint) {
  lv_obj_t *o=legend_shape(parent,x+2,y,6,6,tint,2);
  lv_obj_set_style_bg_opa(o,LV_OPA_TRANSP,0);lv_obj_set_style_border_width(o,2,0);lv_obj_set_style_border_color(o,color(tint),0);
  legend_shape(parent,x,y+5,10,7,tint,1);legend_shape(parent,x+4,y+7,2,3,0x151a2b,0);
}
static void legend_theme(void) {
  if(current!=24 && current!=25) {
  lv_obj_set_style_bg_color(screen,color(0x0c101d),0);
  lv_obj_set_style_bg_grad_color(screen,color(0x19192c),0);lv_obj_set_style_bg_grad_dir(screen,LV_GRAD_DIR_VER,0);
  lv_obj_t *ring=legend_ring(screen,53,49,254,0x22283b);lv_obj_move_to_index(ring,0);
  ring=legend_ring(screen,65,61,230,0x1d2336);lv_obj_move_to_index(ring,0);
  static const int stars[][2]={{80,83},{275,91},{47,179},{309,174},{79,268},{276,264}};
  for(unsigned i=0;i<6;i++){
    lv_obj_t *dot=legend_shape(screen,stars[i][0],stars[i][1],2,2,i%2?0x766d85:0x53647b,0);lv_obj_move_to_index(dot,0);
  }
  legend_shape(screen,109,48,24,1,0x6b5f55,0);legend_shape(screen,227,48,24,1,0x6b5f55,0);
  }
  legend_star(screen,100,45,0xd9bd8b);legend_star(screen,254,45,0xd9bd8b);
  lv_obj_set_style_text_color(title,color(0xefe0b9),0);
  lv_obj_set_style_text_color(subtitle,color(0x9e9cb7),0);
  lv_obj_set_style_bg_color(back_button,color(0x282840),0);lv_obj_set_style_bg_grad_dir(back_button,LV_GRAD_DIR_NONE,0);
  lv_obj_set_style_border_color(back_button,color(0x8880a3),0);lv_obj_set_style_border_width(back_button,1,0);
  lv_obj_set_style_bg_color(back_button,color(0x49415f),LV_STATE_PRESSED);
}
static void legend_list_view(void) {
  legend_theme();bool unlocked=pp_legend_unlocked(g);unsigned badges=0;char s[64];
  text(subtitle,unlocked?"选择神兽 · 胜利获得专属蛋":"集齐八枚徽章，开启神兽挑战");
  static const unsigned badge_colors[]={0xc3bda8,0x92cbe9,0xe8b06d,0xd3a4dd,0xecacc1,0xead279,0xe6938a,0x9dcaac};
  for(unsigned i=0;i<PP_GYM_COUNT;i++) {
    bool owned=(g->gym.badges>>i)&1;badges+=owned;
    legend_shape(screen,111+i*14,83,10,4,owned?badge_colors[i]:0x34374b,1);
  }
  snprintf(s,sizeof(s),"%u/8",badges);lv_obj_t *l=label(screen,s,229,77,34,0xc6b79b);lv_obj_set_style_text_font(l,&pp_font_12,0);
  for(unsigned i=0;i<PP_LEGEND_COUNT;i++) {
    int x=i<3?47+i*92:93+(i-3)*92,y=i<3?22:116;
    unsigned tint=unlocked?legend_tints[i]:0x71758d;
    lv_obj_t *card=button(body,"",x,y,82,86,700+i);
    lv_obj_set_style_radius(card,12,0);lv_obj_set_style_bg_color(card,color(unlocked?legend_fills[i]:0x202435),0);
    lv_obj_set_style_bg_grad_color(card,color(0x171d2e),0);lv_obj_set_style_bg_grad_dir(card,LV_GRAD_DIR_VER,0);
    lv_obj_set_style_border_color(card,color(unlocked?legend_tints[i]:0x42485d),0);
    lv_obj_set_style_border_opa(card,unlocked?155:150,0);
    lv_obj_set_style_bg_color(card,color(legend_fills[i]),LV_STATE_PRESSED);
    lv_obj_set_style_border_opa(card,255,LV_STATE_PRESSED);lv_obj_set_style_border_width(card,2,LV_STATE_PRESSED);
    legend_shape(card,31,0,20,2,tint,0);
    lv_obj_t *halo=legend_shape(card,18,7,46,46,tint,LV_RADIUS_CIRCLE);lv_obj_set_style_bg_opa(halo,18,0);
    lv_obj_t *im=lv_image_create(card);pp_image_set_src(im,paths[pp_pokedex[pp_legend_dex(i)].art][0]);lv_obj_set_pos(im,17,5);
    if(!unlocked){lv_obj_set_style_image_recolor(im,color(0x6e728e),0);lv_obj_set_style_image_recolor_opa(im,180,0);legend_lock(card,64,7,0x9b93b0);}
    else if((g->legend.defeated>>i)&1){
      legend_shape(card,65,10,3,6,0xc7dca4,0);legend_shape(card,68,13,3,3,0xc7dca4,0);legend_shape(card,71,7,3,6,0xc7dca4,0);
    }
    label(card,pp_legends[i].name,1,51,80,unlocked?0xe7e5e6:0xa8a8bc);
    l=label(card,unlocked?((g->legend.defeated>>i)&1?"已胜利":"Lv.60"):"未解锁",1,69,80,tint);lv_obj_set_style_text_font(l,&pp_font_12,0);
  }
}
static void legend_detail_view(void) {
  legend_theme();unsigned i=legend_selected,tint=legend_tints[i];bool unlocked=pp_legend_unlocked(g);char s[96];
  text(title,pp_legends[i].name);
  snprintf(s,sizeof(s),"Lv.60 · %s",legend_types[i]);text(subtitle,unlocked?s:"集齐八枚徽章后开放");
  lv_obj_t *panel=legend_shape(body,67,10,226,86,legend_fills[i],14);
  lv_obj_set_style_bg_grad_color(panel,color(0x191d30),0);lv_obj_set_style_bg_grad_dir(panel,LV_GRAD_DIR_HOR,0);
  lv_obj_set_style_border_width(panel,1,0);lv_obj_set_style_border_color(panel,color(0x4c455e),0);
  legend_ring(panel,13,8,70,0x5c586f);legend_ring(panel,18,13,60,tint);
  lv_obj_t *im=lv_image_create(panel);pp_image_set_src(im,paths[pp_pokedex[pp_legend_dex(i)].art][0]);lv_obj_set_pos(im,24,19);
  lv_obj_t *l=label(panel,"胜利奖励",95,7,120,0xb2a8b9);lv_obj_set_style_text_font(l,&pp_font_12,0);
  im=lv_image_create(panel);pp_image_set_src(im,icon_paths[5]);lv_obj_set_pos(im,139,26);
  lv_obj_set_style_image_recolor(im,color(tint),0);lv_obj_set_style_image_recolor_opa(im,80,0);
  snprintf(s,sizeof(s),"%s的蛋",pp_legends[i].name);l=label(panel,s,88,61,132,0xead9b7);lv_obj_set_style_text_font(l,&pp_font_12,0);
  legend_star(panel,104,32,0x83717f);legend_star(panel,194,40,0x83717f);
  l=label(body,"选择出战伙伴",91,101,178,0xb3acbf);lv_obj_set_style_text_font(l,&pp_font_12,0);
  legend_shape(body,62,108,28,1,0x4c425d,0);legend_shape(body,270,108,28,1,0x4c425d,0);
  int start=180-(int)g->count*25;
  for(unsigned pet=0;pet<g->count;pet++) {
    bool active=selected==pet;lv_obj_t *b=button(body,"",start+pet*50,120,46,44,140+pet);
    lv_obj_set_style_radius(b,9,0);lv_obj_set_style_bg_color(b,color(active?0x3e3348:0x1b2132),0);lv_obj_set_style_bg_grad_dir(b,LV_GRAD_DIR_NONE,0);
    lv_obj_set_style_border_color(b,color(active?0xe7c98d:0x45445c),0);lv_obj_set_style_border_width(b,active?2:1,0);
    im=lv_image_create(b);pp_image_set_src(im,avatar_paths[pet_art(pet)]);lv_obj_set_pos(im,3,2);
    if(active)legend_shape(b,18,39,10,2,0xe7c98d,0);
  }
  snprintf(s,sizeof(s),"%s · Lv.%u",pp_pet_name(g,selected),g->growth[selected].level);l=label(body,s,55,166,250,0xc5bbce);lv_obj_set_style_text_font(l,&pp_font_12,0);
  lv_obj_t *b=button(screen,unlocked?"开始挑战":"尚未解锁",116,262,128,28,710);
  lv_obj_set_style_radius(b,10,0);lv_obj_set_style_bg_color(b,color(0x53405c),0);lv_obj_set_style_bg_grad_dir(b,LV_GRAD_DIR_NONE,0);
  lv_obj_set_style_border_color(b,color(0xd8ba87),0);lv_obj_set_style_text_color(lv_obj_get_child(b,0),color(0xffe9b8),0);
  lv_obj_set_style_bg_color(b,color(0x77607f),LV_STATE_PRESSED);
  if(!unlocked){lv_obj_add_state(b,LV_STATE_DISABLED);lv_obj_set_style_bg_color(b,color(0x25283b),LV_STATE_DISABLED);lv_obj_set_style_border_color(b,color(0x58566d),LV_STATE_DISABLED);}
}
static void legend_result_view(void) {
  legend_theme();unsigned i=g->legend.active-1,tint=legend_tints[i];bool won=g->wild.phase==PP_WILD_WON;char s[96];
  text(title,won?"挑战成功":"挑战结束");text(subtitle,won?"神兽蛋已存入背包":"照顾好伙伴，再来挑战");
  legend_ring(body,133,20,94,0x454058);legend_ring(body,140,27,80,tint);
  lv_obj_t *halo=legend_shape(body,146,33,68,68,tint,LV_RADIUS_CIRCLE);lv_obj_set_style_bg_opa(halo,14,0);
  lv_obj_t *im=lv_image_create(body);pp_image_set_src(im,paths[pp_pokedex[pp_legend_dex(i)].art][0]);lv_obj_set_pos(im,156,43);
  legend_star(body,115,48,tint);legend_star(body,239,61,tint);legend_star(body,203,14,0xd3b57f);
  snprintf(s,sizeof(s),"%s · Lv.60",pp_legends[i].name);label(body,s,60,116,240,0xe8d9bf);
  lv_obj_t *panel=legend_shape(body,73,143,214,48,0x27253b,12);
  lv_obj_set_style_border_color(panel,color(won?0x8f7e66:0x4c4b62),0);lv_obj_set_style_border_width(panel,1,0);
  if(won) {
    im=lv_image_create(panel);pp_image_set_src(im,icon_paths[5]);lv_obj_set_pos(im,12,8);
    lv_obj_set_style_image_recolor(im,color(tint),0);lv_obj_set_style_image_recolor_opa(im,80,0);
    snprintf(s,sizeof(s),"%s的蛋",pp_legends[i].name);label(panel,s,49,4,155,0xf0dfba);
    snprintf(s,sizeof(s),"背包 ×%u",g->legend.eggs[i]);lv_obj_t *l=label(panel,s,49,26,155,0xa9b5bc);lv_obj_set_style_text_font(l,&pp_font_12,0);
  }else label(panel,"本次未获得神兽蛋",10,15,194,0xb5b0c2);
  lv_obj_t *l=label(body,won?"背包选择 · 庭院孵化":"调整伙伴，再次挑战",55,197,250,0x9d96ac);lv_obj_set_style_text_font(l,&pp_font_12,0);
}

/* Static menus reuse resident icons and native scenery. */
static void menu_theme(void) {
  for(unsigned side=0;side<2;side++) {
    legend_shape(screen,side?240:100,48,20,1,0x527a7c,0);
    legend_shape(screen,side?266:92,47,2,3,0xb9cdb5,1);
  }
  lv_obj_set_style_text_color(title,color(0xf2e6cd),0);
  lv_obj_set_style_text_font(subtitle,&pp_font_12,0);
  lv_obj_set_style_text_color(subtitle,color(0x94b5b4),0);
  lv_obj_set_style_bg_color(back_button,color(0x1a4149),0);
  lv_obj_set_style_border_color(back_button,color(0x6b9f9e),0);
  lv_obj_set_style_border_width(back_button,1,0);
}
static const void *training_frame(unsigned art,unsigned pose,uint32_t elapsed,bool left){unsigned frame=pp_animation_frame(art,pose,elapsed);return left?left_paths[art][frame]:paths[art][frame];}
static const void *fishing_frame(unsigned art,unsigned pose,uint32_t elapsed){return paths[art][pp_animation_frame(art,pose,elapsed)];}
static void fishing_leave(void){pp_ui_open(1);}
static void app_menu_view(void) {
  menu_theme();text(title,"APP");
  text(subtitle,"");
  const char *names[]={"旅行","历练","道馆","仓库","场景","钓鱼","计步器","塔罗牌","宠物医院","徽章","神兽","设置","宠物图鉴","收藏品"};
  const int pages[]={5,33,6,16,9,31,14,10,32,20,24,11,15,8};
  const unsigned ids[]={8,9,9,5,11,0,8,12,10,10,10,13,10,10};
  const unsigned accents[]={0x81cec0,0x9edbbf,0xe9bd85,0xa8d8c5,0x8dd3bd,0x8bd6d8,0x91d4be,0xe9b6c7,0x9fdbc1,0xe8d7a2,0xc8b5ed,0x9cbce6,0xa4c5e8,0xf0ce89};
  const unsigned fills[]={0x244c48,0x284d43,0x493d30,0x284d43,0x244b48,0x25525c,0x284b43,0x4a3547,0x28534b,0x474435,0x3c3554,0x2b4055,0x2a4152,0x493f31};
  for(unsigned i=app_page*9;i<14 && i<app_page*9+9;i++) {
    unsigned slot=i%9;unsigned row=slot/3,cols=app_page&&row==1?2:3;int x=app_page?(360-(int)cols*84+20)/2+(int)(slot%3)*84:74+(int)(slot%3)*74,y=app_page?104+(int)row*84:68+(int)(slot/3)*72;
    lv_obj_t *tile=button(screen,"",x,y,64,64,pages[i]);lv_obj_clean(tile);
    lv_obj_set_style_radius(tile,16,0);lv_obj_set_style_bg_color(tile,color(fills[i]),0);
    lv_obj_set_style_bg_grad_color(tile,color(0x142e39),0);lv_obj_set_style_bg_grad_dir(tile,LV_GRAD_DIR_VER,0);
    lv_obj_set_style_border_color(tile,color(accents[i]),0);lv_obj_set_style_border_opa(tile,70,0);
    lv_obj_set_style_bg_color(tile,color(fills[i]),LV_STATE_PRESSED);
    lv_obj_set_style_border_opa(tile,180,LV_STATE_PRESSED);
    lv_obj_t *gleam=legend_shape(tile,18,1,28,1,accents[i],0);lv_obj_set_style_bg_opa(gleam,80,0);
    lv_obj_t *im=lv_image_create(tile);
    pp_image_set_src(im,pages[i]==33?training_icon_path:pages[i]==32?hospital_icon_path:pages[i]==31?fishing_icon_path:pages[i]==6?gym_icon_path:pages[i]==16?warehouse_icon_path:pages[i]==9?scene_icon_path:pages[i]==14?step_icon_path:pages[i]==15?pokedex_icon_path:pages[i]==20?gym_paths[3]:pages[i]==24?avatar_paths[pp_pokedex[pp_legend_dex(3)].art]:icon_paths[ids[i]]);
    lv_obj_align(im,LV_ALIGN_TOP_MID,0,pages[i]==24?3:7);lv_obj_remove_flag(im,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *l=label(tile,names[i],0,46,64,0xe5e9e2);lv_obj_set_style_text_font(l,&pp_font_12,0);
    if(pages[i]==24 && !pp_legend_unlocked(g))legend_lock(tile,49,5,0xa6a0b8);
  }
  for(unsigned i=0;i<2;i++){int w=i==app_page?12:4;legend_shape(screen,171+(int)i*18-w/2,285,w,4,i==app_page?0xbadcc4:0x4d6c74,2);}
  switch_arrow(screen,0,720,false);switch_arrow(screen,0,721,true);
}
static void scene_menu_view(void) {
  menu_theme();char s[80];text(subtitle,pp_scene_names[scene_page]);
  lv_obj_set_pos(subtitle,60,70);lv_obj_set_width(subtitle,240);
  lv_obj_set_style_text_font(subtitle,&pp_font_18,0);
  lv_obj_set_style_text_color(subtitle,color(0xf3e7cc),0);
  bool owned=(g->scenes&(1u<<scene_page))!=0,active=g->scene==scene_page;
  /* A native, unscaled crop reaches the panel arc. Equal 108px margins keep
   * the 144px panorama centered vertically, with controls mirrored around it. */
  lv_obj_t *window=lv_obj_create(screen);lv_obj_remove_style_all(window);
  lv_obj_set_pos(window,0,108);lv_obj_set_size(window,360,144);
  lv_obj_remove_flag(window,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t *im=lv_image_create(window);pp_image_set_src(im,scene_paths[scene_page]);
  lv_obj_set_pos(im,0,-108);lv_obj_remove_flag(im,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  for(unsigned side=0;side<2;side++)
    switch_arrow(screen,0,side?173:172,side);
  lv_obj_t *state=legend_shape(screen,102,226,156,20,0x102b34,10);
  lv_obj_set_style_bg_opa(state,218,0);
  snprintf(s,sizeof(s),"%02u / %02u · %s",scene_page+1,PP_SCENES,active?"使用中":owned?"已解锁":"未解锁");
  lv_obj_t *l=label(state,s,0,2,156,active?0xc4e9c5:owned?0xb4d9d4:0xf0d69b);lv_obj_set_style_text_font(l,&pp_font_12,0);
  unsigned required=pp_scene_required(scene_page);
  if(active)snprintf(s,sizeof(s),"使用中");else if(owned)snprintf(s,sizeof(s),"使用");else snprintf(s,sizeof(s),"需解锁%u种宠物",required);
  lv_obj_t *apply=button(screen,s,88,264,184,28,174);
  lv_obj_set_style_bg_color(apply,color(owned?0x326d60:0x685532),0);lv_obj_set_style_bg_grad_dir(apply,LV_GRAD_DIR_NONE,0);
  lv_obj_set_style_border_color(apply,color(owned?0x84bba3:0xc5a56b),0);
  if(active||!owned){lv_obj_add_state(apply,LV_STATE_DISABLED);lv_obj_set_style_opa(apply,120,LV_STATE_DISABLED);}

}

static const void *intro_frame(unsigned actor,unsigned pose,unsigned elapsed,bool left) {
  unsigned art=pp_base_art(actor+1),frame=pp_animation_frame(art,pose,elapsed);
  return left?left_paths[art][frame]:paths[art][frame];
}

/* Normalize the bottom dock after page-specific content and themes. */
static void footer_scan(lv_obj_t *parent,lv_obj_t **primary,unsigned *count) {
  for(unsigned i=0;i<lv_obj_get_child_count(parent);i++) {
    lv_obj_t *b=lv_obj_get_child(parent,i);
    if(lv_obj_check_type(b,&lv_button_class) && b!=back_button && b!=again && lv_obj_get_child_count(b)) {
      lv_obj_t *l=lv_obj_get_child(b,0);lv_area_t a;lv_obj_get_coords(b,&a);
      if(lv_obj_check_type(l,&lv_label_class) && lv_label_get_text(l)[0] && lv_obj_get_width(b)>=46 && a.y1>=240 && lv_obj_get_height(b)<=40) {
        if(a.y1<294 && current!=27) {
          lv_area_t parent_area;lv_obj_get_coords(parent,&parent_area);
          lv_obj_set_y(b,260-parent_area.y1);lv_obj_set_height(b,32);
        }
        pp_footer_text_style(b);
        if(a.y1<294){*primary=b;(*count)++;}
      }
    }
    footer_scan(b,primary,count);
  }
}
static void footer_normalize(unsigned page) {
  if(back_button) {
    lv_obj_t *l=lv_obj_get_child(back_button,0);
    if(!strcmp(lv_label_get_text(l),"完成"))pp_footer_done(back_button);
    else pp_footer_back(back_button);
  }
  lv_obj_update_layout(screen);lv_obj_t *primary=NULL;unsigned count=0;
  footer_scan(screen,&primary,&count);
  if(count==1 && page!=23 && page!=27 && page!=30) {
    lv_point_t measured;lv_text_get_size(&measured,lv_label_get_text(lv_obj_get_child(primary,0)),&pp_font_14,0,0,LV_COORD_MAX,LV_TEXT_FLAG_NONE);
    int width=measured.x>112?184:128;
    lv_area_t parent;lv_obj_get_coords(lv_obj_get_parent(primary),&parent);
    lv_obj_set_pos(primary,(360-width)/2-parent.x1,260-parent.y1);
    lv_obj_set_size(primary,width,32);pp_footer_text_style(primary);
  }
  if(travel_claim){lv_obj_set_pos(travel_claim,116,260);lv_obj_set_size(travel_claim,128,32);pp_footer_text_style(travel_claim);}
  if(again){lv_obj_set_pos(again,210,296);lv_obj_set_size(again,52,32);pp_footer_text_style(again);}
}
/* One identity card for courtyard status and stored individual details. */
static void pet_identity_card(lv_obj_t *parent,unsigned art,const char *pet_name,const pp_growth *growth,bool open_details) {
  char s[80];
  lv_obj_t *identity=lv_obj_create(parent);lv_obj_remove_style_all(identity);
  lv_obj_set_pos(identity,64,0);lv_obj_set_size(identity,232,90);
  lv_obj_set_style_radius(identity,18,0);
  lv_obj_set_style_bg_color(identity,color(0x132630),0);
  lv_obj_set_style_bg_opa(identity,LV_OPA_COVER,0);
  lv_obj_set_style_border_color(identity,color(0x2a4650),0);
  lv_obj_set_style_border_width(identity,1,0);
  lv_obj_remove_flag(identity,LV_OBJ_FLAG_SCROLLABLE);
  if(open_details){lv_obj_add_flag(identity,LV_OBJ_FLAG_CLICKABLE);lv_obj_add_event_cb(identity,action,LV_EVENT_CLICKED,(void *)(intptr_t)760);}
  else lv_obj_remove_flag(identity,LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t *portrait_pad=lv_obj_create(identity);lv_obj_remove_style_all(portrait_pad);
  lv_obj_set_pos(portrait_pad,12,8);lv_obj_set_size(portrait_pad,52,52);
  lv_obj_set_style_radius(portrait_pad,15,0);
  lv_obj_set_style_bg_color(portrait_pad,color(0x244044),0);
  lv_obj_set_style_bg_opa(portrait_pad,LV_OPA_COVER,0);
  lv_obj_remove_flag(portrait_pad,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t *portrait=lv_image_create(portrait_pad);
  pp_image_set_src(portrait,paths[art][0]);
  lv_obj_set_pos(portrait,2,2);
  lv_obj_t *name=label(identity,pet_name,80,8,128,0xe4eee4);
  lv_obj_set_style_text_font(name,&pp_font_18,0);
  lv_label_set_long_mode(name,LV_LABEL_LONG_DOT);lv_obj_set_height(name,24);
  lv_obj_set_style_text_align(name,LV_TEXT_ALIGN_LEFT,0);
  if(open_details){lv_obj_t *more=label(identity,LV_SYMBOL_RIGHT,214,12,14,0x8cb7aa);lv_obj_set_style_text_font(more,&lv_font_montserrat_14,0);}
  snprintf(s,sizeof(s),"Lv.%u",growth->level);
  lv_obj_t *level=label(identity,s,80,34,44,0xb6dfcc);
  lv_obj_set_style_text_font(level,&pp_font_12,0);
  lv_obj_set_style_bg_color(level,color(0x25483f),0);
  lv_obj_set_style_bg_opa(level,LV_OPA_COVER,0);
  lv_obj_set_style_radius(level,6,0);
  lv_obj_set_style_pad_ver(level,2,0);
  snprintf(s,sizeof(s),"%u / %u",growth->xp,pp_xp_needed(growth->level));
  lv_obj_t *xp=label(identity,s,128,36,88,0x8aa6ac);
  lv_obj_set_style_text_font(xp,&pp_font_12,0);
  lv_obj_set_style_text_align(xp,LV_TEXT_ALIGN_RIGHT,0);
  lv_obj_t *bar=lv_bar_create(identity);
  lv_obj_set_pos(bar,80,58);lv_obj_set_size(bar,136,4);
  lv_bar_set_range(bar,0,pp_xp_needed(growth->level));
  lv_bar_set_value(bar,growth->xp,LV_ANIM_OFF);
  lv_obj_set_style_bg_color(bar,color(0x29414b),0);
  lv_obj_set_style_bg_color(bar,color(0xc2d69b),LV_PART_INDICATOR);
  pp_pet_type_badges_create(identity,art,80,68);
}

void pp_ui_open(unsigned page) {
  if(shop_reel.active && page!=3)return;
  if (page == 7) page=1;
  if (page > 33)
    page = 0;
  if (starter_mode && page != 11)
    page = intro_seen ? 13 : 22;
  if(page==11 && current!=11)settings_section=0;
  if (selected >= g->count && page != 12)
    selected = 0;
  /* The newborn already belongs to the roster/reserve. Leaving its reveal
   * (or replacement picker) dismisses only the placement choice. A hidden
   * choice must not block travel forever or suppress the next timed hatch.
   * A hatch queued behind a travel panel has not been shown yet: retain it. */
  if(hatch_result.active && (current==30 || (current==29 && hatch_replacing)) &&
     page!=30 && page!=5 && !(page==29 && hatch_replacing)) {
    hatch_result.active=false;hatch_replacing=false;
    egg_present_seen=g->egg_end!=0;
    roster_count_seen=g->count;warehouse_count_seen=pp_warehouse_count(g);
  }
  if(page==5)travel_home_deferred=false;
  if(!travel_home_deferred && (travel_notice_active || travel_receipt.active || (travel_page_safe(page) && travel_present())))page=5;
  evolution = NULL;training_level=evo_training=false;
  back_button = NULL;
  evo_choosing = false;
  if(page==21 && g->gym.mode)gym_selected=g->gym.mode-1;
  if(page==9 && current!=9)scene_page=g->scene;
  if(page==2 && current!=2)bag_page=0;
  unsigned previous_page=current;uint32_t real_now=lv_tick_get();
  if(previous_page==0 && page!=0){home_left_at=real_now;home_away=true;pp_home_behavior_clear(&home_behavior);}
  if(home_conflict.active)home_conflict_release(home_time);
  pp_home_conflict_pause(&home_conflict,home_time);home_conflict_particles=0;
  if(page==0 && previous_page!=0 && home_away && real_now-home_left_at>=60000 &&
     (!home_has_welcomed || real_now-home_welcomed_at>=120000))home_welcome_pending=true;
  if(page==0)home_away=false;
  current = page;
  gym_clear_stamp = NULL;
  gym_guards = NULL;
  if (page != 5 && !travel_home_deferred) travel_receipt.active = false;
  refresh_pending = false;
  pp_battle_view_close();
  pp_intro_view_close();
  gift_overlay=NULL;
  wild_overlay=wild_fighter=wild_fight_action=wild_resume=NULL;
  wild_alert=wild_alert_pet=wild_alert_mark=NULL;wild_alert_src=NULL;
  memset(&wild_result,0,sizeof(wild_result));
  memset(&travel_result,0,sizeof(travel_result));
  memset(&hatch_reveal,0,sizeof(hatch_reveal));
  travel_layout=false;travel_backdrop=travel_claim=travel_activity=travel_reward_label=NULL;
  travel_motion=(typeof(travel_motion)){.last=lv_tick_get()};
  battery_group=battery_outline=battery_cap=battery_fill=battery_value=NULL;
  battery_shown_width=-1;battery_charge_tick=lv_tick_get();
  memset(shop_cards,0,sizeof(shop_cards));shop_lit=shop_glow=-1;
  lv_obj_clean(screen);
  home_tree_front=NULL;memset(home_symbols,0,sizeof(home_symbols));memset(home_symbol_kind,0,sizeof(home_symbol_kind));
  memset(pet_img, 0, sizeof(pet_img));
  memset(pet_src, 0, sizeof(pet_src));
  memset(activity_pet, 0, sizeof(activity_pet));
  memset(activity_src, 0, sizeof(activity_src));
  last_scene = 999;
  again = NULL;
  lv_obj_set_style_bg_color(screen, color(0x080e17), 0);
  lv_obj_set_style_bg_grad_color(screen, color(0x050a11), 0);
  lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
  /* The physical panel clips the circle. Fill the entire framebuffer so
   * software corner antialiasing cannot expose LVGL's default white layer. */
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(screen, 0, 0);
  lv_obj_set_style_clip_corner(screen, false, 0);
  lv_obj_set_style_border_width(screen, 0, 0);
  lv_obj_set_style_outline_width(screen, 0, 0);
  lv_obj_set_style_shadow_width(screen, 0, 0);
  lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
  lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  if (page == 33) {
    pp_training_view_open(screen,g,asset_root,training_frame,fishing_leave);body=screen;
  } else if (page == 32) {
    pp_hospital_view_open(screen,g,asset_root,fishing_frame,fishing_leave);body=screen;
  } else if (page == 31) {
    pp_fishing_view_open(screen,g,asset_root,fishing_frame,fishing_leave);body=screen;
  } else if (page == 22) {
    const void *types[3]={type_paths[0],type_paths[1],type_paths[2]};
    pp_intro_view_open(screen,scene_paths[0],types,intro_frame,intro_finish);
    body=screen;
  } else if (page == 13) {
    lv_obj_t *heading = label(screen,"选择伙伴",80,48,200,0xe4eee9);
    lv_obj_set_style_text_font(heading,&pp_font_title_22,0);
    label(screen,"选一位伙伴，一起开始冒险吧",46,85,268,0x8fa9b7);
    for (unsigned i = 0; i < 3; i++) {
      int x = 30 + (int)i * 102;
      starter_card[i] = button(screen,"",x,112,96,136,201+i);
      lv_obj_set_style_border_width(starter_card[i],2,0);
      lv_obj_t *element = lv_image_create(starter_card[i]);
      pp_image_set_src(element,type_paths[i]);
      lv_obj_set_pos(element,32,8);
      lv_obj_t *portrait = lv_image_create(starter_card[i]);
      pp_image_set_src(portrait,starter_paths[i]);
      lv_obj_set_pos(portrait,8,46);
    }
    starter_choice = 1;
    lv_obj_set_style_border_color(starter_card[0],color(0xa7e6ac),0);
    lv_obj_set_style_bg_color(starter_card[0],color(0x294a50),0);
    starter_name = label(screen,pp_pet_names[1],70,252,220,0xcbe3d3);
    lv_obj_set_style_text_font(starter_name,&pp_font_18,0);
    lv_obj_t *start = button(screen,"出发",126,286,108,38,210);
    lv_obj_set_style_text_font(lv_obj_get_child(start,0),&pp_font_18,0);pp_footer_done(start);
    body = screen;
  } else if (page == 0) {
    home_real_tick=lv_tick_get();
    if(!home_initialized) {next_talk=home_time+18000;talk_until=0;}
    if(!home_initialized || home_fx_scene!=g->scene) {
      if(!home_initialized)memset(home_motion,0,sizeof(home_motion));
      else for(unsigned i=0;i<PP_PETS;i++)if(home_motion[i].initialized){
        home_motion_state *m=&home_motion[i];
        pp_home_ground_project(g->scene,&m->x,&m->y);
        m->origin_x=m->x;m->origin_y=m->y;m->started=home_time;
        m->interacting=false;
        if(g->scene==PP_MIRROR_LAKE_SCENE)m->started-=pp_lake_nearest_phase(m->x,m->y);
      }
      home_epoch=home_time;home_fx_scene=g->scene;
      ambient_seed=g->rng ^ (home_epoch*2654435761u) ^ (++ambient_visits*2246822519u);
    }
    home_initialized=true;
    memset(pet_ground_layer,0,sizeof(pet_ground_layer));
    pp_scene_view_init();
    body = lv_image_create(screen);
    lv_obj_add_flag(body,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(body,home_ground_event,LV_EVENT_SHORT_CLICKED,NULL);
    lv_obj_set_pos(body, 0, 0);
    lv_obj_set_size(body, 360, 360);
    pp_scene_view_set(body, scene_paths[g->scene], g->scene, home_time-home_epoch);
    last_scene = g->scene;
    pp_scene_effect_view_open(screen,tumbleweed_frames);
    lv_obj_t *hud = lv_obj_create(screen);
    lv_obj_remove_style_all(hud);
    lv_obj_set_pos(hud, 91, 26);
    lv_obj_set_size(hud, 178, 84);
    lv_obj_set_style_radius(hud, 22, 0);
    lv_obj_set_style_bg_color(hud, color(0x112738), 0);
    lv_obj_set_style_bg_opa(hud, 0, 0);
    lv_obj_remove_flag(hud, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    clock_shadow = label(screen, "--:--", 86, 30, 190, 0x6c6592);
    lv_obj_set_style_text_font(clock_shadow, &pp_clock_38, 0);
    clock_label = label(screen, "--:--", 85, 29, 190, 0xd5c6ff);
    lv_obj_set_style_text_font(clock_label, &pp_clock_38, 0);
    lv_obj_set_style_text_opa(clock_label, LV_OPA_COVER, 0);
    coin_icon = lv_image_create(screen);
    pp_image_set_src(coin_icon, hud_coin_large_path);
    lv_obj_set_pos(coin_icon, 150, 67);
    coin_shadow = label(screen, "", 173, 67, 120, 0x344866);
    coin = label(screen, "", 172, 66, 120, 0xe0e1ff);
    lv_obj_set_style_text_font(coin,&pp_font_18,0);
    lv_obj_set_style_text_font(coin_shadow,&pp_font_18,0);
    lv_obj_set_width(coin_shadow,120);
    lv_obj_set_style_text_align(coin,LV_TEXT_ALIGN_LEFT,0);
    lv_obj_set_style_text_align(coin_shadow,LV_TEXT_ALIGN_LEFT,0);
    bubble_box=lv_obj_create(screen);lv_obj_remove_style_all(bubble_box);lv_obj_set_size(bubble_box,176,44);
    lv_obj_set_style_bg_color(bubble_box, color(0x123c60), 0);
    lv_obj_set_style_bg_opa(bubble_box, 235, 0);
    lv_obj_set_style_radius(bubble_box, 8, 0);
    lv_obj_remove_flag(bubble_box,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    bubble_badge=lv_obj_create(bubble_box);lv_obj_remove_style_all(bubble_badge);
    lv_obj_set_pos(bubble_badge,5,8);lv_obj_set_size(bubble_badge,30,28);
    lv_obj_set_style_radius(bubble_badge,7,0);lv_obj_set_style_bg_color(bubble_badge,color(0x9be2da),0);lv_obj_set_style_bg_opa(bubble_badge,65,0);
    bubble_icon=lv_image_create(bubble_box);lv_obj_set_pos(bubble_icon,8,10);
    lv_obj_remove_flag(bubble_badge,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(bubble_icon,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    bubble = label(bubble_box, talk_text, 8, 6, 160, 0xf1f0ff);
    lv_obj_set_style_text_font(bubble, &pp_font_12, 0);
    lv_obj_set_style_text_align(bubble,LV_TEXT_ALIGN_LEFT,0);
    lv_obj_remove_flag(bubble,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_label_set_long_mode(bubble,LV_LABEL_LONG_WRAP);home_bubble_layout();
    for (unsigned i = 0; i < PP_PETS; i++) {
      pet_img[i] = lv_image_create(screen);
      lv_obj_set_size(pet_img[i], PP_SPRITE_SIZE, PP_SPRITE_SIZE);
      lv_obj_set_pos(pet_img[i], px[i], py[i] < 115 ? 115 : py[i] > 208 ? 208 : py[i]);
      lv_obj_add_flag(pet_img[i], LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_ext_click_area(pet_img[i], 9);
      lv_obj_add_event_cb(pet_img[i], pet_event, LV_EVENT_SHORT_CLICKED,
                          (void *)(uintptr_t)i);
      lv_obj_add_event_cb(pet_img[i], pet_event, LV_EVENT_LONG_PRESSED,
                          (void *)(uintptr_t)i);
    }
    if(g->scene==PP_MIRROR_LAKE_SCENE) {
      home_tree_front=lv_image_create(screen);
      lv_obj_remove_flag(home_tree_front,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
      if(!pp_scene_view_front_set(home_tree_front,lake_front_path)) {
        lv_obj_delete(home_tree_front);home_tree_front=NULL;
      }
    }
    for(unsigned i=0;i<PP_PETS;i++) {
      home_symbols[i]=lv_obj_create(screen);lv_obj_remove_style_all(home_symbols[i]);
      lv_obj_set_size(home_symbols[i],14,14);
      lv_obj_remove_flag(home_symbols[i],LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_add_flag(home_symbols[i],LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_event_cb(home_symbols[i],home_symbol_draw,LV_EVENT_DRAW_MAIN,(void *)(uintptr_t)i);
    }
    /* Taller scenery may reach the HUD; keep readouts above the tree cutout. */
    lv_obj_move_foreground(clock_shadow);
    lv_obj_move_foreground(clock_label);
    lv_obj_move_foreground(coin_icon);
    lv_obj_move_foreground(coin_shadow);
    lv_obj_move_foreground(coin);
    lv_obj_move_foreground(bubble_box);
    egg_glow = lv_obj_create(screen);lv_obj_remove_style_all(egg_glow);
    lv_obj_set_size(egg_glow,PP_EGG_GLOW_WIDTH,PP_EGG_GLOW_HEIGHT);
    lv_obj_remove_flag(egg_glow,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(egg_glow,LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(egg_glow,egg_glow_draw,LV_EVENT_DRAW_MAIN,NULL);
    egg_icon = lv_image_create(screen);
    pp_image_set_src(egg_icon, icon_paths[5]);
    lv_obj_set_pos(egg_icon, 65, 224);
    lv_obj_remove_flag(egg_icon,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(egg_icon,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(egg_icon,8);
    lv_obj_add_event_cb(egg_icon,egg_notice,LV_EVENT_CLICKED,NULL);
    egg_rim=lv_image_create(screen);
    lv_obj_remove_flag(egg_rim,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(egg_rim,LV_OBJ_FLAG_HIDDEN);
    drop = label(screen, "", 145, 267, 18, 0x583c21);
    lv_obj_set_size(drop,18,4);lv_obj_set_style_radius(drop,LV_RADIUS_CIRCLE,0);
    lv_obj_set_style_bg_color(drop,color(0x20382b),0);lv_obj_set_style_bg_opa(drop,50,0);
    lv_obj_remove_flag(drop,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    drop_icon = lv_image_create(screen);
    pp_image_set_src(drop_icon, small_paths[0]);
    lv_obj_set_pos(drop_icon, 164, 234);
    lv_obj_t *dock = lv_obj_create(screen);
    lv_obj_remove_style_all(dock);
    lv_obj_set_pos(dock, 92, 261);
    lv_obj_set_size(dock, 176, 72);
    lv_obj_set_style_radius(dock, 32, 0);
    lv_obj_set_style_bg_color(dock, color(0x0e2539), 0);
    lv_obj_set_style_bg_opa(dock, 0, 0);
    lv_obj_remove_flag(dock, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    const char *captions[] = {"APP", "BAG", "SHOP"};
    for (unsigned n = 0; n < 3; n++) {
      lv_obj_t *shadow = label(screen,captions[n],103+n*52,266,52,0x163145);
      lv_obj_set_style_text_font(shadow,&pp_font_nav_14,0);
      lv_obj_t *caption = label(screen,captions[n],102+n*52,265,52,0xe1ddff);
      lv_obj_set_style_text_font(caption,&pp_font_nav_14,0);
      lv_obj_t *b = button(screen, "", 106 + n * 52, 284, 44, 44, n + 1);
      lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_bg_color(b, color(0x103b75), 0);
      lv_obj_set_style_border_color(b, color(0x63baff), 0);
      lv_obj_set_style_border_width(b, 2, 0);
      lv_obj_t *im = lv_obj_get_child(b, 1);
      pp_image_set_src(im, nav_paths[n]);
      lv_obj_center(im);
    }
    battery_group=lv_obj_create(screen);lv_obj_remove_style_all(battery_group);
    lv_obj_add_event_cb(battery_group,home_battery_bolt_draw,LV_EVENT_DRAW_POST,NULL);
    lv_obj_set_style_bg_color(battery_group,color(0x0a2530),0);lv_obj_set_style_bg_opa(battery_group,170,0);lv_obj_set_style_radius(battery_group,7,0);
    lv_obj_set_pos(battery_group,145,331);lv_obj_set_size(battery_group,70,18);
    lv_obj_remove_flag(battery_group,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    battery_outline=lv_obj_create(battery_group);lv_obj_remove_style_all(battery_outline);
    lv_obj_set_pos(battery_outline,4,4);lv_obj_set_size(battery_outline,15,9);
    lv_obj_set_style_border_width(battery_outline,1,0);lv_obj_set_style_radius(battery_outline,1,0);
    battery_cap=lv_obj_create(battery_group);lv_obj_remove_style_all(battery_cap);
    lv_obj_set_pos(battery_cap,19,6);lv_obj_set_size(battery_cap,2,5);lv_obj_set_style_bg_opa(battery_cap,255,0);
    battery_fill=lv_obj_create(battery_group);lv_obj_remove_style_all(battery_fill);
    lv_obj_set_pos(battery_fill,6,6);lv_obj_set_size(battery_fill,10,5);lv_obj_set_style_bg_opa(battery_fill,255,0);
    battery_value=label(battery_group,"--",26,0,44,0xb2dfbe);lv_obj_set_style_text_font(battery_value,&pp_font_12,0);
    lv_obj_set_style_text_align(battery_value,LV_TEXT_ALIGN_LEFT,0);
    for(unsigned i=0;i<lv_obj_get_child_count(battery_group);i++)lv_obj_remove_flag(lv_obj_get_child(battery_group,i),LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    home_battery_update();
    lv_obj_move_foreground(bubble_box);
    update_home();
  } else {
    /* Shared resident bitmap: no runtime blur, scaling or input interception.
     * Story, battle and map artwork retain their dedicated backdrops. */
    if(page==9) {
      lv_obj_t *background=lv_image_create(screen);
      pp_scene_view_blurred_set(background,scene_paths[scene_page]);
      lv_obj_set_pos(background,0,0);
      lv_obj_remove_flag(background,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    } else if(page!=6 && page!=17 && page!=21 && page!=23 && page!=30 &&
              !(page==5 && (travel_notice_active || travel_receipt.active))) {
      lv_obj_t *background=lv_image_create(screen);
      pp_image_set_src(background,menu_background_path);
      lv_obj_set_pos(background,0,0);
      lv_obj_remove_flag(background,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    }
    title = label(screen, page==11?settings_titles[settings_section]:nav[page], 70, 38, 220, 0xf3ecdc);
    lv_obj_set_style_text_font(title, &pp_font_18, 0);
    subtitle = label(screen, "", 90, 62, 180, 0x6a979a);
    (void)title;
    (void)subtitle;
    body = lv_obj_create(screen);
    lv_obj_remove_style_all(body);
    lv_obj_set_pos(body, 0, 76);
    lv_obj_set_size(body, 360, 215);
    if (page == 6) {
      lv_obj_set_y(body, 64);
      lv_obj_set_height(body, 227);
    }
    lv_obj_remove_flag(body, LV_OBJ_FLAG_SCROLLABLE);
    back_button = button(
        screen, "返回",
        page == 6 && (g->battling || g->battle_hp > 0 || g->enemy_hp > 0) ? 96
                                                                          : 145,
        292, 70, 36, (page == 1 || page == 2 || page == 3 || page == 4) ? 0 : page==16?(warehouse_swap?186:1):page==26?(info_from_warehouse?772:4):page==27||page==28||page==29?26:1);
    if (page == 1 || page == 9 || page == 8 || page == 6 || page == 14 || page == 15 || page == 16 || page >= 17) {
      lv_obj_set_pos(back_button, 162, 294);
      lv_obj_set_size(back_button, 36, 36);
      lv_obj_set_style_radius(back_button, LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_bg_color(back_button, color(0x15517d), 0);
      lv_obj_set_style_bg_grad_dir(back_button, LV_GRAD_DIR_NONE, 0);
      lv_obj_set_style_border_color(back_button, color(0x66b8e8), 0);
      lv_obj_set_style_border_width(back_button, 2, 0);
      lv_obj_center(lv_obj_get_child(back_button, 0));
    }
    if(page>=17) {
      lv_obj_set_pos(subtitle,45,62);lv_obj_set_width(subtitle,270);lv_obj_set_style_text_font(subtitle,&pp_font_12,0);
      lv_obj_remove_event_cb(back_button,action);
      unsigned target=page==26?(info_from_warehouse?772:4):page==27||page==28?26:page==29?(hatch_replacing?30:26):page==25?24:page==23?654:page==18?17:page==19?18:page==21?18:1;
      lv_obj_add_event_cb(back_button,action,LV_EVENT_CLICKED,(void *)(intptr_t)target);
    }
    if(page==3 && g->wild.phase==PP_WILD_WON && g->wild.outcome==PP_WILD_CAPTURE_READY){lv_obj_remove_event_cb(back_button,action);lv_obj_add_event_cb(back_button,action,LV_EVENT_CLICKED,(void *)(intptr_t)655);}
    if(page==23 && g->legend.active)legend_result_view();
    if(page==23 && !g->legend.active) {
      wild_result_view();
    }

    if(page==1)app_menu_view();
    if(page==24)legend_list_view();
    if(page==25)legend_detail_view();
    if(page==17) {
      const pp_gym_def *d=&pp_gyms[gym_selected];
      text(title,d->city);lv_obj_set_y(title,34);
      lv_obj_set_style_text_font(title,&pp_font_title_22,0);
      char recommendation[48];snprintf(recommendation,sizeof(recommendation),"推荐等级 Lv.%u",pp_gym_recommended_level(gym_selected));
      text(subtitle,recommendation);lv_obj_set_style_text_font(subtitle,&pp_font_18,0);
      /* The town extends to the physical panel arc, like the battle arena. */
      lv_obj_t *terrain=lv_image_create(body);pp_image_set_src(terrain,gym_art_paths[gym_selected][3]);lv_obj_set_pos(terrain,0,24);
      lv_obj_t *entrance=button(body,"",120,38,120,122,522);
      lv_obj_set_style_bg_opa(entrance,LV_OPA_TRANSP,LV_PART_MAIN|LV_STATE_DEFAULT);
      lv_obj_set_style_bg_opa(entrance,LV_OPA_TRANSP,LV_PART_MAIN|LV_STATE_PRESSED);
      lv_obj_set_style_border_width(entrance,0,0);
      unsigned command=g->gym.mode==gym_selected+1?(g->battling?6:g->gym.result?21:522):522;
      lv_obj_t *enter=button(body,command==6?"继续挑战":command==21?"查看收获":pp_gym_unlocked(g,gym_selected)?"进入道馆":"尚未解锁",124,184,112,28,command);
      lv_obj_set_style_bg_color(enter,color(0x374c48),0);
      lv_obj_set_style_bg_grad_dir(enter,LV_GRAD_DIR_NONE,0);
      lv_obj_set_style_border_color(enter,color(0xb8aa82),0);
      if(g->gym.badges&(1ULL<<gym_selected)) {
        gym_clear_stamp=lv_image_create(body);pp_image_set_src(gym_clear_stamp,gym_defeated_paths[gym_selected]);
        lv_obj_set_pos(gym_clear_stamp,112,10);
      } else {
        gym_guards=lv_image_create(body);pp_image_set_src(gym_guards,gym_guard_paths[gym_selected]);
        lv_obj_set_pos(gym_guards,pp_gym_door_positions[gym_selected][0],pp_gym_door_positions[gym_selected][1]-76);
        lv_obj_add_flag(gym_guards,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(gym_guards,action,LV_EVENT_CLICKED,(void *)(intptr_t)522);
      }
      for(unsigned side=0;side<2;side++) {
        unsigned destination=(gym_selected+(side?1:PP_GYM_COUNT-1))%PP_GYM_COUNT;
        switch_arrow(body,76,540+destination,side);
      }
    }
    if(page==18) {
      const pp_gym_def *d=&pp_gyms[gym_selected];
      char place[64];snprintf(place,sizeof(place),"%s · %s",d->city,d->type);text(title,d->title);text(subtitle,place);
      lv_obj_t *im=lv_image_create(body);pp_image_set_src(im,gym_art_paths[gym_selected][0]);lv_obj_set_pos(im,148,5);
      char recommendation[48];snprintf(recommendation,sizeof(recommendation),"推荐等级 Lv.%u",pp_gym_recommended_level(gym_selected));
      label(body,recommendation,80,70,200,0xe1d5ba);
      for(unsigned i=0;i<3;i++) {
        im=lv_image_create(body);pp_image_set_src(im,avatar_paths[d->team[i].art]);lv_obj_set_pos(im,81+i*78,91);
        char t[40];snprintf(t,sizeof(t),"%s",d->team[i].name);lv_obj_t *l=label(body,t,65+i*78,130,72,0xa4b7bf);lv_obj_set_style_text_font(l,&pp_font_12,0);
        snprintf(t,sizeof(t),"Lv.%u",d->team[i].level);l=label(body,t,65+i*78,148,72,0xcbbd94);lv_obj_set_style_text_font(l,&pp_font_12,0);
      }
      button(body,g->battling?"继续挑战":"开始战斗",108,187,144,28,g->battling?6:512);
    }
    if(page==19) {
      char t[80];snprintf(t,sizeof(t),"已选 %u / 3 · 按选择顺序出场",gym_team_count);text(subtitle,t);
      for(unsigned i=0;i<g->count;i++) {
        unsigned row=i/3,col=i%3,cols=g->count-row*3;if(cols>3)cols=3;int x=(360-(int)cols*78+6)/2+(int)col*78,y=8+row*82;
        unsigned order=0;for(unsigned j=0;j<gym_team_count;j++)if(gym_team[j]==i)order=j+1;
        snprintf(t,sizeof(t),"%u · Lv.%u",order,g->growth[i].level);
        if(!order)snprintf(t,sizeof(t),"Lv.%u",g->growth[i].level);
        bool occupied=g->traveler==(int)i || (g->drop_active && g->drop_target==i);
        lv_obj_t *tile=button(body,"",x,y,72,76,500+i);lv_obj_set_style_bg_color(tile,color(occupied?0x26313a:order?0x2c514c:0x183145),0);lv_obj_set_style_border_color(tile,color(occupied?0x596b72:order?0xb7db92:0x395064),0);
        lv_obj_t *im=lv_image_create(tile);pp_image_set_src(im,avatar_paths[pet_art(i)]);lv_obj_set_pos(im,15,0);
        lv_obj_t *l=label(tile,t,0,54,70,order?0xd8ecb7:0x9fb8c5);lv_obj_set_style_text_font(l,&pp_font_12,0);
        l=label(tile,occupied?(g->traveler==(int)i?"旅行中":"使用中"):pp_form_name(g,i),0,39,70,occupied?0x93a2a7:0xc4d0d9);lv_obj_set_style_text_font(l,&pp_font_12,0);
      }
      if(g->count<3) {lv_obj_t *l=label(body,"至少需要三只伙伴\n可前往SHOP孵蛋或调入候补",55,102,250,0xe1bc8b);lv_obj_set_style_text_font(l,&pp_font_12,0);}
      button(body,"开始挑战",112,177,136,34,510);
    }
    if(page==20) {
      char t[96];unsigned count=0;for(unsigned i=0;i<PP_GYM_COUNT;i++)count+=!!(g->gym.badges&(1ULL<<i));
      snprintf(t,sizeof(t),"已获得 %u / 8",count);text(subtitle,t);
      for(unsigned i=0;i<8;i++) {
        int x=66+(i%4)*60,y=14+(i/4)*78;
        lv_obj_t *im=lv_image_create(body);pp_image_set_src(im,gym_art_paths[i<PP_GYM_COUNT?i:0][2]);lv_obj_set_pos(im,x,y);
        if(i>=PP_GYM_COUNT || !(g->gym.badges&(1ULL<<i)))lv_obj_set_style_image_recolor_opa(im,230,0);
        lv_obj_set_style_image_recolor(im,color(0x263b4a),0);
        if(i<PP_GYM_COUNT) {
          lv_obj_t *hit=button(body,"",x,y,48,48,530+i);lv_obj_set_style_bg_opa(hit,0,0);lv_obj_set_style_border_width(hit,0,0);
        }
      }
      const pp_gym_def *d=&pp_gyms[gym_selected];
      if(g->gym.badges&(1ULL<<gym_selected))snprintf(t,sizeof(t),"%s · 推荐 Lv.%u",d->badge,pp_gym_recommended_level(gym_selected));
      else snprintf(t,sizeof(t),"%s · 尚未获得",d->badge);
      label(body,t,66,156,228,0xcbbd99);
      snprintf(t,sizeof(t),"前往%s",d->city);button(body,t,116,187,128,28,522);
    }
    if(page==21) {
      const pp_gym_def *d=pp_gym_current(g);bool win=g->gym.result==1;char t[96];
      snprintf(t,sizeof(t),"%s · %s",win?(g->gym.first_clear?"首次通关":"再次获胜"):"惜败",d->title);text(subtitle,t);
      lv_obj_t *im=lv_image_create(body);pp_image_set_src(im,gym_art_paths[gym_selected][win?1:0]);lv_obj_set_pos(im,148,2);
      if(win)snprintf(t,sizeof(t),g->gym.first_clear?"获得%s":"%s已收藏",d->badge);else snprintf(t,sizeof(t),"照顾伙伴，再来挑战");label(body,t,66,72,228,0xe0d1b0);
      snprintf(t,sizeof(t),"金币 +%lu · 出场伙伴 +%u XP",(unsigned long)g->gym.reward_coins,g->gym.reward_xp);label(body,t,48,98,264,0xb4d3bf);
      if(win&&g->gym.reward_item<PP_COLLECTION) {
        unsigned id=g->gym.reward_item;char path[320];collection_path(path,sizeof(path),id<PP_COLLECTION?id:0,false);
        im=lv_image_create(body);pp_image_set_src(im,path);lv_obj_set_pos(im,164,120);
        snprintf(t,sizeof(t),"%s · %s",pp_qualities[pp_collectibles[id].quality].name,pp_collectibles[id].name);
        lv_obj_t *l=label(body,t,52,158,256,0xc8b2e4);lv_obj_set_style_text_font(l,&pp_font_12,0);
      }
      button(body,win?"徽章展馆":"调整编队",116,188,128,28,win?20:512);
    }
    if (page == 15) {
      unsigned order[PP_DEX_COUNT]; pp_dex_order(g, order);
      unsigned pages=(PP_DEX_COUNT+PETDEX_PAGE_SIZE-1)/PETDEX_PAGE_SIZE;
      char s[64];
      for(unsigned slot=0;slot<PETDEX_PAGE_SIZE && petdex_page*PETDEX_PAGE_SIZE+slot<PP_DEX_COUNT;slot++) {
        unsigned entry=order[petdex_page*PETDEX_PAGE_SIZE+slot];
        const pp_dex_entry *p=&pp_pokedex[entry];
        bool owned=pp_dex_owned(g,entry);
        lv_obj_t *tile=lv_obj_create(body);lv_obj_remove_style_all(tile);
        unsigned row=slot<3?0:slot<7?1:2;
        unsigned col=slot-(row==0?0:row==1?3:7);
        int width=row==1?52:62,avatar_x=(width-40)/2;
        lv_obj_set_pos(tile,(row==1?69:81)+col*(row==1?57:66),6+row*68);lv_obj_set_size(tile,width,62);
        lv_obj_remove_flag(tile,LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_radius(tile,10,0);lv_obj_set_style_bg_opa(tile,255,0);
        lv_obj_set_style_bg_color(tile,color(owned?0x183b41:0x111f2b),0);
        lv_obj_set_style_border_width(tile,1,0);lv_obj_set_style_border_color(tile,color(owned?0x578d79:0x233644),0);
        lv_obj_t *im=lv_image_create(tile);pp_image_set_src(im,avatar_paths[p->art]);lv_obj_set_pos(im,avatar_x,2);
        if(!owned) {
          lv_obj_set_style_image_recolor(im,color(0x4c5a68),0);lv_obj_set_style_image_recolor_opa(im,255,0);
          lv_obj_t *q=label(tile,"?",avatar_x,12,40,0xb9c6cc);lv_obj_set_style_text_font(q,&pp_font_18,0);
        }
        snprintf(s,sizeof(s),"%03u",p->national_id);
        lv_obj_t *l=label(tile,owned?p->name:s,0,44,width,owned?0xd0e6d7:0x778a99);
        lv_obj_set_style_text_font(l,&pp_font_12,0);
      }
      if(pages>1) {
      snprintf(s,sizeof(s),"%u / %u",petdex_page+1,pages);
      lv_obj_t *l=label(screen,s,144,282,72,0x6e8a9b);lv_obj_set_style_text_font(l,&pp_font_12,0);
      switch_arrow(screen,0,175,false);switch_arrow(screen,0,176,true);
      }
    }
    if (page == 14) {
      lv_obj_t *im=lv_image_create(body); pp_image_set_src(im,step_icon_path); lv_obj_set_pos(im,164,0);
      lv_obj_t *l=label(body,"今日步数",80,33,200,0x91b7b1); lv_obj_set_style_text_font(l,&pp_font_12,0);
      step_number=label(body,"0",65,53,230,0xd6edcf); lv_obj_set_style_text_font(step_number,&pp_clock_36,0);
      step_bar=lv_bar_create(body); lv_obj_set_pos(step_bar,80,106); lv_obj_set_size(step_bar,200,5);
      lv_obj_set_style_bg_color(step_bar,color(0x274248),0); lv_obj_set_style_bg_color(step_bar,color(0x9bdab4),LV_PART_INDICATOR);
      l=label(body,"金币             经验 /只",60,124,240,0x88aaa8); lv_obj_set_style_text_font(l,&pp_font_12,0);
      step_rewards=label(body,"",55,143,250,0xffdb8e);
      step_rate=label(body,"",50,174,260,0x8eabb8); lv_obj_set_style_text_font(step_rate,&pp_font_12,0);
      step_total=label(body,"",65,196,230,0x718e9b); lv_obj_set_style_text_font(step_total,&pp_font_12,0);
      update_steps();
    }
    if (page == 2 || page == 3) {
      lv_obj_set_height(body,224);
      char s[80];
      snprintf(s, sizeof(s), "%lu", (unsigned long)g->coins);
      lv_point_t money_size;
      lv_text_get_size(&money_size,s,&pp_font_14,0,0,LV_COORD_MAX,LV_TEXT_FLAG_NONE);
      int money_x=(360-20-money_size.x)/2;
      lv_obj_t *money_icon=lv_image_create(screen);
      pp_image_set_src(money_icon,hud_coin_path);
      lv_obj_set_pos(money_icon,money_x,68);
      label(screen,s,money_x+20,67,money_size.x,0x83c6b3);
      unsigned entries[PP_SHOP_ITEMS+PP_LEGEND_COUNT],total=PP_SHOP_ITEMS,pages=1,start=0;
      if(page==2) {
        total=bag_entries(entries);pages=(total+BAG_PAGE_SIZE-1)/BAG_PAGE_SIZE;if(!pages)pages=1;
        if(bag_page>=pages)bag_page=pages-1;
        start=bag_page*BAG_PAGE_SIZE;
        if(pages>1) {
          snprintf(s,sizeof(s),"%u / %u",bag_page+1,pages);
          lv_obj_t *page_label=label(screen,s,150,86,60,0x799b9c);lv_obj_set_style_text_font(page_label,&pp_font_12,0);
          switch_arrow(screen,0,722,false);switch_arrow(screen,0,723,true);
        }
      }
      if(page==2 && !total)label(body,"背包空空的",65,94,230,0x91adaf);
      unsigned capacity=page==2?BAG_PAGE_SIZE:9;
      unsigned visible=total-start;if(visible>capacity)visible=capacity;
      for(unsigned slot=0;slot<visible;slot++) {
        unsigned i=page==2?entries[start+slot]:pp_shop_slot(slot);
        bool lottery=page==3 && slot==4;
        bool egg=!lottery && i>=PP_SHOP_ITEMS;unsigned legend=egg?i-PP_SHOP_ITEMS:0;
        unsigned count=egg?g->legend.eggs[legend]:lottery?0:pp_item_count(g,i);
        bool bag=page==2;
        unsigned cols=bag?4:3,row=slot/cols;
        int width=bag?52:72,height=bag?56:64;
        int x=64+(slot%cols)*(bag?60:80),y=(bag?30:22)+row*(bag?62:68);
        lv_obj_t *b=button(body,"",x,y,width,height,lottery?790:egg?730+legend:(bag?120:100)+i);
        if(page==3)shop_cards[slot]=b;
        if(lottery){
          lv_obj_set_style_radius(b,8,0);lv_obj_set_style_bg_color(b,color(0x544127),0);lv_obj_set_style_border_color(b,color(0xcda962),0);
          lv_obj_t *im=lv_image_create(b);pp_image_set_src(im,shop_lottery_source);lv_obj_set_pos(im,10,1);
          snprintf(s,sizeof(s),"%u",PP_SHOP_PRICE);
          lv_point_t size;lv_text_get_size(&size,s,&pp_font_shop_10,0,0,LV_COORD_MAX,LV_TEXT_FLAG_NONE);
          int left=(72-15-size.x)/2;
          im=lv_image_create(b);pp_image_set_src(im,shop_coin_source);lv_obj_set_pos(im,left,51);
          lv_obj_t *price=label(b,s,left+15,51,size.x,0xffdb83);lv_obj_set_style_text_font(price,&pp_font_shop_10,0);continue;
        }
        lv_obj_set_style_radius(b,8,0);
        lv_obj_set_style_bg_color(b,color(0x173b40),0);
        lv_obj_set_style_bg_grad_color(b,color(0x102b34),0);
        lv_obj_set_style_border_color(b,color(i==PP_TREE_ESSENCE?0x8dbd7a:0x386264),0);
        if(!bag){lv_obj_set_style_bg_opa(b,LV_OPA_TRANSP,0);lv_obj_set_style_bg_opa(b,80,LV_STATE_PRESSED);lv_obj_set_style_border_width(b,0,0);}
        lv_obj_t *im=egg?lv_image_create(b):lv_obj_get_child(b,1),*l=lv_obj_get_child(b,0);
        if(egg) {
          pp_image_set_src(im,avatar_paths[pp_pokedex[pp_legend_dex(legend)].art]);
          lv_obj_set_style_border_color(b,color(0x716a97),0);
        }
        lv_obj_set_align(l,LV_ALIGN_TOP_LEFT);
        if(!bag)pp_image_set_src(im,shop_item_sources[i]);
        lv_obj_set_pos(im,(width-(bag?(egg?40:32):40))/2,egg?0:bag?4:2);
        if(page==2) {
          snprintf(s,sizeof(s),"×%u",count);
          text(l,s);lv_obj_set_width(l,width-4);lv_obj_set_pos(l,2,40);
          lv_obj_set_style_text_font(l,&pp_font_12,0);
          lv_obj_set_style_text_color(l,color(count?0xc9e6cf:0x718c90),0);
          if(!count)lv_obj_set_style_image_opa(im,80,0);
        } else {
          /* One quiet footer row: item name left, gold price right. */
          text(l,pp_items[i].name);lv_obj_set_width(l,42);lv_obj_set_pos(l,4,49);
          lv_obj_set_style_text_font(l,&pp_font_shop_10,0);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_LEFT,0);
          snprintf(s,sizeof(s),"%u",pp_items[i].price);
          lv_point_t size;lv_text_get_size(&size,s,&pp_font_shop_10,0,0,LV_COORD_MAX,LV_TEXT_FLAG_NONE);
          lv_obj_t *price=label(b,s,68-size.x,49,size.x,0xe9cf83);lv_obj_set_style_text_font(price,&pp_font_shop_10,0);
        }
      }
    }
    if (page == 4) {
      pp_pet *p=&g->pets[selected];
      pp_growth *growth=&g->growth[selected];
      char s[80];
      text(subtitle,pp_nature_names[g->identity[selected].nature]);
      pet_identity_card(body,pet_art(selected),pp_pet_name(g,selected),growth,true);
      const char *names[]={"饱腹","清洁","健康","心情","精力"};
      const unsigned ids[]={0,1,2,3,14};
      const uint32_t colors[]={0xe3b979,0x79bcd6,0xe78d9f,0xb69bde,0xe8d58d};
      const unsigned values[]={p->food,p->clean,p->health,p->mood,p->energy};
      for(unsigned i=0;i<5;i++) {
        int x=70+(int)i*45;
        lv_obj_t *im=lv_image_create(body);pp_image_set_src(im,small_paths[ids[i]]);
        lv_obj_set_pos(im,x+8,96);
        lv_obj_t *caption=label(body,names[i],x,124,40,0x8da6aa);
        lv_obj_set_style_text_font(caption,&pp_font_12,0);
        snprintf(s,sizeof(s),"%u",values[i]);
        uint32_t tint=values[i]<25?0xee8591:colors[i];
        label(body,s,x,141,40,tint);
        lv_obj_t *meter=lv_bar_create(body);
        lv_obj_set_pos(meter,x+4,160);lv_obj_set_size(meter,32,4);
        lv_bar_set_value(meter,values[i],LV_ANIM_OFF);
        lv_obj_set_style_bg_color(meter,color(0x1b3440),0);
        lv_obj_set_style_bg_color(meter,color(tint),LV_PART_INDICATOR);
      }
      unsigned gate=pp_evolution_level(g,selected);
      lv_obj_t *evolve=NULL;
      if(pp_evolution_locked(g,selected)) {
        lv_obj_t *l=label(body,"已选择永久不进化",70,176,220,0x96b7b0);
        lv_obj_set_style_text_font(l,&pp_font_12,0);
      } else if(pp_evolution_stone(g,selected)) {
        snprintf(s,sizeof(s),"%s · %u币",pp_evolution_stone(g,selected),PP_THUNDER_STONE_PRICE);
        evolve=button(body,s,94,154,172,32,177);
      } else if(gate) {
        snprintf(s,sizeof(s),growth->level>=gate?"突破进化":"Lv.%u  可突破",gate);
        evolve=button(body,s,94,154,172,32,167);
        if(growth->level<gate)lv_obj_add_state(evolve,LV_STATE_DISABLED);
      } else {
        lv_obj_t *l=label(body,"最终形态",80,176,200,0x8da5af);
        lv_obj_set_style_text_font(l,&pp_font_12,0);
      }
      if(evolve) {
        lv_obj_set_style_radius(evolve,11,0);
        lv_obj_set_style_bg_color(evolve,color(0x2d342d),0);
        lv_obj_set_style_bg_grad_dir(evolve,LV_GRAD_DIR_NONE,0);
        lv_obj_set_style_border_color(evolve,color(0x746c49),0);
        lv_obj_set_pos(evolve,94,186);lv_obj_set_size(evolve,172,28);pp_footer_text_style(evolve);
      }
      /* Keep all three actions on one baseline inside the circular safe area. */
      pet_swap_button(screen,104,294);
      lv_obj_set_pos(back_button,162,286);lv_obj_set_size(back_button,36,36);
      lv_obj_set_style_radius(back_button,12,0);
      lv_obj_center(lv_obj_get_child(back_button,0));
      if(g->count<PP_PETS) {
        lv_obj_t *add=button(screen,"",220,294,36,36,184);
        pp_footer_text_style(add);
        lv_obj_t *glyph=lv_obj_get_child(add,0);lv_label_set_text(glyph,LV_SYMBOL_PLUS);
        lv_obj_set_style_text_font(glyph,&lv_font_montserrat_14,0);lv_obj_center(glyph);
        lv_obj_set_style_text_color(glyph,color(0xb9e8d4),0);
      }
    }
    if(page==5 && travel_receipt.active)travel_result_view();
    else if(page==5 && travel_notice_active)travel_notice_view();
    if ((page == 5 && !travel_receipt.active && !travel_notice_active) || page == 6 || page == 7) {
      live_status = label(body, "", 45, 0, 270, 0x9bdaca);
      live_log = label(body, "", 53, 116, 254, 0x9ab1b7);
      lv_obj_set_height(live_log, 94);
      lv_label_set_long_mode(live_log, LV_LABEL_LONG_CLIP);
      lv_obj_set_style_text_align(live_log, LV_TEXT_ALIGN_LEFT, 0);
      if (page == 5) {
        travel_layout = g->traveler >= 0 || travel_receipt.active;
        if (!travel_layout) {
          pet_picker(50);
          button(body, "出发", 97, 165, 166, 38, 160);
          lv_obj_add_flag(live_log, LV_OBJ_FLAG_HIDDEN);
        } else {
          unsigned destination=travel_receipt.active?travel_receipt.destination:pp_travel_destination(g);
          text(subtitle, pp_travel_name(destination));
          lv_obj_set_y(live_status, 4);
          lv_obj_set_style_text_font(live_status, &pp_font_12, 0);
          lv_obj_t *im = lv_image_create(body);
          travel_backdrop = im;
          if(!pp_scene_view_strip_set(im,travel_sources[destination],0))pp_image_set_src(im,travel_sources[destination]);
          lv_obj_set_pos(im, 0, 32); /* Full-width scenery at screen y108, like battle. */
          travel_motion.destination=destination;
          travel_motion.effects=lv_obj_create(body);lv_obj_remove_style_all(travel_motion.effects);
          lv_obj_set_pos(travel_motion.effects,0,32);lv_obj_set_size(travel_motion.effects,360,112);
          lv_obj_remove_flag(travel_motion.effects,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
          lv_obj_add_event_cb(travel_motion.effects,travel_scenery_draw,LV_EVENT_DRAW_MAIN,NULL);
          lv_obj_t *pet = lv_image_create(body);
          unsigned art = travel_receipt.active ? travel_receipt.art : pet_art(g->traveler);
          travel_portrait(pet, paths[art][pp_animation_frame(art, PP_POSE_WALK, 0)]);
          lv_obj_set_pos(pet, 144, 76);
          activity_pet[0] = pet;
          travel_activity = label(body, "", 50, 150, 260, 0xdce8ee);
          lv_obj_set_pos(live_log, 50, 172);
          lv_obj_set_size(live_log, 260, 18);
          lv_obj_set_style_text_align(live_log, LV_TEXT_ALIGN_CENTER, 0);
          lv_obj_set_style_text_font(live_log, &pp_font_12, 0);
          travel_reward_label = label(body, "", 55, 192, 250, 0xe4c88d);
          lv_obj_set_style_text_font(travel_reward_label, &pp_font_12, 0);
          travel_claim = button(screen, "领取", 188, 288, 72, 32, 161);
          lv_obj_set_size(back_button, 72, 32);
          lv_obj_set_y(back_button, 288);
          live();
        }
      }
      if (page == 6) {
        battle_layout = g->battling || g->battle_hp > 0 || g->enemy_hp > 0;
        lv_obj_add_flag(live_status, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(live_log, LV_OBJ_FLAG_HIDDEN);
        if (!battle_layout) {
          pet_picker(24);
          for (unsigned slot = 0; slot < PP_MOVE_SLOTS; slot++) {
            lv_obj_t *chip = button(
                body, pp_move_at(g->pets[selected].species, slot)->name,
                78 + (slot % 2) * 104, 121 + (slot / 2) * 28, 100, 24, 999);
            lv_obj_remove_flag(chip, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_radius(chip, 8, 0);
            lv_obj_set_style_text_font(lv_obj_get_child(chip, 0), &pp_font_12,
                                       0);
          }
          button(body, "开始", 110, 181, 140, 32, 162);
        } else {
          again = button(screen, "再来", 188, 288, 64, 32, 162);
          lv_obj_add_flag(again, LV_OBJ_FLAG_HIDDEN);
          lv_obj_add_flag(title, LV_OBJ_FLAG_HIDDEN);
          pp_battle_view_create(body, g, asset_root, again, back_button);
          if(g->wild.phase>=PP_WILD_BATTLE){lv_obj_remove_event_cb(back_button,action);lv_obj_add_event_cb(back_button,action,LV_EVENT_CLICKED,(void *)(intptr_t)0);}
        }
      }
      if (page == 7) {
        lives = label(body, "", 100, 28, 160, 0xf4ab9b);
        for (unsigned i = 0; i < 6; i++) {
          stairs[i] = lv_obj_create(body);
          lv_obj_set_pos(stairs[i], 60 + i * 36, 68 + i * 12);
          lv_obj_set_size(stairs[i], 34, 21);
          lv_obj_set_style_border_width(stairs[i], 0, 0);
          lv_obj_set_style_radius(stairs[i], 4, 0);
        }
        icon(body, 10, 160, 118);
        button(body, "开始", 52, 169, 76, 38, 163);
        button(body, "↔", 137, 169, 86, 38, 164);
        button(body, "领取", 232, 169, 76, 38, 165);
        lv_obj_add_flag(live_log, LV_OBJ_FLAG_HIDDEN);
      }
      live();
    }
    if (page == 8) {
      char s[80];
      unsigned total=pp_collection_total(collection_quality),pages=collection_pages();
      if (dex_page>=pages) dex_page=0;
      snprintf(s,sizeof(s),"%s %u/%u   %u/%u",collection_quality==PP_QUALITY_ALL?"全部":pp_qualities[collection_quality].name,
        pp_collection_owned(g,collection_quality),total,dex_page+1,pages);
      lv_obj_t *count=label(screen,s,55,62,250,0x87a6b9);
      lv_obj_set_style_text_font(count,&pp_font_12,0);
      const char *tabs[]={"全","普","精","稀","史","传","特"};
      for (unsigned tab=0;tab<7;tab++) {
        unsigned q=tab?tab-1:PP_QUALITY_ALL,c=q==PP_QUALITY_ALL?0xd6e3ed:pp_qualities[q].color;
        lv_obj_t *b=button(screen,tabs[tab],74+tab*31,81,28,24,300+q);
        lv_obj_set_style_bg_opa(b,q==collection_quality?70:0,0);
        lv_obj_set_style_bg_color(b,color(c),0);
        lv_obj_set_style_border_width(b,0,0);
        lv_obj_set_style_text_color(lv_obj_get_child(b,0),color(c),0);
        lv_obj_set_style_text_font(lv_obj_get_child(b,0),&pp_font_12,0);
      }
      for (unsigned i=0;i<64 && dex_page*64+i<total;i++) {
        unsigned id=pp_collection_id(collection_quality,dex_page*64+i);
        unsigned c=pp_qualities[pp_collectibles[id].quality].color;
        bool owned=pp_has(g,id);char path[320];
        lv_obj_t *b=button(screen,"",80+(i%8)*25,111+(i/8)*22,23,21,1000+id);
        lv_obj_set_style_bg_opa(b,0,0);lv_obj_set_style_border_width(b,0,0);
        collection_path(path,sizeof(path),id,true);
        lv_obj_t *im=lv_image_create(b);pp_image_set_src(im,path);lv_obj_align(im,LV_ALIGN_TOP_MID,0,-1);
        lv_obj_set_style_opa(im,owned?255:65,0);lv_obj_remove_flag(im,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_t *dot=lv_obj_create(b);lv_obj_remove_style_all(dot);lv_obj_set_size(dot,owned?8:3,2);
        lv_obj_set_style_bg_color(dot,color(c),0);lv_obj_set_style_bg_opa(dot,owned?255:110,0);
        lv_obj_align(dot,LV_ALIGN_BOTTOM_MID,0,0);lv_obj_remove_flag(dot,LV_OBJ_FLAG_CLICKABLE);
      }
      if (pages>1) for (unsigned side=0;side<2;side++)
        switch_arrow(screen,0,170+side,side);
    }
    if(page==9)scene_menu_view();
    if (page == 16) {
      menu_theme();char s[64];unsigned total=warehouse_view_count(),pages=(total+WAREHOUSE_PAGE_SIZE-1)/WAREHOUSE_PAGE_SIZE;
      if(reserve_page>=pages)reserve_page=pages?pages-1:0;
      if(!pp_warehouse_sorted_page(g,warehouse_sort_mode,reserve_page,WAREHOUSE_PAGE_SIZE,warehouse_visible_slots)) {
        warehouse_sort_mode=PP_WAREHOUSE_DEFAULT;notice("内存不足");
      }
      snprintf(s,sizeof(s),"%u只    %u / %u",total,pages?reserve_page+1:0,pages);text(subtitle,s);
      if(!total)label(body,"仓库空空的",65,75,230,0x91adaf);
      for(unsigned i=0;i<WAREHOUSE_PAGE_SIZE&&reserve_page*WAREHOUSE_PAGE_SIZE+i<total;i++){
        pp_stored_pet p;unsigned n=warehouse_visible_slots[i];warehouse_view_get(n,&p);bool active=n<g->count;unsigned row_count=total-reserve_page*WAREHOUSE_PAGE_SIZE-(i/3)*3;if(row_count>3)row_count=3;int x=(360-(int)row_count*78+6)/2+(i%3)*78,y=8+(i/3)*68;
        lv_obj_t *tile=button(body,"",x,y,72,66,3000+i);lv_obj_clean(tile);
        lv_obj_set_style_bg_color(tile,color(0x173d40),0);lv_obj_set_style_border_color(tile,color(active?0x9bd9b1:0x426d65),0);lv_obj_set_style_border_width(tile,active?2:1,0);int inset=active?1:0;
        lv_obj_t *im=lv_image_create(tile);pp_image_set_src(im,avatar_paths[pp_stored_art(&p)]);lv_obj_set_pos(im,15-inset,-inset);
        lv_obj_t *l=label(tile,pp_stored_name(&p),2-inset,33-inset,66,0xdce9d5);lv_obj_set_style_text_font(l,&pp_font_12,0);
        lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_set_height(l,15);
        snprintf(s,sizeof(s),"Lv.%u",p.growth.level);l=label(tile,s,2-inset,48-inset,66,0x95c8af);lv_obj_set_style_text_font(l,&pp_font_12,0);
      }
      if(pages>1){switch_arrow(screen,0,182,false);switch_arrow(screen,0,183,true);}
      lv_obj_t *sort=button(screen,"",208,288,44,44,773);pp_nav_apply(sort,false);
      lv_obj_remove_event_cb(sort,pp_nav_draw);lv_obj_add_event_cb(sort,pp_nav_sort_draw,LV_EVENT_DRAW_MAIN,(void *)(uintptr_t)warehouse_sort_mode);
    }
    if(page==26){
      menu_theme();pp_stored_pet p={0};
      if(info_warehouse)pp_warehouse_get(g,warehouse_selected,&p);else {p.pet=g->pets[selected];p.growth=g->growth[selected];p.identity=g->identity[selected];}
      char s[80];snprintf(s,sizeof(s),"性格 · %s",pp_nature_names[p.identity.nature]);text(subtitle,s);
      pet_identity_card(body,pp_stored_art(&p),pp_stored_name(&p),&p.growth,false);
      const char *stats[]={"HP","攻击","防御","特攻","特防","速度"};
      static const unsigned axes[]={0,0,1,3,4,2};
      for(unsigned i=0;i<6;i++){
        int x=78+(i%2)*112,y=100+(i/2)*27;unsigned up=p.identity.nature/5,down=p.identity.nature%5;
        const char *change=i&&up!=down?(axes[i]==up?"+":axes[i]==down?"-":""):"";
        snprintf(s,sizeof(s),"%s%s  %u",stats[i],change,p.identity.iv[i]);lv_obj_t *l=label(body,s,x,y,92,0xc9ddd4);lv_obj_set_style_text_font(l,&pp_font_12,0);
        lv_obj_t *bar=lv_bar_create(body);lv_obj_set_pos(bar,x+3,y+22);lv_obj_set_size(bar,86,3);lv_bar_set_range(bar,0,31);lv_bar_set_value(bar,p.identity.iv[i],LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar,color(0x27434a),0);lv_obj_set_style_bg_color(bar,color(0x8ac6a5),LV_PART_INDICATOR);
      }
      bool can_store=!info_warehouse&&info_from_warehouse&&g->count>1;
      button(screen,"命名",info_warehouse?76:can_store?108:144,262,info_warehouse?68:72,28,761);
      if(can_store)button(screen,"收回",188,262,72,28,181);
      if(info_warehouse){button(screen,"放生",146,262,68,28,762);button(screen,g->count<PP_PETS?"上场":"替换",216,262,68,28,764);}
    }
    if(page==27){
      menu_theme();text(subtitle,"a-z · 8");lv_obj_set_y(body,76);lv_obj_set_height(body,212);
      name_value=label(screen,name_draft,65,104,230,0xf1e5c8);lv_obj_set_style_text_font(name_value,&pp_font_18,0);
      for(unsigned i=0;i<26;i++){
        static const char keys[]="qwertyuiopasdfghjklzxcvbnm";unsigned row=i<10?0:i<19?1:2,col=i-(row==0?0:row==1?10:19);
        char key[2]={keys[i],0};int x=(row==0?45:row==1?58:85)+col*27;
        button(screen,key,x,140+row*40,26,34,4000+keys[i]-'a');
      }
      button(screen,"删除",64,258,72,32,767);button(screen,"清空",144,258,72,32,766);
      button(screen,"保存",224,258,72,32,765);name_update();
    }
    if(page==30 && hatch_result.active)hatch_result_view();
    if(page==29){
      menu_theme();
      if(hatch_replacing){text(title,"替换哪位伙伴？");text(subtitle,"被替换的伙伴将进入仓库");}
      for(unsigned i=0;i<g->count;i++){
        unsigned cols=g->count-(i/3)*3;if(cols>3)cols=3;int x=(360-(int)cols*78+6)/2+(i%3)*78,y=12+(i/3)*82;
        lv_obj_t *card=button(body,"",x,y,72,76,4200+i);
        lv_obj_t *im=lv_image_create(card);pp_image_set_src(im,paths[pet_art(i)][0]);lv_obj_set_pos(im,12,0);
        lv_obj_t *l=label(card,pp_pet_name(g,i),3,39,66,0xdce9d5);lv_obj_set_style_text_font(l,&pp_font_12,0);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_set_height(l,16);
        char value[24];snprintf(value,sizeof(value),"Lv.%u",g->growth[i].level);l=label(card,value,3,54,66,0x95c8af);lv_obj_set_style_text_font(l,&pp_font_12,0);
      }
    }
    if(page==28){
      menu_theme();pp_stored_pet p;pp_warehouse_get(g,warehouse_selected,&p);
      lv_obj_t *im=lv_image_create(body);pp_image_set_src(im,paths[pp_stored_art(&p)][0]);lv_obj_set_pos(im,156,26);
      label(body,pp_stored_name(&p),60,92,240,0xe4e6cd);label(body,"确认放生？",65,126,230,0xe2b8a0);
      lv_obj_t *b=button(body,"确认放生",116,174,128,32,763);lv_obj_set_style_bg_color(b,color(0x62434a),0);
    }
    if (page == 10) {
      bool drawn = g->clock_ready && g->tarot_day == g->day;
      lv_obj_t *card = lv_image_create(body);
      pp_image_set_src(card, large_paths[12]);lv_obj_set_pos(card, 140, 0);
      label(body, drawn ? pp_tarot_names[g->tarot_card] : "?",55,89,250,0xeac3e1);
      lv_obj_t *l=label(body,drawn?(pp_tarot_active(g,g->tarot_card)?pp_tarot_effects[g->tarot_card]:"明日翻牌开启增益"):"每日一牌 · 当天有效",45,117,270,0xa6c6c4);
      lv_obj_set_style_text_font(l,&pp_font_12,0);
      l=label(body,drawn?(pp_tarot_active(g,g->tarot_card)?"今日生效 · 跨日结束":"今日已翻牌"):"翻牌获 10 金币",55,138,250,0x7c97a4);lv_obj_set_style_text_font(l,&pp_font_12,0);
      lv_obj_t *b=button(body, drawn ? "明日再来" : "翻牌",94,166,172,38,166);
      if(drawn) lv_obj_add_state(b,LV_STATE_DISABLED);
    }
    if (page == 11) {
      if(!settings_section){
        for(unsigned i=1;i<=3;i++){
          lv_obj_t *row=button(body,settings_titles[i],66,24+(i-1)*56,228,44,800+i);
          lv_obj_set_style_bg_color(row,color(0x132d37),0);
          lv_obj_set_style_bg_grad_dir(row,LV_GRAD_DIR_NONE,0);lv_obj_set_style_border_width(row,0,0);
          lv_obj_t *name=lv_obj_get_child(row,0);lv_obj_set_width(name,160);lv_obj_align(name,LV_ALIGN_LEFT_MID,16,0);lv_obj_set_style_text_align(name,LV_TEXT_ALIGN_LEFT,0);
          lv_obj_t *arrow=lv_label_create(row);lv_label_set_text(arrow,LV_SYMBOL_RIGHT);lv_obj_set_style_text_font(arrow,LV_FONT_DEFAULT,0);lv_obj_set_style_text_color(arrow,color(0x9cc9b8),0);lv_obj_align(arrow,LV_ALIGN_RIGHT_MID,-16,0);
        }
        lv_obj_t *version=label(screen,"v" PP_APP_VERSION,100,271,160,0x8196a5);
        lv_obj_set_style_text_font(version,&pp_font_12,0);
      }else{
        lv_obj_remove_event_cb(back_button,action);lv_obj_add_event_cb(back_button,action,LV_EVENT_CLICKED,(void *)(intptr_t)800);
        if(settings_section==1){
          settings_row(body,"显示时间",28,PP_UI_SHOW_TIME,display_options&PP_UI_SHOW_TIME);
          settings_row(body,"显示金币",76,PP_UI_SHOW_COINS,display_options&PP_UI_SHOW_COINS);
          settings_row(body,"显示电量",124,PP_UI_SHOW_BATTERY,display_options&PP_UI_SHOW_BATTERY);
        }else if(settings_section==2)settings_row(body,"每日随机场景",28,0,g->random_scene);
        else settings_row(body,"历练自动补给",28,8,training_auto_supply);
      }
    }
    if (page == 12) {
      const pp_collection_entry *entry=&pp_collectibles[selected];
      const pp_quality_entry *quality=&pp_qualities[entry->quality];
      char s[128],path[320];
      label(body,entry->name,45,5,270,quality->color);
      collection_path(path,sizeof(path),selected,false);
      lv_obj_t *im=lv_image_create(body);pp_image_set_src(im,path);lv_obj_set_pos(im,164,50);
      snprintf(s,sizeof(s),"%s · %s",quality->name,pp_has(g,selected)?"已收藏":"未获得");
      label(body,s,50,96,260,quality->color);
      if (selected==PP_PURE_HEART) snprintf(s,sizeof(s),"为伙伴选择永久不进化时赠送");
      else if(selected==PP_GYM_CROWN)snprintf(s,sizeof(s),"八座道馆首次通关后获得");
      else if(selected==PP_DEX_TOME)snprintf(s,sizeof(s),"宠物图鉴全部点亮后获得");
      else snprintf(s,sizeof(s),"旅行归来 · %s",pp_scene_names[entry->scene]);
      lv_obj_t *l=label(body,s,40,132,280,0x8da5af);lv_obj_set_style_text_font(l,&pp_font_12,0);
      if (selected<PP_TRAVEL_COLLECTION) {
        snprintf(s,sizeof(s),"同品质 %u/%u · 占旅行品种 %.2f%%",pp_collection_owned(g,entry->quality),quality->count,100.0*quality->count/PP_TRAVEL_COLLECTION);
        l=label(body,s,40,164,280,0x8da5af);lv_obj_set_style_text_font(l,&pp_font_12,0);
      }
    }
  }

  if(back_button)footer_normalize(page);
  if(page==16&&back_button)lv_obj_set_y(back_button,288);
  last_revision = g->revision;
  if(page==0){update_home();wild_overlay_tick();gift_overlay_tick();}
  /* A page replacement must also repaint uncovered background and the dock.
   * Dirty rectangles from deleted children alone can leave old panel pixels. */
  lv_obj_invalidate(screen);
}
void pp_ui_clock(const char *s) {
  if (current == 0 && clock_label) {
    text(clock_label, s);
    text(clock_shadow, s);
  }
}
void pp_ui_hardware(bool assets, bool imu) {
  assets_ok = assets;
  imu_ok = imu;
}
static bool training_notice_tick(void){
 if(!ui_screen_on || (current!=0&&current!=33) || evolution || starter_mode || gift_overlay || wild_overlay || g->battling || g->wild.phase || g->drop_active || travel_notice_active || travel_receipt.active || !pp_training_growth_pending(g))return false;
 pp_training_record *r=&g->training;
 for(unsigned i=0;i<r->count;i++){
  if(r->end_levels[i]<=r->start_levels[i])continue;
  unsigned pick=r->picks[i],bit=1u<<i;pp_stored_pet p;if(!pp_training_pet(g,pick,&p))continue;
  training_notice_index=i;training_notice_page=current;
  if(!(r->growth_seen&bit)){
   training_level=true;evo_choosing=false;evo_done=NULL;evo_pick=pick;evo_before_art=evo_current_art=pp_stored_art(&p);refresh_pending=false;
   evolution=lv_obj_create(screen);lv_obj_remove_style_all(evolution);lv_obj_set_size(evolution,360,360);lv_obj_set_style_bg_color(evolution,color(0x0c1c30),0);lv_obj_set_style_bg_opa(evolution,255,0);lv_obj_remove_flag(evolution,LV_OBJ_FLAG_SCROLLABLE);
   lv_obj_t *l=label(evolution,"历练升级",70,42,220,0x9fe0b6);lv_obj_set_style_text_font(l,&pp_font_18,0);
   l=label(evolution,pp_stored_name(&p),60,82,240,0xd8e9dc);lv_obj_set_style_text_font(l,&pp_font_14,0);
   evo_pet=lv_image_create(evolution);lv_obj_set_pos(evo_pet,132,112);evo_sprite_source=NULL;memset(&evolution_clock,0,sizeof(evolution_clock));
   char line[96];snprintf(line,sizeof(line),"Lv.%u → Lv.%u",r->start_levels[i],r->end_levels[i]);l=label(evolution,line,60,222,240,0xffe6a0);lv_obj_set_style_text_font(l,&pp_font_18,0);
   pp_footer_text_style(button(evolution,"继续",116,274,128,32,191));evolution_tick();return true;
  }
  if(!(r->evolution_seen&bit)){
   if(!pp_roster_evolution_preview(g,pick,&evo_preview))continue;
   pp_result offer=pp_evolution_offer(&evo_preview,0);
   if(offer!=PP_OK){r->evolution_seen|=bit;g->revision++;continue;}
   if(pick<PP_PETS && pp_evolution_offer(g,pick)==PP_BUSY)return false;
   if(pick>=PP_PETS && g->fishing_partner==pick-PP_PETS+1)return false;
   evo_branch=0;evolution_open(pick,pp_evolution_stone(&evo_preview,0)!=NULL);evo_training=evolution!=NULL;return evo_training;
  }
 }
 return false;
}
/* Opening a choice never mutates the pet or spends coins. */
static void evolution_open(unsigned pet, bool stone) {
  if (evolution) return;
  evo_pick=pet;pp_game *state=g;
  if(pet>=PP_PETS){if(!pp_roster_evolution_preview(g,pet,&evo_preview))return;state=&evo_preview;pet=0;}
  evo_return_page=current;
  if(state->pets[pet].species!=67)evo_branch=0;
  pp_result result = pp_evolution_offer(state, pet);
  if (result != PP_OK) { notice(pp_message(result)); return; }
  evo_pet_id = pet; evo_before_art = pp_art_id(state,pet); evo_current_art = evo_before_art;
  evo_stone = stone; evo_declined = false; evo_choosing = true;
  evo_tint = evo_colors[state->pets[pet].species % (sizeof(evo_colors)/sizeof(evo_colors[0]))];
  evo_sprite_source = NULL; memset(&evolution_clock,0,sizeof(evolution_clock));
  refresh_pending = false;
  evolution = lv_obj_create(screen);lv_obj_remove_style_all(evolution);
  lv_obj_set_size(evolution,360,360);lv_obj_set_style_bg_color(evolution,color(0x0c1c30),0);
  lv_obj_set_style_bg_opa(evolution,255,0);lv_obj_remove_flag(evolution,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t *back=button(evolution,"返回",80,56,44,44,190);
  pp_nav_apply(back,false);
  lv_obj_t *l=label(evolution,"成长的选择",70,42,220,evo_tint);
  lv_obj_set_style_text_font(l,&pp_font_18,0);
  char str[80];
  if(evo_branch)snprintf(str,sizeof(str),"%s · %s · 200币",pp_evolution_branch_name(evo_branch),evo_branch==1?"水伊布":evo_branch==2?"雷伊布":"火伊布");
  else snprintf(str,sizeof(str),"%s · Lv.%u",pp_form_name(state,pet),state->growth[pet].level);
  l=label(evolution,str,60,72,240,0xc7d9d5);lv_obj_set_style_text_font(l,&pp_font_12,0);
  evo_pet=lv_image_create(evolution);lv_obj_set_pos(evo_pet,132,94);
  if(state->pets[pet].species==67 && !evo_branch) {
    for(unsigned i=1;i<=3;i++){
      char option[80];snprintf(option,sizeof(option),"%s · %s",pp_evolution_branch_name(i),i==1?"水伊布":i==2?"雷伊布":"火伊布");
      pp_footer_text_style(button(evolution,option,88,190+(i-1)*36,184,32,186+i));
    }
    l=label(evolution,"每条分支200币 · 选择后确认",60,302,240,0xbbe6df);
    lv_obj_set_style_text_font(l,&pp_font_12,0);evolution_tick();return;
  }
  l=label(evolution,"保持原样后，这只伙伴将永久不再进化",50,191,260,0xe3c9aa);
  lv_obj_set_style_text_font(l,&pp_font_12,0);
  l=label(evolution,"并获得收藏品「纯净之心」",50,222,260,0xbbe6df);
  lv_obj_set_style_text_font(l,&pp_font_12,0);
  pp_footer_text_style(button(evolution,stone?"进化 · 200币":"进化",88,244,184,32,178));
  pp_footer_text_style(button(evolution,"保持原样 · 永久",88,286,184,32,179));
  evolution_tick();
}
/* Commit both the permanent decision and gift atomically, then present it. */
static void evolution_choose(bool decline) {
  if (!evolution || !evo_choosing) return;
  unsigned pet=evo_pet_id, before_art=evo_before_art; bool stone=evo_stone;
  const char *stone_name=evo_branch?pp_evolution_branch_name(evo_branch):stone?pp_evolution_stone(evo_state(),pet):NULL;
  pp_result result=pp_roster_evolution_choose(g,evo_pick,decline,evo_branch);
  if (result != PP_OK) {
    if (result == PP_NO_COINS) { notice(pp_message(result)); lv_obj_move_foreground(toast); return; }
    selected=pet;pp_ui_open(4);notice(pp_message(result));return;
  }
  lv_obj_delete(evolution); evolution=NULL; evo_choosing=false; evo_declined=decline;
  if(evo_pick<PP_PETS)evo_postponed_level[pet]=0;
  else pp_roster_evolution_preview(g,evo_pick,&evo_preview);
  evo_pet_id = pet;
  evo_before_art = before_art;
  evo_tint = evo_colors[evo_state()->pets[pet].species % (sizeof(evo_colors)/sizeof(evo_colors[0]))];
  evo_sprite_source = NULL;
  memset(&evolution_clock, 0, sizeof(evolution_clock));
  evo_started = lv_tick_get();
  refresh_pending = false;
  evolution = lv_obj_create(screen);
  lv_obj_remove_style_all(evolution);
  lv_obj_set_size(evolution, 360, 360);
  lv_obj_set_style_bg_color(evolution, color(0x0c1c30), 0);
  lv_obj_set_style_bg_opa(evolution, 255, 0);
  lv_obj_remove_flag(evolution, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(evolution, LV_OBJ_FLAG_CLICKABLE);
  char title[64];snprintf(title,sizeof(title),"%s进化",stone_name?stone_name:"");
  lv_obj_t *l = label(evolution, decline ? "守护最初的自己" : stone ? title : "突破进化", 70, 47, 220, evo_tint);
  lv_obj_set_style_text_font(l, &pp_font_18, 0);
  char str[96];
  snprintf(str, sizeof(str), "%s", decline ? "成长，也可以是\n守护最初的自己。" : "每一次成长，都有你的陪伴。");
  l = label(evolution, str, 55, 78, 250, 0x91b8c8);
  lv_obj_set_style_text_font(l, &pp_font_12, 0);
  /* Quiet layered glow, thin orbital rings and small coherent pixel motes. */
  for (unsigned i = 0; i < 5; i++) {
    evo_glow[i] = lv_obj_create(evolution);
    lv_obj_remove_style_all(evo_glow[i]);
    lv_obj_set_style_radius(evo_glow[i], LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(evo_glow[i], color(evo_tint), 0);
    lv_obj_remove_flag(evo_glow[i], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  }
  for (unsigned i = 0; i < 3; i++) {
    evo_ring[i] = lv_obj_create(evolution);
    lv_obj_remove_style_all(evo_ring[i]);
    lv_obj_set_style_radius(evo_ring[i], LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(evo_ring[i], 1, 0);
    lv_obj_set_style_border_color(evo_ring[i], color(evo_tint), 0);
    lv_obj_remove_flag(evo_ring[i], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  }
  for (unsigned i = 0; i < 32; i++) {
    evo_spark[i] = lv_obj_create(evolution);
    lv_obj_remove_style_all(evo_spark[i]);
    lv_obj_set_style_bg_color(evo_spark[i],
        lv_color_mix(color(0xf5f7ef), color(evo_tint), i % 4 == 0 ? 150 : 0), 0);
    lv_obj_remove_flag(evo_spark[i], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  }
  evo_pet = lv_image_create(evolution);
  lv_obj_set_pos(evo_pet, 132, 118);
  evo_gift = lv_image_create(evolution);pp_image_set_src(evo_gift,pure_heart_path);
  lv_obj_set_pos(evo_gift,252,145);lv_obj_set_style_opa(evo_gift,0,0);
  evo_name = label(evolution, "", 60, 227, 240, evo_tint);
  lv_obj_set_style_text_font(evo_name, &pp_font_18, 0);
  evo_hint = l = label(evolution, decline ? "获得收藏品 · 纯净之心\n这只伙伴将永久保持当前形态" : "新形态已解锁", 60, 253, 240, 0x8baab9);
  lv_obj_set_style_text_font(l, &pp_font_12, 0);
  lv_obj_set_style_opa(evo_hint, 0, 0);
  evo_done = button(evolution, "完成", 125, 282, 110, 38, 168);
  pp_footer_done(evo_done);
  lv_obj_add_flag(evo_done, LV_OBJ_FLAG_HIDDEN);
  evolution_tick();
}
static float evo_progress(unsigned age, unsigned start, unsigned duration) {
  if (age <= start) return 0;
  if (age >= start + duration) return 1;
  float t = (float)(age - start) / duration;
  return t * t * (3 - 2 * t);
}
static void evo_circle(lv_obj_t *obj, int radius, unsigned opacity, bool ring) {
  lv_obj_set_size(obj, radius * 2, radius * 2);
  lv_obj_set_pos(obj, 180 - radius, 166 - radius);
  if (ring) lv_obj_set_style_border_opa(obj, opacity, 0);
  else lv_obj_set_style_bg_opa(obj, opacity, 0);
}
static void evo_sprite(unsigned art, unsigned pose, uint32_t now) {
  unsigned frame = pp_animation_update(&evolution_clock, art, pose, now);
  const void *source = paths[art][frame];
  evo_current_art = art;
  if (source == evo_sprite_source) return;
  evo_sprite_source = source;
  const lv_image_dsc_t *im = pp_image_descriptor(source);
  if (!im || (im->header.cf != LV_COLOR_FORMAT_I4 && im->header.cf != LV_COLOR_FORMAT_I8)) {
    pp_image_set_src(evo_pet, source);
    lv_obj_set_pos(evo_pet, 156, 142);
    return;
  }
  for (unsigned y=0;y<96;y++) for(unsigned x=0;x<96;x++) {
    unsigned sx=x*pp_image_canvas_width(im)/96,sy=y*pp_image_canvas_height(im)/96;
    evo_pixels[y*96+x]=pp_image_indexed_pixel(im,sx,sy);
  }
  evo_descriptor = (lv_image_dsc_t){.header = {.magic=LV_IMAGE_HEADER_MAGIC,
    .cf=LV_COLOR_FORMAT_ARGB8888, .flags=LV_IMAGE_FLAGS_MODIFIABLE,
    .w=96, .h=96, .stride=96*4}, .data_size=sizeof(evo_pixels), .data=(const uint8_t *)evo_pixels};
  lv_image_cache_drop(&evo_descriptor);
  lv_image_set_src(evo_pet, &evo_descriptor);
  lv_obj_invalidate(evo_pet);
}
static void evolution_tick(void) {
  uint32_t now = lv_tick_get();
  if(training_level){evo_sprite(evo_before_art,PP_POSE_HAPPY,now);return;}
  unsigned age = now - evo_started;
  if (evo_choosing) { evo_sprite(evo_before_art,PP_POSE_IDLE,now);lv_obj_set_pos(evo_pet,132,94);return; }
  float gather = evo_progress(age, 0, 900);
  float burst = evo_progress(age, 1000, 750);
  float settle = evo_progress(age, 1700, 500);
  float light = evo_declined ? 0 : evo_progress(age, 400, 600) * (1 - evo_progress(age, 1250, 600));
  for (unsigned i = 0; i < 5; i++) {
    int radius = 84 - (int)i * 10 - (int)(gather * 8) + (int)(burst * 12);
    evo_circle(evo_glow[i], radius, (unsigned)((8 + i * 2) * (0.4f + light * 0.6f) * (1 - settle * 0.8f)), false);
  }
  for (unsigned i = 0; i < 3; i++) {
    int radius = age < 1000 ? 78 - (int)(gather * 30) + (int)i * 5
                             : 40 + (int)(burst * (40 + i * 5));
    unsigned opacity = age < 1000 ? (unsigned)(80 + gather * 60)
                                 : (unsigned)(160 * (1 - burst));
    evo_circle(evo_ring[i], radius, opacity, true);
  }
  for (unsigned i = 0; i < 32; i++) {
    float angle = i * 6.283185f / 32 + age * (i % 2 ? 0.0006f : -0.00045f);
    float distance = age < 1000 ? 78 - gather * (35 + i % 5 * 4)
                               : 24 + burst * (45 + i % 5 * 5);
    int size = i % 5 == 0 ? 3 : 2;
    lv_obj_set_size(evo_spark[i], size, size);
    lv_obj_set_pos(evo_spark[i], 180 + (int)(cosf(angle) * distance) - size/2,
                   166 + (int)(sinf(angle) * distance) - size/2);
    float shimmer = 0.7f + 0.3f * sinf(age * 0.004f + i);
    lv_obj_set_style_bg_opa(evo_spark[i], (unsigned)(210 * shimmer * (1 - settle)), 0);
  }
  evo_sprite(age < 1100 ? evo_before_art : pp_art_id(evo_state(),evo_pet_id),
             age < 1850 ? PP_POSE_IDLE : PP_POSE_HAPPY, now);
  lv_obj_set_style_image_recolor(evo_pet, lv_color_mix(color(0xf9fbf1), color(evo_tint), 210), 0);
  lv_obj_set_style_image_recolor_opa(evo_pet, (unsigned)(light * 255), 0);
  lv_obj_set_y(evo_pet, 118 - (int)(sinf(gather * 3.14159f) * 3 * (1 - burst)));
  if (age >= 1800) text(evo_name, pp_form_name(evo_state(), evo_pet_id));
  lv_obj_set_style_opa(evo_name, (unsigned)(evo_progress(age, 1800, 400) * 255), 0);
  lv_obj_set_style_opa(evo_hint, (unsigned)(settle * 255), 0);
  lv_obj_set_style_opa(evo_gift,evo_declined?(unsigned)(settle*255):0,0);
  if (age >= 2200) lv_obj_remove_flag(evo_done, LV_OBJ_FLAG_HIDDEN);
}
static void battle_timer(lv_timer_t *timer) {
  (void)timer;
  if (current == 6 && !refresh_pending && ui_screen_on)
    pp_battle_view_tick();
}
/* Called once by BUTTON_LONG_PRESS_START on the app task under the display
 * lock. Touch long presses remain available to individual controls/pet info. */
void pp_ui_boot_hold(void) {
  if(!g || !ui_screen_on || starter_mode || shop_reel.active)return;
  if(current==0){pp_ui_pat_all();return;}
  for(lv_indev_t *in=lv_indev_get_next(NULL);in;in=lv_indev_get_next(in))lv_indev_wait_release(in);
  /* BOOT only postpones a travel panel the player actually leaves. A trip
   * finishing behind training must stay queued when returning from training. */
  travel_home_deferred=travel_home_deferred || (current==5 &&
    (travel_notice_active || travel_receipt.active || pp_travel_describe(g).ready));
  pp_ui_open(0);
}
void pp_ui_bind_input(lv_indev_t *input) {
  /* The one-second touchscreen threshold is only for pet information. */
  if(input)lv_indev_set_long_press_time(input,1000);
}
void pp_ui_create(pp_game *game) {
  warehouse_sort_mode=PP_WAREHOUSE_DEFAULT;
  home_time=0;home_initialized=false;drop_was_active=false;
  memset(talk_cursors,0,sizeof(talk_cursors));memset(ambient_talk_seq,0,sizeof(ambient_talk_seq));talk_text[0]=0;talk_until=0;talk_icon=-1;talk_care=false;
  memset(home_motion,0,sizeof(home_motion));memset(home_clocks,0,sizeof(home_clocks));
  memset(happy_until,0,sizeof(happy_until));memset(facing_left,0,sizeof(facing_left));
  memset(&shop_reel,0,sizeof(shop_reel));
  g = game;pp_shop_retire_legacy(g);training_level=evo_training=false;
  pp_training_view_bind(game);
  egg_drop_deadline=g->egg_end;egg_drop_playing=false;egg_drop_started=0;
  pp_home_behavior_init(&home_behavior,g->rng^0x6a09e667u,0,g->scene);
  pp_home_conflict_init(&home_conflict,g->rng^0xbb67ae85u,0);home_conflict_particles=0;
  home_away=home_has_welcomed=home_welcome_pending=false;
  memset(home_outputs,0,sizeof(home_outputs));
  memset(&travel_receipt,0,sizeof(travel_receipt));
  travel_notice_active=travel_home_deferred=false;ui_screen_on=true;
  memset(&travel_result,0,sizeof(travel_result));
  memset(&hatch_result,0,sizeof(hatch_result));hatch_replacing=false;
  egg_present_seen=g->egg_end!=0;roster_count_seen=g->count;warehouse_count_seen=pp_warehouse_count(g);
  gym_selected=g->gym.mode?g->gym.mode-1:0;
  if(!g->gym.mode)while(gym_selected+1<PP_GYM_COUNT && pp_gym_unlocked(g,gym_selected+1))gym_selected++;
  for(lv_indev_t *in=lv_indev_get_next(NULL);in;in=lv_indev_get_next(in))pp_ui_bind_input(in);
  screen = lv_screen_active();
  notice_create();
  lv_obj_t *bottom = lv_display_get_layer_bottom(lv_obj_get_display(screen));
  lv_obj_set_style_bg_color(bottom, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(bottom, LV_OPA_COVER, 0);
  starter_mode = pp_needs_starter(game);
  pp_ui_open(starter_mode ? 13 : 0);
  lv_timer_create(animate, PP_FRAME_MS, NULL);
  lv_timer_create(battle_timer, PP_BATTLE_VIEW_FRAME_MS, NULL);
}

unsigned pp_ui_page(void) { return current; }
void pp_ui_screen(bool on) {
  /* The device reports this every loop. Only edges may rebase animation time. */
  if(ui_screen_on==on)return;
  travel_motion.last=lv_tick_get();
  battery_charge_tick=lv_tick_get();shop_reel.tick=battery_charge_tick;
  if(ui_screen_on && !on){
    if(home_conflict.active)home_conflict_release(home_time);
    pp_home_conflict_pause(&home_conflict,home_time);
  }
  ui_screen_on=on;
  pp_hospital_view_screen(on);
  pp_training_view_screen(on);
  if(current==6)pp_battle_view_resync_clock();
}

bool pp_ui_evolving(void) { return evolution != NULL; }
pp_travel_view_state pp_ui_travel_probe(void) {
  pp_travel_view_state state={0};
  if (current!=5 || !travel_layout) return state;
  lv_obj_update_layout(screen);
  state.active=true;state.receipt=travel_receipt.active;
  if(travel_notice_active) {
    state.confirmation=true;state.claimable=true;state.ready=true;
    snprintf(state.activity,sizeof(state.activity),"旅行完成");
    snprintf(state.event,sizeof(state.event),"确认领取");return state;
  }
  if(travel_receipt.active) {
    state.ready=!lv_obj_has_flag(back_button,LV_OBJ_FLAG_HIDDEN);
    state.coins=travel_receipt.coins;state.xp=travel_receipt.xp;
    state.before_level=travel_receipt.reward.before.level;state.after_level=travel_receipt.reward.after.level;
    snprintf(state.event,sizeof(state.event),"%s",travel_receipt.item);
    snprintf(state.reward,sizeof(state.reward),"金币 +%u · 经验 +%u",state.coins,state.xp);
    return state;
  }
  lv_area_t area;lv_obj_get_coords(travel_backdrop,&area);
  state.claimable=!lv_obj_has_flag(travel_claim,LV_OBJ_FLAG_HIDDEN);
  state.x=area.x1;state.y=area.y1;state.width=lv_obj_get_width(travel_backdrop);
  state.height=lv_obj_get_height(travel_backdrop);
  state.motion_ms=travel_motion.elapsed;state.effects=travel_motion.particles;
  state.background_source=lv_image_get_src(travel_backdrop);
  snprintf(state.activity,sizeof(state.activity),"%s",lv_label_get_text(travel_activity));
  snprintf(state.event,sizeof(state.event),"%s",lv_label_get_text(live_log));
  snprintf(state.reward,sizeof(state.reward),"%s",lv_label_get_text(travel_reward_label));
  return state;
}
pp_evolution_view_state pp_ui_evolution_probe(void) {
  if (!evolution) return (pp_evolution_view_state){0};
  if(training_level)return (pp_evolution_view_state){.active=true,.level=true,.before_art=evo_before_art,.current_art=evo_current_art};
  return (pp_evolution_view_state){.active=true,
    .choice=evo_choosing, .declined=evo_declined,
    .ready=!evo_choosing && !lv_obj_has_flag(evo_done, LV_OBJ_FLAG_HIDDEN),
    .age=lv_tick_get()-evo_started, .before_art=evo_before_art,
    .current_art=evo_current_art, .tint=evo_tint,
    .silhouette=lv_obj_get_style_image_recolor_opa(evo_pet, 0)};
}

pp_home_state pp_ui_home_probe(unsigned pet) {
  pp_home_state state = {0};
  if (current != 0 || pet >= g->count || !pet_img[pet]) return state;
  lv_obj_update_layout(screen);
  state.x = lv_obj_get_x(pet_img[pet]);
  state.y = lv_obj_get_y(pet_img[pet]);
  state.ground_y=home_motion[pet].y;
  state.nest_front=lv_obj_get_index(pet_img[pet])>lv_obj_get_index(egg_rim);
  state.behavior=home_behavior.pets[pet].kind;state.symbol=home_symbol_kind[pet];
  state.left = facing_left[pet];
  state.pose=home_clocks[pet].pose;
  state.frame=pp_animation_frame(pet_art(pet),state.pose,home_time-home_clocks[pet].start);
  state.visible=!lv_obj_has_flag(pet_img[pet],LV_OBJ_FLAG_HIDDEN);
  state.sleeping=g->pets[pet].sleeping;
  state.bubble_visible = !lv_obj_has_flag(bubble_box, LV_OBJ_FLAG_HIDDEN);
  state.bubble_x = lv_obj_get_x(bubble_box);
  state.bubble_y = lv_obj_get_y(bubble_box);
  state.bubble_width=lv_obj_get_width(bubble_box);state.bubble_height=lv_obj_get_height(bubble_box);
  state.bubble_icon=state.bubble_visible?talk_icon:-1;
  snprintf(state.bubble_text,sizeof(state.bubble_text),"%s",lv_label_get_text(bubble));
  state.speaker = talk_pet;
  state.drop_x = lv_obj_get_x(drop_icon);
  state.drop_y = lv_obj_get_y(drop_icon);
  state.drop_visible = !lv_obj_has_flag(drop_icon, LV_OBJ_FLAG_HIDDEN);
  state.drop_width=lv_obj_get_width(drop_icon);state.drop_height=lv_obj_get_height(drop_icon);
  state.drop_opacity=lv_obj_get_style_opa(drop_icon,0);state.drop_age=home_time-drop_started;
  state.egg_x=lv_obj_get_x(egg_icon);state.egg_y=lv_obj_get_y(egg_icon);
  state.egg_width=lv_obj_get_width(egg_icon);state.egg_height=lv_obj_get_height(egg_icon);
  state.egg_dropping=egg_drop_playing;
  return state;
}

unsigned pp_ui_pokedex_page(void) { return petdex_page; }

void pp_ui_scene_fx_preview(bool enabled) { ambient_preview=enabled; }
pp_home_conflict_state pp_ui_home_conflict_probe(void) {
  return (pp_home_conflict_state){.active=home_conflict.active,.hit=home_conflict.hit,
    .age=home_time-home_conflict.started,.attacker=home_conflict.attacker,.target=home_conflict.target,
    .slot=home_conflict.slot,.particles=home_conflict_particles};
}
#ifndef ESP_PLATFORM
bool pp_ui_home_conflict_preview(unsigned a,unsigned b,unsigned slot) {
  if(current!=0 || !ui_screen_on || evolution || wild_overlay || gift_overlay)return false;
  pp_home_actor actors[PP_PETS];home_inputs(actors);
  if(a>=g->count || b>=g->count || !actors[a].available || !actors[b].available)return false;
  if(!pp_home_conflict_start(&home_conflict,g,home_time,a,b,slot))return false;
  home_conflict_begin();return true;
}
#endif

unsigned pp_ui_collection_quality(void) {return collection_quality;}
unsigned pp_ui_collection_page(void) {return dex_page;}

pp_gym_map_state pp_ui_gym_map_probe(void) {
  return (pp_gym_map_state){.active=current==17,.gym=gym_selected,
    .cleared=current==17 && gym_clear_stamp!=NULL,
    .welcoming=current==17 && gym_guards!=NULL};
}

void pp_ui_scene_motion(bool enabled) { pp_scene_view_enabled(enabled); }

bool pp_ui_gift_pending(void) {return gift_overlay!=NULL;}
bool pp_ui_wild_alert(void) {return wild_alert!=NULL;}
bool pp_ui_wild_pending(void) {return wild_overlay!=NULL;}
bool pp_ui_wild_capturing(void) {return wild_result.active;}
unsigned pp_ui_wild_fighter(void) {return wild_selected;}

void pp_ui_scene_fx_enabled(bool enabled) { ambient_enabled=enabled; }

unsigned pp_ui_app_page(void) {return app_page;}
unsigned pp_ui_scene_selected(void) {return scene_page;}

bool pp_ui_home_interaction(unsigned kind,unsigned first,unsigned second,int x,int y) {
 if(current!=0)return false;
 pp_home_actor in[PP_PETS];home_inputs(in);
 pp_home_behavior_clear(&home_behavior);
 return pp_home_behavior_request(&home_behavior,home_time,kind,first,second,x,y,in,g->count);
}
