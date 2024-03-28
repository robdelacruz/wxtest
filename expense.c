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
    ExpenseContext *ctx = (ExpenseContext*) malloc(sizeof(ExpenseContext));
    ctx->expfile = str_new(0);
    ctx->expfiledb = NULL;
    ctx->xps = array_new(0);
    ctx->cats = array_new(0);

    ctx->dt = date_new_today();
    date_set_day(ctx->dt, 1);
    ctx->dttmp = date_new_today();
    date_dup(ctx->dttmp, ctx->dt);
    date_set_next_month(ctx->dttmp);

    return ctx;
}
void ctx_free(ExpenseContext *ctx) {
    str_free(ctx->expfile);
    if (ctx->expfiledb)
        sqlite3_close_v2(ctx->expfiledb);
    date_free(ctx->dt);
    date_free(ctx->dttmp);
    array_free(ctx->xps);
    array_free(ctx->cats);
    free(ctx);
}
void ctx_close(ExpenseContext *ctx) {
    str_assign(ctx->expfile, "");
    if (ctx->expfiledb)
        sqlite3_close_v2(ctx->expfiledb);
    ctx->expfiledb = NULL;

    date_assign_today(ctx->dt);
    date_set_day(ctx->dt, 1);
    date_dup(ctx->dttmp, ctx->dt);
    date_set_next_month(ctx->dttmp);

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
void ctx_set_prev_month(ExpenseContext *ctx) {
    date_set_prev_month(ctx->dt);
}
void ctx_set_next_month(ExpenseContext *ctx) {
    date_set_next_month(ctx->dt);
}
void ctx_set_year(ExpenseContext *ctx, int year) {
    if (year >= 1900)
        date_set_year(ctx->dt, year);
}
void ctx_set_month(ExpenseContext *ctx, int month) {
    if (month >= 1 && month <= 12)
        date_set_month(ctx->dt, month);
    date_set_day(ctx->dt, 1);
}

int ctx_refresh_categories(ExpenseContext *ctx) {
    return db_select_cat(ctx->expfiledb, ctx->cats);
}

// Updates the current year and month and queries expenses occuring on the year and month.
// To leave the year/month unchanged, pass -1 or a value outside the valid range.
//
// Ex.
//   ctx_refresh_expenses(ctx, -1, 3)    // set month to March, year is unchanged
//   ctx_refresh_expenses(ctx, 2023, -1) // set year to 2023, month is unchanged
//   ctx_refresh_expenses(ctx, 2023, 3)  // set year/month to 2023 March
//   ctx_refresh_expenses(ctx, 1899, 0)  // outside the valid ranges, year/month unchanged
//
int ctx_refresh_expenses(ExpenseContext *ctx) {
    char min_date[ISO_DATE_LEN+1];
    char max_date[ISO_DATE_LEN+1];

    if (!ctx_is_open_expfile(ctx))
        return 1;

    // Generate date range for min_date <= data < max_date
    // Given year=2024, month=3
    // min_date = "2024-03-01"
    // max_date = "2024-04-01"

    date_dup(ctx->dttmp, ctx->dt);
    date_set_next_month(ctx->dttmp);
    date_to_iso(ctx->dt, min_date, sizeof(min_date));
    date_to_iso(ctx->dttmp, max_date, sizeof(max_date));

    return db_select_exp(ctx->expfiledb, min_date, max_date, ctx->xps);
}

