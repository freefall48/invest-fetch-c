//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include <stdio.h>
#include <curl/curl.h>

#include "nzx/handler.h"

int main() {
    curl_global_init(CURL_GLOBAL_NOTHING);
    memoryChunk_t* chunk = nzxFetchData();
    NZXNode_t* head = NULL;
//    nzxExtractMarketListings(chunk, &head);
    nzxExtractMarketPrices(chunk, &head);
    nzxStoreMarketPrices(head);
//    nzxStoreMarketListings(head);
    listing_t* current = nzxPopListing(&head);
    while (current) {

        nzxFreeListing(current);
        current = nzxPopListing(&head);
    }
    nzxFreeMemoryChunk(chunk);
    curl_global_cleanup();
}