//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//


#ifndef INVEST_FETCH_C_PRICEHANDLER_H
#define INVEST_FETCH_C_PRICEHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <libpq-fe.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "../helpers/postgres.h"


#include "models.h"
#include "httpOps.h"

#define NZX_TABLE_IDF "<tbody>"

#define NZX_CODE_IDF "<tr class=\"\" title=\""
#define NZX_COMP_IDF "<a title=\""
//Need a better way to define this one?
#define NZX_PRICE_IDF "<td class=\"text-right\" data-title=\"Price\">\n      "


void nzxExtractMarketPrices(memoryChunk_t *chunk, nzxNode_t **head);

int nzxStoreMarketPrices(nzxNode_t *head);

void nzxExtractMarketListings(memoryChunk_t *chunk, nzxNode_t **head);

int nzxStoreMarketListings(nzxNode_t *head);


#endif //INVEST_FETCH_C_PRICEHANDLER_H
