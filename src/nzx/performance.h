//
// Created by Matthew Johnson on 9/04/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#ifndef INVEST_FETCH_C_PERFORMANCE_H
#define INVEST_FETCH_C_PERFORMANCE_H

#include "httpOps.h"
#include "../helpers/postgres.h"
#include <libpq-fe.h>
#include <string.h>
#include <netinet/in.h>

typedef struct nzxListingPerformance nzxPerform_t;
typedef struct nzxPerformanceList nzxPerformanceList_t;

struct nzxPerformanceList {
    nzxPerform_t *node;
    nzxPerformanceList_t *next;
};

struct nzxListingPerformance {
    char *code;             // Code of the company
    float eps;              // Earnings per share
    float nta;              // Net tangible assets per share
    float gdy;              // Gross dividend yield
    long si;                // Securities issued.
    long volume;            // Number of shares traded
};

int nzxGetUpdateCodes(nzxPerformanceList_t **head);

int nzxExtractListingPerformance(memoryChunk_t *chunk, nzxPerform_t *node);

int nzxStoreListingPerformance(nzxPerformanceList_t **head);

#endif //INVEST_FETCH_C_PERFORMANCE_H
