//
// Created by Matthew Johnson on 12/04/2020.
// Copyright (c) 2020 LocalNetwork NZ. All rights reserved.
//

#ifndef INVEST_FETCH_C_POSTGRES_H
#define INVEST_FETCH_C_POSTGRES_H

#include <libpq-fe.h>
#include <stdlib.h>

struct pg_conn *postgresConnect(void);

#endif //INVEST_FETCH_C_POSTGRES_H
