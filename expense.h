#ifndef EXPENSE_H
#define EXPENSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite3/sqlite3.h"
#include "clib.h"
#include "db.h"

typedef struct {
    str_t *expfile;
    sqlite3 *expfiledb;
    int year, month;
    array_t *xps;
    array_t *cats;
} ExpenseContext;

const char *exp_strerror(int errnum);

ExpenseContext *ctx_new();
void ctx_free(ExpenseContext *ctx);
void ctx_close(ExpenseContext *ctx);

int ctx_create_expense_file(ExpenseContext *ctx, const char *filename);
int ctx_open_expense_file(ExpenseContext *ctx, const char *filename);
int ctx_init_from_args(ExpenseContext *ctx, int argc, char **argv);
int ctx_is_open_expfile(ExpenseContext *ctx);
void ctx_set_date(int year, int month);

int ctx_refresh_categories(ExpenseContext *ctx);
int ctx_refresh_expenses(ExpenseContext *ctx, int year, int month);
int ctx_refresh_expenses_prev_month(ExpenseContext *ctx);
int ctx_refresh_expenses_next_month(ExpenseContext *ctx);
int ctx_expenses_sum_amount(ExpenseContext *ctx, int year, int month, double *sum);

#ifdef __cplusplus
}
#endif

#endif
