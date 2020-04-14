//
// Created by Matthew Johnson on 29/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include "threadPool.h"

/*
 * FIFO queued job
 */
typedef struct job {
    struct job *jobNext;        /* linked list of jobs */
    void *(*jobFunc)(void *);    /* function to call */
    void *jobArg;        /* its argument */
} job_t;

/*
 * List of active worker threads, linked through their stacks.
 */
typedef struct active {
    struct active *activeNext;    /* linked list of threads */
    pthread_t activeTid;    /* active thread id */
} active_t;

/*
 * The thread pool, opaque to the clients.
 */
struct threadPool {
    threadPool_t *poolForw;     /* circular linked list */
    threadPool_t *poolBack;     /* of all thread pools */
    pthread_mutex_t poolMutex;  /* protects the pool data */
    pthread_cond_t poolBusycv;  /* synchronization in pool_queue */
    pthread_cond_t poolWorkcv;  /* synchronization with workers */
    pthread_cond_t poolWaitcv;  /* synchronization in pool_wait() */
    active_t *poolActive;       /* list of threads performing work */
    job_t *poolHead;            /* head of FIFO job queue */
    job_t *poolTail;            /* tail of FIFO job queue */
    pthread_attr_t poolAttr;    /* attributes of the workers */
    unsigned int poolFlags;     /* see below */
    unsigned int poolLinger;    /* seconds before idle workers exit */
    int poolMinimum;            /* minimum number of worker threads */
    int poolMaximum;            /* maximum number of worker threads */
    int poolNthreads;           /* current number of worker threads */
    int poolIdle;               /* number of idle workers */
};

/* poolFlags */
#define    POOL_WAIT    0x01u        /* waiting in thrPoolWait() */
#define    POOL_DESTROY    0x02u        /* pool is being destroyed */

/* the list of all created and not yet destroyed thread pools */
static threadPool_t *thrPools = NULL;

/* protects thrPools */
static pthread_mutex_t thrPoolLock = PTHREAD_MUTEX_INITIALIZER;

/* set of all signals */
static sigset_t signalSet;

static void *workerThread(void *);

static int
createWorker(threadPool_t *pool) {
    sigset_t oset;
    int error;
    pthread_t thread; /* Do not need to keep a reference to this thread. */

    pthread_sigmask(SIG_SETMASK, &signalSet, &oset);
    error = pthread_create(&thread, &pool->poolAttr, workerThread, pool);
    pthread_sigmask(SIG_SETMASK, &oset, NULL);
    return error;
}

/*
 * Worker thread is terminating.  Possible reasons:
 * - excess idle thread is terminating because there is no work.
 * - thread was cancelled (pool is being destroyed).
 * - the job function called pthread_exit().
 * In the last case, create another worker thread
 * if necessary to keep the pool populated.
 */
static void
workerCleanup(void *arg) {
    threadPool_t *pool = (threadPool_t *) arg;
    --pool->poolNthreads;
    if (pool->poolFlags & POOL_DESTROY) {
        if (pool->poolNthreads == 0)
            (void) pthread_cond_broadcast(&pool->poolBusycv);
    } else if (pool->poolHead != NULL &&
               pool->poolNthreads < pool->poolMaximum &&
               createWorker(pool) == 0) {
        pool->poolNthreads++;
    }
    pthread_mutex_unlock(&pool->poolMutex);
}

static void
notify_waiters(threadPool_t *pool) {
    if (pool->poolHead == NULL && pool->poolActive == NULL) {
        pool->poolFlags &= ~POOL_WAIT;
        pthread_cond_broadcast(&pool->poolWaitcv);
    }
}

/*
 * Called by a worker thread on return from a job.
 */
static void
jobCleanup(void *arg) {
    pthread_t myTid = pthread_self();
    active_t *activep;
    active_t **activepp;

    threadPool_t *pool = (threadPool_t *) arg;

    pthread_mutex_lock(&pool->poolMutex);
    for (activepp = &pool->poolActive;
         (activep = *activepp) != NULL;
         activepp = &activep->activeNext) {
        if (activep->activeTid == myTid) {
            *activepp = activep->activeNext;
            break;
        }
    }
    if (pool->poolFlags & POOL_WAIT)
        notify_waiters(pool);
}

