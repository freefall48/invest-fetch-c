//
// Created by Matthew Johnson on 13/04/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include "manager.h"

#define ENV_PORT "REDIS_PORT"
#define ENV_IP "REDIS_IP"
#define REDIS_SUB_COMMAND "SUBSCRIBE investTasks"
#define REDIS_DELIM ","

#define OP_TASK_ADD 1
#define OP_TASK_DEL 2

#define JOB_PRICE 1
#define JOB_LISTINGS 2
#define JOB_PERFORMANCE 3


#define NZX_BOARD_ENV "NZX_BOARD_SRC"
#define NZX_INST_ENV "NZX_INST_SRC"

void *collectListings(void *args) {
    /* Download the market data. */
    char *url = getenv(NZX_BOARD_ENV);
    if (!url) {
        logCrit("ENV %s is not set!", NZX_BOARD_ENV);
        return NULL;
    }
    memoryChunk_t *chunk = nzxFetchData(url);
    nzxNode_t *head = NULL;
    /* Process and store the market listings. */
    nzxExtractMarketListings(chunk, &head);
    nzxStoreMarketListings(head);
    /* Finished with the data so free the memory. */
    nzxDrainListings(&head);
    nzxFreeMemoryChunk(chunk);
    return NULL;
}

void *collectPrices(void *args) {
    /* Download the market data. */
    char *url = getenv(NZX_BOARD_ENV);
    if (!url) {
        logCrit("ENV %s is not set!", NZX_BOARD_ENV);
        return NULL;
    }
    memoryChunk_t *chunk = nzxFetchData(url);
    nzxNode_t *head = NULL;
    /* Process and store the market listings. */
    nzxExtractMarketPrices(chunk, &head);
    nzxStoreMarketPrices(head);
    /* Finished with the data so free the memory. */
    nzxDrainListings(&head);
    nzxFreeMemoryChunk(chunk);
    return NULL;
}

void *collectPerformance(void *args) {
    nzxPerformanceList_t *head = NULL;
    nzxPerformanceList_t *iter;

    char *url = getenv(NZX_INST_ENV);
    if (!url) {
        logCrit("ENV %s is not set!", NZX_INST_ENV);
        return NULL;
    }

    nzxGetUpdateCodes(&head);
    iter = head;
    while (iter != NULL) {

        char buff[strlen(url) + sizeof(iter->node->code) + 1];
        snprintf(buff, sizeof(buff), "%s%s", url, iter->node->code);
        memoryChunk_t *chunk = nzxFetchData(buff);
        nzxExtractListingPerformance(chunk, iter->node);

        nzxFreeMemoryChunk(chunk);
        iter = iter->next;
    }
    nzxStoreListingPerformance(&head);
    return NULL;
}

static void processRedisStream(scheduler_t *scheduler, redisReply *data) {
    task_t *task;
    char *taskOpt, *taskId, *taskRepeat, *taskCron, *taskFunc, *raw;
    int op, job, repeatable;

    /* Extract the header data from the message. */
    raw = data->element[2]->str;
    taskOpt = strtok_r(raw, REDIS_DELIM, &raw);
    taskId = strtok_r(raw, REDIS_DELIM, &raw);
    if (taskOpt == NULL || taskId == NULL) {
        logError("Malformed command packet header")
        return;
    }
    /* Parse the requested operation. */
    op = (int) strtol(taskOpt, NULL, 10);
    /* If the result is 0, test for an error. */
    if (op == 0) {
        /* If a conversion error occurred. */
        if (errno == EINVAL) {
            logError("Could not parse the op header arg: %d", errno);
        } else {
            logError("Requested reserved op code 0")
        }
        return;
    }
    /* Perform the operation requested. */
    switch (op) {
        case OP_TASK_ADD:
            taskCron = strtok_r(raw, REDIS_DELIM, &raw);
            taskRepeat = strtok_r(raw, REDIS_DELIM, &raw);
            taskFunc = strtok_r(raw, REDIS_DELIM, &raw);
            /* Check if there was enough data in the pkt. */
            if (taskCron == NULL || taskRepeat == NULL || taskFunc == NULL) {
                logError("Not enough data to form packet body")
                return;
            }
            /* Parse the requested job. */
            job = (int) strtol(taskFunc, NULL, 10);
            /* If the result is 0, test for an error. */
            if (job == 0) {
                /* If a conversion error occurred. */
                if (errno == EINVAL) {
                    logError("Could not parse the job arg: %d", errno);
                } else {
                    logError("Requested reserved job function 0")
                }
                return;
            }
            /* Parse the repeatability. */
            repeatable = (int) strtol(taskRepeat, NULL, 10);
            /* If the result is 0, test for an error. */
            if (repeatable == 0) {
                /* If a conversion error occurred. */
                if (errno == EINVAL) {
                    logError("Could not parse the repeatable arg: %d", errno);
                    return;
                }
            } else if (repeatable < 0 || repeatable > 1) {
                logError("Invalid repeat option: %d", repeatable)
                return;
            }
            switch (job) {
                case JOB_PRICE:
                    task = taskCreate(taskId, taskCron, repeatable, collectPrices);
                    break;
                case JOB_LISTINGS:
                    task = taskCreate(taskId, taskCron, repeatable, collectListings);
                    break;
                case JOB_PERFORMANCE:
                    task = taskCreate(taskId, taskCron, repeatable, collectPerformance);
                    break;
                default:
                    logError("Unknown task function requested: %d", job);
                    return;
            }
            taskAdd(scheduler, task);
            break;
        case OP_TASK_DEL:
            taskDelete(scheduler, taskId);
            break;
        default:
            logError("Unknown op code: %d", op)
    }
}

static redisContext *connectRedis(void) {
    int port;
    char *ip, *strPort;
    redisContext *context;

    strPort = getenv(ENV_PORT);
    ip = getenv(ENV_IP);
    /* Check that they actually have values. */
    if (!strPort || !ip) {
        logCrit("REDIS ENVs incomplete!")
        return NULL;
    }
    /* Try parse the port to int. */
    port = (int) strtol(strPort, NULL, 10);
    /* If the result is 0, test for an error. */
    if (port == 0) {
        /* If a conversion error occurred. */
        if (errno == EINVAL) {
            logError("Conversion error occurred: %d\n", errno);
            return NULL;
        }
    }
    /* Try connect to redis. */
    context = redisConnect(ip, port);
    if (context == NULL) {
        logCrit("Cannot allocate redis context.")
        return NULL;
    } else if (context->err) {
        logCrit("Error: %s", context->errstr)
        return NULL;
    }
    return context;
}

void managerStreamTasks(void) {
    redisContext *conn;
    redisReply *reply;
    pthread_t t1;
    scheduler_t *scheduler;
    /* Connect to the redis server. */
    conn = connectRedis();
    if (conn == NULL) {
        return;
    }
    /* Create the scheduler. */
    scheduler = schedulerCreate();
    pthread_create(&t1, NULL, schedulerProcess, scheduler);

    reply = redisCommand(conn, REDIS_SUB_COMMAND);
    freeReplyObject(reply);
    while (redisGetReply(conn, (void *) &reply) == REDIS_OK) {
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
            processRedisStream(scheduler, reply);
        }
        freeReplyObject(reply);
    }
}