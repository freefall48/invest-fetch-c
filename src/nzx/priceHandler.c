//
// Created by Matthew Johnson on 27/03/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include "priceHandler.h"

static char *
extractCode(char **ptr) {
    char *code;
    char *end;
    int len;

    *ptr += strlen(NZX_CODE_IDF);
    end = strchr((*ptr) + 1, '"');
    len = (int) (end - (*ptr));
    code = malloc((len + 1) * sizeof(char));
    code[len] = '\0';
    strncpy(code, *ptr, len);

    return code;
}


static listing_t
generatePriceListing(char **ptr) {
    listing_t listing;
    char *end;
    int len;
    /*
     * First extract the the code for this record. Then look for the
     * price data.
     */
    listing.Code = extractCode(ptr);
    /*
     * Find the price and convert to a double
     */
    *ptr = strstr(*ptr, NZX_PRICE_IDF);
    *ptr += strlen(NZX_PRICE_IDF) + 1; // We don't want the dollar sign
    end = strchr(*ptr, ' ');

    listing.Price = strtof(*ptr, &end);

    listing.Company = NULL; // Needs to be null for when the listing is freed.

    return listing;
}

static void
toNbof(const float in, float *out) {
    uint32_t *i = (uint32_t *) &in;
    uint16_t *r = (uint16_t *) out;

    r[0] = htons((uint16_t) ((*i) >> 16u));
    r[1] = htons((uint16_t) *i);
}

int
nzxStoreMarketPrices(nzxNode_t *head) {
    PGconn *conn;
    conn = postgresConnect();

    if (!conn) {
        logCrit("Failed to connect to the Postgres server.")
        return -1;
    }
    /* Request the time from the server, takes the byte order and incorrect time configurations out. */
    PGresult *tm = PQexec(
            conn,
            "SELECT DATE_TRUNC('minute', (now() AT time zone 'Pacific/Auckland') - interval '20 minutes')::timestamp"
    );
    if (tm == NULL) {
        logCrit("Failed to get the time from the postgres server")
        PQfinish(conn);
        return -1;
    } else if (PQresultStatus(tm) != PGRES_TUPLES_OK) {
        logCrit("Postgres did not return a valid time set.")
    }
    if (PQgetisnull(tm, 0, 0)) {
        logCrit("Server send no time data.")
        PQclear(tm);
        PQfinish(conn);
        return -1;
    }
    char *timestamp = PQgetvalue(tm, 0, 0);

    while (head) {
        float converted; // This is now in network byte order
        toNbof(head->listing.Price, &converted);

        const char *const paramValues[3] = {timestamp, head->listing.Code, (char *) &converted};
        int paramLengths[3] = {(int) strlen(timestamp), (int) strlen(head->listing.Code), sizeof(converted)};
        int paramFormats[3] = {0, 0, 1};

        PGresult *res = PQexecParams(
                conn,
                "INSERT INTO nzx.prices (time, code, price) VALUES ($1, $2, $3::real);",
                3,
                NULL,
                paramValues,
                paramLengths,
                paramFormats,
                0);

        if (res == NULL || PQresultStatus(res) != PGRES_COMMAND_OK) {
            logError("Problem is: %s", PQerrorMessage(conn))
        }
        head = head->next;
        PQclear(res);
    }
    PQclear(tm);
    PQfinish(conn);
    return 0;
}

static char *
locateData(memoryChunk_t **chunk) {
    char *ptr;

    // Move the pointer to the start of the data table
    ptr = strstr((*chunk)->memory, NZX_TABLE_IDF);
    // Move the ptr to the first data row
    ptr = strstr(ptr, NZX_CODE_IDF);
    return ptr;
}

void
nzxExtractMarketPrices(memoryChunk_t *chunk, nzxNode_t **head) {
    char *ptr;

    ptr = locateData(&chunk);
    // Check there is actually a table in the HTML downloaded
    if (!ptr) {
        logError("Invalid html. Contains no data.")
        return;
    }

    while (ptr) {
        listing_t listing = generatePriceListing(&ptr);

        nzxPushListing(head, listing);
        // Move the ptr to the next data row if it exists
        ptr = strstr(ptr, NZX_CODE_IDF);
    }

}

static listing_t
generateCoreListing(char **ptr) {
    listing_t listing;
    char *end;
    int len;
    // Extract the listing code
    listing.Code = extractCode(ptr);

    // Find the end of the company name
    *ptr = strstr(*ptr, NZX_COMP_IDF);
    *ptr += strlen(NZX_COMP_IDF);
    end = strchr((*ptr) + 1, '"');
    len = (int) (end - (*ptr));

    listing.Company = malloc((len + 1) * sizeof(char));
    listing.Company[len] = '\0';
    strncpy(listing.Company, *ptr, len);

    return listing;
}

int
nzxStoreMarketListings(nzxNode_t *head) {

    PGconn *conn;
    conn = postgresConnect();

    if (!conn) {
        logCrit("Failed to connect to the Postgres server.")
        return -1;
    }

    while (head) {
        const char *const paramValues[2] = {head->listing.Code, head->listing.Company};

        PGresult *res = PQexecParams(
                conn,
                "INSERT INTO nzx.listings (code, company) VALUES ($1, $2) ON CONFLICT (code) DO NOTHING;",
                2,
                NULL,
                paramValues,
                NULL,
                NULL,
                0);

        if (res == NULL) {
            logError("Problem is: %s", PQerrorMessage(conn))
        }

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            logError("Problem is: %s", PQerrorMessage(conn))
        } else {
            logDebug("Inserted into Postgres successfully.")
        }
        head = head->next;
        PQclear(res);
    }
    PQfinish(conn);
    return 0;
}

void
nzxExtractMarketListings(memoryChunk_t *chunk, nzxNode_t **head) {
    char *ptr;

    ptr = locateData(&chunk);
    // Check there is actually a table in the HTML downloaded
    if (!ptr) {
        logError("Invalid html. Contains no data.")
        return;
    }

    // Loop through all the rows of the table
    while (ptr) {
        listing_t listing = generateCoreListing(&ptr);
        nzxPushListing(head, listing);
        // Move the ptr to the next data row if it exists
        ptr = strstr(ptr, NZX_CODE_IDF);
    }
}

