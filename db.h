#ifndef EXP_H
#define EXP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "clib.h"
#include "sqlite3/sqlite3.h"

#define DB_OK               0
#define DB_FILE_EXISTS      201
#define DB_FILE_NOT_FOUND   202
#define DB_MKSTEMP_ERR      203
#define DB_NOT_EXPFILE      204

typedef struct {
    uint64_t catid;
    str_t *name;
} cat_t;

typedef struct {
    uint64_t expid;
    date_t date;
    str_t *desc;
    double amt;
    uint64_t catid;
    str_t *catname;
} exp_t;

typedef struct {
    int year;
    double total;
} yeartotal_t;

cat_t *cat_new();
void cat_free(cat_t *cat);
void cat_dup(cat_t *destcat, cat_t *srccat);
int cat_is_valid(cat_t *cat);

exp_t *exp_new();
void exp_free(exp_t *xp);
void exp_dup(exp_t *dest, exp_t *src);
int exp_is_valid(exp_t *xp);

int file_exists(const char *file);
int create_tmp_expense_file(str_t *retdbfile, sqlite3 **pdb);
int create_expense_file(const char *dbfile, sqlite3 **pdb);
int open_expense_file(const char *dbfile, sqlite3 **pdb);

int db_select_cat(sqlite3 *db, array_t *cats);
int db_find_cat_by_id(sqlite3 *db, uint64_t catid, cat_t *cat);
int db_find_cat_by_name(sqlite3 *db, const char *name, uint64_t *catid);
int db_add_cat(sqlite3 *db, cat_t *cat);
int db_edit_cat(sqlite3 *db, cat_t *cat);
int db_del_cat(sqlite3 *db, uint64_t catid);

int db_select_exp(sqlite3 *db, date_t min_date, date_t max_date, array_t *xps);
int db_find_exp_by_id(sqlite3 *db, uint64_t expid, exp_t *xp);
int db_sum_amount_exp(sqlite3 *db, date_t min_date, date_t max_date, double *sum);
int db_count_exp_with_catid(sqlite3 *db, uint64_t catid, long *count);
int db_get_yeartotals(sqlite3 *db, array_t *yeartotals);

int db_add_exp(sqlite3 *db, exp_t *xp);
int db_edit_exp(sqlite3 *db, exp_t *xp);
int db_del_exp(sqlite3 *db, uint64_t expid);
int db_update_exp_change_catid(sqlite3 *db, uint64_t old_catid, uint64_t new_catid);

#ifdef __cplusplus
}
#endif

#endif
