#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* send must complete its DMA read before returning. The source is compact
 * little-endian RGB565; only the fixed DMA staging block is byte-swapped. */
typedef bool (*pp_display_send)(void *ctx,unsigned y,unsigned rows,
                               const uint8_t *pixels,size_t bytes);
static inline bool pp_display_transfer(unsigned width,unsigned height,
    const uint8_t *source,uint8_t *stage,size_t capacity,
    pp_display_send send,void *ctx) {
  if(!width || !height || width>360 || height>360 || (width&1) ||
      !source || !stage || !send || capacity<width*2)return false;
  unsigned rows=capacity/(width*2);
  for(unsigned y=0;y<height;y+=rows) {
    unsigned count=height-y<rows?height-y:rows;
    size_t bytes=(size_t)width*count*2;
    const uint8_t *src=source+(size_t)y*width*2;
    for(size_t i=0;i<bytes;i+=2) {stage[i]=src[i+1];stage[i+1]=src[i];}
    if(!send(ctx,y,count,stage,bytes))return false;
  }
  return true;
}
