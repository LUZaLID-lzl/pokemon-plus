#include "battle_view.h"
#include "moves.h"
#include "pet_types.h"
#include "battle_type_icons.h"
#include "battle_relation.h"
#include "pet_animation.h"
#include "image_assets.h"
#include "scene_view.h"
#include "battle_entrance.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifndef ESP_PLATFORM
#include <assert.h>
#endif
LV_FONT_DECLARE(pp_font_12);
LV_FONT_DECLARE(pp_font_14);
LV_FONT_DECLARE(pp_font_18);
#define FX_COUNT 48
#define PET_SIZE 56
#define PET_LEFT_X 52
#define PET_RIGHT_X 252
#define PET_Y 96
#define PET_CENTER_LEFT 80
#define PET_CENTER_RIGHT 280
#define CAST_MS 900u
#define IMPACT_MS 360u
#define STORY_CAPACITY 16
#define STORY_MS 1500u
static struct {
  pp_game *g;
  lv_obj_t *types[2][2],*relations;
  uint8_t relation_factor[2],relation_kind[2],relation_level[2];
  uint16_t relation_power[2];
  lv_obj_t *pet[2], *bar[2], *hp_fill[2][8], *hp[2], *stage, *story, *damage, *retry, *back, *fx_image, *fx_layer;
  struct {int x,y,w,h;unsigned color,alpha;} fx[FX_COUNT];
  lv_area_t fx_area[2];
  const void *paths[2][PP_FRAME_COUNT];
  char root[320], strip[320], effects[4][4][320];
  lv_obj_t *balls[2][3], *ball_top[2][3], *ball_bottom[2][3], *ball_center[2][3], *names[2];
  unsigned actor[2], observed_round;
  unsigned ball_state[2][3];int hp_drawn[2];
  pp_entry_state entry_style[2];
  const void *src[2], *fx_src;
  int observed[2], shown[2], from[2], target[2], loss[2];
  unsigned species[2], art[2], used, slot;
  pp_animation_clock clocks[2];
  uint32_t start, story_start, wall_last, timeline;
  unsigned clock_remainder,entrance_mask,entrance_ms;
  char stories[STORY_CAPACITY][128];
  unsigned story_head, story_count;
  bool reply_told, result_told, story_visible;
  bool immune[2];
  bool active, round, settled;
  pp_battle_view_state probe;
} v;
static uint32_t pet_pixels[2][PET_SIZE*PET_SIZE];
static lv_image_dsc_t pet_descriptors[2];
#ifndef ESP_PLATFORM
static struct {
  bool enabled, actor_check;
  unsigned family, once_mask;
  char root[512];
  uint16_t durations[6][8];
} catalog_preview;
void pp_battle_view_catalog_preview(unsigned family, const char *frames_root,
                                   const uint16_t durations[6][8], unsigned once_mask, bool actor_check) {
  catalog_preview.enabled=family<PP_MOVE_FAMILY_COUNT && frames_root && durations;
  if(!catalog_preview.enabled)return;
  catalog_preview.family=family;catalog_preview.once_mask=once_mask;
  catalog_preview.actor_check=actor_check;
  snprintf(catalog_preview.root,sizeof(catalog_preview.root),"S:%s",frames_root);
  memcpy(catalog_preview.durations,durations,sizeof(catalog_preview.durations));
}
static unsigned catalog_frame(unsigned pose,uint32_t now) {
  pp_animation_clock *clock=&v.clocks[0];
  if(!clock->initialized || clock->pose!=pose) {
    clock->initialized=true;clock->pose=pose;clock->start=now;
  }
  unsigned action=pose/4,total=0,frame=0;
  for(unsigned i=0;i<8;i++)total+=catalog_preview.durations[action][i];
  unsigned elapsed=now-clock->start;
  if((catalog_preview.once_mask&(1u<<action)) && elapsed>=total)return action*8+7;
  elapsed%=total;
  while(frame<7 && elapsed>=catalog_preview.durations[action][frame])
    elapsed-=catalog_preview.durations[action][frame++];
  return action*8+frame;
}
#endif
static void pet_frame(unsigned side,const void *source) {
  const lv_image_dsc_t *im=pp_image_descriptor(source);
  if(!im || (im->header.cf!=LV_COLOR_FORMAT_I4 && im->header.cf!=LV_COLOR_FORMAT_I8)) {
    pp_image_set_src(v.pet[side],source);return;
  }
  for(unsigned y=0;y<PET_SIZE;y++) for(unsigned x=0;x<PET_SIZE;x++) {
    unsigned sx=(2*x+1)*pp_image_canvas_width(im)/(2*PET_SIZE);
    unsigned sy=(2*y+1)*pp_image_canvas_height(im)/(2*PET_SIZE);
    pet_pixels[side][y*PET_SIZE+x]=pp_image_indexed_pixel(im,sx,sy);
  }
  /* These descriptors deliberately bypass the decoder cache: their bounded
   * buffers are repainted only when the animation frame changes. */
  pet_descriptors[side]=(lv_image_dsc_t){
    .header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_ARGB8888,
      .flags=LV_IMAGE_FLAGS_MODIFIABLE,.w=PET_SIZE,.h=PET_SIZE,.stride=PET_SIZE*4},
    .data_size=sizeof(pet_pixels[side]),.data=(const uint8_t *)pet_pixels[side]};
  lv_image_cache_drop(&pet_descriptors[side]);
  lv_image_set_src(v.pet[side],&pet_descriptors[side]);
  lv_obj_invalidate(v.pet[side]);
}

static const unsigned palette[] = {0xffdf69, 0x8fe68b, 0xff9a63, 0x6edaff,
                                   0xe6d2c1, 0xb8d961, 0xc78de0, 0xb4e8ed, 0xd6b685,
                                   0xbcb397, 0xf4a8c5, 0xd695ed, 0xe6a083, 0xa8e5f3, 0xcbd8e2, 0x9f87dc,
                                   0x796b99, 0x9296ff};
