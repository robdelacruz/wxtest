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
#include "expense.h"

const char *exp_strerror(int errnum) {
    if (errnum == 0)
        return "OK";
    if (errnum > 0 && errnum <= SQLITE_DONE)
        return "sqlite3 error";
    if (errnum == DB_FILE_EXISTS)
        return "File exists";
    if (errnum == DB_FILE_NOT_FOUND)
        return "File not found";
    if (errnum == DB_MKSTEMP_ERR)
        return "Error creating temp file";
    if (errnum == DB_NOT_EXPFILE)
        return "Not an expense file";
    return "Unknown error";
}

#define INIT_NUM_EXPENSES 1024
#define INIT_NUM_CATEGORIES 64
#define INIT_NUM_SUBTOTALS 500
#define INIT_NUM_CATTOTALS 500

ExpenseContext *ctx_new() {
    ExpenseContext *ctx;

    ctx = (ExpenseContext*) malloc(sizeof(ExpenseContext));
    ctx->expfile = str_new(0);
    ctx->expfiledb = NULL;
    ctx->xps = arrayexp_new(INIT_NUM_EXPENSES);
    ctx->cats = arraycat_new(INIT_NUM_CATEGORIES);
    ctx->subtotals = array_new(INIT_NUM_SUBTOTALS);
    ctx->cattotals = array_new(INIT_NUM_CATTOTALS);
    date_to_cal(date_today(), &ctx->year, &ctx->month, &ctx->day);

    return ctx;
}
void ctx_free(ExpenseContext *ctx) {
    ctx_close(ctx);
    str_free(ctx->expfile);

    arrayexp_free(ctx->xps);
    arraycat_free(ctx->cats);
    array_free(ctx->subtotals);
    array_free(ctx->cattotals);

    free(ctx);
}
void ctx_close(ExpenseContext *ctx) {
    str_assign(ctx->expfile, "");

    if (ctx->expfiledb)
        sqlite3_close_v2(ctx->expfiledb);
    ctx->expfiledb = NULL;

    date_to_cal(date_today(), &ctx->year, &ctx->month, &ctx->day);

    arrayexp_clear(ctx->xps);
    arraycat_clear(ctx->cats);
    for (size_t i=0; i < ctx->subtotals->len; i++)
        free(ctx->subtotals->items[i]);
    for (size_t i=0; i < ctx->cattotals->len; i++)
        cattotal_free(ctx->cattotals->items[i]);
    array_clear(ctx->subtotals);
    array_clear(ctx->cattotals);
}

int ctx_create_expense_file(ExpenseContext *ctx, const char *filename) {
    int z;
    sqlite3 *expfiledb;

    z = create_expense_file(filename, &expfiledb);
    if (z != 0)
        return z;

    ctx_close(ctx);
    ctx->expfiledb = expfiledb;
    str_assign(ctx->expfile, filename);

    ctx_refresh_categories(ctx);
    ctx_refresh_expenses(ctx);
    ctx_refresh_subtotals(ctx);
    ctx_refresh_cattotals(ctx);

    return 0;
}

int ctx_open_expense_file(ExpenseContext *ctx, const char *filename) {
    int z;
    sqlite3 *expfiledb;

    z = open_expense_file(filename, &expfiledb);
    if (z != 0)
        return z;

    ctx_close(ctx);
    ctx->expfiledb = expfiledb;
    str_assign(ctx->expfile, filename);

    ctx_refresh_categories(ctx);
    ctx_refresh_expenses(ctx);
    ctx_refresh_subtotals(ctx);
    ctx_refresh_cattotals(ctx);

    return 0;
}

int ctx_init_from_args(ExpenseContext *ctx, int argc, char **argv) {
    const char *expfile = NULL;

    if (argc <= 1)
        return 0;

    for (int i=1; i < argc; i++) {
        char *arg = argv[i];

        // <program> create <expense file>
        if (strcmp(arg, "create") == 0) {
            if (i == argc-1) {
                printf("Usage:\n%s create <expense file>\n", argv[0]);
                return 1;
            }
            expfile = argv[i+1];
            return ctx_create_expense_file(ctx, expfile);
        }

        // <program> <expense file>
        expfile = arg;
        break;
    }

    assert(expfile != NULL);
    if (!file_exists(expfile)) {
        printf("File '%s' doesn't exist.\n", expfile);
        return 1;
    }
    return ctx_open_expense_file(ctx, expfile);
}

int ctx_is_open_expfile(ExpenseContext *ctx) {
    if (ctx->expfile->len > 0 && ctx->expfiledb != NULL)
        return 1;
    return 0;
}

void ctx_set_date(ExpenseContext *ctx, int year, int month, int day) {
    if (year >= 1900)
        ctx->year = year;
    if (month >= 0 && month <= 12)
        ctx->month = month;
    if (day >= 0 && day <= 31)
        ctx->day = day;
}
void ctx_set_date_previous_month(ExpenseContext *ctx) {
    if (ctx->month == 0)
        ctx->month = 1;
    date_t dt = date_from_cal(ctx->year, ctx->month, 1);
    dt = date_prev_month(dt);
    date_to_cal(dt, &ctx->year, &ctx->month, &ctx->day);
}
void ctx_set_date_next_month(ExpenseContext *ctx) {
    if (ctx->month == 0) {
        ctx->month = 1;
        return;
    }
    date_t dt = date_from_cal(ctx->year, ctx->month, 1);
    dt = date_next_month(dt);
    date_to_cal(dt, &ctx->year, &ctx->month, &ctx->day);
}

