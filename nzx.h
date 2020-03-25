#ifndef TABLER_H
#define TABLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "nzxList.h"

typedef struct MemoryChunk {
  char *memory;
  size_t size;
} memoryChunk_t;

memoryChunk_t* 
FetchNZXData(void);

void 
extractMarketListings(memoryChunk_t* chunk, NZXNode_t** head);

void 
freeMemoryChunk(memoryChunk_t* chunk);

#endif