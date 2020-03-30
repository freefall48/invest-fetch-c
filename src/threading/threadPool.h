//
// Created by Matthew Johnson on 29/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#ifndef INVEST_FETCH_C_THREADPOOL_H
#define INVEST_FETCH_C_THREADPOOL_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

/*
 * The threadPool_t type is opaque to the client.
 * It is created by thrPoolCreate() and must be passed
 * unmodified to the remainder of the interfaces.
 */
typedef	struct ThreadPool threadPool_t;

/*
 * Create a thread pool.
 *	min_threads:	the minimum number of threads kept in the pool,
 *			always available to perform work requests.
 *	max_threads:	the maximum number of threads that can be
 *			in the pool, performing work requests.
 *	linger:		the number of seconds excess idle worker threads
 *			(greater than min_threads) linger before exiting.
 *	attr:		attributes of all worker threads (can be NULL);
 *			can be destroyed after calling thr_pool_create().
 * On error, thrPoolCreate() returns NULL with errno set to the error code.
 */
threadPool_t *thrPoolCreate(uint16_t min_threads, uint16_t max_threads, uint16_t linger, pthread_attr_t *attr);

/*
 * Enqueue a work request to the thread pool job queue.
 * If there are idle worker threads, awaken one to perform the job.
 * Else if the maximum number of workers has not been reached,
 * create a new worker thread to perform the job.
 * Else just return after adding the job to the queue;
 * an existing worker thread will perform the job when
 * it finishes the job it is currently performing.
 *
 * The job is performed as if a new detached thread were created for it:
 *	pthread_create(NULL, attr, void *(*func)(void *), void *arg);
 *
 * On error, thrPoolQueue() returns -1 with errno set to the error code.
 */
int thrPoolQueue(threadPool_t *pool, void *(*func)(void *), void *arg);

/*
 * Wait for all queued jobs to complete.
 */
void thrPoolWait(threadPool_t *pool);

/*
 * Cancel all queued jobs and destroy the pool.
 */
void thrPoolDestroy(threadPool_t *pool);


#endif //INVEST_FETCH_C_THREADPOOL_H
