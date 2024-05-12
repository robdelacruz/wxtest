#ifndef EXPVIEW_H
#define EXPVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite3/sqlite3.h"
#include "clib.h"
#include "db.h"

typedef struct {
    sqlite3 *db;
    date_t startdt;
    date_t enddt;
    array_t *xps;
} ExpView;

ExpView *expview_new(sqlite3 *db, date_t startdt, date_t enddt);
void expview_free(ExpView *ev);

int expview_refresh_expenses(ExpView *ev);

#ifdef __cplusplus
}
#endif

#endif
