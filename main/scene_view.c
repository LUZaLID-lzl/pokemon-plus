#include "scene_view.h"
#include "image_assets.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include "scene_motion.h"
#include "pet_animation.h"
#include "fishing_scenery.h"
#include <stdlib.h>
#include <string.h>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#endif
static uint16_t *canvas,*base,*native;
#define SCENE_BUFFER_BYTES (360u*360u*2u+PP_SCENE_PIXELS*4u)
static lv_image_dsc_t descriptor,front_descriptor;
static uint16_t *front_canvas;
static lv_obj_t *front_image;
#define FRONT_BUFFER_BYTES (360u*360u*3u)
static unsigned last_tick=UINT32_MAX;
static bool enabled=true,ready;
void pp_scene_view_enabled(bool value) { enabled=value; }
void pp_scene_view_init(void) {
  if(canvas)return;
#ifdef ESP_PLATFORM
  canvas=heap_caps_malloc(SCENE_BUFFER_BYTES,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
#else
  canvas=malloc(SCENE_BUFFER_BYTES);
#endif
  if(!canvas)return;
  memset(canvas,0,SCENE_BUFFER_BYTES);
  base=canvas+360u*360u;native=base+PP_SCENE_PIXELS;
  descriptor=(lv_image_dsc_t){.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565,
    .w=360,.h=360,.stride=720},.data_size=360u*360u*2u,.data=(const uint8_t *)canvas};
}
static bool load_native_scene(const char *path) {
  const lv_image_dsc_t *source=pp_image_descriptor(path);
  if(!canvas || !source || (source->header.cf!=LV_COLOR_FORMAT_RGB565 && source->header.cf!=LV_COLOR_FORMAT_I8 && source->header.cf!=LV_COLOR_FORMAT_I4) || source->header.w!=360 || source->header.h!=360) {
    return false;
  }
  for(unsigned y=0;y<120;y++)for(unsigned x=0;x<120;x++) {
    if(source->header.cf==LV_COLOR_FORMAT_RGB565) {
      unsigned at=y*3u*source->header.stride+x*6u;
      base[y*120+x]=(uint16_t)(source->data[at]|(source->data[at+1]<<8));
    } else {
      uint32_t c=pp_image_indexed_pixel(source,x*3,y*3);
      base[y*120+x]=(uint16_t)(((c>>19)&31)<<11 | ((c>>10)&63)<<5 | ((c>>3)&31));
    }
  }
  return true;
}
static void bind_canvas(lv_obj_t *image) {
  lv_image_set_src(image,&descriptor);
  lv_image_set_inner_align(image,LV_IMAGE_ALIGN_TOP_LEFT);
  lv_image_set_pivot(image,0,0);lv_image_set_antialias(image,false);
  lv_image_set_scale(image,256);
  lv_obj_set_size(image,360,360);
}
void pp_scene_view_set(lv_obj_t *image,const char *path,unsigned scene,uint32_t elapsed) {
  ready=false;front_image=NULL;
  if(!load_native_scene(path)) {
    lv_image_set_scale(image,256);pp_image_set_src(image,path);return;
  }
  ready=true;last_tick=UINT32_MAX;
  pp_scene_view_tick(image,scene,elapsed);
  bind_canvas(image);
}
/* Menu entry only: reuse the courtyard canvas and two existing 120px scratch
 * planes. Two separable box passes soften the scene without any extra assets,
 * heap allocation, animation-time work or access to game state. */
static void blur_canvas(lv_obj_t *image,unsigned brightness) {
  for(unsigned pass=0;pass<4;pass++) {
    const uint16_t *input=pass%2?native:base;
    uint16_t *output=pass%2?base:native;
    for(int y=0;y<120;y++)for(int x=0;x<120;x++) {
      unsigned red=0,green=0,blue=0;
      for(int delta=-3;delta<=3;delta++) {
        int sx=x+(pass%2?0:delta),sy=y+(pass%2?delta:0);
        if(sx<0)sx=0;
        if(sx>119)sx=119;
        if(sy<0)sy=0;
        if(sy>119)sy=119;
        uint16_t c=input[sy*120+sx];red+=c>>11;green+=(c>>5)&63;blue+=c&31;
      }
      output[y*120+x]=(uint16_t)((red/7)<<11 | (green/7)<<5 | blue/7);
    }
  }
  for(unsigned y=0;y<120;y++)for(unsigned x=0;x<120;x++) {
    uint16_t c=base[y*120+x];
    /* Leave enough scene color to read the setting behind light menu text. */
    unsigned red=(c>>11)*brightness/100+1,green=((c>>5)&63)*brightness/100+3,blue=(c&31)*brightness/100+2;
    c=(uint16_t)(red<<11 | green<<5 | blue);
    for(unsigned dy=0;dy<3;dy++)for(unsigned dx=0;dx<3;dx++)canvas[(y*3+dy)*360+x*3+dx]=c;
  }
  bind_canvas(image);
}
void pp_scene_view_blurred_set(lv_obj_t *image,const char *path) {
  ready=false;front_image=NULL;last_tick=UINT32_MAX;
  if(!load_native_scene(path)) {pp_image_set_src(image,path);return;}
  blur_canvas(image,36);
}

