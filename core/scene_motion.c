#include "scene_motion.h"
#include "fishing_scenery.h"
#include <string.h>
typedef struct { uint8_t y,x,n; } pp_scene_run;
typedef struct { uint16_t kind,period,phase,y0,y1,start,count; } pp_scene_feature;
typedef struct { uint16_t start,count; } pp_scene_range;
#include "scene_motion_config.h"
bool pp_scene_motion_safe(int x,int y) {
  if(x<0 || x>=120 || y<0 || y>=120)return false;
  for(int dy=0;dy<=2;dy+=2)for(int dx=0;dx<=2;dx+=2) {
    int px=x*3+dx,py=y*3+dy,xx=px*2-359,yy=py*2-359;
    if(xx*xx+yy*yy>=342*342 || (px>=80 && px<280 && py>=22 && py<112) ||
       (px>=98 && px<262 && py>=258 && py<328))return false;
  }
  return true;
}
static unsigned pulse(unsigned p) { return p<128?p:255-p; }
static uint16_t light(uint16_t c,unsigned amount,unsigned kind) {
  unsigned r=(c>>11)&31,g=(c>>5)&63,b=c&31;
  r+=amount*(kind==3?2:1)/16;g+=amount/8;b+=amount*(kind==3?1:2)/16;
  if(r>31)r=31;
  if(g>63)g=63;
  if(b>31)b=31;
  return (uint16_t)((r<<11)|(g<<5)|b);
}
static bool contains(const pp_scene_feature *f,int x,int y) {
  if(x<0 || x>=120 || y<0 || y>=120)return false;
  unsigned lo=f->start,hi=lo+f->count;
  while(lo<hi) {
    unsigned mid=lo+(hi-lo)/2;const pp_scene_run *r=&motion_runs[mid];
    if(y<r->y || (y==r->y && x<r->x))hi=mid;
    else if(y>r->y || x>=r->x+r->n)lo=mid+1;
    else return true;
  }
  return false;
}
void pp_scene_motion_render(unsigned scene,uint32_t elapsed,const uint16_t *base,uint16_t *out) {
  if(scene==PP_FISHING_SCENERY){pp_fishing_scenery_render(elapsed,base,out);return;}
  memcpy(out,base,PP_SCENE_PIXELS*sizeof(*out));
  if(scene>=sizeof(motion_scenes)/sizeof(*motion_scenes))return;
  const pp_scene_range *range=&motion_scenes[scene];
  for(unsigned n=0;n<range->count;n++) {
    const pp_scene_feature *f=&motion_features[range->start+n];
    unsigned t=(elapsed%f->period+f->phase)%f->period;
    unsigned phase=t*256/f->period;
    int sway=phase<32?0:phase<112?1:phase<160?0:phase<240?-1:0;
    for(unsigned j=0;j<f->count;j++) {
      const pp_scene_run *r=&motion_runs[f->start+j];int y=r->y;
      if(f->kind==0) {
        /* The bottom third is a fixed pivot; only native leaf pixels sway. */
        int dx=y*3>(int)(f->y0+f->y1*2)?0:sway;
        if(!dx)continue;
        /* Bend connected interior pixels;keep the silhouette and dark gaps fixed.
         * This avoids isolated green pixels and erase/duplicate edge halos. */
        for(int x=r->x;x<r->x+r->n;x++)
          if(x-dx>=r->x && x-dx<r->x+r->n)out[y*120+x]=base[y*120+x-dx];
      } else for(int x=r->x;x<r->x+r->n;x++) {
        uint16_t c=base[y*120+x];unsigned amount=0;
        if(f->kind==1) {
          unsigned wave=(phase+y*13u)%256;int dx=wave<96?1:wave<160?0:-1;
          int sx=x+dx;
          if(sx>=r->x && sx<r->x+r->n)c=base[y*120+sx];
          unsigned crest=(y*17u+256-phase)%256;
          amount=crest<36?(36-crest):0;
        } else if(f->kind==5) {
          int sx=x-(int)(pulse(phase)/32);
          if(contains(f,sx,y))c=base[y*120+sx];
          unsigned crest=(y*19u+256-phase)%256;
          amount=crest<40?12+(40-crest)/2:0;
        } else if(f->kind==4) {
          amount=pulse((phase+x*37u+y*19u)%256)/2;
        } else {
          amount=pulse(phase)/5;
          unsigned band=(y*9u+x*3u+256-phase)%256;
          if(band<26)amount+=(26-band);
        }
        out[y*120+x]=light(c,amount,f->kind);
      }
    }
  }
}
