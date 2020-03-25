#ifndef NZXLIST_H
#define NZXLIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Listing {
  // Company Markers
  char *Code;
  char *Company;
  // Changers
  unsigned int Price;
  unsigned int Volume;
  unsigned int Value;
  unsigned int Capilisation;
  unsigned int TradeCount;

} listing_t;

typedef struct NZXNode {
  listing_t listing;
  struct NZXNode* next;
} NZXNode_t;


void 
pushListing (NZXNode_t** head, listing_t entry);

listing_t*
popListing (NZXNode_t** head);

void
freeListing(listing_t* listing);


#endif