static void *
workerThread(void *arg) {
    threadPool_t *pool = (threadPool_t *) arg;
    int timedout;
    job_t *job;
    void *(*func)(void *);
    active_t active;
    struct timespec ts;

    /*
     * This is the worker's main loop.  It will only be left
     * if a timeout occurs or if the pool is being destroyed.
     */
    pthread_mutex_lock(&pool->poolMutex);
    pthread_cleanup_push(workerCleanup, pool)
    active.activeTid = pthread_self();
            for (;;) {
                /*
                 * We don't know what this thread was doing during
                 * its last job, so we reset its signal mask and
                 * cancellation state back to the initial values.
                 */
                pthread_sigmask(SIG_SETMASK, &signalSet, NULL);
                pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
                pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

                timedout = 0;
                pool->poolIdle++;
                if (pool->poolFlags & POOL_WAIT)
                    notify_waiters(pool);
                while (pool->poolHead == NULL &&
                       !(pool->poolFlags & POOL_DESTROY)) {
                    if (pool->poolNthreads <= pool->poolMinimum) {
                        (void) pthread_cond_wait(&pool->poolWorkcv, &pool->poolMutex);
                    } else {
                        (void) clock_gettime(CLOCK_REALTIME, &ts);
                        ts.tv_sec += pool->poolLinger;
                        if (pool->poolLinger == 0 ||
                            pthread_cond_timedwait(&pool->poolWorkcv, &pool->poolMutex, &ts) == ETIMEDOUT) {
                            timedout = 1;
                            break;
                        }
                    }
                }
                pool->poolIdle--;
                if (pool->poolFlags & POOL_DESTROY)
                    break;
                if ((job = pool->poolHead) != NULL) {
                    timedout = 0;
                    func = job->jobFunc;
                    arg = job->jobArg;
                    pool->poolHead = job->jobNext;
                    if (job == pool->poolTail)
                        pool->poolTail = NULL;
                    active.activeNext = pool->poolActive;
                    pool->poolActive = &active;
                    pthread_mutex_unlock(&pool->poolMutex);
                    pthread_cleanup_push(jobCleanup, pool)
                    free(job);
                    /*
                     * Call the specified job function.
                     */
                    func(arg);
                    /*
                     * If the job function calls pthread_exit(), the thread
                     * calls jobCleanup(pool) and workerCleanup(pool);
                     * the integrity of the pool is thereby maintained.
                     */
                    pthread_cleanup_pop(1);    /* jobCleanup(pool) */
                }
                if (timedout && pool->poolNthreads > pool->poolMinimum) {
                    /*
                     * We timed out and there is no work to be done
                     * and the number of workers exceeds the minimum.
                     * Exit now to reduce the size of the pool.
                     */
                    break;
                }
            }
    pthread_cleanup_pop(1);    /* workerCleanup(pool) */
    return NULL;
}

static void
cloneAttributes(pthread_attr_t *newAttr, pthread_attr_t *oldAttr) {
    struct sched_param param;
    void *addr;
    size_t size;
    int value;

    pthread_attr_init(newAttr);

    if (oldAttr != NULL) {
        pthread_attr_getstack(oldAttr, &addr, &size);
        /* don't allow a non-NULL thread stack address */
        pthread_attr_setstack(newAttr, NULL, size);

        pthread_attr_getscope(oldAttr, &value);
        pthread_attr_setscope(newAttr, value);

        pthread_attr_getinheritsched(oldAttr, &value);
        pthread_attr_setinheritsched(newAttr, value);

        pthread_attr_getschedpolicy(oldAttr, &value);
        pthread_attr_setschedpolicy(newAttr, value);

        pthread_attr_getschedparam(oldAttr, &param);
        pthread_attr_setschedparam(newAttr, &param);

        pthread_attr_getguardsize(oldAttr, &size);
        pthread_attr_setguardsize(newAttr, size);
    }

    /* make all pool threads be detached threads */
    pthread_attr_setdetachstate(newAttr, PTHREAD_CREATE_DETACHED);
}

