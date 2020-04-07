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
    logger_t *logger = (logger_t *) args;
    /* Download the market data. */
    memoryChunk_t *chunk = nzxFetchData(logger);
    nzxNode_t *head = NULL;
    /* Process and store the market listings. */
    nzxExtractMarketListings(chunk, &head, logger);
    nzxStoreMarketListings(head, logger);
    /* Finished with the data so free the memory. */
    nzxDrainListings(&head, logger);
    nzxFreeMemoryChunk(chunk);
    return NULL;
}

void *
collectPrices(void *args) {
    logger_t *logger = (logger_t *) args;
    /* Download the market data. */
    memoryChunk_t *chunk = nzxFetchData(logger);
    nzxNode_t *head = NULL;
    /* Process and store the market listings. */
    nzxExtractMarketPrices(chunk, &head, logger);
    nzxStoreMarketPrices(head, logger);
    /* Finished with the data so free the memory. */
    nzxDrainListings(&head, logger);
    nzxFreeMemoryChunk(chunk);
    return NULL;
}

int main() {
    curl_global_init(CURL_GLOBAL_NOTHING);
    loggerInit(0, DEBUG);

    taskNode_t *head = NULL;
    task_t taskListings = {
            .startHour = 21,
            .endHour = 6,
            .rateHour = 0,
            .rateMinute = 2,
            .prev = NULL,
            .func = collectListings,
            .id = 1
    };
    task_t taskPrices = {
            .startHour = 21,
            .endHour = 6,
            .rateHour = 0,
            .rateMinute = 1,
            .prev = NULL,
            .func = collectPrices,
            .id = 2
    };
    taskAdd(&head, &taskListings);
    taskAdd(&head, &taskPrices);

    taskProcessor(logger, &head);
    curl_global_cleanup();



}