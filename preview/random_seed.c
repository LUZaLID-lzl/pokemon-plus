#define _POSIX_C_SOURCE 200809L
#include "random_seed.h"
#include <stdio.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#else
#include <unistd.h>
#endif

uint32_t pp_simulator_seed(bool reproducible) {
  if(reproducible)return 42;
  uint32_t seed=0;
#ifdef _WIN32
  if(BCryptGenRandom(NULL,(PUCHAR)&seed,sizeof(seed),BCRYPT_USE_SYSTEM_PREFERRED_RNG)>=0 && seed)
    return seed;
  LARGE_INTEGER counter={0};FILETIME wall;
  QueryPerformanceCounter(&counter);GetSystemTimeAsFileTime(&wall);
  uint64_t ticks=(uint64_t)counter.QuadPart ^ ((uint64_t)wall.dwHighDateTime<<32) ^ wall.dwLowDateTime;
  uint32_t process=GetCurrentProcessId();
#else
  FILE *random=fopen("/dev/urandom","rb");
  if(random) {
    bool complete=fread(&seed,sizeof(seed),1,random)==1;
    fclose(random);
    if(complete && seed)return seed;
  }
  struct timespec counter={0},wall={0};
  clock_gettime(CLOCK_MONOTONIC,&counter);clock_gettime(CLOCK_REALTIME,&wall);
  uint64_t ticks=(uint64_t)counter.tv_sec*1000000000u+counter.tv_nsec;
  ticks^=(uint64_t)wall.tv_sec*1000000000u+wall.tv_nsec;
  uint32_t process=(uint32_t)getpid();
#endif
  /* Keep gameplay available if the OS random source fails. Mix both clocks,
   * process and a call counter rather than returning a fixed fallback seed. */
  static uint32_t calls;
  uint64_t mixed=ticks ^ ((uint64_t)process<<32) ^ ++calls;
  mixed=(mixed^(mixed>>30))*UINT64_C(0xbf58476d1ce4e5b9);
  mixed=(mixed^(mixed>>27))*UINT64_C(0x94d049bb133111eb);
  mixed^=mixed>>31;
  seed=(uint32_t)mixed ^ (uint32_t)(mixed>>32);
  return seed?seed:1;
}
