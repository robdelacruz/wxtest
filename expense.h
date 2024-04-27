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
    int year, month, day;
    array_t *xps;
    array_t *cats;
    array_t *yeartotals;
} ExpenseContext;

const char *exp_strerror(int errnum);

ExpenseContext *ctx_new();
void ctx_free(ExpenseContext *ctx);
void ctx_close(ExpenseContext *ctx);

int ctx_create_expense_file(ExpenseContext *ctx, const char *filename);
int ctx_open_expense_file(ExpenseContext *ctx, const char *filename);
int ctx_init_from_args(ExpenseContext *ctx, int argc, char **argv);
int ctx_is_open_expfile(ExpenseContext *ctx);
void ctx_set_date(ExpenseContext *ctx, int year, int month, int day);
void ctx_set_date_previous_month(ExpenseContext *ctx);
void ctx_set_date_next_month(ExpenseContext *ctx);

int ctx_refresh_categories(ExpenseContext *ctx);
int ctx_refresh_expenses(ExpenseContext *ctx);
int ctx_expenses_subtotal_year(ExpenseContext *ctx, int year, double *sum);
int ctx_expenses_subtotal_month(ExpenseContext *ctx, int year, int month, double *sum);
int ctx_expenses_subtotal_day(ExpenseContext *ctx, int year, int month, int day, double *sum);
int ctx_refresh_yeartotals(ExpenseContext *ctx);

int ctx_delete_category(ExpenseContext *ctx, uint64_t catid);

#ifdef __cplusplus
}
#endif

#endif
