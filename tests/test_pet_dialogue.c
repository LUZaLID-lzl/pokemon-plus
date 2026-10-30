#include "pet_dialogue.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
  (void)argv;
  assert(pp_talk_context(0,29,80,80,80,80,0)==PP_TALK_HUNGRY);
  assert(pp_talk_context(0,80,29,80,80,80,0)==PP_TALK_DIRTY);
  assert(pp_talk_context(0,10,10,29,10,10,0)==PP_TALK_UNWELL);
  assert(pp_talk_context(0,80,80,80,80,29,0)==PP_TALK_TIRED);
  assert(pp_talk_context(0,80,80,80,29,80,0)==PP_TALK_LONELY);
  for(unsigned scene=0;scene<13;scene++) {
    assert(pp_talk_context(scene,30,30,30,30,30,0)==PP_TALK_SCENE0+scene);
    assert(pp_talk_context(scene,80,80,80,80,80,2)==PP_TALK_IDLE);
  }
  const unsigned items[]={0,1,2,3,4,8};const int icons[]={0,1,2,3,14,8};
  for(unsigned i=0;i<6;i++)assert(pp_talk_catalog[pp_talk_item(items[i])].icon==icons[i]);
  for(unsigned pet=0;pet<5;pet++) {
    uint16_t cursors[PP_TALK_TOPICS]={0};
    for(unsigned topic=0;topic<PP_TALK_TOPICS;topic++) {
      const char *previous="";
      for(unsigned n=0;n<1000;n++) {
        pp_talk_line line=pp_talk_pick(cursors,topic,pet*7);
        assert(strlen(line.text)<96 && strcmp(previous,line.text));previous=line.text;
        if(argc>1&&n<12)printf("%u\t%u\t%d\t%s\n",pet,topic,line.icon,line.text);
      }
    }
  }
  if(argc==1)puts("PASS: pet dialogue care icons, needs priority, 13 scenes, threshold boundaries, 140000 selections without consecutive repeats");
}
