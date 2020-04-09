//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include <stdio.h>
#include <curl/curl.h>
#include "logging/logger.h"

#include "nzx/handler.h"
#include "scheduler/schedule.h"

void *
collectListings(void *args) {
    /* Download the market data. */
    memoryChunk_t *chunk = nzxFetchData();
    nzxNode_t *head = NULL;
    /* Process and store the market listings. */
    nzxExtractMarketListings(chunk, &head);
    nzxStoreMarketListings(head);
    /* Finished with the data so free the memory. */
    nzxDrainListings(&head);
    nzxFreeMemoryChunk(chunk);
    return NULL;
}

void *
collectPrices(void *args) {
    /* Download the market data. */
    memoryChunk_t *chunk = nzxFetchData();
    nzxNode_t *head = NULL;
    /* Process and store the market listings. */
    nzxExtractMarketPrices(chunk, &head);
    nzxStoreMarketPrices(head);
    /* Finished with the data so free the memory. */
    nzxDrainListings(&head);
    nzxFreeMemoryChunk(chunk);
    return NULL;
}

int main() {
    curl_global_init(CURL_GLOBAL_NOTHING);
    loggerInit(0, DEBUG);

    taskNode_t *head = NULL;
    task_t *taskListings = taskCreate("0 20 22 * * 0-4", collectListings);

//    task_t *taskPricesPM = taskCreate("0 */20 22-23 * * 0-4", collectPrices);
//    task_t *taskPricesAM = taskCreate("0 */20 0-5 * * 1-5", collectPrices);
    task_t *taskPricesAM = taskCreate("0 * * * * *", collectPrices);

    taskAdd(&head, taskListings);
    taskAdd(&head, taskPricesAM);
//    taskAdd(&head, taskPricesPM);

    taskProcessor(&head);
    curl_global_cleanup();


}