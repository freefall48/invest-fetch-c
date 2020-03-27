//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#ifndef INVEST_FETCH_C_SCHEDULE_H
#define INVEST_FETCH_C_SCHEDULE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

typedef struct Task {
    /*
     * Starting and ending times for this task.
     */
    uint8_t startHour;
    uint8_t startMinute;
    uint8_t endHour;
    uint8_t endMinute;

    time_t next;
    time_t prev;

    /*
     * The function to call when the timer elapses.
     */
    void (*func)(void);
} task_t;

typedef struct TaskNode {
    task_t task;
    struct TaskNode *next;
} taskNode_t;

void taskAdd(taskNode_t **head, task_t task);

#endif //INVEST_FETCH_C_SCHEDULE_H
