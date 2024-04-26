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

ExpenseContext *ctx_new() {
    ExpenseContext *ctx;

    ctx = (ExpenseContext*) malloc(sizeof(ExpenseContext));
    ctx->expfile = str_new(0);
    ctx->expfiledb = NULL;
    ctx->xps = array_new(0);
    ctx->cats = array_new(0);
    date_to_cal(date_today(), &ctx->year, &ctx->month, &ctx->day);

    return ctx;
}
void ctx_free(ExpenseContext *ctx) {
    ctx_close(ctx);
    str_free(ctx->expfile);
    array_free(ctx->xps);
    array_free(ctx->cats);
    free(ctx);
}
void ctx_close(ExpenseContext *ctx) {
    str_assign(ctx->expfile, "");

    if (ctx->expfiledb)
        sqlite3_close_v2(ctx->expfiledb);
    ctx->expfiledb = NULL;

    date_to_cal(date_today(), &ctx->year, &ctx->month, &ctx->day);

    for (int i=0; i < ctx->xps->len; i++)
        exp_free(ctx->xps->items[i]);
    for (int i=0; i < ctx->cats->len; i++)
        cat_free(ctx->cats->items[i]);
    array_clear(ctx->xps);
    array_clear(ctx->cats);
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
    return db_sum_amount_exp(ctx->expfiledb, startdate, enddate, sum);
}
int ctx_expenses_subtotal_month(ExpenseContext *ctx, int year, int month, double *sum) {
    date_t startdate, enddate;

    *sum = 0.0;
    if (!ctx_is_open_expfile(ctx))
        return 1;

    startdate = date_from_cal(year, month, 1);
    enddate = date_next_month(startdate);
    return db_sum_amount_exp(ctx->expfiledb, startdate, enddate, sum);
}
int ctx_expenses_subtotal_day(ExpenseContext *ctx, int year, int month, int day, double *sum) {
    date_t startdate, enddate;

    *sum = 0.0;
    if (!ctx_is_open_expfile(ctx))
        return 1;

    startdate = date_from_cal(year, month, day);
    enddate = date_next_day(startdate);
    return db_sum_amount_exp(ctx->expfiledb, startdate, enddate, sum);
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

