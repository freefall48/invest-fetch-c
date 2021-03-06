//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//


#ifndef INVEST_FETCH_C_MODELS_H
#define INVEST_FETCH_C_MODELS_H

#include <stdio.h>
#include <stdlib.h>
#include "../logging/logger.h"

typedef struct Listing {
    // Company Markers
    char *Code;
    char *Company;
    // Changers
    float Price;
    unsigned int Volume;
    unsigned int Value;
    unsigned int Capitalization;
    unsigned int TradeCount;

} listing_t;

typedef struct NZXNode {
    listing_t listing;
    struct NZXNode *next;
} nzxNode_t;

void nzxDrainListings(nzxNode_t **head);

void nzxPushListing(nzxNode_t **head, listing_t entry);

listing_t *
nzxPopListing(nzxNode_t **head);

int
nzxListingsCount(nzxNode_t *head);

void
nzxFreeListing(listing_t *listing);

#endif //INVEST_FETCH_C_MODELS_H