threadPool_t *
thrPoolCreate(uint16_t minThreads, uint16_t maxThreads, uint16_t linger, pthread_attr_t *attr) {
    threadPool_t *pool;

    sigfillset(&signalSet);

    if (minThreads > maxThreads || maxThreads < 1) {
        errno = EINVAL;
        return (NULL);
    }

    if ((pool = malloc(sizeof(*pool))) == NULL) {
        errno = ENOMEM;
        return (NULL);
    }
    pthread_mutex_init(&pool->poolMutex, NULL);
    pthread_cond_init(&pool->poolBusycv, NULL);
    pthread_cond_init(&pool->poolWorkcv, NULL);
    pthread_cond_init(&pool->poolWaitcv, NULL);
    pool->poolActive = NULL;
    pool->poolHead = NULL;
    pool->poolTail = NULL;
    pool->poolFlags = 0;
    pool->poolLinger = linger;
    pool->poolMinimum = minThreads;
    pool->poolMaximum = maxThreads;
    pool->poolNthreads = 0;
    pool->poolIdle = 0;

    /*
     * We cannot just copy the attribute pointer.
     * We need to initialize a new pthread_attr_t structure using
     * the values from the caller-supplied attribute structure.
     * If the attribute pointer is NULL, we need to initialize
     * the new pthread_attr_t structure with default values.
     */
    cloneAttributes(&pool->poolAttr, attr);

    /* insert into the global list of all thread pools */
    pthread_mutex_lock(&thrPoolLock);
    if (thrPools == NULL) {
        pool->poolForw = pool;
        pool->poolBack = pool;
        thrPools = pool;
    } else {
        thrPools->poolBack->poolForw = pool;
        pool->poolForw = thrPools;
        pool->poolBack = thrPools->poolBack;
        thrPools->poolBack = pool;
    }
    pthread_mutex_unlock(&thrPoolLock);

    return pool;
}

int
thrPoolQueue(threadPool_t *pool, void *(*func)(void *), void *arg) {
    job_t *job;

    if ((job = malloc(sizeof(*job))) == NULL) {
        errno = ENOMEM;
        return (-1);
    }
    job->jobNext = NULL;
    job->jobFunc = func;
    job->jobArg = arg;

    pthread_mutex_lock(&pool->poolMutex);

    if (pool->poolHead == NULL)
        pool->poolHead = job;
    else
        pool->poolTail->jobNext = job;
    pool->poolTail = job;

    if (pool->poolIdle > 0) {
        pthread_cond_signal(&pool->poolWorkcv);
    } else if (pool->poolNthreads < pool->poolMaximum &&
               createWorker(pool) == 0) {
        pool->poolNthreads++;
    }

    pthread_mutex_unlock(&pool->poolMutex);
    return 0;
}

void
thrPoolWait(threadPool_t *pool) {
    pthread_mutex_lock(&pool->poolMutex);
    pthread_cleanup_push((void *) pthread_mutex_unlock, &pool->poolMutex)
    while (pool->poolHead != NULL || pool->poolActive != NULL) {
        pool->poolFlags |= POOL_WAIT;
        (void) pthread_cond_wait(&pool->poolWaitcv, &pool->poolMutex);
    }
    pthread_cleanup_pop(1);    /* pthread_mutex_unlock(&pool->poolMutex); */
}

void
thrPoolDestroy(threadPool_t *pool) {
    active_t *activep;
    job_t *job;

    pthread_mutex_lock(&pool->poolMutex);
    pthread_cleanup_push((void *) pthread_mutex_unlock, &pool->poolMutex)

    /* mark the pool as being destroyed; wakeup idle workers */
    pool->poolFlags |= POOL_DESTROY;
    pthread_cond_broadcast(&pool->poolWorkcv);

    /* cancel all active workers */
    for (activep = pool->poolActive;
         activep != NULL;
         activep = activep->activeNext)
        pthread_cancel(activep->activeTid);

    /* wait for all active workers to finish */
    while (pool->poolActive != NULL) {
        pool->poolFlags |= POOL_WAIT;
        pthread_cond_wait(&pool->poolWaitcv, &pool->poolMutex);
    }

    /* the last worker to terminate will wake us up */
    while (pool->poolNthreads != 0)
        pthread_cond_wait(&pool->poolBusycv, &pool->poolMutex);

    pthread_cleanup_pop(1);    /* pthread_mutex_unlock(&pool->poolMutex); */

    /*
     * Unlink the pool from the global list of all pools.
     */
    pthread_mutex_lock(&thrPoolLock);
    if (thrPools == pool)
        thrPools = pool->poolForw;
    if (thrPools == pool)
        thrPools = NULL;
    else {
        pool->poolBack->poolForw = pool->poolForw;
        pool->poolForw->poolBack = pool->poolBack;
    }
    pthread_mutex_unlock(&thrPoolLock);

    /*
     * There should be no pending jobs, but just in case...
     */
    for (job = pool->poolHead; job != NULL; job = pool->poolHead) {
        pool->poolHead = job->jobNext;
        free(job);
    }
    pthread_attr_destroy(&pool->poolAttr);
    free(pool);
}