void pp_scene_view_tick(lv_obj_t *image,unsigned scene,uint32_t elapsed) {
  unsigned period=scene==PP_FISHING_SCENERY?33u:PP_SCENE_MOTION_TICK;
  unsigned tick=elapsed/period;
  if(!ready || tick==last_tick)return;
  last_tick=tick;
  if(enabled)pp_scene_motion_render(scene,tick*period,base,native);
  else memcpy(native,base,PP_SCENE_PIXELS*2u);
  lv_area_t dirty={.x1=360,.y1=360,.x2=-1,.y2=-1};
  lv_area_t bands[6];for(unsigned i=0;i<6;i++)bands[i]=dirty;
  for(int y=0;y<120;y++)for(int x=0;x<120;x++) {
    uint16_t c=native[y*120+x];int px=x*3,py=y*3;
    if(canvas[py*360+px]==c)continue;
    for(int dy=0;dy<3;dy++)for(int dx=0;dx<3;dx++) {
      unsigned pixel=(py+dy)*360+px+dx;canvas[pixel]=c;
      if(front_image)front_canvas[pixel]=c;
    }
    if(px<dirty.x1)dirty.x1=px;
    if(py<dirty.y1)dirty.y1=py;
    if(px+2>dirty.x2)dirty.x2=px+2;
    if(py+2>dirty.y2)dirty.y2=py+2;
    if(scene==PP_FISHING_SCENERY){lv_area_t *b=&bands[y/20];if(px<b->x1)b->x1=px;if(py<b->y1)b->y1=py;if(px+2>b->x2)b->x2=px+2;if(py+2>b->y2)b->y2=py+2;}
  }
  /* Exact3x blocks avoid transform half-pixel shifts. No redraw at rest. */
  if(scene==PP_FISHING_SCENERY){for(unsigned i=0;i<6;i++)if(bands[i].x2>=0)lv_obj_invalidate_area(image,&bands[i]);}
  else if(dirty.x2>=0) {lv_obj_invalidate_area(image,&dirty);if(front_image)lv_obj_invalidate_area(front_image,&dirty);}
}

/* One reusable foreground; initialized on entry, never allocated/read per frame.
 * Colors follow the same animated native pixels, alpha follows the silhouette. */
bool pp_scene_view_front_set(lv_obj_t *image,const char *path) {
  front_image=NULL;
  const lv_image_dsc_t *mask=pp_image_descriptor(path);
  if(!ready || !mask || mask->header.w!=120 || mask->header.h!=120 ||
     (mask->header.cf!=LV_COLOR_FORMAT_I8 && mask->header.cf!=LV_COLOR_FORMAT_I4))return false;
  if(!front_canvas) {
#ifdef ESP_PLATFORM
    front_canvas=heap_caps_malloc(FRONT_BUFFER_BYTES,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
#else
    front_canvas=malloc(FRONT_BUFFER_BYTES);
#endif
    if(!front_canvas)return false;
    front_descriptor=(lv_image_dsc_t){.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565A8,
      .w=360,.h=360,.stride=720},.data_size=FRONT_BUFFER_BYTES,.data=(const uint8_t *)front_canvas};
  }
  memcpy(front_canvas,canvas,360u*360u*2u);
  uint8_t *alpha=(uint8_t *)(front_canvas+360u*360u);
  for(unsigned y=0;y<120;y++)for(unsigned x=0;x<120;x++) {
    uint8_t a=pp_image_indexed_pixel(mask,x,y)>>24;
    for(unsigned dy=0;dy<3;dy++)for(unsigned dx=0;dx<3;dx++)alpha[(y*3+dy)*360+x*3+dx]=a;
  }
  front_image=image;lv_image_set_src(image,&front_descriptor);
  lv_image_set_inner_align(image,LV_IMAGE_ALIGN_TOP_LEFT);
  lv_image_set_pivot(image,0,0);lv_image_set_antialias(image,false);lv_image_set_scale(image,256);
  lv_obj_set_pos(image,0,0);lv_obj_set_size(image,360,360);
  return true;
}

void pp_scene_view_pixel_set(lv_obj_t *image,const char *path,unsigned scale,int offset_x,int offset_y){
  pp_scene_view_init();const lv_image_dsc_t *source=pp_image_descriptor(path);
  ready=false;front_image=NULL;last_tick=UINT32_MAX;
  if(!canvas||!source||!scale||(source->header.cf!=LV_COLOR_FORMAT_I4&&source->header.cf!=LV_COLOR_FORMAT_I8)){pp_image_set_src(image,path);return;}
  for(int y=0;y<360;y++)for(int x=0;x<360;x++){
    int sx=(x-offset_x)/(int)scale,sy=(y-offset_y)/(int)scale;
    if(sx<0)sx=0;
    if(sy<0)sy=0;
    if(sx>=source->header.w)sx=source->header.w-1;
    if(sy>=source->header.h)sy=source->header.h-1;
    uint32_t c=pp_image_indexed_pixel(source,sx,sy);
    canvas[y*360+x]=(uint16_t)(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));
  }
  lv_image_cache_drop(&descriptor);bind_canvas(image);lv_obj_invalidate(image);
}

