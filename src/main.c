//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include <curl/curl.h>
#include "logging/logger.h"
#include "redis/manager.h"

int main() {
    curl_global_init(CURL_GLOBAL_NOTHING);
    loggerInit(0, DEBUG);
    managerStreamTasks();
    curl_global_cleanup();
}