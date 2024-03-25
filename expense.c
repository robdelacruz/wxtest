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

static void ctx_set_current_date(ExpenseContext *ctx) {
    time_t timenow;
    struct tm tmnow;

    timenow = time(NULL);
    localtime_r(&timenow, &tmnow);
    ctx->month = tmnow.tm_mon+1;
    ctx->year = tmnow.tm_year + 1990;
}

static ExpenseContext *create_empty_ctx() {
    ExpenseContext *ctx = (ExpenseContext*) malloc(sizeof(ExpenseContext));
    ctx->expfile = str_new(0);
    ctx->expfiledb = NULL;
    ctx_set_current_date(ctx);
    ctx->xps = array_new(0);
    ctx->cats = array_new(0);
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
    array_free(ctx->xps);
    array_free(ctx->cats);
    free(ctx);
}

int ctx_refresh_categories(ExpenseContext *ctx) {
    return db_select_cat(ctx->expfiledb, ctx->cats);
}

int ctx_refresh_expenses(ExpenseContext *ctx, int year, int month) {
    char min_date[ISO_DATE_LEN+1];
    char max_date[ISO_DATE_LEN+1];
    struct tm tm;
    int z;

    assert(year >= 1900);
    assert(month >= 1 && month <= 12);

    // Generate date range for min_date <= data < max_date
    // Given year=2024, month=3
    // min_date = "2024-03-01"
    // max_date = "2024-04-01"

    memset(&tm, 0, sizeof(struct tm));
    tm.tm_year = year-1900;
    tm.tm_mon = month-1;
    tm.tm_mday = 1;
    strftime(min_date, sizeof(min_date), "%F", &tm);

    tm.tm_mon++;
    if (tm.tm_mon > 11) {
        tm.tm_mon = 0;
        tm.tm_year++;
    }
    strftime(max_date, sizeof(max_date), "%F", &tm);

    z = db_select_exp(ctx->expfiledb, min_date, max_date, ctx->xps);
    return z;
}