void pp_scene_view_pixel_blurred_set(lv_obj_t *image,const char *path,unsigned scale,int offset_x,int offset_y){
  pp_scene_view_pixel_set(image,path,scale,offset_x,offset_y);
  if(!canvas||lv_image_get_src(image)!=&descriptor)return;
  for(unsigned y=0;y<120;y++)for(unsigned x=0;x<120;x++)base[y*120+x]=canvas[(y*3)*360+x*3];
  blur_canvas(image,68);
  lv_image_cache_drop(&descriptor);lv_obj_invalidate(image);
}

static unsigned panorama_offset;
static lv_image_dsc_t strip_descriptor;
static unsigned strip_offset;
void pp_scene_view_strip_tick(lv_obj_t *image,unsigned elapsed){
 if(!canvas||lv_image_get_src(image)!=&strip_descriptor)return;
 unsigned offset=elapsed/25%720;
 if(offset==strip_offset)return;
 strip_offset=offset;
 const uint16_t *source=canvas+360u*112u;
 for(unsigned y=0;y<112;y++){
  const uint16_t *row=source+y*720;
  unsigned first=720-offset;if(first>360)first=360;
  memcpy(canvas+y*360,row+offset,first*2);
  if(first<360)memcpy(canvas+y*360+first,row,(360-first)*2);
 }
 lv_obj_invalidate(image);
}
bool pp_scene_view_strip_set(lv_obj_t *image,const void *source,unsigned elapsed){
 pp_scene_view_init();const lv_image_dsc_t *src=pp_image_descriptor(source);
 if(!canvas||!src||src->header.w!=360||src->header.h!=112||
    (src->header.cf!=LV_COLOR_FORMAT_I4&&src->header.cf!=LV_COLOR_FORMAT_I8))return false;
 ready=false;front_image=NULL;
 /* Output + two mirrored source strips fit inside the existing 360x360 canvas. */
 uint16_t *tile=canvas+360u*112u;
 for(unsigned y=0;y<112;y++)for(unsigned x=0;x<360;x++){
  uint32_t c=pp_image_indexed_pixel(src,x,y);
  uint16_t pixel=(uint16_t)(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));
  tile[y*720+x]=tile[y*720+719-x]=pixel;
 }
 strip_descriptor=(lv_image_dsc_t){.header={.magic=LV_IMAGE_HEADER_MAGIC,
  .cf=LV_COLOR_FORMAT_RGB565,.w=360,.h=112,.stride=720},
  .data_size=360u*112u*2u,.data=(const uint8_t *)canvas};
 lv_image_cache_drop(&strip_descriptor);lv_image_set_src(image,&strip_descriptor);
 lv_image_set_scale(image,256);lv_image_set_inner_align(image,LV_IMAGE_ALIGN_TOP_LEFT);
 lv_obj_set_size(image,360,112);strip_offset=UINT32_MAX;
 pp_scene_view_strip_tick(image,elapsed);return true;
}
void pp_scene_view_panorama_tick(lv_obj_t *image,unsigned elapsed){
 unsigned offset=(elapsed/25)%360; /* 40px/s, sampled at single display-pixel precision. */
 if(offset==panorama_offset)return;
 panorama_offset=offset;
 for(unsigned y=0;y<60;y++){
  uint16_t *row=canvas+(108+y*2)*360;
  unsigned first=offset/2,odd=offset&1;
  for(unsigned x=0;x<180;x++){
   uint16_t c=base[y*180+(x+first)%180];
   unsigned at=x*2;row[at]=c;
   row[at+1]=odd?base[y*180+(x+first+1)%180]:c;
  }
  memcpy(row+360,row,720);
 }
 lv_area_t band={0,108,359,227};lv_obj_invalidate_area(image,&band);
}
void pp_scene_view_panorama_set(lv_obj_t *image,const char *path,unsigned elapsed){
 pp_scene_view_init();ready=false;front_image=NULL;
 const lv_image_dsc_t *src=pp_image_descriptor(path);
 if(!canvas||!src||src->header.w!=180||src->header.h!=180){pp_image_set_src(image,path);return;}
 for(unsigned y=0;y<60;y++)for(unsigned x=0;x<180;x++){
  uint32_t c=pp_image_indexed_pixel(src,x,y+54);
  base[y*180+x]=(uint16_t)(((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31));
 }
 for(unsigned n=0;n<360*360;n++)canvas[n]=0x1166;
 bind_canvas(image);panorama_offset=UINT32_MAX;
 pp_scene_view_panorama_tick(image,elapsed);lv_obj_invalidate(image);
}
