#include "asset_row_rle.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t *read_file(const char *path,size_t *n) {
  FILE *f=fopen(path,"rb");assert(f);fseek(f,0,SEEK_END);*n=ftell(f);rewind(f);
  uint8_t *p=malloc(*n);assert(p && fread(p,1,*n,f)==*n);fclose(f);return p;
}
int main(int argc,char **argv) {
  assert(argc==3);size_t n,raw_size;uint8_t *source=read_file(argv[1],&n),*raw=read_file(argv[2],&raw_size);
  assert(pp_row_rle_size(source,n)==raw_size);
  uint8_t *out=malloc(raw_size+16);assert(out);memset(out,0xa5,raw_size+16);
  assert(pp_row_rle_decode(source,n,out,raw_size) && !memcmp(out,raw,raw_size));
  for(unsigned i=0;i<16;i++)assert(out[raw_size+i]==0xa5);
  assert(!pp_row_rle_decode(source,n,out,raw_size-1));
  for(size_t cut=0;cut<n;cut+=97)assert(!pp_row_rle_decode(source,cut,out,raw_size));
  size_t row_at=24+(source[13]==9?64:1024);uint8_t old=source[row_at];
  source[row_at]=0;assert(!pp_row_rle_decode(source,n,out,raw_size));
  source[row_at]=3;assert(!pp_row_rle_decode(source,n,out,raw_size));
  source[row_at]=2;uint8_t run=source[row_at+1];source[row_at+1]=0;
  assert(!pp_row_rle_decode(source,n,out,raw_size));source[row_at+1]=run;source[row_at]=old;
  source[8]^=1;assert(!pp_row_rle_size(source,n));
  free(source);free(raw);free(out);return 0;
}
