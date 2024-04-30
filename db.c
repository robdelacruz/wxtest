#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "db.h"
#include "sqlite3/sqlite3.h"

int db_sqlite_errno;

static void db_print_err(sqlite3 *db, const char *sql) {
    fprintf(stderr, "SQL: %s\nError: %s\n", sql, sqlite3_errmsg(db));
}
static void db_handle_err(sqlite3 *db, sqlite3_stmt *stmt, const char *sql) {
    db_print_err(db, sql);
    sqlite3_finalize(stmt);
}
static int prepare_sql(sqlite3 *db, const char *sql, sqlite3_stmt **stmt) {
    return sqlite3_prepare_v2(db, sql, -1, stmt, 0);
}
static int db_is_database_file(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='(any)'";
    z = prepare_sql(db, s, &stmt);
    if (z == SQLITE_NOTADB)
        return 0;
    sqlite3_finalize(stmt);
    return 1;
}
static int db_is_tables_exist(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "SELECT * FROM sqlite_master WHERE type='table' AND name='cat'";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return 0;
    }
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);

    s = "SELECT * FROM sqlite_master WHERE type='table' AND name='exp'";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return 0;
    }
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    return 1;
}
static int db_init_tables(sqlite3 *db) {
    const char *s;
    int z;

    s = "CREATE TABLE IF NOT EXISTS cat (cat_id INTEGER PRIMARY KEY NOT NULL, name TEXT);"
        "CREATE TABLE IF NOT EXISTS exp (exp_id INTEGER PRIMARY KEY NOT NULL, date INTEGER, desc TEXT NOT NULL DEFAULT '', amt REAL NOT NULL DEFAULT 0.0, cat_id INTEGER NOT NULL DEFAULT 1);";
    z = sqlite3_exec(db, s, 0, 0, NULL);
    if (z != 0)
        sqlite3_close_v2(db);
    return z;
}

int file_exists(const char *file) {
    struct stat st;
    if (stat(file, &st) == 0)
        return 1;
    return 0;
}

int create_tmp_expense_file(str_t *retdbfile, sqlite3 **pdb) {
    char fmt[] = "/tmp/expXXXXXX";
    int fd;

    fd = mkstemp(fmt);
    if (fd == -1) {
        return DB_MKSTEMP_ERR;
    }
    close(fd);

    str_assign(retdbfile, fmt);
    return create_expense_file(fmt, pdb);
}

int create_expense_file(const char *dbfile, sqlite3 **pdb) {
    struct stat st;
    sqlite3 *db;
    int z;

    if (stat(dbfile, &st) == 0)
        return DB_FILE_EXISTS;

    z = sqlite3_open(dbfile, pdb);
    db = *pdb;
    if (z != 0) {
        sqlite3_close_v2(db);
        return z;
    }
    z = db_init_tables(db);
    if (z != 0) {
        sqlite3_close_v2(db);
        return z;
    }
    return 0;
}

int open_expense_file(const char *dbfile, sqlite3 **pdb) {
    sqlite3 *db;
    int z;

    if (!file_exists(dbfile))
        return DB_FILE_NOT_FOUND;

    z = sqlite3_open(dbfile, pdb);
    db = *pdb;
    if (z != 0)
        return z;

    if (!db_is_database_file(db)) {
        sqlite3_close_v2(db);
        return DB_NOT_EXPFILE;
    }
    if (!db_is_tables_exist(db)) {
        sqlite3_close_v2(db);
        return DB_NOT_EXPFILE;
    }

    fprintf(stderr, "Opened dbfile '%s'\n", dbfile);
    return 0;
}

cat_t *cat_new() {
    cat_t *cat = (cat_t*) malloc(sizeof(cat_t));
    cat->catid = 0;
    cat->name = str_new(15);
    return cat;
}
void cat_free(cat_t *cat) {
    str_free(cat->name);
    free(cat);
}
void cat_dup(cat_t *dest, cat_t *src) {
    str_assign(dest->name, src->name->s);
}
int cat_is_valid(cat_t *cat) {
    if (cat->catid == 0 || cat->name->len == 0)
        return 0;
    return 1;
}

