#pragma once
/* Pure native120 RGB565 scenery. Fixed buffers, no RNG or runtime allocations. */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define PP_FISHING_SCENERY 13u
static inline bool pp_pond_safe(int x,int y){int dx=x*2-119,dy=y*2-119;return dx*dx+dy*dy<114*114;}
static inline bool pp_pond_water(int x,int y){
  if(y<46 || y>112)return false;
  int left=y<55?50:y<72?49:y<93?31:22;
  int right=y<55?94:y<72?107:114;
  return x>=left && x<=right;
}
static inline bool pp_pond_leaf(uint16_t c){
  unsigned r=c>>11,g=(c>>5)&63,b=c&31;
  return g>r+8 && g>b*2+6;
}
static inline uint16_t pp_pond_mix(uint16_t c,uint16_t ink,unsigned a){
  unsigned r=((c>>11)*(255-a)+(ink>>11)*a)/255;
  unsigned g=(((c>>5)&63)*(255-a)+((ink>>5)&63)*a)/255;
  unsigned b=((c&31)*(255-a)+(ink&31)*a)/255;
  return (uint16_t)((r<<11)|(g<<5)|b);
}
static inline void pp_pond_dot(uint16_t *out,int x,int y,uint16_t c,unsigned a,bool water){
  if(x<1 || x>118 || y<1 || y>118 || !pp_pond_safe(x,y) || (water&&!pp_pond_water(x,y)))return;
  unsigned n=(unsigned)y*120+(unsigned)x;out[n]=pp_pond_mix(out[n],c,a);
}
static inline void pp_fishing_scenery_render(uint32_t t,const uint16_t *base,uint16_t *out){
  memcpy(out,base,14400*sizeof(*out));
  /* Bend only connected green interiors. Trunks, silhouette and dock stay fixed. */
  for(int y=2;y<54;y++)for(int x=2;x<118;x++){
    if(!pp_pond_safe(x,y) || (x>=46 && x<=83 && y>=28))continue;
    unsigned p=(t+(x<60?0:1700)+(unsigned)y*21)%6400;
    int dx=p<800?0:p<2800?1:p<4000?0:p<6000?-1:0;
    unsigned n=(unsigned)y*120+(unsigned)x;
    if(dx&&pp_pond_leaf(base[n])&&pp_pond_leaf(base[n-1])&&pp_pond_leaf(base[n+1]))out[n]=base[(int)n-dx];
  }
  /* Three expanding elliptical ripples, with staggered origins and fade-out. */
  const int centers[3][2]={{73,54},{89,70},{95,90}};
  for(unsigned i=0;i<3;i++){
    unsigned period=3600+i*800,age=(t+i*1300)%period;
    int rx=2+(int)(age*9/period),ry=1+rx/3;
    unsigned alpha=125-age*110/period;
    for(int y=-ry;y<=ry;y++)for(int x=-rx;x<=rx;x++){
      int d=x*x*ry*ry+y*y*rx*rx-rx*rx*ry*ry;
      if(d<0)d=-d;
      if(d<=rx*rx/2)pp_pond_dot(out,centers[i][0]+x,centers[i][1]+y,0x8e99,alpha,true);
    }
  }
  /* Fish swim through the water, fade in, linger, then vanish below the surface. */
  for(unsigned i=0;i<3;i++){
    unsigned age=(t+i*2700)%9000;
    unsigned a=age<1500?age*95/1500:age<4500?95:age<6500?(6500-age)*95/2000:0;
    int x=i==1?96-(int)(age*24/9000):i==2?100-(int)(age*13/9000):54+(int)(age*32/9000),y=i==2?90:61+(int)i*17;
    for(int dy=-1;dy<=1;dy++)for(int dx=-3;dx<=3;dx++)if(!dy||(dx>=-1&&dx<=1))pp_pond_dot(out,x+dx,y+dy,0x09e8,a,true);
    int tail=i?4:-4;
    pp_pond_dot(out,x+tail,y-1,0x09e8,a,true);pp_pond_dot(out,x+tail,y+1,0x09e8,a,true);
  }
  /* Five tiny leaves drift from the banks; wet leaves follow the slow current. */
  for(unsigned i=0;i<5;i++){
    unsigned period=7600+i*600,age=(t+i*1900)%period;
    int bend=(int)((age*12/period)%6);if(bend>3)bend=6-bend;
    int dx=(int)(age*23/period),x=i%2?102-dx+bend:15+dx+bend,y=25+(int)(age*76/period);
    unsigned a=age<600?age*220/600:age>period-1800?(period-age)*220/1800:220;
    uint16_t color=i%2?0xbde9:0x9d4b;
    pp_pond_dot(out,x,y,color,a,false);pp_pond_dot(out,x+1,y,color,a,false);
    pp_pond_dot(out,x+(i%2?1:0),y+1,color,a,false);pp_pond_dot(out,x-1,y+1,0x6b66,a,false);
  }
}
