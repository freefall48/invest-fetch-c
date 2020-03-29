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
}

static void
calcNextRun(struct tm *ptm, task_t *task) {
    /* Stop DST as UTC does not use this */
    ptm->tm_isdst = -1;
    /* Check if this task has a run period that wraps over midnight UTC */
    if (task->startHour > task->endHour) {
        if (ptm->tm_hour >= task->startHour || ptm->tm_hour < task->endHour) {
            /* Add the offset for the next run */
            ptm->tm_hour += task->rateHour;
            ptm->tm_min += task->rateMinute;
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
            ptm->tm_hour = task->startHour;
            ptm->tm_min = 0;
            ptm->tm_sec = 0;
        }
    }
    /* Reform the tm struct to represent the changes to time and handle
     * any overflows caused. */
    mktime(ptm);
}

void
taskAdd(taskNode_t **head, task_t *task) {
    /* Get the current UTC time and work out when this task next needs to be called. */
    struct tm *ptm;
    taskNode_t  *current;
    currentUTC(&ptm);
    calcNextRun(ptm, task);
    task->next = *ptm;
    taskNode_t *node = (taskNode_t *) malloc(sizeof(taskNode_t));
    node->task = *task;
    /* Check if the queue is empty of the task should be the first in the queue. */
    if (*head == NULL || mktime(&((*head)->task.next)) >= mktime(&(node->task.next))) {
        node->next = *head;
        *head = node;
    }
    else {
        current = *head;
        /* Move to the correct position this task should be added. */
        while (current->next != NULL && mktime(&(current->next->task.next)) < mktime(&(node->task.next)))
        {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
    }
}

int
taskProcessor(taskNode_t **head)
{
    while (1) {
        /* Get the current UTC time. */
        struct tm *ptm;
        currentUTC(&ptm);
        /* Check we have not been given an empty list of tasks. */
        if (*head == NULL) {
            return -1;
        }
        /* Get the next task to be executed. */
        taskNode_t *next = (*head) -> next;
        task_t task = (*head)->task;
        /* Calculate and wait until the task should be executed. */
        unsigned int duration = mktime(&task.next) - mktime(ptm);
        if (duration < 0) {
            return -1;
        }
        sleep(duration);
        task.func();
        /* Add the task back into the queue. */
        (*head) = next;
        taskAdd(head, &task);
    }
}