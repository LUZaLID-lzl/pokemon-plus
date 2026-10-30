#pragma once
/* Static backgrounds and fishing sprites only: decode once before publishing resident descriptors.
 * Device uses the ESP32-S3 ROM inflater with its work state in the PSRAM pool.
 * Host tests use system zlib to stand in for that ROM primitive. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "asset_row_rle.h"
#ifdef PP_ASSET_HOST_TEST
#include <zlib.h>
typedef struct { unsigned unused; } pp_inflate_work;
#else
/* Pinned, unmodified ESP-IDF 5.5.3 ROM ABI; some local SDK installs omit it.
 * Origin/hash: docs/idf-rom-miniz-source.json. The ROM implementation stays
 * unchanged and its work state still occupies the bounded startup PSRAM pool. */
#include "pp_rom_miniz.h"
typedef tinfl_decompressor pp_inflate_work;
#endif
#define PP_BACKGROUND_POOL_LIMIT (5u*1024u*1024u+320u*1024u)
static inline bool pp_deflate_magic(const uint8_t *p,size_t n) {
  return n>=8 && !memcmp(p,"PPZLIB1\0",8);
}
static inline size_t pp_deflate_size(const uint8_t *p,size_t n) {
  if(!pp_deflate_magic(p,n) || n<=24)return 0;
  const uint8_t *h=p+12;
  unsigned cf=h[1],w=pp_rle_u16(h+4),rows=pp_rle_u16(h+6),stride=pp_rle_u16(h+8);
  if(h[0]!=0x19 || (cf!=9 && cf!=10 && cf!=18 && cf!=20) || pp_rle_u16(h+2) || pp_rle_u16(h+10) || !w || !rows ||
     stride!=(cf==9?(w+1)/2:cf==10?w:w*2))return 0;
  uint64_t size=12+(cf==9?64:cf==10?1024:0)+(uint64_t)stride*rows+(cf==20?(uint64_t)w*rows:0);
  if(size>2u*1024u*1024u || size!=pp_rle_u32(p+8))return 0;
  return (size_t)size;
}
static inline bool pp_deflate_decode(const uint8_t *p,size_t n,uint8_t *dst,size_t cap,pp_inflate_work *work) {
  size_t total=pp_deflate_size(p,n);
  if(!total || cap<total || !work)return false;
  memcpy(dst,p+12,12);
#ifdef PP_ASSET_HOST_TEST
  (void)work;z_stream stream={0};
  stream.next_in=(Bytef *)p+24;stream.avail_in=(uInt)(n-24);
  stream.next_out=dst+12;stream.avail_out=(uInt)(total-12);
  if(inflateInit(&stream)!=Z_OK)return false;
  int status=inflate(&stream,Z_FINISH);
  bool good=status==Z_STREAM_END && stream.total_in==n-24 && stream.total_out==total-12;
  inflateEnd(&stream);return good;
#else
  tinfl_init(work);
  size_t input=n-24,output=total-12;
  tinfl_status status=tinfl_decompress(work,p+24,&input,dst+12,dst+12,&output,
    TINFL_FLAG_PARSE_ZLIB_HEADER|TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF|TINFL_FLAG_COMPUTE_ADLER32);
  return status==TINFL_STATUS_DONE && input==n-24 && output==total-12;
#endif
}
static inline bool pp_static_background_path(const char *path) {
  if(!strcmp(path,"icons-large/10.bin") || !strcmp(path,"icons-large/12.bin"))return true;
  if(!strcmp(path,"hospital/room.bin") || !strcmp(path,"ui/menu-bg.bin"))return true;
  if((!strcmp(path,"fishing/pond.bin") || !strncmp(path,"fishing/fish-",13) || !strncmp(path,"fishing/thumb-",14)))return true;
  return !strncmp(path,"training/",9) || !strncmp(path,"scene-fronts/",13) || !strncmp(path,"thumbs/",7) || !strncmp(path,"scenes/",7) || !strncmp(path,"strips/",7) ||
         !strncmp(path,"battle-scenes/",14) || !strncmp(path,"travel-scenes/",14) ||
         (!strncmp(path,"gyms/",5) && (!strcmp(path+5,"map.bin") || !strcmp(path+5,"arena.bin") ||
          (strlen(path)>9 && !strcmp(path+strlen(path)-8,"-map.bin")) ||
          (strlen(path)>11 && !strcmp(path+strlen(path)-10,"-arena.bin"))));
}
