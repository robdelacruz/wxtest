#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "db.h"
#include "expview.h"

#define INIT_NUM_EXPVIEW_EXPENSES 1024

ExpView *expview_new(sqlite3 *db, date_t startdt, date_t enddt) {
    ExpView *ev;

    ev = (ExpView*) malloc(sizeof(ExpView));
    ev->db = db;
    ev->startdt = startdt;
    ev->enddt = enddt;
    ev->xps = arrayexp_new(INIT_NUM_EXPVIEW_EXPENSES);

    return ev;
}
void expview_free(ExpView *ev) {
    arrayexp_free(ev->xps);

    free(ev);
}

int expview_refresh_expenses(ExpView *ev) {
    return db_select_exp(ev->db, ev->startdt, ev->enddt, ev->xps);
}

