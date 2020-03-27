//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include "httpOps.h"

static size_t
writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t i = size * nmemb;
    memoryChunk_t *mem = (memoryChunk_t *)userp;

    char *ptr = realloc(mem->memory, mem->size + i + 1);
    if(ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, i);
    mem->size += i;
    mem->memory[mem->size] = 0;

    return i;
}

memoryChunk_t*
nzxFetchData(void)
{
    CURL *curl_handle;
    CURLcode res;

    memoryChunk_t* chunk = malloc(sizeof (memoryChunk_t));

    chunk->memory = malloc(1);
    chunk->size = 0;

    curl_handle = curl_easy_init();
    // Set the options for this curl handler
    curl_easy_setopt(curl_handle, CURLOPT_URL, NZX_URL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.132 Safari/537.36");

    res = curl_easy_perform(curl_handle);
    // Check for errors
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    } else {
        printf("%lu bytes retrieved\n", (unsigned long)chunk->size);
    }

    // Clean up and return
    curl_easy_cleanup(curl_handle);
    return chunk;

}

void
nzxFreeMemoryChunk(memoryChunk_t* chunk)
{
    free(chunk->memory);
    free(chunk);
}
