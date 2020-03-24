#include <stdio.h>
#include <curl/curl.h>

#include "nzx.h"

int main(void) {
    curl_global_init(CURL_GLOBAL_ALL);
    memoryChunk_t* chunk = FetchNZXData();

    ExtractListings(chunk);

    freeMemoryChunk(chunk);
    curl_global_cleanup();
}