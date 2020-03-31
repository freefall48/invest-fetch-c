//
// Created by Matthew Johnson on 30/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include "logger.h"

static const char *LOGLEVEL_STRING[] = {
        FOREACH_LOGLEVEL(GENERATE_LOGGER_STRING)
};

struct Logger {
    pthread_mutex_t logger_mutex; /* Protect the logger output */
    uint8_t silent;
    uint8_t minimumLevel;
};

logger_t *
createLogger(uint8_t silent, uint8_t minimumLevel) {
    logger_t *logger = (logger_t *) malloc(sizeof(logger_t));

    logger->minimumLevel = minimumLevel;
    logger->silent = silent;

    pthread_mutex_init(&logger->logger_mutex, NULL);

    return logger;
}

void
loggerLog(logger_t *logger, LogLevel logLevel, const char *fmt, ...) {
    pthread_mutex_lock(&logger->logger_mutex);
    /* Check if this message should be logged */
    if (logLevel < logger->minimumLevel) {
        return;
    }
    /* Get the current UTC time */
    time_t now = time(NULL);
    struct tm *ptm = gmtime(&now);

    char buffer[80];
    /* Produce the date/time in a format that is easy to read in log files etc. */
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", ptm);
    if (!logger->silent) {
        fprintf(stderr, "[%s] - %-5.5s - ", buffer, LOGLEVEL_STRING[logLevel]);

        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);

        fprintf(stderr, "\n");
        fflush(stderr);
    }

    pthread_mutex_unlock(&logger->logger_mutex);
}