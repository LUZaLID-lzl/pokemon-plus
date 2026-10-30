#include "scene_effect_view.h"
#include "src/misc/lv_area_private.h"
#include <limits.h>
#include <string.h>

/* Leave LVGL's invalidation queue room for pets, scenery and HUD. Invalidating
 * each moving block separately overflows it and forces a full-screen redraw. */
#define DAMAGE_LIMIT 12
static lv_obj_t *overlay;
static pp_scene_particle shown[PP_SCENE_FX_MAX];
static unsigned shown_count;
static const lv_image_dsc_t *(*tumbleweed)[8];
typedef struct { lv_area_t area[DAMAGE_LIMIT];unsigned count; } damage_list;
static lv_area_t bounds(const pp_scene_particle *p) {
  return (lv_area_t){p->x,p->y,p->x+p->w-1,p->y+p->h-1};
}
static bool same(const pp_scene_particle *a,const pp_scene_particle *b) {
  return a->x==b->x && a->y==b->y && a->w==b->w && a->h==b->h &&
    a->color==b->color && a->opacity==b->opacity && a->shape==b->shape;
}
static void damage_add(damage_list *list,lv_area_t area) {
  lv_area_t screen={0,0,359,359};
  if(!lv_area_intersect(&area,&area,&screen))return;
  unsigned best=0;int32_t cost=INT32_MAX;
  for(unsigned i=0;i<list->count;i++) {
    lv_area_t joined;lv_area_join(&joined,&list->area[i],&area);
    int32_t extra=lv_area_get_size(&joined)-lv_area_get_size(&list->area[i])-lv_area_get_size(&area);
    if(extra<cost){best=i;cost=extra;}
  }
  if(list->count && (cost<=0 || list->count==DAMAGE_LIMIT)) {
    lv_area_join(&list->area[best],&list->area[best],&area);
  } else list->area[list->count++]=area;
}
static void draw(lv_event_t *event) {
  lv_layer_t *layer=lv_event_get_layer(event);
  lv_draw_rect_dsc_t rect;lv_draw_rect_dsc_init(&rect);
  lv_draw_image_dsc_t image;lv_draw_image_dsc_init(&image);
  for(unsigned i=0;i<shown_count;i++) {
    const pp_scene_particle *p=&shown[i];
    lv_area_t area=bounds(p),clip;
    if(!lv_area_intersect(&clip,&area,&layer->_clip_area))continue;
    if(p->shape>=PP_FX_TUMBLE_0 && p->shape<=PP_FX_TUMBLE_7) {
      unsigned size=(p->w-32)/4,frame=p->shape-PP_FX_TUMBLE_0;
      image.src=tumbleweed && size<3?tumbleweed[size][frame]:NULL;
      image.opa=p->opacity;
      if(image.src)lv_draw_image(layer,&image,&area);
      continue;
    }
    pp_scene_rect parts[PP_SCENE_FX_PARTS];unsigned count=pp_scene_fx_parts(p,parts);
    rect.bg_color=lv_color_hex(p->color);
    for(unsigned j=0;j<count;j++) {
      const pp_scene_rect *r=&parts[j];
      area=(lv_area_t){r->x,r->y,r->x+r->w-1,r->y+r->h-1};
      if(!lv_area_intersect(&clip,&area,&layer->_clip_area))continue;
      rect.bg_opa=r->opacity;lv_draw_rect(layer,&rect,&area);
    }
  }
}
void pp_scene_effect_view_open(lv_obj_t *parent,const lv_image_dsc_t *sprites[3][8]) {
  shown_count=0;tumbleweed=sprites;
  overlay=lv_obj_create(parent);lv_obj_remove_style_all(overlay);
  lv_obj_set_pos(overlay,0,0);lv_obj_set_size(overlay,360,360);
  lv_obj_remove_flag(overlay,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(overlay,draw,LV_EVENT_DRAW_MAIN,NULL);
}
void pp_scene_effect_view_update(const pp_scene_particle *particles,unsigned count) {
  if(!overlay)return;
  if(count>PP_SCENE_FX_MAX)count=PP_SCENE_FX_MAX;
  damage_list dirty={0};
  unsigned limit=count>shown_count?count:shown_count;
  for(unsigned i=0;i<limit;i++) {
    if(i<count && i<shown_count && same(&shown[i],&particles[i]))continue;
    if(i<shown_count)damage_add(&dirty,bounds(&shown[i]));
    if(i<count)damage_add(&dirty,bounds(&particles[i]));
  }
  memcpy(shown,particles,count*sizeof(*shown));shown_count=count;
  for(unsigned i=0;i<dirty.count;i++)lv_obj_invalidate_area(overlay,&dirty.area[i]);
}
