#include "image_assets.h"
#include "scene_view.h"
#include "esp_partition.h"
#include "esp_heap_caps.h"
#include "asset_row_rle.h"
#include "asset_deflate.h"
#include "fishing_scenery.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t *bank;
static size_t bank_size;
static unsigned allocations;
static esp_partition_t partition;
const esp_partition_t *esp_partition_find_first(int t,int st,const char *name) {
 assert(t==1 && st==0x40 && !strcmp(name,"assets"));return &partition;
}
int esp_partition_read(const esp_partition_t *p,size_t off,void *dst,size_t n) {
 assert(p==&partition && off+n<=bank_size);memcpy(dst,bank+off,n);return ESP_OK;
}
int esp_partition_mmap(const esp_partition_t *p,size_t off,size_t n,int type,const void **out,esp_partition_mmap_handle_t *handle) {
 assert(p==&partition && !off && n==bank_size && type==1);*out=bank;*handle=1;return ESP_OK;
}
void *heap_caps_calloc(size_t n,size_t sz,unsigned caps) {
 assert(caps==(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));allocations++;return calloc(n,sz);
}
void *heap_caps_malloc(size_t n,unsigned caps) {
 assert(caps==(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));assert(n==316800u || n==388800u);allocations++;return malloc(n);
}
void heap_caps_free(void *p){free(p);}
static uint32_t word(const uint8_t *p){uint32_t n;memcpy(&n,p,4);return n;}
int main(int argc,char **argv) {
 assert(argc==2 || argc==3);FILE *in=fopen(argv[1],"rb");assert(in);fseek(in,0,SEEK_END);bank_size=ftell(in);rewind(in);
 bank=malloc(bank_size);assert(bank && fread(bank,1,bank_size,in)==bank_size);fclose(in);partition.size=bank_size;
 if(argc==3) {
  memset(bank+16,0,16);memcpy(bank+16,"mismatch",8);
  assert(!pp_image_bank_init());assert(!pp_image_bank_version());
  puts("PASS: mismatched resource version rejected before mapping or allocation");return 0;
 }
 assert(!memcmp(bank,"PPASSET3",8));
 assert(!strcmp((const char *)bank+16,PP_APP_VERSION));
 assert(pp_image_bank_init());
 assert(!strcmp(pp_image_bank_version(),PP_APP_VERSION));
 unsigned count=word(bank+8),compressed=0,names=32,entry_size=names+8;
 for(unsigned i=0;i<count;i++) {const uint8_t *e=bank+32+i*entry_size;if(pp_row_rle_magic(bank+word(e+names),word(e+names+4)) || pp_deflate_magic(bank+word(e+names),word(e+names+4)))compressed++;}
 unsigned expected_allocations=compressed?2:1;
 assert(pp_image_probe().images==count && allocations==expected_allocations);
 for(unsigned repeat=0;repeat<3;repeat++) for(unsigned i=0;i<count;i++) {
  const uint8_t *entry=bank+32+i*entry_size;char path[100];snprintf(path,sizeof(path),"S:/assets/%s",entry);
  const lv_image_dsc_t *im=pp_image_descriptor(path);assert(im);
  const uint8_t *stored=bank+word(entry+names);size_t size=word(entry+names+4);
  if(pp_deflate_magic(stored,size) || pp_row_rle_magic(stored,size)) {
   assert(pp_static_background_path((const char *)entry));bool deflated=pp_deflate_magic(stored,size);
   size_t n=deflated?pp_deflate_size(stored,size):pp_row_rle_size(stored,size);uint8_t *raw=malloc(n);assert(raw);
   pp_inflate_work *work=calloc(1,sizeof(*work));assert(work);
   assert(deflated?pp_deflate_decode(stored,size,raw,n,work):pp_row_rle_decode(stored,size,raw,n));free(work);assert(!memcmp(&im->header,raw,sizeof(lv_image_header_t)));
   assert(im->data_size==n-sizeof(lv_image_header_t) && !memcmp(im->data,raw+sizeof(lv_image_header_t),im->data_size));free(raw);
  } else {
   assert(im->data==stored+sizeof(lv_image_header_t));
   assert(im->data_size==size-sizeof(lv_image_header_t));
  }
  assert(pp_image_descriptor(path)==im);
 }
 assert(allocations==expected_allocations && !pp_image_probe().file_reads);
 assert(!pp_image_descriptor("S:/assets/no-such-image.bin"));
 lv_init();lv_display_t *display=lv_display_create(360,360);
 lv_obj_t *image=lv_image_create(lv_screen_active());
 pp_scene_view_init();assert(allocations==expected_allocations+1);
 const lv_image_dsc_t *stable=NULL;
 for(unsigned scene=0;scene<13;scene++) {
  char path[100];snprintf(path,sizeof(path),"S:/assets/scenes/%u.bin",scene);
  pp_scene_view_set(image,path,scene,0);
  const lv_image_dsc_t *canvas=lv_image_get_src(image);
  assert(canvas && canvas->header.cf==LV_COLOR_FORMAT_RGB565 && canvas->header.w==360);
  if(stable)assert(canvas==stable);
  stable=canvas;
  for(unsigned tick=0;tick<=45000;tick+=33)pp_scene_view_tick(image,scene,tick);
  assert(lv_image_get_src(image)==stable && allocations==expected_allocations+1);
  assert(!pp_image_probe().file_reads);
 }
 pp_scene_view_set(image,"S:/assets/fishing/pond.bin",PP_FISHING_SCENERY,0);
 assert(lv_image_get_src(image)==stable);
 for(unsigned tick=0;tick<=45000;tick+=120){
  pp_scene_view_tick(image,PP_FISHING_SCENERY,tick);
  assert(lv_image_get_src(image)==stable&&allocations==expected_allocations+1&&!pp_image_probe().file_reads);
 }
 puts("PASS: ESP fishing pond reuses existing scene canvas,45s zero frame allocations or image reads");
 lv_obj_t *front=lv_image_create(lv_screen_active());
 const lv_image_dsc_t *mask=pp_image_descriptor("S:/assets/scene-fronts/12.bin");assert(mask);
 for(unsigned visit=0;visit<3;visit++) {
  pp_scene_view_enabled(visit!=1);
  pp_scene_view_set(image,"S:/assets/scenes/12.bin",12,0);
  assert(pp_scene_view_front_set(front,"S:/assets/scene-fronts/12.bin"));
  const lv_image_dsc_t *overlay=lv_image_get_src(front);
  assert(overlay->header.cf==LV_COLOR_FORMAT_RGB565A8 && overlay->data_size==388800);
  for(unsigned tick=0;tick<=45000;tick+=120) {
   pp_scene_view_tick(image,12,tick);
   assert(!memcmp(overlay->data,stable->data,259200));
   assert(allocations==expected_allocations+2 && !pp_image_probe().file_reads);
  }
  const uint8_t *alpha=overlay->data+259200;
  unsigned opaque=0,clear=0;
  for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++) {
   unsigned expected=pp_image_indexed_pixel(mask,x/3,y/3)>>24;
   assert(alpha[y*360+x]==expected);if(expected)opaque++;else clear++;
  }
  assert(opaque>10000 && clear>50000);
 }
 lv_display_delete(display);
 puts("PASS: real ESP scenery branch:13 scenes, stable360px canvas, 316800-byte scene + reusable388800-byte foreground,45s/scene zero frame allocations or reads");
 printf("PASS: ESP image-bank branch: %u real assets, %u exact background decodes; one metadata and at most one background PSRAM pool; all pet frames remain Flash zero-copy; stable descriptors, no per-frame allocations/file reads\n",count,compressed);
 return 0;
}
