//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//
#include "schedule.h"

struct schedulerTask {

    time_t next;

    const char *cronExpression;

    void *(*func)(void *);

};

task_t *
taskCreate(const char *exePattern, void *(*func)(void *)) {
    task_t *task = (task_t *) malloc(sizeof(task_t));

    task->func = func;
    task->cronExpression = exePattern;

    return task;
}

void
taskAdd(taskNode_t **head, task_t *task) {
    /* Get the current UTC time and work out when this task next needs to be called. */
    taskNode_t *current, *node;
    cron_expr expr;
    const char *err = NULL;

    memset(&expr, 0, sizeof(expr));
    cron_parse_expr(task->cronExpression, &expr, &err);

    if (err) {
        logError("Error in CRON expression")
        return;
    }

    time_t cur = time(NULL);
    time_t next = cron_next(&expr, cur);

    task->next = next;

    node = (taskNode_t *) malloc(sizeof(taskNode_t));
    node->task = task;
    /* Check if the queue is empty of the task should be the first in the queue. */
    if (*head == NULL || ((*head)->task->next >= node->task->next)) {
        node->next = *head;
        *head = node;
    } else {
        current = *head;
        /* Move to the correct position this task should be added. */
        while (current->next != NULL && (current->next->task->next < node->task->next)) {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
    }
}

void
taskProcessor(taskNode_t **head)
{
    threadPool_t *threadPool = thrPoolCreate(1, 2, 120, NULL);
    while (1) {
        /* Check we have not been given an empty list of tasks. */
        if (*head == NULL) {
            break;
        }
        /* Get the next task to be executed. */
        taskNode_t *next = (*head)->next;
        task_t *task = (*head)->task;
        /* Calculate and wait until the task should be executed. */
        long int duration = task->next - time(NULL);
        /* Sleep if we need to. */
        if (duration > 0) {
            char buffer[80];
            /* Produce the date/time in a format that is easy to read in log files etc. */
            strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", gmtime(&task->next));
            logInfo("Next task executing at: %s.", buffer)
            sleep(duration);
        }
        if (thrPoolQueue(threadPool, task->func, NULL) == -1) {
            logError("Failed to send a task to the threadpool.")
        }
        logInfo("Sent a task to the threadpool.")

        /* Add the task back into the queue. */
        free(*head);
        (*head) = next;
        taskAdd(head, task);
    }
    /* Something has broken or has requested this processor to stop. So cleanup
     * the pool. */
    thrPoolWait(threadPool);
    thrPoolDestroy(threadPool);
}