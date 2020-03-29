//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include <stdio.h>
#include <curl/curl.h>

#include "nzx/handler.h"
#include "scheduler/schedule.h"

void
hi(void)
{
    puts("I was called!");
}

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
task_t task2;

/* Define the start and end time for this task*/
task.startHour = 2;
task.endHour = 4;
task.rateHour = 1;
task.rateMinute = 2;

task.func = hi;

/* Define the start and end time for this task*/
//task2.startHour = 4;
//task2.endHour = 7;
//task2.rateHour = 1;
//task2.rateMinute = 20;

//taskAdd(&head, &task2);
taskAdd(&head, &task);


printf("Task1   : %s", asctime(&task.next));

taskProcessor(&head);

}