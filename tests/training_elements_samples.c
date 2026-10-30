#include "training_scenery.h"
#include <stdio.h>
static bool comma;
static void paint(void *data,int x,int y,int w,int h,unsigned color,unsigned alpha){
 (void)data;printf("%s[%d,%d,%d,%d,%u,%u]",comma?",":"",x,y,w,h,color,alpha);comma=true;
}
int main(void){for(unsigned scene=0;scene<6;scene++)for(unsigned time=0;time<=6400;time+=40){printf("[%u,%u,[",scene,time);comma=false;pp_training_battle_elements(scene,time,paint,NULL);puts("]]");}return 0;}
