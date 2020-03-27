//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//


#ifndef INVEST_FETCH_C_HTTPOPS_H
#define INVEST_FETCH_C_HTTPOPS_H

#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#define NZX_URL "https://www.nzx.com/markets/NZSX"

typedef struct MemoryChunk {
    char *memory;
    size_t size;
} memoryChunk_t;


memoryChunk_t *nzxFetchData(void);

void nzxFreeMemoryChunk(memoryChunk_t *chunk);

#endif //INVEST_FETCH_C_HTTPOPS_H