exp_t *exp_new() {
    exp_t *xp = (exp_t*) malloc(sizeof(exp_t));
    xp->expid = 0;
    xp->date = date_today();
    xp->desc = str_new(0);
    xp->amt = 0.0;
    xp->catid = 0;
    xp->catname = str_new(0);
    return xp;
}
void exp_free(exp_t *xp) {
    str_free(xp->desc);
    str_free(xp->catname);
    free(xp);
}
void exp_dup(exp_t *dest, exp_t *src) {
    dest->expid = src->expid;
    dest->date = src->date;
    str_assign(dest->desc, src->desc->s);
    dest->amt = src->amt;
    dest->catid = src->catid;
    str_assign(dest->catname, src->catname->s);
}
int exp_is_valid(exp_t *xp) {
    if (xp->desc->len == 0)
        return 0;
    return 1;
}

cattotal_t *cattotal_new() {
    cattotal_t *ct = (cattotal_t *) malloc(sizeof(cattotal_t));
    ct->catid = 0;
    ct->catname = str_new(0);
    ct->total = 0.0;
    ct->count = 0;
    return ct;
}
void cattotal_free(cattotal_t *ct) {
    str_free(ct->catname);
    free(ct);
}

int db_find_cat_by_id(sqlite3 *db, uint64_t catid, cat_t *cat) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    cat->catid = 0;
    str_assign(cat->name, "");

    s = "SELECT cat_id, name FROM cat WHERE cat_id=?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_bind_int(stmt, 1, catid);

    z = sqlite3_step(stmt);
    if (z < SQLITE_ROW) {
        db_handle_err(db, stmt, s);
        return z;
    }
    if (z == SQLITE_ROW) {
        cat->catid = sqlite3_column_int64(stmt, 0);
        str_assign(cat->name, (char*)sqlite3_column_text(stmt, 1));
    }

    sqlite3_finalize(stmt);
    return 0;
}
int db_find_cat_by_name(sqlite3 *db, const char *name, uint64_t *catid) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    *catid = 0;

    s = "SELECT cat_id, name FROM cat WHERE name=?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_bind_text(stmt, 1, name, -1, NULL);

    z = sqlite3_step(stmt);
    if (z < SQLITE_ROW) {
        db_handle_err(db, stmt, s);
        return z;
    }
    if (z == SQLITE_ROW) {
        *catid = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return 0;
}
int db_select_cat(sqlite3 *db, array_t *cats) {
    cat_t *cat;
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "SELECT cat_id, name FROM cat ORDER BY name";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }

    for (size_t i=0; i < cats->len; i++)
        cat_free((cat_t *)cats->items[i]);
    array_clear(cats);

    while ((z = sqlite3_step(stmt)) == SQLITE_ROW) {
        cat = cat_new();
        cat->catid = sqlite3_column_int64(stmt, 0);
        str_assign(cat->name, (char*)sqlite3_column_text(stmt, 1));
        array_add(cats, cat);
    }
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);
    return 0;
}
int db_add_cat(sqlite3 *db, cat_t *cat) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "INSERT INTO cat (name) VALUES (?);";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_text(stmt, 1, cat->name->s, -1, NULL);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);

    cat->catid = sqlite3_last_insert_rowid(db);
    return 0;
}
int db_edit_cat(sqlite3 *db, cat_t *cat) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "UPDATE cat SET name = ? WHERE cat_id = ?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_text(stmt, 1, cat->name->s, -1, NULL);
    assert(z == 0);
    z = sqlite3_bind_int(stmt, 2, cat->catid);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);
    return 0;
}
int db_del_cat(sqlite3 *db, uint64_t catid) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "DELETE FROM cat WHERE cat_id = ?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, catid);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int db_select_exp(sqlite3 *db, date_t min_date, date_t max_date, array_t *xps) {
    exp_t *xp;
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "SELECT exp_id, date, desc, amt, exp.cat_id, IFNULL(cat.name, '') "
        "FROM exp "
        "LEFT OUTER JOIN cat ON exp.cat_id = cat.cat_id "
        "WHERE date >= ? AND date < ? "
        "ORDER BY date ";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, min_date);
    assert(z == 0);
    z = sqlite3_bind_int(stmt, 2, max_date);
    assert(z == 0);

    for (size_t i=0; i < xps->len; i++)
        exp_free((exp_t *)xps->items[i]);
    array_clear(xps);

    while ((z = sqlite3_step(stmt)) == SQLITE_ROW) {
        xp = exp_new();
        xp->expid = sqlite3_column_int64(stmt, 0);
        xp->date = sqlite3_column_int(stmt, 1);
        str_assign(xp->desc, (char*)sqlite3_column_text(stmt, 2));
        xp->amt = sqlite3_column_double(stmt, 3);
        xp->catid = sqlite3_column_int64(stmt, 4);
        str_assign(xp->catname, (char*)sqlite3_column_text(stmt, 5));

        array_add(xps, xp);
    }
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);
    return 0;
}
int db_find_exp_by_id(sqlite3 *db, uint64_t expid, exp_t *xp) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "SELECT exp_id, date, desc, amt, exp.cat_id, IFNULL(cat.name, '') "
        "FROM exp "
        "LEFT OUTER JOIN cat ON exp.cat_id = cat.cat_id "
        "WHERE exp_id = ?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, expid);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z < SQLITE_ROW) {
        db_handle_err(db, stmt, s);
        return z;
    }
    if (z == SQLITE_ROW) {
        xp->expid = sqlite3_column_int64(stmt, 0);
        xp->date = sqlite3_column_int(stmt, 1);
        str_assign(xp->desc, (char*)sqlite3_column_text(stmt, 2));
        xp->amt = sqlite3_column_double(stmt, 3);
        xp->catid = sqlite3_column_int64(stmt, 4);
        str_assign(xp->catname, (char*)sqlite3_column_text(stmt, 5));
    }

    sqlite3_finalize(stmt);
    return 0;
}
int db_subtotal_exp(sqlite3 *db, date_t min_date, date_t max_date, double *sum) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    *sum = 0.0;

    s = "SELECT SUM(amt) "
        "FROM exp "
        "WHERE date >= ? AND date < ?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, min_date);
    assert(z == 0);
    z = sqlite3_bind_int(stmt, 2, max_date);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z < SQLITE_ROW) {
        db_handle_err(db, stmt, s);
        return z;
    }
    if (z == SQLITE_ROW) {
        *sum = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return 0;
}
// Pass month=0 to get subtotal for entire year
int db_subtotal_exp_year_month(sqlite3 *db, int year, int month, double *total) {
    date_t startdt, enddt;

    assert(month >= 0 && month <= 12);

    if (month == 0) {
        startdt = date_from_cal(year, 1, 1);
        enddt = date_from_cal(year+1, 1, 1);
    } else {
        startdt = date_from_cal(year, month, 1);
        enddt = date_next_month(startdt);
    }

    *total = 0;
    return db_subtotal_exp(db, startdt, enddt, total);
}