int ctx_refresh_categories(ExpenseContext *ctx) {
    return db_select_cat(ctx->expfiledb, ctx->cats);
}

int ctx_refresh_expenses(ExpenseContext *ctx) {
    date_t startdate;
    date_t enddate;

    if (!ctx_is_open_expfile(ctx))
        return 1;

    if (ctx->month == 0) {
        // 1 year
        startdate = date_from_cal(ctx->year, 1, 1);
        enddate = date_from_cal(ctx->year+1, 1, 1);
    } else {
        // 1 month
        startdate = date_from_cal(ctx->year, ctx->month, 1);
        enddate = date_next_month(startdate);
    }
    return db_select_exp(ctx->expfiledb, startdate, enddate, ctx->xps);
}

int ctx_expenses_subtotal_year(ExpenseContext *ctx, int year, double *sum) {
    date_t startdate, enddate;

    *sum = 0.0;
    if (!ctx_is_open_expfile(ctx))
        return 1;

    startdate = date_from_cal(year, 1, 1);
    enddate = date_from_cal(year+1, 1, 1);
    return db_subtotal_exp(ctx->expfiledb, startdate, enddate, sum);
}
int ctx_expenses_subtotal_month(ExpenseContext *ctx, int year, int month, double *sum) {
    date_t startdate, enddate;

    *sum = 0.0;
    if (!ctx_is_open_expfile(ctx))
        return 1;

    startdate = date_from_cal(year, month, 1);
    enddate = date_next_month(startdate);
    return db_subtotal_exp(ctx->expfiledb, startdate, enddate, sum);
}
int ctx_expenses_subtotal_day(ExpenseContext *ctx, int year, int month, int day, double *sum) {
    date_t startdate, enddate;

    *sum = 0.0;
    if (!ctx_is_open_expfile(ctx))
        return 1;

    startdate = date_from_cal(year, month, day);
    enddate = date_next_day(startdate);
    return db_subtotal_exp(ctx->expfiledb, startdate, enddate, sum);
}

int ctx_refresh_subtotals(ExpenseContext *ctx) {
    int high_year, low_year;
    subtotal_t *st;
    double total;
    int z;
    sqlite3 *db = ctx->expfiledb;

    for (size_t i=0; i < ctx->subtotals->len; i++)
        free(ctx->subtotals->items[i]);
    array_clear(ctx->subtotals);

    z = db_get_exp_highest_year(db, &high_year);
    if (z != 0)
        return z;
    z = db_get_exp_lowest_year(db, &low_year);
    if (z != 0)
        return z;

    for (int year=high_year; year >= low_year; year--) {
        // Year + 12 months subtotals
        for (int i=0; i <= 12; i++) {
            z = db_subtotal_exp_year_month(db, year, i, &total);
            if (z != 0)
                return z;
            st = (subtotal_t *) malloc(sizeof(subtotal_t));
            st->year = year;
            st->month = i;
            st->total = total;
            array_add(ctx->subtotals, st);
        }
    }

    return 0;
}
int ctx_refresh_subtotals_year_month(ExpenseContext *ctx, int year, int month) {
    int z;
    int istart=-1;
    subtotal_t *st;
    double total;

    assert(month >= 0 && month <= 12);

    for (size_t i=0; i < ctx->subtotals->len; i++) {
        st = (subtotal_t *)ctx->subtotals->items[i];
        if (st->year == year && st->month == 0) {
            istart = i;
            break;
        }
    }
    if (istart == -1) {
        ctx_refresh_subtotals(ctx);
        return 0;
    }
    assert(istart+12 < ctx->subtotals->len);
    if (istart+12 >= ctx->subtotals->len) {
        ctx_refresh_subtotals(ctx);
        return 0;
    }

    // Update year subtotal
    z = db_subtotal_exp_year_month(ctx->expfiledb, year, 0, &total);
    if (z != 0)
        return z;
    st = (subtotal_t *)ctx->subtotals->items[istart];
    assert(st->month == 0);
    st->year = year;
    st->month = 0;
    st->total = total;

    // Update month subtotal
    z = db_subtotal_exp_year_month(ctx->expfiledb, year, month, &total);
    if (z != 0)
        return z;
    st = (subtotal_t *)ctx->subtotals->items[istart+month];
    assert(st->month == month);
    st->year = year;
    st->total = total;

    return 0;
}

int ctx_refresh_cattotals(ExpenseContext *ctx) {
    date_t startdate;
    date_t enddate;

    if (ctx->month == 0) {
        // 1 year
        startdate = date_from_cal(ctx->year, 1, 1);
        enddate = date_from_cal(ctx->year+1, 1, 1);
    } else {
        // 1 month
        startdate = date_from_cal(ctx->year, ctx->month, 1);
        enddate = date_next_month(startdate);
    }
    return db_select_cattotals(ctx->expfiledb, startdate, enddate, ctx->cattotals);
}

int ctx_delete_category(ExpenseContext *ctx, uint64_t catid) {
    int z;
    sqlite3 *db = ctx->expfiledb;

    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    z = db_update_exp_change_catid(db, catid, 0);
    if (z != 0) {
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
        return z;
    }
    z = db_del_cat(db, catid);
    if (z != 0) {
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
        return z;
    }

    sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
    return 0;
}

