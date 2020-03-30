//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//
#include "schedule.h"

static void
currentUTC(struct tm **ptm)
{
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

static void
calcNextRun(struct tm *ptm, task_t *task) {
    /* Check if this task should only be repeated daily. */
    if (task->startHour == task->endHour) {
        /* Check if this task has been run before. */
        if (ptm->tm_hour > task->startHour) {
            ptm->tm_hour = task->startHour;
            ptm->tm_min = 0;
            ptm->tm_sec = 0;
        } else {
            /* Add a day so its called tomorrow. */
            ptm->tm_mday += 1;
            ptm->tm_hour = task->startHour;
            ptm->tm_min = 0;
            ptm->tm_sec = 0;
        }
    }
    /* Check if this task has a run period that wraps over midnight UTC */
    else if (task->startHour > task->endHour) {
        if (ptm->tm_hour >= task->startHour || ptm->tm_hour < task->endHour) {
            if (task->prev != NULL) {
                /* Add the offset for the next run */
                ptm->tm_hour += task->rateHour;
                ptm->tm_min += task->rateMinute;
                mktime(ptm);
                /* Check we are not calling after the end time. */
                if (ptm->tm_hour >= task->endHour) {
                    ptm->tm_hour = task->endHour;
                    ptm->tm_min = 0;
                    ptm->tm_sec = 0;
                }
            }
        } else {
            ptm->tm_hour = task->startHour;
            ptm->tm_min = 0;
            ptm->tm_sec = 0;
        }
    } else {
        if (ptm->tm_hour >= task->startHour && ptm->tm_hour < task->endHour) {
            /* Add the offset for the next run */
            ptm->tm_hour += task->rateHour;
            ptm->tm_min += task->rateMinute;
        } else {
            /* Check if this task has been run before. */
            if (ptm->tm_hour > task->startHour) {
                ptm->tm_hour = task->startHour;
                ptm->tm_min = 0;
                ptm->tm_sec = 0;
            } else {
                /* Add a day so its called tomorrow. */
                ptm->tm_mday += 1;
                ptm->tm_hour = task->startHour;
                ptm->tm_min = 0;
                ptm->tm_sec = 0;
            }
        }
    }
    /* Reform the tm struct to represent the changes to time and handle
     * any overflows caused. */
    mktime(ptm);
}

void
taskAdd(taskNode_t **head, task_t *task) {
    /* Get the current UTC time and work out when this task next needs to be called. */
    struct tm *ptm, *runTime;
    taskNode_t  *current, *node;

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
taskProcessor(taskNode_t **head)
{
    threadPool_t *threadPool = thrPoolCreate(1, 2, 120, NULL);
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
            printf("[Scheduler] Waiting until %s", asctime(task.next));
            sleep(duration);
        }
        thrPoolQueue(threadPool, task.func, NULL);
        printf("[Scheduler] Sent task '%d' to worker pool queue\n", task.id);
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