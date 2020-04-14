//
// Created by Matthew Johnson on 9/04/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include "performance.h"

#define NZX_ACT_IDF "<td><strong>Volume</strong></td>"
#define NZX_PERF_IDF "<td><strong>EPS</strong></td>"

static inline char *locateActData(memoryChunk_t **chunk) {
    return strstr((*chunk)->memory, NZX_ACT_IDF);
}

static void removeAllChars(char *str, char c, int n) {
    char *pr = str, *pw = str;
    int i = 0;
    while (*pr && i < n) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

static void
toNbof(const float in, float *out) {
    uint32_t *i = (uint32_t * ) & in;
    uint16_t *r = (uint16_t *) out;

    r[0] = htons((uint16_t)((*i) >> 16u));
    r[1] = htons((uint16_t) * i);
}

int nzxStoreListingPerformance(nzxPerformanceList_t **head) {
    struct pg_conn *conn;
    PGresult *res;
    float eps, nta, gdy;
    unsigned long si, volume;

    conn = postgresConnect();


    if (!conn) {
        logCrit("Could not connect to Postgres Server.")
        return -1;
    }
    while (*head != NULL) {
        nzxPerform_t *entry = (*head)->node;

        int n = 1;
        if (*(char *) &n == 1) {
            si = __builtin_bswap64(entry->si);
            volume = __builtin_bswap64(entry->volume);
            toNbof(entry->eps, &eps);
            toNbof(entry->nta, &nta);
            toNbof(entry->gdy, &gdy);
        } else {
            si = entry->si;
            volume = entry->volume;
            eps = entry->eps;
            nta = entry->nta;
            gdy = entry->gdy;
        }

        const char *const paramValues[6] = {
                (char *) &(eps),
                (char *) &(nta),
                (char *) &(gdy),
                (char *) &(volume),
                (char *) &(si),
                entry->code
        };
        int paramLengths[6] = {
                sizeof(eps),
                sizeof(nta),
                sizeof(gdy),
                sizeof(volume),
                sizeof(si),
                (int) strlen(entry->code)
        };
        int paramFormats[6] = {1, 1, 1, 1, 1, 0};
        res = PQexecParams(
                conn,
                "UPDATE nzx.listings SET eps=$1, nta=$2, gdy=$3, volume=$4, si=$5, update=false, last_updated=date_trunc('minute', now())::timestamptz WHERE code = $6;",
                6,
                NULL,
                paramValues,
                paramLengths,
                paramFormats,
                0);

        if (res == NULL || PQresultStatus(res) != PGRES_COMMAND_OK) {
            logError("Problem is: %s", PQerrorMessage(conn))
        }
        nzxPerformanceList_t *next = (*head)->next;
        free((*head)->node->code);
        free((*head)->node);
        free(*head);
        (*head) = next;
        PQclear(res);
    }
    PQfinish(conn);
    return 0;
}

int nzxExtractListingPerformance(memoryChunk_t *chunk, nzxPerform_t *node) {
    char *ptr, *end;
    int size;

    ptr = locateActData(&chunk);
    if (!ptr) {
        logError("Does not contain any activity data.")
        return -1; // If it doesnt have this data it cant have any other
    }

    ptr = strstr(ptr, "<td class=\"text-right\">") + sizeof("<td class=\"text-right\">") - 1;
    end = strstr(ptr, "<");
    size = (int) (end - ptr);
    removeAllChars(ptr, ',', size);
    node->volume = strtol(ptr, NULL, 10);

    ptr = strstr(ptr, "<td><strong>EPS</strong></td>");
    ptr = strstr(ptr, "<td class=\"text-right\">") + sizeof("<td class=\"text-right\">") - 1;
    end = strstr(ptr, "<");
    size = (int) (end - ptr);
    removeAllChars(ptr, '$', size);
    node->eps = strtof(ptr, NULL);

    ptr = strstr(ptr, "<td><strong>NTA</strong></td>");
    ptr = strstr(ptr, "<td class=\"text-right\">") + sizeof("<td class=\"text-right\">") - 1;
    end = strstr(ptr, "<");
    size = (int) (end - ptr);
    removeAllChars(ptr, '$', size);
    node->nta = strtof(ptr, NULL);

    ptr = strstr(ptr, "<td><strong>Gross Div Yield</strong></td>");
    ptr = strstr(ptr, "<td class=\"text-right\">") + sizeof("<td class=\"text-right\">") - 1;
    node->gdy = strtof(ptr, NULL);

    ptr = strstr(ptr, "<td><strong>Securities Issued</strong></td>");
    ptr = strstr(ptr, "<td class=\"text-right\">") + sizeof("<td class=\"text-right\">") - 1;
    end = strstr(ptr, "<");
    size = (int) (end - ptr);
    removeAllChars(ptr, ',', size);
    node->si = strtol(ptr, NULL, 10);
    return 0;
}

int nzxGetUpdateCodes(nzxPerformanceList_t **head) {
    struct pg_conn *conn;
    PGresult *res;

    conn = postgresConnect();

    if (!conn) {
        logCrit("Could not connect to Postgres Server.")
        return -1;
    }

    res = PQexec(conn, "select code from nzx.listings order by update DESC, last_updated LIMIT 20;");

    if (res == NULL || PQresultStatus(res) != PGRES_TUPLES_OK) {
        logError("Problem is: %s", PQerrorMessage(conn))
        PQclear(res);
        PQfinish(conn);
        return -1;
    }

    for (int row = 0; row < PQntuples(res); row++) {
        if (!PQgetisnull(res, row, 0)) {

            nzxPerformanceList_t *entry = (nzxPerformanceList_t *) malloc(sizeof(nzxPerformanceList_t));
            entry->node = (nzxPerform_t *) malloc(sizeof(nzxPerform_t));

            char *code = PQgetvalue(res, row, 0);
            size_t len = strlen(code) + 1;
            entry->node->code = (char *) malloc(sizeof(char) * len);
            strncpy(entry->node->code, code, len);

            if (head == NULL) {
                entry->next = NULL;
            } else {
                entry->next = *head;
            }
            *head = entry;
        }
    }
    PQclear(res);
    PQfinish(conn);
    return 0;
}