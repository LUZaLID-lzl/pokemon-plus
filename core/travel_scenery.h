#pragma once
#include <stdbool.h>
/* Strip-local ambient accents. Visual time only: no game RNG or rewards. */
typedef void (*pp_travel_paint)(void *,int,int,int,int,unsigned,unsigned);
static inline void pp_travel_scenery(unsigned destination,unsigned ms,pp_travel_paint paint,void *data){
  for(unsigned i=0;i<7;i++){
    unsigned phase=(ms/70+i*11)%40;
    unsigned glow=phase<20?phase:39-phase;
    int x=380-(int)((ms/(destination==5?20:40)+i*61)%400);
    int y=12+(int)((i*23+ms/(destination==3?95:310))%82);
    if(destination==0||destination==6||destination==7){
      unsigned tint=destination==6?0xf0d69b:destination==7?0xb5ce8c:0xc5dfa1;
      paint(data,x,y,5,2,tint,145);paint(data,x+2,y-1,2,4,tint,175);
      paint(data,x+4,y+2,3,1,0xe6dab0,100);
    }else if(destination==1){
      y=78+(int)(i%3)*9;
      paint(data,x,y,10+(int)glow/3,1,0xe4f6ec,75+glow*5);
      paint(data,x+4,y+2,7,1,0xb7e9e5,50+glow*3);
    }else if(destination==2||destination==8){
      y=102-(int)((ms/65+i*17)%90);
      unsigned tint=destination==8?0xffbc77:0xf4d3a2;
      paint(data,x,y,2,3,tint,70+glow*6);
      if(destination==8)paint(data,x-1,y+1,4,1,0xff985e,60+glow*3);
    }else if(destination==3){
      paint(data,x,y,2,2,0xf2f8ff,145+glow*4);
      if(i%3==0){paint(data,x-2,y,6,1,0xe5efff,150);paint(data,x,y-2,1,6,0xe5efff,150);}
    }else if(destination==4){
      paint(data,x-2,y-2,5,5,0xdce8a4,12+glow*2);
      paint(data,x,y,1,2,0xf6edaf,90+glow*8);
      paint(data,x-1,y,3,1,0xe4ed9e,55+glow*5);
    }else if(destination==5){
      paint(data,x,y,18+(int)glow/2,2,0xeaf7f8,30+glow*2);
      paint(data,x+5,y-2,12,2,0xeaf7f8,20+glow);
    }else{
      int wing=(ms/130+i)%2?3:1;
      paint(data,x-3,y,3,wing,0xe3caf2,185);
      paint(data,x+1,y,3,wing,0xffe8c4,190);
      paint(data,x,y+1,1,2,0x9b83ad,150);
    }
  }
}
