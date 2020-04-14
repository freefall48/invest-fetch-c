//
// Created by Matthew Johnson on 13/04/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#ifndef INVEST_FETCH_C_MANAGER_H
#define INVEST_FETCH_C_MANAGER_H

#include <hiredis/hiredis.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#include "../logging/logger.h"
#include "../nzx/priceHandler.h"
#include "../nzx/performance.h"
#include "../scheduler/schedule.h"

void managerStreamTasks(void);

#endif //INVEST_FETCH_C_MANAGER_H
