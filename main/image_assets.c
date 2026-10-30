#include "image_assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asset_row_rle.h"
#ifdef ESP_PLATFORM
#include "asset_deflate.h"
#endif
#ifdef ESP_PLATFORM
#include "esp_partition.h"
#include "esp_heap_caps.h"
#endif
#define IMAGE_LIMIT 16500
#define BYTE_LIMIT (30u * 1024u * 1024u)
#define PP_ASSET_HEADER_SIZE 32u
#define PP_ASSET_VERSION_SIZE 16u
typedef struct {
  const char *path;
  lv_image_dsc_t descriptor;
} pp_resident_image;
#ifdef ESP_PLATFORM
/* Metadata belongs in PSRAM; keep internal RAM available for LCD DMA. */
static pp_resident_image *images;
static uint8_t *decoded_backgrounds;
#else
static pp_resident_image images[IMAGE_LIMIT];
#endif
static pp_image_stats stats;
static size_t image_bytes(const lv_image_header_t *h) {
  if (!h->w || !h->h || (h->flags & ~PP_IMAGE_CENTERED_PET) || h->magic != LV_IMAGE_HEADER_MAGIC) return 0;
  if (h->flags && (h->w>48 || h->h>48 || (h->w%2) || (h->h%2) ||
      (h->cf!=LV_COLOR_FORMAT_I4 && h->cf!=LV_COLOR_FORMAT_I8))) return 0;
  if (h->cf == LV_COLOR_FORMAT_I4 && h->stride == (h->w+1u)/2u)
    return 64 + (size_t)h->stride * h->h;
  if (h->cf == LV_COLOR_FORMAT_I8 && h->stride == h->w)
    return 1024 + (size_t)h->stride * h->h;
  if ((h->cf == LV_COLOR_FORMAT_RGB565 || h->cf == LV_COLOR_FORMAT_RGB565A8) && h->stride == h->w*2u)
    return (size_t)h->stride*h->h + (h->cf==LV_COLOR_FORMAT_RGB565A8 ? (size_t)h->w*h->h : 0);
  return 0;
}
#ifdef ESP_PLATFORM
static const uint8_t *bank;
static uint32_t bank_count, bank_size;
static esp_partition_mmap_handle_t bank_handle;
static char bank_version[PP_ASSET_VERSION_SIZE+1];
static uint32_t word(const uint8_t *p) { uint32_t n; memcpy(&n,p,4); return n; }
static int compare_image(const void *a, const void *b) {
  return strcmp(((const pp_resident_image *)a)->path, ((const pp_resident_image *)b)->path);
}
bool pp_image_bank_init(void) {
  const esp_partition_t *p=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,0x40,"assets");
  uint8_t header[PP_ASSET_HEADER_SIZE]={0};
  bank_version[0]=0;
  if (!p || esp_partition_read(p,0,header,sizeof(header))!=ESP_OK) return false;
  /* Resource format 3 binds the bank to the compiled application version.
   * Do not accept earlier unversioned formats: assets and executable must be
   * deployed as a matched pair to keep the image ABI deterministic. */
  if (memcmp(header,"PPASSET3",8) || !memchr(header+16,0,PP_ASSET_VERSION_SIZE)) return false;
  memcpy(bank_version,header+16,PP_ASSET_VERSION_SIZE);
  bank_version[PP_ASSET_VERSION_SIZE]=0;
  if (strcmp(bank_version,PP_APP_VERSION)) { bank_version[0]=0; return false; }
  unsigned names=32;
  unsigned entry_size=names+8;
  bank_count=word(header+8); bank_size=word(header+12);
  if (!bank_count || bank_count>IMAGE_LIMIT || bank_size>p->size ||
      PP_ASSET_HEADER_SIZE+bank_count*entry_size>bank_size) return false;
  const void *mapped;
  if (esp_partition_mmap(p,0,bank_size,ESP_PARTITION_MMAP_DATA,&mapped,&bank_handle)!=ESP_OK) return false;
  bank=mapped;
  size_t decoded_bytes=0;bool has_deflate=false;
  /* Static backgrounds decode once into a bounded PSRAM pool. Pet frames
   * retain native Flash pointers and both stable facings. ROM inflater state
   * lives at the end of this same pool, never on the 8KiB main-task stack. */
  for(unsigned i=0;i<bank_count;i++) {
    const uint8_t *e=bank+PP_ASSET_HEADER_SIZE+i*entry_size;
    uint32_t start=word(e+names),size=word(e+names+4);
    if(!memchr(e,0,names) || start<PP_ASSET_HEADER_SIZE+bank_count*entry_size || start>bank_size || size>bank_size-start) goto invalid;
    bool deflated=pp_deflate_magic(bank+start,size);
    if(deflated || pp_row_rle_magic(bank+start,size)) {
      size_t n=deflated?pp_deflate_size(bank+start,size):pp_row_rle_size(bank+start,size);
      if(!pp_static_background_path((const char *)e) || !n || n>PP_BACKGROUND_POOL_LIMIT-decoded_bytes) goto invalid;
      decoded_bytes+=(n+3u)&~3u;has_deflate|=deflated;
      if(decoded_bytes>PP_BACKGROUND_POOL_LIMIT) goto invalid;
    }
  }
  images=heap_caps_calloc(bank_count,sizeof(*images),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  if (!images) { bank=NULL;return false; }
  size_t work_at=(decoded_bytes+7u)&~7u;
  if(has_deflate && (work_at>PP_BACKGROUND_POOL_LIMIT || sizeof(pp_inflate_work)>PP_BACKGROUND_POOL_LIMIT-work_at)) goto invalid;
  size_t pool_size=has_deflate?work_at+sizeof(pp_inflate_work):decoded_bytes;
  if(pool_size) {
    decoded_backgrounds=heap_caps_calloc(1,pool_size,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if(!decoded_backgrounds) goto invalid;
  }
  size_t total=0;
  size_t decoded_at=0;
  for (unsigned i=0;i<bank_count;i++) {
    const uint8_t *e=bank+PP_ASSET_HEADER_SIZE+i*entry_size;
    uint32_t start=word(e+names),size=word(e+names+4);
    if (!memchr(e,0,names) || start<PP_ASSET_HEADER_SIZE+bank_count*entry_size || size<sizeof(lv_image_header_t) || start>bank_size || size>bank_size-start) goto invalid;
    const uint8_t *native=bank+start;size_t native_size=size;
    if(pp_deflate_magic(native,size)) {
      native_size=pp_deflate_size(native,size);
      pp_inflate_work *work=(pp_inflate_work *)(decoded_backgrounds+work_at);
      if(!pp_deflate_decode(native,size,decoded_backgrounds+decoded_at,decoded_bytes-decoded_at,work)) goto invalid;
      native=decoded_backgrounds+decoded_at;decoded_at+=(native_size+3u)&~3u;
    } else if(pp_row_rle_magic(native,size)) {
      native_size=pp_row_rle_size(native,size);
      if(!pp_row_rle_decode(native,size,decoded_backgrounds+decoded_at,decoded_bytes-decoded_at)) goto invalid;
      native=decoded_backgrounds+decoded_at;decoded_at+=(native_size+3u)&~3u;
    }
    lv_image_header_t h;memcpy(&h,native,sizeof(h));
    size_t payload=native_size-sizeof(h);
    if (!image_bytes(&h) || payload!=image_bytes(&h)) goto invalid;
    if (h.flags && word(native+sizeof(h))) goto invalid;
    images[i]=(pp_resident_image){.path=(const char *)e,
      .descriptor={.header=h,.data_size=payload,.data=native+sizeof(h)}};
    total+=payload;
  }
  /* Path sorting in the packer is component-based; sort by full name here. */
  qsort(images,bank_count,sizeof(*images),compare_image);
  for (unsigned i=1;i<bank_count;i++) if (!strcmp(images[i-1].path,images[i].path)) goto invalid;
  stats.images=bank_count;stats.bytes=total;
  return true;
invalid:
  bank_version[0]=0;
  heap_caps_free(decoded_backgrounds);decoded_backgrounds=NULL;
  heap_caps_free(images);images=NULL;bank=NULL;return false;
}
static const void *resolve(const char *path) {
  if (!bank || strncmp(path,"S:/assets/",10)) return path;
  unsigned lo=0,hi=bank_count;
  while (lo<hi) {
    unsigned mid=lo+(hi-lo)/2;
    int order=strcmp(path+10,images[mid].path);
    if (!order) {stats.memory_hits++;return &images[mid].descriptor;}
    if (order<0) hi=mid;else lo=mid+1;
  }
  return path;
}
#else
bool pp_image_bank_init(void) { return true; }
static const void *resolve(const char *path) {
  for (unsigned i = 0; i < stats.images; i++) {
    if (!strcmp(images[i].path, path)) {
      stats.memory_hits++;
      return &images[i].descriptor;
    }
  }
  /* Keep native scenes and sprites resident so local scenery sampling and
   * pet animation never reopen files after a scene has loaded. */
  if (stats.images == IMAGE_LIMIT)
    return path;
  if (strncmp(path, "S:", 2))
    return path;
  FILE *file = fopen(path + 2, "rb");
  stats.file_reads++;
  if (!file)
    return path;
  lv_image_header_t header;
  if (fread(&header, 1, sizeof(header), file) != sizeof(header) || !image_bytes(&header)) {
    fclose(file); return path;
  }
  size_t size=image_bytes(&header);
  if (size > 131072u || stats.bytes + size > BYTE_LIMIT) {
    fclose(file);
    return path;
  }
  uint8_t *data = malloc(size);
  char *name = malloc(strlen(path) + 1);
  if (!data || !name || fread(data, 1, size, file) != size) {
    free(data);
    free(name);
    fclose(file);
    return path;
  }
  fclose(file);
  if (header.flags && (data[0] || data[1] || data[2] || data[3])) {
    free(data);free(name);return path;
  }
  strcpy(name, path);
  unsigned i = stats.images++;
  images[i].path = name;
  images[i].descriptor = (lv_image_dsc_t){.header = header, .data_size = size, .data = data};
  stats.bytes += size;
  return &images[i].descriptor;
}
#endif
const char *pp_image_bank_version(void) {
#ifdef ESP_PLATFORM
  return bank_version[0] ? bank_version : NULL;
#else
  return PP_APP_VERSION;
#endif
}
void pp_image_preload(const char *path) { (void)resolve(path); }
void pp_image_set_src(lv_obj_t *object, const void *source) {
  if (source && lv_image_src_get_type(source) == LV_IMAGE_SRC_FILE)
    source = resolve(source);
  lv_image_set_src(object, source);
  if (source && lv_image_src_get_type(source)==LV_IMAGE_SRC_VARIABLE &&
      (((const lv_image_dsc_t *)source)->header.flags & PP_IMAGE_CENTERED_PET)) {
    /* Only transparent margins were omitted. Preserve the48px object canvas,
     * baseline and center while LVGL draws native Flash pixels directly. */
    lv_obj_set_size(object,48,48);
    lv_image_set_inner_align(object,LV_IMAGE_ALIGN_CENTER);
  }
}
pp_image_stats pp_image_probe(void) { return stats; }

const lv_image_dsc_t *pp_image_descriptor(const void *source) {
  if (source && lv_image_src_get_type(source)==LV_IMAGE_SRC_FILE) source=resolve(source);
  return source && lv_image_src_get_type(source)==LV_IMAGE_SRC_VARIABLE ? source : NULL;
}

unsigned pp_image_canvas_width(const lv_image_dsc_t *im) {
  return im->header.flags & PP_IMAGE_CENTERED_PET ? 48u : im->header.w;
}
unsigned pp_image_canvas_height(const lv_image_dsc_t *im) {
  return im->header.flags & PP_IMAGE_CENTERED_PET ? 48u : im->header.h;
}
uint32_t pp_image_indexed_pixel(const lv_image_dsc_t *im,unsigned x,unsigned y) {
  if (!im || (im->header.cf!=LV_COLOR_FORMAT_I4 && im->header.cf!=LV_COLOR_FORMAT_I8)) return 0;
  unsigned dx=(pp_image_canvas_width(im)-im->header.w)/2;
  unsigned dy=(pp_image_canvas_height(im)-im->header.h)/2;
  if (x<dx || y<dy || x-dx>=im->header.w || y-dy>=im->header.h) return 0;
  x-=dx;y-=dy;
  unsigned palette=im->header.cf==LV_COLOR_FORMAT_I4?64:1024;
  const uint8_t *indices=im->data+palette;
  unsigned index=im->header.cf==LV_COLOR_FORMAT_I4
    ? (indices[y*im->header.stride+x/2]>>(x%2?0:4))&15
    : indices[y*im->header.stride+x];
  uint32_t pixel;memcpy(&pixel,im->data+index*4,4);return pixel;
}
