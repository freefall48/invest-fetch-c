//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include <stdio.h>
#include <curl/curl.h>

#include "nzx/handler.h"
#include "scheduler/schedule.h"

void*
collectListings(void *args)
{
    printf("Collecting listings...");
    fflush(stdout);
    /* Download the market data. */
    memoryChunk_t *chunk = nzxFetchData();
    nzxNode_t *head = NULL;
    /* Process and store the market listings. */
    nzxExtractMarketListings(chunk, &head);
    nzxStoreMarketListings(head);
    /* Finished with the data so free the memory. */
    nzxDrainListings(&head);
    nzxFreeMemoryChunk(chunk);
    printf("done\n");
    return NULL;
}

void*
collectPrices(void *args)
{
    printf("Collecting prices...");
    fflush(stdout);
    /* Download the market data. */
    memoryChunk_t *chunk = nzxFetchData();
    nzxNode_t *head = NULL;
    /* Process and store the market listings. */
    nzxExtractMarketPrices(chunk, &head);
    nzxStoreMarketPrices(head);
    /* Finished with the data so free the memory. */
    nzxDrainListings(&head);
    nzxFreeMemoryChunk(chunk);
    printf("done\n");
    return NULL;
}

void *
test(void *args)
{
    puts("Hi");
    return NULL;
}

int main() {
//    curl_global_init(CURL_GLOBAL_NOTHING);
    taskNode_t* head = NULL;
////    task_t taskListings = {
////            .startHour = 21,
////            .endHour = 6,
////            .rateHour = 0,
////            .rateMinute = 2,
////            .prev = NULL,
////            .func = collectListings
////    };
    task_t taskPrices = {
            .startHour = 21,
            .endHour = 20,
            .rateHour = 0,
            .rateMinute = 1,
            .prev = NULL,
            .func = test
    };
////    taskAdd(&head, &taskListings);
    taskAdd(&head, &taskPrices);
//
    taskProcessor(&head);
//    curl_global_cleanup();
}