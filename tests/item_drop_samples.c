#include "item_drop.h"
#include <stdio.h>
#include <stdint.h>
int main(void){
 for(unsigned size=24;size<=32;size+=8)for(int spot=0;spot<24;spot++)for(unsigned age=0;age<=1200;age+=10){
  pp_item_drop_state s=pp_item_drop(age,70+spot%6*40,145+spot/6*22,size);
  printf("%u %d %u %d %d %u\n",size,spot,age,s.x,s.y,s.opacity);
 }
 pp_item_drop_state s=pp_item_drop(UINT32_MAX,70,145,24);
 printf("24 0 4294967295 %d %d %u\n",s.x,s.y,s.opacity);
 return 0;
}
