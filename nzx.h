#ifndef TABLER_H
#define TABLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

typedef struct MemoryChunk {
  char *memory;
  size_t size;
} memoryChunk_t;

typedef struct Listing {
  char *code;
  char *company;
} listing_t;

memoryChunk_t* 
FetchNZXData(void);

void 
ExtractListings(memoryChunk_t* chunk);

void 
freeMemoryChunk(memoryChunk_t* chunk);

#endif