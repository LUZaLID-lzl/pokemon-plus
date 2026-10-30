/* Reuse the real ESP mmap/PSRAM host harness, then compare to independent PNGs. */
#define main bank_main
#include "test_image_bank.c"
#undef main
#include <zlib.h>
int main(int argc,char **argv) {
 assert(argc==3);assert(bank_main(2,argv)==0);
 FILE *in=fopen(argv[2],"rb");assert(in);uint8_t header[12];
 assert(fread(header,1,12,in)==12 && !memcmp(header,"PPCANV1\0",8));
 unsigned count=word(header+8),trimmed=0;
 uint32_t pixels[96*96];
 for(unsigned i=0;i<count;i++) {
  uint8_t record[76];assert(fread(record,1,76,in)==76 && memchr(record,0,64));
  char path[100];snprintf(path,sizeof(path),"S:/assets/%s",(const char *)record);
  const lv_image_dsc_t *im=pp_image_descriptor(path);assert(im);
  assert(pp_image_canvas_width(im)==48 && pp_image_canvas_height(im)==48);
  trimmed+=(im->header.flags & PP_IMAGE_CENTERED_PET)!=0;
  const unsigned sizes[]={48,56,96};
  for(unsigned n=0;n<3;n++) {
   unsigned size=sizes[n];
   for(unsigned y=0;y<size;y++)for(unsigned x=0;x<size;x++) {
    unsigned sx=(2*x+1)*48/(2*size),sy=(2*y+1)*48/(2*size);
    pixels[y*size+x]=pp_image_indexed_pixel(im,sx,sy);
   }
   uint32_t actual=(uint32_t)crc32(0,(const Bytef *)pixels,size*size*sizeof(uint32_t));
   if(actual!=word(record+64+4*n))fprintf(stderr,"canvas mismatch:%s size%u\n",path,size);
   assert(actual==word(record+64+4*n));
  }
  assert(!pp_image_indexed_pixel(im,48,0) && !pp_image_indexed_pixel(im,0,48));
 }
 assert(fgetc(in)==EOF && trimmed>0);fclose(in);
 assert(!pp_image_probe().file_reads);
 printf("PASS: %u complete pet frames (%u trimmed) match original48px PNGs at48px courtyard,56px battle,96px evolution; fixed centers/alpha/colors and no frame reads\n",count,trimmed);
 return 0;
}
