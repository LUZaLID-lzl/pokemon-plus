#include "training_scenery.h"
#include <stdio.h>
int main(void){
 for(unsigned s=0;s<6;s++)for(unsigned n=1;n<=3;n++)for(unsigned t=0;t<=40000;t+=50)for(unsigned i=0;i<n;i++){
  int x,y;bool left;pp_training_position(s,i,n,t,&x,&y,&left);printf("[%u,%u,%u,%u,%d,%d,%u]\n",s,n,t,i,x,y,left);
 }
 return 0;
}
