#include "asset_deflate.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static uint8_t *read_file(const char *p,size_t *n) {
 FILE *f=fopen(p,"rb");assert(f);fseek(f,0,SEEK_END);*n=ftell(f);rewind(f);
 uint8_t *d=malloc(*n+1);assert(d && fread(d,1,*n,f)==*n);d[*n]=0;fclose(f);return d;
}
int main(int argc,char **argv) {
 assert(argc==3);size_t n,raw_n;
 uint8_t *p=read_file(argv[1],&n),*raw=read_file(argv[2],&raw_n);
 assert(pp_deflate_size(p,n)==raw_n);
 uint8_t *guard=malloc(raw_n+32);assert(guard);memset(guard,0xa5,raw_n+32);uint8_t *dst=guard+16;
 pp_inflate_work *work=calloc(1,sizeof(*work));assert(work);
 assert(pp_deflate_decode(p,n,dst,raw_n,work) && !memcmp(dst,raw,raw_n));
 assert(!pp_deflate_decode(p,n,dst,raw_n-1,work));
 assert(!pp_deflate_decode(p,n,dst,raw_n,NULL));
 for(size_t cut=0;cut<n;cut+=97)assert(!pp_deflate_decode(p,cut,dst,raw_n,work));
 assert(!pp_deflate_decode(p,n-1,dst,raw_n,work));
 assert(!pp_deflate_decode(p,n+1,dst,raw_n,work));
 p[n-1]^=1;assert(!pp_deflate_decode(p,n,dst,raw_n,work));p[n-1]^=1;
 p[8]^=1;assert(!pp_deflate_size(p,n));p[8]^=1;
 uint8_t cf=p[13];p[13]=0xff;assert(!pp_deflate_size(p,n));p[13]=cf;
 p[20]^=1;assert(!pp_deflate_size(p,n));p[20]^=1;
 for(unsigned i=0;i<16;i++)assert(guard[i]==0xa5 && guard[raw_n+16+i]==0xa5);
 assert(!pp_static_background_path("pets/45-0.bin") && !pp_static_background_path("pets-left/45-0.bin"));
 free(guard);free(work);free(raw);free(p);return 0;
}
