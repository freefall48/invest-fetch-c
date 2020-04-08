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
#include <string.h>
#include <time.h>
#include "../lib/cron.h"

#include "../threading/threadPool.h"
#include "../logging/logger.h"

typedef struct schedulerTask task_t;

typedef struct TaskNode {
    task_t *task;
    struct TaskNode *next;
} taskNode_t;

void taskAdd(taskNode_t **head, task_t *task);

void taskProcessor(taskNode_t **head);

task_t *taskCreate(const char *exePattern, void *(*func)(void *));

#endif //INVEST_FETCH_C_SCHEDULE_H
