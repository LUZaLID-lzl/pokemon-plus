#pragma once
#include <stdint.h>
#include <stddef.h>
#define ESP_OK 0
#define ESP_PARTITION_TYPE_DATA 1
#define ESP_PARTITION_MMAP_DATA 1
typedef int esp_partition_mmap_handle_t;
typedef struct {size_t size;} esp_partition_t;
const esp_partition_t *esp_partition_find_first(int type,int subtype,const char *label);
int esp_partition_read(const esp_partition_t *,size_t,void *,size_t);
int esp_partition_mmap(const esp_partition_t *,size_t,size_t,int,const void **,esp_partition_mmap_handle_t *);
