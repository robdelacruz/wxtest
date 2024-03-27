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

static ExpenseContext *create_empty_ctx() {
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

ExpenseContext *ctx_create_expense_file(const char *filename, str_t *err) {
    int z;
    sqlite3 *expfiledb;
    ExpenseContext *ctx = NULL;

    z = create_expense_file(filename, &expfiledb, err);
    if (z != 0) {
        return NULL;
    }

    ctx = create_empty_ctx();
    str_assign(ctx->expfile, filename);
    ctx->expfiledb = expfiledb;

    if (err != NULL)
        str_assign(err, "");
    return ctx;
}

ExpenseContext *ctx_open_expense_file(const char *filename, str_t *err) {
    int z;
    sqlite3 *expfiledb;
    ExpenseContext *ctx = NULL;

    z = open_expense_file(filename, &expfiledb, err);
    if (z != 0) {
        return NULL;
    }

    ctx = create_empty_ctx();
    str_assign(ctx->expfile, filename);
    ctx->expfiledb = expfiledb;

    if (err != NULL)
        str_assign(err, "");
    return ctx;
}

ExpenseContext *ctx_init_args(int argc, char **argv) {
    const char *expfile = NULL;

    if (argc <= 1)
        return create_empty_ctx();

    for (int i=1; i < argc; i++) {
        char *arg = argv[i];

        // <program> create <expense file>
        if (strcmp(arg, "create") == 0) {
            if (i == argc-1) {
                printf("Usage:\n%s create <expense file>\n", argv[0]);
                return NULL;
            }
            expfile = argv[i+1];
            return ctx_create_expense_file(expfile, NULL);
        }

        // <program> <expense file>
        expfile = arg;
        break;
    }

    assert(expfile != NULL);
    if (!file_exists(expfile)) {
        printf("File '%s' doesn't exist.\n", expfile);
        return NULL;
    }
    return ctx_open_expense_file(expfile, NULL);
}

void ctx_close(ExpenseContext *ctx) {
    str_free(ctx->expfile);
    sqlite3_close_v2(ctx->expfiledb);
    date_free(ctx->dt);
    date_free(ctx->dttmp);
    array_free(ctx->xps);
    array_free(ctx->cats);
    free(ctx);
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
int ctx_refresh_expenses(ExpenseContext *ctx, int year, int month) {
    char min_date[ISO_DATE_LEN+1];
    char max_date[ISO_DATE_LEN+1];

    if (year >= 1900)
        date_set_year(ctx->dt, year);
    if (month >= 1 && month <= 12)
        date_set_month(ctx->dt, month);
    date_set_day(ctx->dt, 1);

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

int ctx_is_open_expfile(ExpenseContext *ctx) {
    if (ctx->expfile->len > 0 && ctx->expfiledb != NULL)
        return 1;
    return 0;
}

