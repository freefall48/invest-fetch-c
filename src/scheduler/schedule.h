//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#ifndef INVEST_FETCH_C_SCHEDULE_H
#define INVEST_FETCH_C_SCHEDULE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "../threading/threadPool.h"

typedef struct Task {
    /*
     * Starting and ending times for this task.
     */
    uint8_t startHour;
    uint8_t endHour;
    uint8_t rateHour;
    uint8_t  rateMinute;

    struct tm *next;
    struct tm *prev;

    uint8_t id;

    /*
     * The function to call when the timer elapses.
     */
    void *(*func)(void*);
} task_t;

typedef struct TaskNode {
    task_t task;
    struct TaskNode *next;
} taskNode_t;

void taskAdd(taskNode_t **head, task_t *task);

void taskProcessor(taskNode_t **head);

#endif //INVEST_FETCH_C_SCHEDULE_H