int db_count_exp_with_catid(sqlite3 *db, uint64_t catid, long *count) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    *count = 0;

    s = "SELECT COUNT(*) "
        "FROM exp "
        "WHERE cat_id = ?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, catid);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z < SQLITE_ROW) {
        db_handle_err(db, stmt, s);
        return z;
    }
    if (z == SQLITE_ROW) {
        *count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return 0;
}

int db_get_exp_years(sqlite3 *db, int *pyears, size_t size_years, int *ret_nyears) {
    sqlite3_stmt *stmt;
    const char *s;
    int nyears=0;
    int highestyear = 10000;
    date_t row_dt;
    int row_year;
    int z;

    *ret_nyears = 0;

    s = "SELECT date FROM exp ORDER BY date DESC";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    while ((z = sqlite3_step(stmt)) == SQLITE_ROW) {
        row_dt = sqlite3_column_int(stmt, 0);
        date_to_cal(row_dt, &row_year, NULL, NULL);
        if (row_year >= highestyear)
            continue;

        highestyear = row_year;
        *pyears = row_year;
        pyears++;
        nyears++;
        if (nyears >= size_years)
            break;
    }
    if (nyears < size_years && z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);

    *ret_nyears = nyears;
    return 0;
}
int db_get_exp_highest_year(sqlite3 *db, int *ret_year) {
    sqlite3_stmt *stmt;
    const char *s;
    date_t row_dt;
    int row_year;
    int z;

    *ret_year = 0;

    s = "SELECT IFNULL(MAX(date), 0) FROM exp";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_step(stmt);
    if (z != SQLITE_ROW) {
        db_handle_err(db, stmt, s);
        return z;
    }
    row_dt = sqlite3_column_int(stmt, 0);
    date_to_cal(row_dt, &row_year, NULL, NULL);
    *ret_year = row_year;

    sqlite3_finalize(stmt);
    return 0;
}
int db_get_exp_lowest_year(sqlite3 *db, int *ret_year) {
    sqlite3_stmt *stmt;
    const char *s;
    date_t row_dt;
    int row_year;
    int z;

    *ret_year = 0;

    s = "SELECT IFNULL(MIN(date), 0) FROM exp";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_step(stmt);
    if (z != SQLITE_ROW) {
        db_handle_err(db, stmt, s);
        return z;
    }
    row_dt = sqlite3_column_int(stmt, 0);
    date_to_cal(row_dt, &row_year, NULL, NULL);
    *ret_year = row_year;

    sqlite3_finalize(stmt);
    return 0;
}

