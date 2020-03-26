#include <stdio.h>
#include <curl/curl.h>

#include "nzx/handler.h"

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    memoryChunk_t* chunk = nzxFetchData();


    NZXNode_t* head = NULL;


//    nzxExtractMarketListings(chunk, &head);
    nzxExtractMarketPrices(chunk, &head);
    nzxStoreMarketPrices(head);
//    nzxStoreMarketListings(head);

    listing_t* current = nzxPopListing(&head);

    while (current) {
//        printf("%s\n", (*current).Code);

//        nzxFreeListing(current);
        current = nzxPopListing(&head);
    }

    nzxFreeMemoryChunk(chunk);
    curl_global_cleanup();
}
