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

date_t date_today() {
    return time(NULL);
}
date_t date_from_cal(uint year, uint month, uint day) {
    time_t t;
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    tm.tm_year = year - 1900;
    tm.tm_mon = month-1;
    tm.tm_mday = day;
    t = mktime(&tm);
    if (t == -1) {
        fprintf(stderr, "date_from_cal(%d, %d, %d) mktime() error\n", year, month, day);
        return 0;
    }
    return t;
}
date_t date_from_iso(char *isodate) {
    time_t t;
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    if (strptime(isodate, "%F", &tm) == NULL) {
        fprintf(stderr, "date_from_iso('%s') strptime() error\n", isodate);
        return 0;
    }
    t = mktime(&tm);
    if (t == -1) {
        fprintf(stderr, "date_assign_iso('%s') mktime() error\n", isodate);
        return 0;
    }
    return t;
}
void date_strftime(date_t dt, const char *fmt, char *buf, size_t buf_len) {
    struct tm tm;
    localtime_r(&dt, &tm);
    strftime(buf, buf_len, fmt, &tm);
}
void date_to_iso(date_t dt, char *buf, size_t buf_len) {
    struct tm tm;
    localtime_r(&dt, &tm);
    strftime(buf, buf_len, "%F", &tm);
}
void date_to_cal(date_t dt, int *retyear, int *retmonth, int *retday) {
    struct tm tm;
    localtime_r(&dt, &tm);
    if (retyear)
        *retyear = tm.tm_year + 1900;
    if (retmonth)
        *retmonth = tm.tm_mon+1;
    if (retday)
        *retday = tm.tm_mday;
}
date_t date_prev_month(date_t dt) {
    int year, month, day;
    date_to_cal(dt, &year, &month, &day);

    if (year == 0)
        return dt;

    if (month == 1)
        return date_from_cal(year-1, 12, day);
    else
        return date_from_cal(year, month-1, day);
}
date_t date_next_month(date_t dt) {
    int year, month, day;
    date_to_cal(dt, &year, &month, &day);

    if (month == 12)
        return date_from_cal(year+1, 1, day);
    else
        return date_from_cal(year, month+1, day);
}
date_t date_prev_day(date_t dt) {
    return dt - 24*60*60;
}
date_t date_next_day(date_t dt) {
    return dt + 24*60*60;
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
        str->s = (char*) realloc(str->s, str->cap);
    }

    strncpy(str->s, s, s_len);
    str->s[s_len] = 0;
    str->len = s_len;
}
void str_sprintf(str_t *str, const char *fmt, ...) {
    char buf[4096];
    va_list args;
    int z;

    va_start(args, fmt);
    z = vsnprintf(buf, sizeof(buf)-1, fmt, args);
    buf[sizeof(buf)-1] = 0;
    va_end(args);

    if (z < 0)
        return;
    str_assign(str, buf);
}
void str_append(str_t *str, const char *s) {
    size_t s_len = strlen(s);
    if (str->len + s_len + 1 > str->cap) {
        str->cap = str->len + s_len + 1;
        str->s = realloc(str->s, str->cap);
    }

    strncpy(str->s + str->len, s, s_len);
    str->len = str->len + s_len;
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
    array_clear(a);
    free(a->items);
    free(a);
}
void array_clear(array_t *a) {
    memset(a->items, 0, a->cap * sizeof(void*));
    a->len = 0;
}
void array_resize(array_t *a, size_t newcap) {
    assert(newcap > a->cap);
    void **p = (void**) realloc(a->items, newcap * sizeof(void*)); 
    if (p == NULL)
        panic("array_resize() out of memory\n");
    a->items = p;
    a->cap = newcap;
    if (a->len > a->cap)
        a->len = a->cap;
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

intarray_t *intarray_new(size_t cap) {
    if (cap == 0)
        cap = 8;
    intarray_t *a = (intarray_t*) malloc(sizeof(intarray_t));
    a->items = (long*) malloc(sizeof(a->items[0]) * cap);
    a->len = 0;
    a->cap = cap;
    return a;
}
void intarray_free(intarray_t *a) {
    intarray_clear(a);
    free(a->items);
    free(a);
}
void intarray_clear(intarray_t *a) {
    memset(a->items, 0, a->cap * sizeof(long));
    a->len = 0;
}
void intarray_resize(intarray_t *a, size_t newcap) {
    assert(newcap > a->cap);
    long *p = (long*) realloc(a->items, newcap * sizeof(long)); 
    if (p == NULL)
        panic("intarray_resize() out of memory\n");
    a->items = p;
    a->cap = newcap;
    if (a->len > a->cap)
        a->len = a->cap;
}
void intarray_add(intarray_t *a, long i) {
    if (a->len >= a->cap)
        intarray_resize(a, a->cap * 2);
    a->items[a->len] = i;
    a->len++;
}
void intarray_del(intarray_t *a, uint idx) {
    for (size_t i=idx; i < a->len-1; i++) {
        a->items[i] = a->items[i+1];
    }
    a->len--;
}
