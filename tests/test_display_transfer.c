#include "display_transfer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t source[360*360*2],original[sizeof(source)];
static uint8_t panel[sizeof(source)],stage[4092+16];
typedef struct {unsigned width,height,next_y,calls,fail_at;} receiver;
static bool receive(void *ctx,unsigned y,unsigned rows,const uint8_t *pixels,size_t bytes) {
  receiver *r=ctx;
  assert(y==r->next_y && rows && y+rows<=r->height);
  assert(bytes==r->width*rows*2 && bytes<=4092 && !(bytes&3));
  assert(pixels==stage && !(bytes%4));
  if(++r->calls==r->fail_at)return false;
  memcpy(panel+(size_t)y*r->width*2,pixels,bytes);r->next_y+=rows;
  return true;
}
static void check(unsigned width,unsigned height) {
  size_t bytes=width*height*2;
  memset(panel,0xcd,sizeof(panel));memset(stage,0xa7,sizeof(stage));
  receiver r={.width=width,.height=height};
  assert(pp_display_transfer(width,height,source,stage,4092,receive,&r));
  assert(r.next_y==height);
  for(size_t i=0;i<bytes;i+=2)assert(panel[i]==source[i+1] && panel[i+1]==source[i]);
  for(size_t i=bytes;i<sizeof(panel);i++)assert(panel[i]==0xcd);
  for(size_t i=4092;i<sizeof(stage);i++)assert(stage[i]==0xa7);
  assert(!memcmp(source,original,sizeof(source)));
  if(width==360 && height==360)assert(r.calls==72);
}
int main(void) {
  for(size_t i=0;i<sizeof(source);i++)source[i]=(i*13+i/720*7)%251;
  memcpy(original,source,sizeof(source));
  check(360,360);check(360,1);check(48,48);check(2,359);check(358,357);check(70,36);
  receiver r={.width=360,.height=360,.fail_at=4};
  assert(!pp_display_transfer(360,360,source,stage,4092,receive,&r));
  assert(r.calls==4 && r.next_y==15); /* No submissions after a failed rectangle. */
  r=(receiver){0};
  assert(!pp_display_transfer(359,20,source,stage,4092,receive,&r));
  assert(!pp_display_transfer(360,20,source,stage,719,receive,&r));
  assert(!r.calls);
  puts("PASS: fixed DMA rectangles cover full screen/bottom row and partial areas exactly; wire RGB565, alignment, canaries, immutable source and failed-submission stop");
}
