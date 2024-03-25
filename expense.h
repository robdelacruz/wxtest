#ifndef EXPENSE_H
#define EXPENSE_H

#include "sqlite3/sqlite3.h"
#include "clib.h"

typedef struct {
    str_t *expfile;
    sqlite3 *expfiledb;
    int month;
    int year;
    array_t *xps;
    array_t *cats;
} ExpenseContext;

ExpenseContext *ctx_create_expense_file(const char *filename, str_t *err);
ExpenseContext *ctx_open_expense_file(const char *filename, str_t *err);
void ctx_close(ExpenseContext *ctx);

int ctx_refresh_categories(ExpenseContext *ctx);
int ctx_refresh_expenses(ExpenseContext *ctx, int year, int month);

#endif
