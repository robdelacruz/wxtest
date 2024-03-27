#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

#include "clib.h"

void quit(const char *s) {
    if (s)
        printf("%s\n", s);
    exit(0);
}
void print_error(const char *s) {
    if (s)
        fprintf(stderr, "%s: %s\n", s, strerror(errno));
    else
        fprintf(stderr, "%s\n", strerror(errno));
}
void panic(const char *s) {
    if (s)
        fprintf(stderr, "%s\n", s);
    abort();
}
void panic_err(const char *s) {
    if (s)
        fprintf(stderr, "%s: %s\n", s, strerror(errno));
    abort();
}

date_t *date_new(time_t t) {
    date_t *dt = (date_t*) malloc(sizeof(date_t));

    dt->time = t;
    localtime_r(&dt->time, &dt->tm);
    return dt;
}
date_t *date_new_today() {
    date_t *dt = (date_t*) malloc(sizeof(date_t));

    dt->time = time(NULL);
    localtime_r(&dt->time, &dt->tm);
    return dt;
}
date_t *date_new_cal(uint year, uint month, uint day) {
    date_t *dt = (date_t*) malloc(sizeof(date_t));

    if (date_assign_cal(dt, year, month, day) != 0) {
        dt->time = time(NULL);
        localtime_r(&dt->time, &dt->tm);
    }
    return dt;
}
date_t *date_new_iso(char *isodate) {
    date_t *dt = (date_t*) malloc(sizeof(date_t));

    if (date_assign_iso(dt, isodate) != 0) {
        dt->time = time(NULL);
        localtime_r(&dt->time, &dt->tm);
    }
    return dt;
}
void date_free(date_t *dt) {
    free(dt);
}
void date_assign_time(date_t *dt, time_t time) {
    dt->time = time;
    localtime_r(&dt->time, &dt->tm);
}
int date_assign_cal(date_t *dt, uint year, uint month, uint day) {
    time_t time;
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    tm.tm_year = year - 1900;
    tm.tm_mon = month-1;
    tm.tm_mday = day;
    time = mktime(&tm);
    if (time == -1) {
        fprintf(stderr, "date_assign(%d, %d, %d) mktime() error\n", year, month, day);
        return 1;
    }

    dt->time = time;
    dt->tm = tm;
    return 0;
}
int date_assign_iso(date_t *dt, char *isodate) {
    time_t time;
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    if (strptime(isodate, "%F", &tm) == NULL) {
        fprintf(stderr, "date_assign_iso('%s') strptime() error\n", isodate);
        return errno;
    }
    time = mktime(&tm);
    if (time == -1) {
        fprintf(stderr, "date_assign_iso('%s') mktime() error\n", isodate);
        return errno;
    }

    dt->time = time;
    dt->tm = tm;
    return 0;
}
void date_to_iso(date_t *dt, char *buf, size_t buf_len) {
    strftime(buf, buf_len, "%F", &dt->tm);
}
void date_strftime(date_t *dt, char *fmt, char *buf, size_t buf_len) {
    strftime(buf, buf_len, fmt, &dt->tm);
}
void date_dup(date_t *dest, date_t *src) {
    dest->time = src->time;
    dest->tm = src->tm;
}
time_t date_time(date_t *dt) {
    return dt->time;
}
int date_year(date_t *dt) {
    return dt->tm.tm_year + 1900;
}
int date_month(date_t *dt) {
    return dt->tm.tm_mon+1;
}
int date_day(date_t *dt) {
    return dt->tm.tm_mday;
}
void date_set_prev_month(date_t *dt) {
    if (dt->tm.tm_year == 0)
        return;

    dt->tm.tm_mon--;
    if (dt->tm.tm_mon < 0) {
        dt->tm.tm_mon = 11;
        dt->tm.tm_year--;
    }
}
void date_set_next_month(date_t *dt) {
    dt->tm.tm_mon++;
    if (dt->tm.tm_mon > 11) {
        dt->tm.tm_mon = 0;
        dt->tm.tm_year++;
    }
}
void date_set_year(date_t *dt, int year) {
    date_assign_cal(dt, year, date_month(dt), date_day(dt));
}
void date_set_month(date_t *dt, int month) {
    date_assign_cal(dt, date_year(dt), month, date_day(dt));
}
void date_set_day(date_t *dt, int day) {
    date_assign_cal(dt, date_year(dt), date_month(dt), day);
}

