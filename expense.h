#ifndef EXPENSE_H
#define EXPENSE_H

#include "sqlite3/sqlite3.h"
#include "clib.h"

typedef struct {
    str_t *expfile;
    sqlite3 *expfiledb;
    date_t *dt;
    date_t *dttmp;
    array_t *xps;
    array_t *cats;
} ExpenseContext;

ExpenseContext *ctx_create_expense_file(const char *filename, str_t *err);
ExpenseContext *ctx_open_expense_file(const char *filename, str_t *err);
ExpenseContext *ctx_init_args(int argc, char **argv);
void ctx_close(ExpenseContext *ctx);

int ctx_is_open_expfile(ExpenseContext *ctx);
void ctx_set_prev_month(ExpenseContext *ctx);
void ctx_set_next_month(ExpenseContext *ctx);
void ctx_set_year(ExpenseContext *ctx, int year);
void ctx_set_month(ExpenseContext *ctx, int month);

int ctx_refresh_categories(ExpenseContext *ctx);
int ctx_refresh_expenses(ExpenseContext *ctx);

#endif
