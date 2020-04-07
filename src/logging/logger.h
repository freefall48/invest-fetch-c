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

#define logDebug(...) loggerLog(DEBUG, __VA_ARGS__);
#define logInfo(...) loggerLog(INFO, __VA_ARGS__);
#define logWarn(...) loggerLog(WARN, __VA_ARGS__);
#define logError(...) loggerLog(ERROR, __VA_ARGS__);
#define logCrit(...) loggerLog(CRIT, __VA_ARGS__);

typedef enum LogLevel {
    FOREACH_LOGLEVEL(GENERATE_LOGGER_ENUM)
} LogLevel;

void loggerLog(LogLevel logLevel, const char *fmt, ...);

void loggerInit(uint8_t silent, uint8_t minimumLevel);

#endif //INVEST_FETCH_C_LOGGER_H