int db_select_cattotals(sqlite3 *db, date_t min_date, date_t max_date, array_t *cattotals) {
    cattotal_t *ct;
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "SELECT exp.cat_id, IFNULL(cat.name, ''), SUM(exp.amt) AS total, COUNT(exp.exp_id) AS count "
        "FROM exp "
        "LEFT OUTER JOIN cat ON exp.cat_id = cat.cat_id "
        "WHERE date >= ? AND date < ? "
        "GROUP BY exp.cat_id "
        "ORDER BY total DESC ";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, min_date);
    assert(z == 0);
    z = sqlite3_bind_int(stmt, 2, max_date);
    assert(z == 0);

    for (size_t i=0; i < cattotals->len; i++)
        cattotal_free((cattotal_t *) cattotals->items[i]);
    array_clear(cattotals);

    while ((z = sqlite3_step(stmt)) == SQLITE_ROW) {
        ct = cattotal_new();
        ct->catid = sqlite3_column_int64(stmt, 0);
        str_assign(ct->catname, (char*)sqlite3_column_text(stmt, 1));
        ct->total = sqlite3_column_double(stmt, 2);
        ct->count = sqlite3_column_int(stmt, 3);
        array_add(cattotals, ct);
    }
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int db_add_exp(sqlite3 *db, exp_t *xp) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "INSERT INTO exp (date, desc, amt, cat_id) VALUES (?, ?, ?, ?);";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, xp->date);
    assert(z == 0);
    z = sqlite3_bind_text(stmt, 2, xp->desc->s, -1, NULL);
    assert(z == 0);
    z = sqlite3_bind_double(stmt, 3, xp->amt);
    assert(z == 0);
    z = sqlite3_bind_int(stmt, 4, xp->catid);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);

    xp->expid = sqlite3_last_insert_rowid(db);
    return 0;
}
int db_edit_exp(sqlite3 *db, exp_t *xp) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "UPDATE exp SET date = ?, desc = ?, amt = ?, cat_id = ? WHERE exp_id = ?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, xp->date);
    assert(z == 0);
    z = sqlite3_bind_text(stmt, 2, xp->desc->s, -1, NULL);
    assert(z == 0);
    z = sqlite3_bind_double(stmt, 3, xp->amt);
    assert(z == 0);
    z = sqlite3_bind_int(stmt, 4, xp->catid);
    assert(z == 0);
    z = sqlite3_bind_int(stmt, 5, xp->expid);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);
    return 0;
}
int db_del_exp(sqlite3 *db, uint64_t expid) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "DELETE FROM exp WHERE exp_id = ?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, expid);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int db_update_exp_change_catid(sqlite3 *db, uint64_t old_catid, uint64_t new_catid) {
    sqlite3_stmt *stmt;
    const char *s;
    int z;

    s = "UPDATE exp SET cat_id = ? WHERE cat_id = ?";
    z = prepare_sql(db, s, &stmt);
    if (z != 0) {
        db_handle_err(db, stmt, s);
        return z;
    }
    z = sqlite3_bind_int(stmt, 1, new_catid);
    assert(z == 0);
    z = sqlite3_bind_int(stmt, 2, old_catid);
    assert(z == 0);

    z = sqlite3_step(stmt);
    if (z != SQLITE_DONE) {
        db_handle_err(db, stmt, s);
        return z;
    }
    sqlite3_finalize(stmt);
    return 0;
}

