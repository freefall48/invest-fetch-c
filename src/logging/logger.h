//
// Created by Matthew Johnson on 30/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#ifndef INVEST_FETCH_C_LOGGER_H
#define INVEST_FETCH_C_LOGGER_H

#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define FOREACH_LOGLEVEL(LEVEL) \
        LEVEL(DEBUG)    \
        LEVEL(INFO)     \
        LEVEL(WARN)  \
        LEVEL(ERROR)    \
        LEVEL(CRIT) \

#define GENERATE_LOGGER_ENUM(ENUM) ENUM,
#define GENERATE_LOGGER_STRING(STRING) #STRING,

#define logDebug(LOGGER, ...) loggerLog(LOGGER, DEBUG, __VA_ARGS__);
#define logInfo(LOGGER, ...) loggerLog(LOGGER, INFO, __VA_ARGS__);
#define logWarn(LOGGER, ...) loggerLog(LOGGER, WARN, __VA_ARGS__);
#define logError(LOGGER, ...) loggerLog(LOGGER, ERROR, __VA_ARGS__);
#define logCrit(LOGGER, ...) loggerLog(LOGGER, CRIT, __VA_ARGS__);

typedef enum LogLevel {
    FOREACH_LOGLEVEL(GENERATE_LOGGER_ENUM)
} LogLevel;

/*
 * The logger_t type is opaque to the client.
 * It is created by createLogger() and must be passed
 * unmodified to the remainder of the interfaces.
 */
typedef struct Logger logger_t;


void loggerLog(logger_t *logger, LogLevel logLevel, const char *fmt, ...);

logger_t *createLogger(uint8_t silent, uint8_t minimumLevel);

#endif //INVEST_FETCH_C_LOGGER_H