static lv_obj_t *caption(lv_obj_t *parent, const char *s, int x, int y, int w,
                         const lv_font_t *font, unsigned c) {
  lv_obj_t *o = lv_label_create(parent);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_width(o, w);
  lv_obj_set_style_text_font(o, font, 0);
  lv_obj_set_style_text_align(o, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(o, lv_color_hex(c), 0);
  lv_label_set_text(o, s);
  return o;
}
static void value(lv_obj_t *o, const char *s) {
  if (strcmp(lv_label_get_text(o), s))
    lv_label_set_text(o, s);
}
/* Narration uses fixed storage and the UI tick; leaving battle cancels it with v. */
static const char *pet_name(unsigned side) {
#ifndef ESP_PLATFORM
  if(!side && catalog_preview.enabled)return pp_move_family_names[catalog_preview.family];
#endif
  for (unsigned i = 0; i < PP_DEX_COUNT; i++)
    if (pp_pokedex[i].art == v.art[side]) return pp_pokedex[i].name;
  return pp_pet_names[v.species[side]];
}
static const pp_move *view_move(unsigned side) {
#ifndef ESP_PLATFORM
  if(!side && catalog_preview.enabled)return pp_move_catalog_at(catalog_preview.family,v.slot);
#endif
  return (v.g->gym.mode || v.g->wild.phase>=PP_WILD_BATTLE)?pp_battle_move(v.g,side,v.slot):pp_move_at(v.species[side],v.slot);
}
static void relation_pixel(int x,int y,unsigned width,unsigned color,unsigned opacity,void *user) {
  lv_event_t *e=user;lv_area_t area;lv_obj_get_coords(v.relations,&area);
  lv_area_t pixel={area.x1+x,area.y1+y,area.x1+x+(int)width-1,area.y1+y};
  lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=lv_color_hex(color);d.bg_opa=opacity;
  lv_draw_rect(lv_event_get_layer(e),&d,&pixel);
}
static void relations_draw(lv_event_t *e) {
  for(unsigned side=0;side<2;side++)if(v.relation_level[side])
    pp_relation_paint(side,v.relation_kind[side],v.relation_level[side],relation_pixel,e);
}
static void actor_relations(void) {
  for(unsigned side=0;side<2;side++){
    bool typed=v.g->gym.mode||v.g->wild.phase>=PP_WILD_BATTLE;
    v.relation_factor[side]=typed?pp_gym_effectiveness(v.g,side,v.slot):100;
    v.relation_power[side]=typed?pp_battle_move_power(v.g,side,v.slot):view_move(side)->power;
    v.relation_kind[side]=pp_relation_kind(v.relation_factor[side],v.relation_power[side]);
  }
  lv_obj_invalidate(v.relations);
}
/* Current form types, independent of the selected move or family base form.
 * Update only on entry/relay; descriptors are constant flash data. */
static void actor_types(unsigned side) {
  const int8_t *types=pp_pet_types[v.art[side]];
  unsigned count=types[1]<0?1:2;
  for(unsigned i=0;i<2;i++) {
    lv_obj_t *icon=v.types[side][i];
    if(i>=count){lv_obj_add_flag(icon,LV_OBJ_FLAG_HIDDEN);continue;}
    lv_image_set_src(icon,&pp_battle_type_icons[(unsigned)types[i]]);
    lv_obj_set_pos(icon,side?224+(int)i*18:136-(int)count*18+2+(int)i*18,0);
    lv_obj_remove_flag(icon,LV_OBJ_FLAG_HIDDEN);
  }
}
void pp_pet_type_badges_create(lv_obj_t *parent,unsigned art,int x,int y) {
  if(art>=PP_DEX_COUNT)return;
  for(unsigned i=0;i<2;i++) {
    int type=pp_pet_types[art][i];if(type<0)continue;
    lv_obj_t *icon=lv_image_create(parent);
    lv_image_set_src(icon,&pp_battle_type_icons[type]);
    lv_obj_set_pos(icon,x+(int)i*64,y);
    lv_obj_t *name=lv_label_create(parent);
    lv_label_set_text(name,pp_battle_type_names[type]);
    lv_obj_set_pos(name,x+(int)i*64+20,y+1);
    lv_obj_set_style_text_font(name,&pp_font_12,0);
    lv_obj_set_style_text_color(name,lv_color_hex(0xb6cbce),0);
  }
}
static void actor_load(unsigned side) {
  pp_game *g=v.g;
  unsigned art=side?(g->wild.phase>=PP_WILD_BATTLE?pp_pokedex[g->wild.dex].art:pp_gym_current(g)->team[g->gym.enemy].art):pp_art_id(g,g->battle_pet);
  v.art[side]=art;
  actor_types(side);
  for(unsigned i=0;i<PP_DEX_COUNT;i++)if(pp_pokedex[i].art==art)v.species[side]=pp_pokedex[i].species;
  for(unsigned f=0;f<pp_pet_frame_count(art);f++) {
    char path[384];snprintf(path,sizeof(path),"%s/%s/%u-%u.bin",v.root,side?"pets-left":"pets",art,f);
    v.paths[side][f]=pp_image_descriptor(path);
  }
  v.hp_drawn[side]=-1;
  v.src[side]=NULL;memset(&v.clocks[side],0,sizeof(v.clocks[side]));
  v.actor[side]=side?g->gym.enemy:g->gym.ally;
  v.observed[side]=v.shown[side]=v.target[side]=side?g->enemy_hp:g->battle_hp;
  v.from[side]=v.shown[side];v.loss[side]=0;
  if(v.names[side]) {
    char text[64];snprintf(text,sizeof(text),"%s Lv.%u",pet_name(side),side?(g->wild.phase>=PP_WILD_BATTLE?g->wild.level:pp_gym_current(g)->team[g->gym.enemy].level):g->growth[g->battle_pet].level);
    value(v.names[side],text);
  }
}
static void team_balls_create(lv_obj_t *parent) {
  for(unsigned side=0;side<2;side++)for(unsigned i=0;i<pp_battle_team_count(v.g);i++) {
    lv_obj_t *ball=lv_obj_create(parent);lv_obj_remove_style_all(ball);
    lv_obj_set_pos(ball,(side?264:52)+i*16,65);lv_obj_set_size(ball,12,12);
    lv_obj_set_style_radius(ball,LV_RADIUS_CIRCLE,0);lv_obj_set_style_clip_corner(ball,true,0);
    lv_obj_set_style_border_width(ball,1,0);lv_obj_set_style_border_color(ball,lv_color_hex(0x182a38),0);
    lv_obj_remove_flag(ball,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);v.balls[side][i]=ball;
    for(unsigned half=0;half<2;half++) {
      lv_obj_t *part=lv_obj_create(ball);lv_obj_remove_style_all(part);lv_obj_set_pos(part,1,half?7:1);lv_obj_set_size(part,10,4);
      lv_obj_set_style_bg_opa(part,255,0);lv_obj_remove_flag(part,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
      if(half)v.ball_bottom[side][i]=part;else v.ball_top[side][i]=part;
    }
    lv_obj_t *center=lv_obj_create(ball);lv_obj_remove_style_all(center);lv_obj_set_pos(center,4,4);lv_obj_set_size(center,4,4);lv_obj_set_style_radius(center,2,0);lv_obj_set_style_bg_opa(center,255,0);v.ball_center[side][i]=center;
  }
}
static void team_balls_tick(void) {
  for(unsigned side=0;side<2;side++)for(unsigned i=0;i<pp_battle_team_count(v.g);i++) {
    unsigned current=side?v.g->gym.enemy:v.g->gym.ally;
    bool alive=(side?v.g->gym.enemy_hp[i]:v.g->gym.hp[i])>0;
    if(i==current)alive=v.shown[side]>0;
    unsigned state=(alive?1u:0u)|(i==current?2u:0u);
    if(v.ball_state[side][i]==state+1)continue;
    v.ball_state[side][i]=state+1;
    lv_obj_set_style_bg_color(v.ball_top[side][i],lv_color_hex(alive?0xed7968:0x40515d),0);
    lv_obj_set_style_bg_color(v.ball_bottom[side][i],lv_color_hex(alive?0xd5e0e5:0x40515d),0);
    lv_obj_set_style_bg_color(v.ball_center[side][i],lv_color_hex(alive?0xffffff:0x61717c),0);
    lv_obj_set_style_border_color(v.balls[side][i],lv_color_hex(i==current&&alive?0xf4d58c:0x182a38),0);
  }
}
static void story_push(const char *text) {
  if (v.story_count >= STORY_CAPACITY) return;
  unsigned tail = (v.story_head + v.story_count++) % STORY_CAPACITY;
  snprintf(v.stories[tail], sizeof(v.stories[tail]), "%s", text);
}
static void story_move(unsigned side) {
  char text[128];
  snprintf(text, sizeof(text), "%s%s使用了%s", side ? "对方的" : "",
           pet_name(side), view_move(side)->name);
  story_push(text);
}
static void story_tick(uint32_t now) {
  if (v.story_visible && now - v.story_start >= STORY_MS) {
    v.story_visible = false;
    lv_obj_add_flag(v.story, LV_OBJ_FLAG_HIDDEN);
  }
  if (!v.story_visible && v.story_count) {
    value(v.story, v.stories[v.story_head]);
    v.story_head = (v.story_head + 1) % STORY_CAPACITY;
    v.story_count--;
    v.story_start = now;
    v.story_visible = true;
    lv_obj_remove_flag(v.story, LV_OBJ_FLAG_HIDDEN);
  }
  if (!v.story_visible) return;
  unsigned age = now - v.story_start;
  unsigned fade = age > 1000 ? age - 1000 : 0;
  lv_obj_set_y(v.story, 176 - (int)(fade * 16 / 500));
  lv_obj_set_style_opa(v.story, 255 - fade * 255 / 500, 0);
}
/* Fixed draw commands replace particle objects. At most two dirty areas retain
 * old and new pixels, so a 40-particle release cannot overflow LVGL's queue. */
static void fx_draw(lv_event_t *e){
 lv_area_t origin;lv_obj_get_coords(v.fx_layer,&origin);
 for(unsigned i=0;i<v.used;i++){
  lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=lv_color_hex(v.fx[i].color);d.bg_opa=v.fx[i].alpha;
  lv_area_t a={origin.x1+v.fx[i].x,origin.y1+v.fx[i].y,origin.x1+v.fx[i].x+v.fx[i].w-1,origin.y1+v.fx[i].y+v.fx[i].h-1};
  lv_draw_rect(lv_event_get_layer(e),&d,&a);
 }
}
static void fx_commit(void){
 lv_area_t next[2]={{360,215,-1,-1},{360,215,-1,-1}},origin;lv_obj_get_coords(v.fx_layer,&origin);
 for(unsigned i=0;i<v.used;i++){
  lv_area_t *a=&next[v.fx[i].x>=180];
  if(v.fx[i].x<a->x1)a->x1=v.fx[i].x;
  if(v.fx[i].y<a->y1)a->y1=v.fx[i].y;
  if(v.fx[i].x+v.fx[i].w-1>a->x2)a->x2=v.fx[i].x+v.fx[i].w-1;
  if(v.fx[i].y+v.fx[i].h-1>a->y2)a->y2=v.fx[i].y+v.fx[i].h-1;
 }
 for(unsigned side=0;side<2;side++){
  lv_area_t dirty=next[side],old=v.fx_area[side];
  if(old.x2>=0){if(old.x1<dirty.x1)dirty.x1=old.x1;if(old.y1<dirty.y1)dirty.y1=old.y1;if(old.x2>dirty.x2)dirty.x2=old.x2;if(old.y2>dirty.y2)dirty.y2=old.y2;}
  v.fx_area[side]=next[side];
  if(dirty.x2>=0){dirty.x1+=origin.x1;dirty.x2+=origin.x1;dirty.y1+=origin.y1;dirty.y2+=origin.y1;lv_obj_invalidate_area(v.fx_layer,&dirty);}
 }
}
static void pixel(int x,int y,int w,int h,unsigned color,unsigned opacity){
 if(v.used>=FX_COUNT)return;
 v.fx[v.used++]=(typeof(v.fx[0])){x,y+6,w,h,color,opacity};
}
static void entry_paint(void *data,int x,int y,int w,int h,unsigned color,unsigned alpha){(void)data;pixel(x,y-6,w,h,color,alpha);}
static void effects(unsigned species, unsigned side, unsigned ms) {
  int source = side ? PET_CENTER_RIGHT : PET_CENTER_LEFT, dest = side ? PET_CENTER_LEFT : PET_CENTER_RIGHT;
  float progress = (float)ms / IMPACT_MS;
  if (progress > 1)
    progress = 1;
  unsigned type = view_move(side)->type;
  unsigned tint = palette[type];
  if (ms >= 100 && ms < IMPACT_MS + 120) {
    if (type == PP_DRAGON) {
      int x=source+(int)((dest-source)*progress);
      for(unsigned i=0;i<24;i++) {
        float angle=i*6.283185f/12+ms*.012f;
        int radius=8+(int)(i/12)*8;
        pixel(x+(int)(cosf(angle)*radius),116+(int)(sinf(angle)*radius),4,4,
              i%3?tint:0xddeeff,240);
      }
    } else if (type == PP_DARK) {
      int x=source+(int)((dest-source)*progress);
      for(unsigned i=0;i<12;i++) {
        int gap=ms<300?(300-(int)ms)/24:0;
        pixel(x-18+(int)i*3,107-gap+(int)(i%2)*4,4,7,i%2?tint:0xbaa2d7,240);
        pixel(x-18+(int)i*3,125+gap-(int)(i%2)*4,4,7,tint,240);
      }
    } else if (type == PP_GROUND) {
      for(unsigned i=0;i<24;i++) {
        int x=source+(int)((dest-source)*progress)+(int)(i%6)*5-15;
        pixel(x,124+(int)(i/6)*3-(int)(sinf(ms*.02f+i)*6),3,3,i%3?tint:0xf3dfb4,235);
      }
    } else if (type == PP_ICE) {
      int x=source+(int)((dest-source)*progress);
      for(unsigned i=0;i<12;i++) {
        int dx=(int)i*3-18,dy=(int)(sinf(ms*.015f+i)*10);
        pixel(x+dx,116+dy,3,9,i%2?tint:0xf4ffff,240);
        pixel(x+dx-3,119+dy,9,3,tint,240);
      }
    } else if (type == PP_STEEL) {
      for(unsigned i=0;i<32;i++) {
        float t=i/31.0f;if(t>progress)break;
        pixel(source+(int)((dest-source)*t),114+(int)(i%3)*3,7,5,i%3?tint:0xffffff,245);
      }
    } else if (type == PP_GHOST) {
      int x=source+(int)((dest-source)*progress);
      for(unsigned i=0;i<24;i++) {
        float a=i*3.141593f/12;
        int r=6+(int)(i%3)*3;
        pixel(x+(int)(cosf(a)*r),116+(int)(sinf(a)*r),4,4,
              i%3?tint:0xd5c7f5,235);
      }
    } else if (type == PP_PSYCHIC) {
      int x=source+(int)((dest-source)*progress);
      for(unsigned ring=0;ring<2;ring++)for(unsigned i=0;i<16;i++) {
        float a=i*6.283185f/16+ms*.006f;
        int radius=10+(int)ring*9;
        pixel(x+(int)(cosf(a)*radius),118+(int)(sinf(a)*radius*.65f),3,3,
              ring?tint:0xf8d9ff,230);
      }
    } else if (type == PP_FIGHTING) {
      int x=source+(int)((dest-source)*progress);
      for(unsigned i=0;i<12;i++) {
        pixel(x-18+(int)i*3,103+(int)i*2,4,4,i%3?tint:0xffe5c5,245);
        if(v.slot==3)pixel(x-18+(int)i*3,127-(int)i*2,4,4,tint,245);
      }
    } else if (type == PP_FAIRY) {
      int x=source+(int)((dest-source)*progress);
      for(unsigned i=0;i<12;i++) {
        int dx=(int)(cosf(i*1.9f+ms*.004f)*18);
        int dy=(int)(sinf(i*1.9f+ms*.004f)*14);
        unsigned c=i%3?tint:0xfff0dc;
        pixel(x+dx-3,118+dy,7,2,c,235);
        pixel(x+dx,115+dy,2,7,c,235);
      }
    } else if (type == PP_BUG || type == PP_POISON || type == PP_FLYING) {
      int x = source + (int)((dest-source)*progress);
      for (unsigned i=0;i<12;i++) {
        int dx=(int)i*3-18;
        int dy=(int)(sinf(i*.5f+ms*.01f)*10);
        if(type==PP_BUG) { /* fine silk strands and compact bites */
          pixel(x+dx,110+dy,5,2,i%3?tint:0xf3f4c7,235);
          pixel(x+dx,123-dy,4,2,tint,235);
        } else if(type==PP_POISON) {
          pixel(x+dx,110+(int)(i%3)*7,9,2,i%2?tint:0xf2c8ff,245);
        } else {
          pixel(x+dx,108+dy,3,9,i%2?tint:0xf0ffff,220);
          pixel(x+dx+4,111+dy,2,6,tint,220);
        }
      }
    } else if (type == PP_NORMAL) {
      int x = source + (int)((dest - source) * progress);
      for (unsigned i = 0; i < 12; i++) {
        if (species == 2) {
          for (unsigned claw = 0; claw < 3; claw++)
            pixel(x - 12 + (int)i * 2 + (int)claw * 6, 104 + (int)i * 2, 3, 4,
                  claw % 2 ? tint : 0xffffff, 240);
        } else
          pixel(x - 18 + (int)i * 3, 108 + (int)(i % 3) * 5, 3, 16,
                i % 2 ? tint : 0xffffff, 240);
      }
    } else if (type == PP_FIRE && v.slot == 2) {
      for (unsigned tooth = 0; tooth < 6; tooth++) {
        int gap = ms < 360 ? (360 - (int)ms) / 24 : 0;
        pixel(dest - 18 + (int)tooth * 6, 106 - gap + (int)(tooth % 2) * 3, 4,
              8, 0xffdea0, 255);
        pixel(dest - 18 + (int)tooth * 6, 126 + gap - (int)(tooth % 2) * 3, 4,
              8, 0xffa448, 255);
      }
    } else if (type == PP_WATER && v.slot == 3) {
      int x = source + (int)((dest - source) * progress);
      for (unsigned i = 0; i < 24; i++) {
        float angle = i * 6.283185f / 24;
        pixel(x + (int)(cosf(angle) * 17), 118 + (int)(sinf(angle) * 17), 4, 4,
              i % 3 ? tint : 0xf2ffff, 255);
      }
    } else if ((type == PP_ELECTRIC && v.slot == 2) ||
               (type == PP_GRASS && v.slot > 0) ||
               (type == PP_WATER && v.slot > 0)) {
      unsigned n = v.slot == 3 ? 12 : 6;
      for (unsigned i = 0; i < n; i++) {
        float t = progress - i * .035f;
        if (t < 0)
          continue;
        int x = source + (int)((dest - source) * t);
        int y = 118 + (int)(sinf(i * 2 + ms * .012f) * 12);
        if (type == PP_GRASS && v.slot == 3)
          y = 94 + (int)(t * 24);
        pixel(x, y, v.slot == 2 ? 9 : 6,
              type == PP_GRASS && v.slot == 1 ? 3 : 6, tint, 255);
        pixel(x + 1, y + 1, 3, 3, 0xf2ffff, 255);
      }
    } else if (type == PP_ELECTRIC) {
      // Jagged current grows from the caster; alternating white-hot core.
      for (unsigned i = 0; i < 28; i++) {
        float t = i / 27.0f;
        if (t > progress)
          break;
        int y = 118 + (int[]){-8, -2, 6, 0}[i % 4];
        pixel(source + (int)((dest - source) * t), y, 6, 6,
              i % 3 ? tint : 0xfff8d6, 255);
      }
    } else if (type == PP_GRASS) {
      // Two flexible stems arc out, snap onto the target, then retract.
      for (unsigned i = 0; i < 32; i++) {
        float t = (i % 16) / 15.0f;
        if (t > progress)
          continue;
        int bend = (int)(sinf(t * 3.14159f) * (ms < 300 ? 20 : 8));
        pixel(source + (int)((dest - source) * t),
              120 + (i < 16 ? -bend : bend), 7, 4, i % 3 ? 0x4fab69 : tint,
              255);
      }
    } else if (type == PP_FIRE) {
      // A trail of small flames rather than a solid laser beam.
      for (unsigned i = 0; i < (v.slot == 3 ? 12u : 5u); i++) {
        float t = progress - i * (v.slot == 3 ? .055f : .13f);
        if (t < 0)
          continue;
        int x = source + (int)((dest - source) * t),
            y = 118 + (int)(sinf(t * 6) * 5);
        pixel(x - 5, y - 4, 12, 10, 0xe85b45, 255);
        pixel(x - 3, y - 8, 8, 12, 0xffa448, 255);
        pixel(x - 1, y - 2, 4, 6, 0xffe49a, 255);
      }
    } else {
      // Continuous water jet with staggered droplets and a pale center.
      for (unsigned i = 0; i < 28; i++) {
        float t = i / 27.0f;
        if (t > progress)
          break;
        pixel(source + (int)((dest - source) * t),
              118 + (int)(sinf(t * 14 - ms * .02f) * 4), 7, i % 3 ? 6 : 10,
              i % 3 ? 0x3baff0 : 0xc1f5ff, 255);
      }
    }
  }
  if (ms >= IMPACT_MS && ms < 700) {
    unsigned age = ms - IMPACT_MS;
    for (unsigned i = 0; i < 8; i++) {
      float angle = i * 3.14159f / 4;
      int radius = 8 + age / 12;
      pixel(dest + (int)(cosf(angle) * radius),
            118 + (int)(sinf(angle) * radius), 4, 4, tint,
            255 - age * 180 / 340);
    }
  }
}
void pp_battle_view_close(void) { v.active = false; }
pp_battle_view_state pp_battle_view_probe(void) {
  pp_battle_view_state state = v.probe;
  state.active = v.active;
  if (v.active && v.story) {
    lv_obj_update_layout(v.story);
    snprintf(state.narration, sizeof(state.narration), "%s",
             v.story_visible ? lv_label_get_text(v.story) : "");
    state.narration_pending = v.story_count;
    state.narration_opacity = v.story_visible ? lv_obj_get_style_opa(v.story, 0) : 0;
    state.narration_y = lv_obj_get_y(v.story);
  }
  return state;
}
void pp_battle_view_create(lv_obj_t *parent, pp_game *g, const char *root,
                           lv_obj_t *retry, lv_obj_t *back) {
  memset(&v, 0, sizeof(v));
  v.hp_drawn[0]=v.hp_drawn[1]=-1;
  v.entry_style[0].opacity=v.entry_style[1].opacity=255;
  v.wall_last = lv_tick_get();
  v.active = true;
  v.g = g;
  v.retry = retry;
  v.back = back;
  snprintf(v.root,sizeof(v.root),"%s",root);
  v.observed_round=g->battle_round;
  v.slot=g->battle_round?(g->battle_round-1)%PP_MOVE_SLOTS:0;
  v.species[0] = g->pets[g->battle_pet].species;
  v.species[1] = (v.species[0] + 1) % PP_SPECIES;
  /* Development rosters may precede their generated animation tables. Never
   * index an unfinished opponent's frames outside the compiled resource bank. */
  for (unsigned i = 0; i < PP_SPECIES && pp_base_art(v.species[1]) >= PP_ART_COUNT; i++)
    v.species[1] = (v.species[1] + 1) % PP_SPECIES;
  v.stage = caption(lv_obj_get_parent(parent), "", 80, 38, 200, &pp_font_18, 0x87a3b2);
  char s[64];
  snprintf(s, sizeof(s), "STAGE %02lu",
           (unsigned long)g->stage +
               (g->battling || g->battle_hp <= 0 ? 1 : 0));
  if(pp_battle_encounter(g))snprintf(s,sizeof(s),"%s",pp_gym_current(g)->title);
  else if(g->gym.mode)snprintf(s,sizeof(s),"%s · 推荐 Lv.%u",pp_gym_current(g)->title,pp_gym_recommended_level(g->gym.mode-1));
  if(g->wild.phase>=PP_WILD_BATTLE)snprintf(s,sizeof(s),g->legend.active?"神兽挑战 · %s":"野外对战 · %s",pp_pokedex[g->wild.dex].name);
  value(v.stage,s);
  for (unsigned i = 0; i < 2; i++) {
    /* Scanline rails meet the circular scene edge, with no outer pill cap.
     * Compute HP over the visible span so hidden offscreen length cannot distort it. */
    v.bar[i]=lv_obj_create(parent);lv_obj_remove_style_all(v.bar[i]);
    lv_obj_set_pos(v.bar[i],i?196:0,20);lv_obj_set_size(v.bar[i],164,8);
    lv_obj_remove_flag(v.bar[i],LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    for(unsigned row=0;row<8;row++) {
      float dy=84+(int)row-179.5f;
      int edge=(int)ceilf(179.5f-sqrtf(180.0f*180.0f-dy*dy));
      int cap=(int[]){2,1,0,0,0,0,1,2}[row];
      lv_obj_t *rail=lv_obj_create(v.bar[i]);lv_obj_remove_style_all(rail);
      lv_obj_set_pos(rail,i?cap:edge,row);lv_obj_set_size(rail,164-edge-cap,1);
      lv_obj_set_style_bg_opa(rail,255,0);
      lv_obj_set_style_bg_color(rail,lv_color_hex(0x263d4d),0);
      lv_obj_remove_flag(rail,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
      v.hp_fill[i][row]=lv_obj_create(v.bar[i]);lv_obj_remove_style_all(v.hp_fill[i][row]);
      lv_obj_set_style_bg_opa(v.hp_fill[i][row],255,0);
      lv_obj_set_style_bg_color(v.hp_fill[i][row],lv_color_hex(i?0xf1598a:0x35d79c),0);
      lv_obj_remove_flag(v.hp_fill[i][row],LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    }
    v.hp[i] = caption(parent, "", i ? 204 : 48, 30, 108, &pp_font_12, 0x9fb7bd);
    v.observed[i] = v.shown[i] = v.target[i] = i ? g->enemy_hp : g->battle_hp;
    for(unsigned type=0;type<2;type++)v.types[i][type]=lv_image_create(parent);
  }
  v.relations=lv_obj_create(parent);lv_obj_remove_style_all(v.relations);
  lv_obj_set_pos(v.relations,144,-1);lv_obj_set_size(v.relations,72,19);
  lv_obj_remove_flag(v.relations,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(v.relations,relations_draw,LV_EVENT_DRAW_MAIN,NULL);
  if(pp_battle_encounter(g))snprintf(v.strip,sizeof(v.strip),"%s/training/%u.bin",root,g->scene);
  else if(g->gym.mode)snprintf(v.strip,sizeof(v.strip),"%s/gyms/%s.bin",root,pp_gym_current(g)->arena_art);
  else snprintf(v.strip, sizeof(v.strip), "%s/battle-scenes/%u.bin", root, g->wild.phase>=PP_WILD_BATTLE?g->wild.scene:g->scene);
  lv_obj_t *bg_parent=parent;
  if(pp_battle_encounter(g)){
    bg_parent=lv_obj_create(parent);lv_obj_remove_style_all(bg_parent);lv_obj_set_pos(bg_parent,0,44);lv_obj_set_size(bg_parent,360,112);lv_obj_remove_flag(bg_parent,LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_CLICKABLE);
  }
  lv_obj_t *bg = lv_image_create(bg_parent);
  pp_image_set_src(bg, v.strip);
  if(pp_battle_encounter(g)){pp_scene_view_pixel_set(bg,v.strip,2,0,0);lv_obj_set_pos(bg,0,-108);}
  else lv_obj_set_pos(bg, 0, 44);
  for (unsigned i = 0; i < 2; i++) {
    unsigned art = i ? pp_base_art(v.species[i]) : pp_art_id(g, g->battle_pet);
    if (art >= PP_ART_COUNT)
      art = pp_base_art(v.species[i]);
    v.art[i] = art;
    actor_types(i);
    for (unsigned f = 0; f < pp_pet_frame_count(art); f++) {
      char path[384];snprintf(path,sizeof(path), "%s/%s/%u-%u.bin",root,i?"pets-left":"pets",art,f);
      v.paths[i][f]=pp_image_descriptor(path);
    }
#ifndef ESP_PLATFORM
    if(!i && catalog_preview.enabled)for(unsigned f=0;f<48;f++) {
      char path[560];snprintf(path,sizeof(path),"%s/%u.bin",catalog_preview.root,f);
      v.paths[i][f]=pp_image_descriptor(path);
      assert(v.paths[i][f] && "Catalog preview must load its actual pet frames");
    }
#endif
    v.pet[i] = lv_image_create(parent);
    lv_obj_set_pos(v.pet[i],i ? PET_RIGHT_X : PET_LEFT_X,PET_Y);
  }
  v.fx_layer=lv_obj_create(parent);lv_obj_remove_style_all(v.fx_layer);
  lv_obj_set_pos(v.fx_layer,0,0);lv_obj_set_size(v.fx_layer,360,215);
  lv_obj_remove_flag(v.fx_layer,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(v.fx_layer,fx_draw,LV_EVENT_DRAW_MAIN,NULL);
  v.fx_area[0]=v.fx_area[1]=(lv_area_t){360,215,-1,-1};
  for (unsigned type = 0; type < 4; type++)
    for (unsigned phase = 0; phase < 4; phase++)
      snprintf(v.effects[type][phase], sizeof(v.effects[type][phase]),
               "%s/effects/%u-%u.bin", root, type, phase);
  v.fx_image = lv_image_create(parent);
  lv_obj_add_flag(v.fx_image, LV_OBJ_FLAG_HIDDEN);
  v.damage = caption(parent, "", 70, 80, 64, &pp_font_18, 0xffebbb);
  lv_obj_set_style_text_outline_stroke_color(v.damage, lv_color_hex(0x152c3c),
                                             0);
  lv_obj_set_style_text_outline_stroke_width(v.damage, 1, 0);
  v.story = caption(parent, "", 56, 176, 248, &pp_font_14, 0xdce8ee);
  lv_label_set_long_mode(v.story, LV_LABEL_LONG_WRAP);
  lv_obj_add_flag(v.story, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(v.story, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

  if(g->gym.mode || g->wild.phase>=PP_WILD_BATTLE) {
    if(g->gym.mode)team_balls_create(lv_obj_get_parent(parent));
    for(unsigned i=0;i<2;i++) {
      v.names[i]=caption(parent,"",i?210:28,157,124,&pp_font_12,0xa5b8c2);
      actor_load(i);
    }
  }
  actor_relations();
  lv_obj_add_flag(v.damage, LV_OBJ_FLAG_HIDDEN);
  if(g->battling&&!g->battle_round)v.entrance_mask=3;
#ifndef ESP_PLATFORM
  if(catalog_preview.enabled)v.entrance_mask=0;
#endif
  pp_battle_view_tick();
}
void pp_battle_view_resync_clock(void){v.wall_last=lv_tick_get();}
void pp_battle_view_tick(void) {
  if (!v.active)
    return;
  pp_game *g = v.g;
  /* One 3/2 clock keeps pet poses, FX, HP and narration synchronized.
   * Accumulate deltas so the LVGL millisecond wrap remains continuous. */
  uint32_t wall = lv_tick_get();
  uint32_t delta=wall-v.wall_last;
  bool training=pp_battle_encounter(g);
  if(v.entrance_mask){v.entrance_ms+=training?delta*3/2:delta;if(v.entrance_ms>=PP_BATTLE_ENTRANCE_MS)v.entrance_mask=0;}
  unsigned denominator=training?4u:2u;
  uint64_t scaled = (uint64_t)delta * (training?9u:3u) + v.clock_remainder;
  v.wall_last = wall;
  v.clock_remainder = scaled % denominator;
  v.timeline += scaled / denominator;
  uint32_t now = v.timeline;
  if(g->gym.mode && (v.actor[0]!=g->gym.ally || v.actor[1]!=g->gym.enemy)) {
    unsigned entrant=v.actor[1]!=g->gym.enemy?1:0;
    v.entrance_mask=0;v.entrance_ms=0;
    for(unsigned i=0;i<2;i++)if(v.actor[i]!=(i?g->gym.enemy:g->gym.ally)){v.entrance_mask|=1u<<i;actor_load(i);}
    v.round=false;v.observed_round=g->battle_round;v.slot=g->battle_round%PP_MOVE_SLOTS;actor_relations();
    char line[96];snprintf(line,sizeof(line),"%s上场了",pet_name(entrant));
    /* An entrance replaces stale narration, without recreating the arena. */
    v.story_count=0;v.story_visible=false;story_push(line);
  }
  bool entrance=v.entrance_mask!=0;
  int actual[] = {g->battle_hp, g->enemy_hp};
  if (!entrance && (actual[0] != v.observed[0] || actual[1] != v.observed[1] || v.observed_round!=g->battle_round)) {
    v.observed_round=g->battle_round;
    v.start = now;
    v.slot = g->battle_round ? (g->battle_round - 1) % PP_MOVE_SLOTS : 0;
    v.round = true;
    v.reply_told = false;
    story_move(0);
    v.settled = false;
    for (unsigned i = 0; i < 2; i++) {
      v.from[i] = v.shown[i];
      v.target[i] = v.observed[i] = actual[i];
      v.loss[i] = v.from[i] - v.target[i];
    }
    actor_relations();
    /* Snapshot the actual matchup at the start of this round. Zero-power
     * moves and other zero-HP changes are not type immunity. */
    for(unsigned side=0;side<2;side++)v.immune[side]=
      (g->gym.mode || g->wild.phase>=PP_WILD_BATTLE) && !v.loss[1-side] &&
      pp_battle_move_power(g,side,v.slot)>0 && !pp_gym_effectiveness(g,side,v.slot);
  }
  unsigned elapsed = now - v.start;
  unsigned duration = (v.target[1]>0) ? CAST_MS * 2 : CAST_MS;
  bool casting = !entrance && v.round && elapsed < duration;
  unsigned side = casting && elapsed >= CAST_MS ? 1 : 0;
  unsigned ms = elapsed % CAST_MS;
  for(unsigned i=0;i<2;i++) {
    unsigned level=!v.relation_power[i]?0:casting&&side==i?(ms>=IMPACT_MS&&ms<800?255:200):120;
    if(level!=v.relation_level[i]){v.relation_level[i]=level;lv_obj_invalidate(v.relations);}
  }
  v.used = 0;
  bool damage_visible=false,immunity_visible=false,effect_visible=false;
  for (unsigned i = 0; i < 2; i++) {
    unsigned impact = (i ? 0 : CAST_MS) + IMPACT_MS;
    if (v.round && elapsed >= impact) {
      unsigned t = elapsed - impact;
      if (t > 220)
        t = 220;
      v.shown[i] = v.from[i] + (v.target[i] - v.from[i]) * (int)t / 220;
    }
    char s[32];
    snprintf(s, sizeof(s), "%d / %d", v.shown[i],
             i ? g->enemy_max_hp : g->battle_max_hp);
    value(v.hp[i], s);
    if(v.hp_drawn[i]!=v.shown[i]){v.hp_drawn[i]=v.shown[i];
    for(unsigned row=0;row<8;row++) {
      float dy=84+(int)row-179.5f;
      int edge=(int)ceilf(179.5f-sqrtf(180.0f*180.0f-dy*dy));
      int maximum=i?g->enemy_max_hp:g->battle_max_hp;
      int span=maximum>0?(164-edge)*v.shown[i]/maximum:0;
      int cap=(int[]){2,1,0,0,0,0,1,2}[row];
      int width=span-cap;
      if(width<=0)lv_obj_add_flag(v.hp_fill[i][row],LV_OBJ_FLAG_HIDDEN);
      else {
        lv_obj_remove_flag(v.hp_fill[i][row],LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(v.hp_fill[i][row],i?164-edge-width:edge,row);
        lv_obj_set_size(v.hp_fill[i][row],width,1);
      }
    }
    }
    unsigned pose = casting && side == i ? PP_POSE_ATTACK : PP_POSE_IDLE;
    if ((!g->battling || v.shown[i]<=0) && !casting)
      pose = v.shown[i] > 0 ? PP_POSE_HAPPY : PP_POSE_SLEEP;
    unsigned frame = pp_animation_update(&v.clocks[i], v.art[i], pose, now);
#ifndef ESP_PLATFORM
    if(!i && catalog_preview.enabled)frame=catalog_frame(pose,now);
#endif
    if (v.src[i] != v.paths[i][frame]) {
      v.src[i] = v.paths[i][frame];
      pet_frame(i, v.src[i]);
    }
    int shift = casting && side == i && ms < 340
                    ? (int)(sinf(ms * 3.14159f / 340) * 8) * (i ? -1 : 1)
                    : 0;
    if (casting && side != i && ms >= IMPACT_MS && ms < 600)
      shift = ((ms / 50) % 2 ? 3 : -3);
    pp_entry_state entry=pp_battle_entry_state((v.entrance_mask&(1u<<i))?v.entrance_ms:PP_BATTLE_ENTRANCE_MS);
    if(entry.opacity!=v.entry_style[i].opacity)lv_obj_set_style_opa(v.pet[i],entry.opacity,0);
    if(entry.white!=v.entry_style[i].white){lv_obj_set_style_image_recolor(v.pet[i],lv_color_hex(0xffffff),0);lv_obj_set_style_image_recolor_opa(v.pet[i],entry.white,0);}
    v.entry_style[i]=entry;
    lv_obj_set_pos(v.pet[i], (i ? PET_RIGHT_X : PET_LEFT_X) + shift, PET_Y+entry.drop);
  }
  if (v.round && !v.reply_told && elapsed >= CAST_MS && v.target[1]>0) {
    story_move(1);
    v.reply_told = true;
  }
  if (!entrance && !g->battling && !casting && !v.result_told) {
    char line[128];
    unsigned defeated = g->battle_hp > 0 ? 1 : 0;
    snprintf(line, sizeof(line), "%s%s倒下了", defeated ? "对方的" : "", pet_name(defeated));
    if(!(g->wild.phase==PP_WILD_LOST && g->battle_hp>0 && g->enemy_hp>0))story_push(line);
    if(pp_battle_encounter(g)){snprintf(line,sizeof(line),"%s · 全队经验 +%u",g->gym.result==1?"历练获胜":"历练结束",g->gym.reward_xp);story_push(line);}
    else story_push(g->gym.mode?(g->gym.result==1?"挑战成功！查看道馆收获":"挑战结束 · 出场伙伴获得经验"):g->wild.phase>=PP_WILD_BATTLE?(g->wild.phase==PP_WILD_WON?(g->legend.active?"挑战成功！神兽蛋已入背包":"对战获胜！查看野外收获"):(g->legend.active?"挑战结束，调整伙伴再来吧":"野生伙伴逃走了")):(g->battle_hp > 0 ? "挑战成功！金币 +25" : "伙伴需要休息，再来挑战吧"));
    v.result_told = true;
  }
  if(g->gym.mode)team_balls_tick();
  story_tick(now);
  v.settled = !entrance && !g->battling && !casting && !v.story_count && !v.story_visible;
  bool paired=v.settled && !g->wild.phase && !pp_battle_encounter(g);
  lv_obj_set_pos(v.back,paired?116:158,paired?292:298);
  bool show_effects=casting;
#ifndef ESP_PLATFORM
  /* Audit the actor under the FX layer against its approved source pixels. */
  if(catalog_preview.enabled && catalog_preview.actor_check)show_effects=false;
#endif
  if (show_effects) {
    unsigned type = view_move(side)->type;
    if (type < 4 && !(type == PP_GRASS && v.slot == 0) && ms < 720) {
      unsigned phase = ms / 180;
      const void *src = v.effects[type][phase];
      if (v.fx_src != src) {
        pp_image_set_src(v.fx_image, src);
        v.fx_src = src;
      }
      float progress = ms < 360 ? ms / 360.0f : 1;
      if (type == PP_FIRE && v.slot == 2)
        progress = 1;
      int source = side ? PET_CENTER_RIGHT : PET_CENTER_LEFT, dest = side ? PET_CENTER_LEFT : PET_CENTER_RIGHT;
      lv_obj_set_pos(v.fx_image,
                     source + (int)((dest - source) * progress) - 24, 100);
      effect_visible=true;
    }
    effects(v.species[side], side, ms);
    int loss = v.loss[side ? 0 : 1];
    immunity_visible=ms>=IMPACT_MS&&ms<800&&v.immune[side];
    if (ms >= IMPACT_MS && ms < 800 && loss > 0) {
      char s[24];
      snprintf(s, sizeof(s), "-%d", loss);
      value(v.damage, s);
      lv_obj_set_pos(v.damage, side ? PET_CENTER_LEFT-32 : PET_CENTER_RIGHT-32,
                     90 - (int)(ms - IMPACT_MS) / 25);
      damage_visible=true;
    }
  }
  if(entrance)for(unsigned i=0;i<2;i++)if(v.entrance_mask&(1u<<i))pp_battle_entry_paint(i,v.entrance_ms,entry_paint,NULL);
  fx_commit();
  if(effect_visible)lv_obj_remove_flag(v.fx_image,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(v.fx_image,LV_OBJ_FLAG_HIDDEN);
  if(damage_visible)lv_obj_remove_flag(v.damage,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(v.damage,LV_OBJ_FLAG_HIDDEN);
  if (v.settled && !g->gym.mode && !g->wild.phase) lv_obj_remove_flag(v.retry, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(v.retry, LV_OBJ_FLAG_HIDDEN);
  v.probe = (pp_battle_view_state){.active = true,
                                   .casting = casting,
                                   .settled = v.settled,
                                   .species = v.species[side],
                                    .slot = v.slot,
                                   .side = side,
                                   .particles = v.used,
                                   .hp = {v.shown[0], v.shown[1]},
                                   .actor={v.actor[0],v.actor[1]},.round=v.observed_round,
                                   .loss={v.loss[0],v.loss[1]},.damage_visible=damage_visible,.immunity_visible=immunity_visible,.damage_side=side?0:1,
                                   .art={v.art[0],v.art[1]},.entrance_mask=v.entrance_mask,.entrance_ms=v.entrance_ms,
                                   .pet_opacity={lv_obj_get_style_opa(v.pet[0],0),lv_obj_get_style_opa(v.pet[1],0)}};
  for(unsigned i=0;i<2;i++){v.probe.relation_factor[i]=v.relation_factor[i];v.probe.relation_kind[i]=v.relation_kind[i];v.probe.relation_level[i]=v.relation_level[i];}
  if(damage_visible)snprintf(v.probe.impact_text,sizeof(v.probe.impact_text),"%s",lv_label_get_text(v.damage));
  if(g->gym.mode)for(unsigned side=0;side<2;side++)for(unsigned i=0;i<3;i++){
    int hp=(side?g->gym.enemy_hp[i]:g->gym.hp[i]);if(i==v.actor[side])hp=v.shown[side];
    if(hp>0)v.probe.alive_mask[side]|=1u<<i;
  }
}
