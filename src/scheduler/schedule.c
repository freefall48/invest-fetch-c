//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//
#include "schedule.h"

#define SCHEDULER_DEL_CURRENT 0x02u
#define SCHEDULER_EXIT 0x01u

struct scheduler {
    pthread_mutex_t queueMutex;     // Protects the scheduler queue.
    pthread_cond_t waitingCond;     // Signaled when the queue is non empty
    taskNode_t *head;               // Pointer to the task as the head of the queue.
    unsigned int schedulerNTasks;   // Number of tasks in the queue.
    unsigned int schedulerFlags;    // Args passed to the scheduler.
    char *currentTaskId;            // The id of the current task the scheduler is processing;
};

struct task {
    char *id;               // Id of the task, must be unique
    time_t next;            // Next execution time of the task
    int repeat;             // Repeat the job until removed or single run
    char *pattern;          // Cron expression used to determine the next execution time.
    void *(*func)(void *);  // Function to execute when the task is called.
};

static inline void taskFree(task_t *task) {
    free(task->id);
    free(task->pattern);
    free(task);
}

static inline void taskNodeFree(taskNode_t *node) {
    taskFree(node->task);
    free(node);
}

task_t *taskCreate(const char *id, const char *pattern, int repeat, void *(*func)(void *)) {
    task_t *task;
    size_t len;

    task = (task_t *) malloc(sizeof(task_t));
    task->func = func;
    task->repeat = repeat;

    len = strlen(pattern) + 1;
    task->pattern = (char *) malloc(len * sizeof(char));
    strncpy(task->pattern, pattern, len);

    len = strlen(id) + 1;
    task->id = (char *) malloc(len * sizeof(char));
    strncpy(task->id, id, len);

    return task;
}

/* Get the next task to be executed. Remove the node from the
 * queue but do not free the task contained.
 */
task_t *taskPop(scheduler_t *scheduler) {
    task_t *task;
    taskNode_t *next;

    pthread_mutex_lock(&(scheduler->queueMutex));
    if (scheduler->head == NULL) {
        task = NULL;
    } else {
        task = scheduler->head->task;
        next = scheduler->head->next;
        free(scheduler->head);
        scheduler->head = next;
        scheduler->schedulerNTasks -= 1;
    }
    pthread_mutex_unlock(&(scheduler->queueMutex));
    return task;
}

void taskDelete(scheduler_t *scheduler, const char *id) {
    taskNode_t *curr, *prev;

    prev = NULL;
    pthread_mutex_lock(&(scheduler->queueMutex));
    /* Check for the special case that it is currently being executed. */
    if (strcmp(scheduler->currentTaskId, id) == 0) {
        scheduler->schedulerFlags |= SCHEDULER_DEL_CURRENT;
    } else {
        for (curr = scheduler->head; curr != NULL; prev = curr, curr = curr->next) {
            if (strcmp(curr->task->id, id) == 0) {
                if (prev == NULL) {
                    scheduler->head = curr->next;
                } else {
                    prev->next = curr->next;
                }
                taskNodeFree(curr);
                scheduler->schedulerNTasks -= 1;
            }
        }
    }
    pthread_mutex_unlock(&(scheduler->queueMutex));
}

static int checkId(taskNode_t *head, const char *id) {
    while (head != NULL) {
        if (strcmp(head->task->id, id) == 0) {
            return 1;
        }
        head = head->next;
    }
    return 0;
}

void taskAdd(scheduler_t *scheduler, task_t *task) {
    /* Get the current UTC time and work out when this task next needs to be called. */
    taskNode_t *current, *node;
    time_t now;
    cron_expr expr;
    const char *err = NULL;

    /* Acquire the scheduler mutex. */
    pthread_mutex_lock(&(scheduler->queueMutex));
    /* Check if there is a task with the same id. */
    if (checkId(scheduler->head, task->id) == 1) {
        logError("task already exists with id: %s", task->id)
        taskFree(task);
        pthread_mutex_unlock(&(scheduler->queueMutex));
        return;
    }
    /* Get the current UTC time.*/
    now = time(NULL);
    memset(&expr, 0, sizeof(expr));
    cron_parse_expr(task->pattern, &expr, &err);
    if (err) {
        logError("Error in CRON expression")
        pthread_mutex_unlock(&(scheduler->queueMutex));
        return;
    }
    task->next = cron_next(&expr, now);

    node = (taskNode_t *) malloc(sizeof(taskNode_t));
    node->task = task;
    /* Check if the queue is empty of the task should be the first in the queue. */
    if (scheduler->head == NULL || ((scheduler->head)->task->next >= node->task->next)) {
        node->next = scheduler->head;
        scheduler->head = node;
    } else {
        current = scheduler->head;
        /* Move to the correct position this task should be added. */
        while (current->next != NULL && (current->next->task->next < node->task->next)) {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
    }
    scheduler->schedulerNTasks += 1;
    pthread_mutex_unlock(&(scheduler->queueMutex));
    pthread_cond_signal(&(scheduler->waitingCond));
}

scheduler_t *schedulerCreate(void) {
    scheduler_t *scheduler;

    if ((scheduler = malloc(sizeof(scheduler_t))) == NULL) {
        return NULL;
    }

    pthread_mutex_init(&scheduler->queueMutex, NULL);
    pthread_cond_init(&scheduler->waitingCond, NULL);
    scheduler->head = NULL;
    scheduler->schedulerNTasks = 0;
    scheduler->schedulerFlags = 0;
    scheduler->currentTaskId = NULL;

    return scheduler;
}

void *schedulerProcess(void *args) {
    scheduler_t *scheduler;
    threadPool_t *threadPool;
    task_t *task;
    struct timespec waitTime;

    scheduler = (scheduler_t *) args;

    threadPool = thrPoolCreate(1, 2, 120, NULL);
    while (1) {
        /* Get the next task to execute. */
        pthread_mutex_lock(&(scheduler->queueMutex));
        if (scheduler->schedulerFlags & SCHEDULER_EXIT) {
            break;
        }
        pthread_mutex_unlock(&(scheduler->queueMutex));
        if ((task = taskPop(scheduler)) == NULL) {
            /* Get the scheduler mutex */
            pthread_mutex_lock(&(scheduler->queueMutex));
            /* Wait until there is data in the queue. */
            timespec_get(&waitTime, TIME_UTC);
            waitTime.tv_sec += 10;
            pthread_cond_timedwait(&(scheduler->waitingCond), &(scheduler->queueMutex), &waitTime);
            pthread_mutex_unlock(&(scheduler->queueMutex));
            continue;
        } else {
            /* Get the scheduler mutex */
            pthread_mutex_lock(&(scheduler->queueMutex));
            /* Set the id of the current task*/
            scheduler->currentTaskId = task->id;
            /* Finished with the mutex so release. */
            pthread_mutex_unlock(&(scheduler->queueMutex));
        }
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
        } else {
            logInfo("Sent a task to the threadpool.")
        }
        /* Add the task back into the queue if it is to be repeated. */
        pthread_mutex_lock(&(scheduler->queueMutex));
        if (scheduler->schedulerFlags & SCHEDULER_DEL_CURRENT) {
            taskFree(task);
            scheduler->schedulerFlags &= ~SCHEDULER_DEL_CURRENT;
            scheduler->currentTaskId = NULL;
        }
        pthread_mutex_unlock(&(scheduler->queueMutex));
        if (task->repeat) {
            taskAdd(scheduler, task);
        } else {
            taskFree(task);
        }
    }
    /* We have been asked to exit. */
    thrPoolWait(threadPool);
    thrPoolDestroy(threadPool);
    pthread_exit(0);
}