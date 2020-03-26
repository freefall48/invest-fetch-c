//
// Created by matth on 26/03/2020.
//

#ifndef INVEST_FETCH_C_HANDLER_H
#define INVEST_FETCH_C_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <libpq-fe.h>

#include "models.h"
#include "httpOps.h"

#define NZX_TABLE_IDF "<tbody>"

#define NZX_CODE_IDF "<tr class=\"\" title=\""
#define NZX_COMP_IDF "<a title=\""
//Need a better way to define this one?
#define NZX_PRICE_IDF "<td class=\"text-right\" data-title=\"Price\">\n      "

#define NZX_POSTGRES_URL "postgresql://postgres:password@localhost/invest"


void nzxExtractMarketListings(memoryChunk_t* chunk, NZXNode_t** head);

void nzxExtractMarketPrices(memoryChunk_t* chunk, NZXNode_t** head);

int nzxStoreMarketListings(NZXNode_t* head);

int nzxStoreMarketPrices(NZXNode_t *head);

#endif //INVEST_FETCH_C_HANDLER_H
