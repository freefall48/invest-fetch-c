//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include <stdio.h>
#include <curl/curl.h>

#include "nzx/handler.h"
#include "scheduler/schedule.h"

int main() {
/*    curl_global_init(CURL_GLOBAL_NOTHING);
    memoryChunk_t *chunk = nzxFetchData();
    NZXNode_t *head = NULL;
//    nzxExtractMarketListings(chunk, &head);
    nzxExtractMarketPrices(chunk, &head);
    nzxStoreMarketPrices(head);
//    nzxStoreMarketListings(head);
    listing_t *current = nzxPopListing(&head);
    while (current) {

        nzxFreeListing(current);
        current = nzxPopListing(&head);
    }
    nzxFreeMemoryChunk(chunk);
    curl_global_cleanup();*/

/* Testing the scheduler */
taskNode_t* head = NULL;
task_t task;

/* Define the start and end time for this task*/
task.startHour = 21;
task.startMinute = 0;
task.endHour = 4;
task.endMinute = 30;

taskAdd(&head, task);


}