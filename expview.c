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

ExpView *expview_new(sqlite3 *db, date_t startdt, date_t enddt) {
    ExpView *ev;

    ev = (ExpView*) malloc(sizeof(ExpView));
    ev->db = db;
    ev->startdt = startdt;
    ev->enddt = enddt;

    return ev;
}
void expview_free(ExpView *ev) {
    for (int i=0; i < ev->xps->len; i++)
        exp_free(ev->xps->items[i]);
    array_free(ev->xps);

    free(ev);
}

int expview_refresh_expenses(ExpView *ev) {
    return db_select_exp(ev->db, ev->startdt, ev->enddt, ev->xps);
}

