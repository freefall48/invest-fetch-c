//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//
#include "schedule.h"

struct schedulerTask {

    time_t next;

    char *cronExpression;

    void *(*func)(void *);

};

static void
currentUTC(struct tm **ptm) {
    time_t now = time(&now);
    /* Check that we actually got a time. */
    if (now == -1) {
        puts("Failed to get the current time.");
    }
    /* Now get the GMT time and check the conversion. */
    *ptm = gmtime(&now);
    if (ptm == NULL) {
        puts("Failed to convert to GMT time");
    }
    /* Stop Daylight savings as UTC does not use this. */
    (*ptm)->tm_isdst = -1;
}

void
taskAdd(taskNode_t **head, task_t *task) {
    /* Get the current UTC time and work out when this task next needs to be called. */
    struct tm *ptm, *runTime;
    taskNode_t *current, *node;
    cron_expr expr;
    const char *err = NULL;

    memset(&expr, 0, sizeof(expr));
    cron_parse_expr(task->cronExpression, &expr, w & err);

    if (err) {
        // TODO: Error
    }

    currentUTC(&ptm);

    calcNextRun(ptm, task);
    runTime = (struct tm *) malloc(sizeof(struct tm));
    *runTime = *ptm;
    task->next = runTime;

    node = (taskNode_t *) malloc(sizeof(taskNode_t));
    node->task = *task;
    /* Check if the queue is empty of the task should be the first in the queue. */
    if (*head == NULL || mktime(((*head)->task.next)) >= mktime((node->task.next))) {
        node->next = *head;
        *head = node;
    }
    else {
        current = *head;
        /* Move to the correct position this task should be added. */
        while (current->next != NULL && mktime((current->next->task.next)) < mktime((node->task.next)))
        {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
    }
}

void
taskProcessor(logger_t *logger, taskNode_t **head)
{
    threadPool_t *threadPool = thrPoolCreate(1, 2, 120, NULL, logger);
    while (1) {
        /* Get the current UTC time. */
        struct tm *ptm;
        currentUTC(&ptm);
        /* Check we have not been given an empty list of tasks. */
        if (*head == NULL) {
            break;
        }
        /* Get the next task to be executed. */
        taskNode_t *next = (*head) -> next;
        task_t task = (*head)->task;
        /* Calculate and wait until the task should be executed. */
        long int duration = mktime(task.next) - mktime(ptm);
        /* Sleep if we need to. */
        if (duration > 0) {
            char buffer[80];
            /* Produce the date/time in a format that is easy to read in log files etc. */
            strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", task.next);
            logInfo(logger, "[Scheduler] Waiting until %s.", buffer)
            sleep(duration);
        }
        if (thrPoolQueue(threadPool, task.func, logger) == -1) {
            logError(logger, "Failed to send task '%d' to worker pool queue.", task.id)
        }
        logInfo(logger, "[Scheduler] Sent task '%d' to worker pool queue.", task.id)
        task.prev = task.next;
        /* Free up memory allocations. */
        if (task.prev != NULL) {
            free(task.prev);
        }
        /* Add the task back into the queue. */
        free(*head);
        (*head) = next;
        taskAdd(head, &task);
    }
    /* Something has broken or has requested this processor to stop. So cleanup
     * the pool. */
    thrPoolWait(threadPool);
    thrPoolDestroy(threadPool);
}