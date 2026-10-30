#include "battle_entrance.h"
#include <stdio.h>
static unsigned first;
static void paint(void *data,int x,int y,int w,int h,unsigned c,unsigned a){(void)data;printf("%s[%d,%d,%d,%d,%u,%u]",first?"":",",x,y,w,h,c,a);first=0;}
int main(void){for(unsigned t=0;t<=1100;t+=10)for(unsigned side=0;side<2;side++){pp_entry_state s=pp_battle_entry_state(t);printf("[%u,%u,%u,%u,%d,[",side,t,s.opacity,s.white,s.drop);first=1;pp_battle_entry_paint(side,t,paint,0);puts("]]");}return 0;}
