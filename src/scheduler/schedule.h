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
#include <semaphore.h>
#include <time.h>
#include "../helpers/cron.h"

#include "../threading/threadPool.h"
#include "../logging/logger.h"

typedef struct task task_t;

typedef struct scheduler scheduler_t;

typedef struct taskNode {
    task_t *task;
    struct taskNode *next;
} taskNode_t;

void taskAdd(scheduler_t *scheduler, task_t *task);

void taskDelete(scheduler_t *scheduler, const char *id);

scheduler_t *schedulerCreate(void);

void *schedulerProcess(void *args);

task_t *taskCreate(const char *id, const char *pattern, int repeat, void *(*func)(void *));

#endif //INVEST_FETCH_C_SCHEDULE_H