arena_t new_arena(uint64_t cap) {
    arena_t a; 

    if (cap == 0)
        cap = SIZE_MEDIUM;

    a.base = malloc(cap);
    if (!a.base)
        panic("Not enough memory to initialize arena");

    a.pos = 0;
    a.cap = cap;
    return a;
}

void free_arena(arena_t a) {
    free(a.base);
}

void *arena_alloc(arena_t *a, uint64_t size) {
    if (a->pos + size > a->cap)
        panic("arena_alloc() not enough memory");

    char *p = (char*)a->base + a->pos;
    a->pos += size;
    return (void*) p;
}

void arena_reset(arena_t *a) {
    a->pos = 0;
}

str_t *str_new(size_t cap) {
    str_t *str;

    if (cap == 0)
        cap = SIZE_SMALL;

    str = (str_t*) malloc(sizeof(str_t));
    str->s = (char*) malloc(cap);
    memset(str->s, 0, cap);
    str->len = 0;
    str->cap = cap;

    return str;
}
void str_free(str_t *str) {
    memset(str->s, 0, str->cap);
    free(str->s);
    free(str);
}
void str_assign(str_t *str, const char *s) {
    size_t s_len = strlen(s);
    if (s_len+1 > str->cap) {
        str->cap *= 2;
        str->s = (char*) malloc(str->cap);
    }

    strncpy(str->s, s, s_len);
    str->s[s_len] = 0;
    str->len = s_len;
}
void str_sprintf(str_t *str, const char *fmt, ...) {
    char *p = NULL;
    va_list args;

    va_start(args, fmt);
    if (vasprintf(&p, fmt, args) == -1)
        panic("vasprintf() out of memory");
    va_end(args);

    str_assign(str, p);
    free(p);
}

array_t *array_new(size_t cap) {
    if (cap == 0)
        cap = 8;
    array_t *a = (array_t*) malloc(sizeof(array_t));
    a->items = (void**) malloc(sizeof(a->items[0]) * cap);
    a->len = 0;
    a->cap = cap;
    return a;
}
void array_free(array_t *a) {
    memset(a->items, 0, a->cap);
    free(a->items);
    free(a);
}
void array_assign(array_t *a, void **items, size_t len, size_t cap) {
    a->items = items;
    a->len = len;
    a->cap = cap;
}
void array_clear(array_t *a) {
    memset(a->items, 0, a->cap);
    a->len = 0;
}
void array_resize(array_t *a, size_t newcap) {
    assert(newcap > a->cap);
    void **p = (void**) realloc(a->items, newcap * sizeof(void*)); 
    if (p == NULL)
        panic("array_realloc() out of memory\n");
    a->items = p;
    a->cap = newcap;
}
void array_add(array_t *a, void *p) {
    if (a->len >= a->cap)
        array_resize(a, a->cap * 2);
    a->items[a->len] = p;
    a->len++;
}
void array_del(array_t *a, uint idx) {
    for (size_t i=idx; i < a->len-1; i++) {
        a->items[i] = a->items[i+1];
    }
    a->len--;
}

// sort_array() implementation functions
static void swap_array(void *array[], int i, int j) {
    void *tmp = array[i];
    array[i] = array[j];
    array[j] = tmp;
}
static int sort_array_partition(void *array[], int start, int end, sort_compare_func_t cmp) {
    int imid = start;
    void *pivot = array[end];

    for (int i=start; i < end; i++) {
        if (cmp(array[i], pivot) < 0) {
            swap_array(array, imid, i);
            imid++;
        }
    }
    swap_array(array, imid, end);
    return imid;
}
static void sort_array_part(void *array[], int start, int end, sort_compare_func_t cmp) {
    if (start >= end)
        return;

    int pivot = sort_array_partition(array, start, end, cmp);
    sort_array_part(array, start, pivot-1, cmp);
    sort_array_part(array, pivot+1, end, cmp);
}
void sort_array(void *array[], size_t array_len, sort_compare_func_t cmpfunc) {
    sort_array_part(array, 0, array_len-1, cmpfunc);
}

