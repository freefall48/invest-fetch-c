//
// Created by Matthew Johnson on 12/04/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#include "postgres.h"

#define POSTGRES_URL_ID "INVEST_POSTGRES"


struct pg_conn *postgresConnect(void) {
    struct pg_conn *conn;

    conn = PQconnectdb(getenv(POSTGRES_URL_ID));
    /*
     * This can only happen if there is not enough memory
     * to allocate the PGconn structure.
     */
    if (conn == NULL) {
        fprintf(stderr, "Out of memory connecting to PostgreSQL.\n");
        return NULL;
    }
    /* check if the connection attempt worked */
    if (PQstatus(conn) != CONNECTION_OK) {
        /*
         * Even if the connection failed, the PGconn structure has been
         * allocated and must be freed.
         */
        PQfinish(conn);
        return NULL;
    }
    /* this program expects the database to return data in UTF-8 */
    PQsetClientEncoding(conn, "UTF8");

    return conn;
}