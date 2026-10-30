#pragma once
/* Lossless static-background bank wrapper. Pet frames remain native/mmap. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline uint16_t pp_rle_u16(const uint8_t *p) {
  return (uint16_t)p[0] | (uint16_t)p[1] << 8;
}
static inline uint32_t pp_rle_u32(const uint8_t *p) {
  return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
         (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static inline bool pp_row_rle_magic(const uint8_t *p, size_t size) {
  return size >= 8 && !memcmp(p,"PPRLE1\0\0",8);
}
static inline size_t pp_row_rle_size(const uint8_t *p, size_t size) {
  if (!pp_row_rle_magic(p,size) || size<24) return 0;
  const uint8_t *h=p+12;
  unsigned cf=h[1],w=pp_rle_u16(h+4),rows=pp_rle_u16(h+6),stride=pp_rle_u16(h+8);
  if (h[0]!=0x19 || (cf!=9 && cf!=10) || pp_rle_u16(h+2) || pp_rle_u16(h+10) ||
      !w || !rows || stride!=(cf==9?(w+1)/2:w)) return 0;
  size_t prefix=12+(cf==9?64:1024);
  uint64_t total=prefix+(uint64_t)stride*rows;
  /* Bounded static images; reject corrupt dimensions before pool allocation. */
  if (total>2u*1024u*1024u || total!=pp_rle_u32(p+8) || size<12+prefix) return 0;
  return (size_t)total;
}
static inline bool pp_row_rle_decode(const uint8_t *src,size_t size,uint8_t *dst,size_t capacity) {
  size_t total=pp_row_rle_size(src,size);
  if (!total || capacity<total) return false;
  unsigned cf=src[13],rows=pp_rle_u16(src+18),stride=pp_rle_u16(src+20);
  size_t prefix=12+(cf==9?64:1024),at=12+prefix,out=prefix;
  memcpy(dst,src+12,prefix);
  for(unsigned y=0;y<rows;y++) {
    if(at>=size) return false;
    unsigned op=src[at++];
    if(op==0) {
      if(!y) return false;
      memcpy(dst+out,dst+out-stride,stride);
    } else if(op==1) {
      if(size-at<stride) return false;
      memcpy(dst+out,src+at,stride);at+=stride;
    } else if(op==2) {
      unsigned written=0;
      while(written<stride) {
        if(size-at<2) return false;
        unsigned n=src[at++],v=src[at++];
        if(!n || n>stride-written) return false;
        memset(dst+out+written,v,n);written+=n;
      }
    } else return false;
    out+=stride;
  }
  return at==size && out==total;
}
