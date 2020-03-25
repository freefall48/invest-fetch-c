#include <stdio.h>
#include <curl/curl.h>

#include "nzx.h"
#include "nzxList.h"

int main(void) {
    curl_global_init(CURL_GLOBAL_ALL);
    memoryChunk_t* chunk = FetchNZXData();


    NZXNode_t* head = NULL;


    extractMarketListings(chunk, &head);

    listing_t* current = popListing(&head);
    while (current) {
        printf("%s\n", (*current).Code);

        freeListing(current);
        current = popListing(&head);
    }

    freeMemoryChunk(chunk);
    curl_global_cleanup();
}