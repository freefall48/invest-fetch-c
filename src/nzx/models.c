//
// Created by matth on 26/03/2020.
//

#include "models.h"

void
nzxPushListing (NZXNode_t** head, listing_t entry)
{
    NZXNode_t* node = (NZXNode_t*) malloc(sizeof(NZXNode_t));

    node->listing = entry;

    // Insert the new node in the correct position of the list
    node->next = *head;
    // Update the head to point to the new start of the list
    *head = node;
}

listing_t*
nzxPopListing (NZXNode_t** head)
{
    if (!*head)
    {
        return NULL;
    }
    else
    {
        NZXNode_t* next = (*head)->next;

        listing_t* listing = (listing_t*) malloc(sizeof(listing_t));
        *listing = (*head)->listing;

        free(*head);
        (*head) = next;

        return listing;
    }
}

int
nzxListingsCount (NZXNode_t* head)
{
    int count = 0;
    while (head) {
        count += 1;
        head = head->next;
    }

    return count;
}

void
nzxFreeListing(listing_t* listing)
{
    free(listing->Company);
    free(listing->Code);

    free(listing);
}
