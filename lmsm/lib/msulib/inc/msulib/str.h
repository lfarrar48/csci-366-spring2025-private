
#ifndef msu_str_H
#define msu_str_H

#include <stdbool.h>
#include <sys/types.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <msulib/hash.h>

typedef struct msu_str msu_str_t;
typedef struct list_of_msu_strs list_of_msu_strs_t;

typedef struct msu_sb {
    char *src;
    size_t len;
    size_t cap;
} msu_sb_t;
typedef msu_sb_t *msu_str_builder_t;

const char *msu_str_data(const msu_str_t *s); // for use like: `printf("%s", msu_str_data(ur_string))`
size_t msu_str_len(const msu_str_t *s);

#define EMPTY_STRING ((const msu_str_t *)NULL)
const msu_str_t *msu_str_new(const char *s);
const msu_str_t *msu_str_new_substring(const char *s, size_t n);
const msu_str_t *msu_str_printf(const char *fmt, ...);
const msu_str_t *msu_str_clone(const msu_str_t *me);
const msu_str_t *msu_str_lower(const msu_str_t *me);
const msu_str_t *msu_str_upper(const msu_str_t *me);
const msu_str_t *msu_str_replace_all(const msu_str_t *me, const msu_str_t *substring, const msu_str_t *replacement);
const msu_str_t *msu_str_substring(const msu_str_t *me, size_t start, size_t end);
const msu_str_t *msu_str_get_line_for_index(const msu_str_t *me, size_t index);
const msu_str_t *msu_str_slice_left(const msu_str_t *me, int length);
const msu_str_t *msu_str_slice_right(const msu_str_t *me, int length);
const msu_str_t *msu_str_trim_ws(const msu_str_t *me);
const msu_str_t *msu_str_trim_ws_left(const msu_str_t *me);
const msu_str_t *msu_str_trim_ws_right(const msu_str_t *me);
size_t msu_str_get_lineno_for_index(const msu_str_t *me, size_t index);
size_t msu_str_get_lineoff_for_index(const msu_str_t *me, size_t index);
char msu_str_at(const msu_str_t *me, size_t index);
void msu_str_free(const msu_str_t *me);

bool msu_str_is_empty(const msu_str_t *me);
bool msu_str_is_blank(const msu_str_t *me);
size_t msu_str_hash(const msu_str_t *me, size_t seed);
bool msu_str_eq(const msu_str_t *me, const msu_str_t *other);
bool msu_str_eqs(const msu_str_t *me, const char *s);
bool msu_str_eqn(const msu_str_t *me, const char *s, size_t n);
bool msu_str_sw(const msu_str_t *me, const msu_str_t *prefix); // sw=startswith
bool msu_str_sws(const msu_str_t *me, const char *prefix);
bool msu_str_swn(const msu_str_t *me, const char *prefix, size_t prefixlen);
bool msu_str_contains(const msu_str_t *me, const msu_str_t *needle);
bool msu_str_containss(const msu_str_t *me, const char *needle);
bool msu_str_containsn(const msu_str_t *me, const char *needle, size_t needlelen);
bool msu_str_find_str(const msu_str_t *me, const msu_str_t *needle, size_t *off);
bool msu_str_rfind_char(const msu_str_t *me, char c, size_t *roff);
bool msu_str_find_char(const msu_str_t *me, char c, size_t *off);
bool msu_str_ends_with(const msu_str_t *me, const char *ending);

list_of_msu_strs_t *msu_str_splitlines(const msu_str_t *s);
list_of_msu_strs_t *msu_str_splitwhite(const msu_str_t *s);
list_of_msu_strs_t *msu_str_split(const msu_str_t *me, const msu_str_t *delim);

bool msu_str_try_parse_int(const msu_str_t *me, int *out);

const msu_str_t *__msu_str_new_unsafe(char *owned);
void __msu_str_raw_unsafe(const msu_str_t **dest, const char *s, size_t n);

msu_str_builder_t msu_str_builder_new();
void msu_str_builder_ensure_capacity(msu_str_builder_t me, size_t size);
void msu_str_builder_reset(msu_str_builder_t me);
void msu_str_builder_free(msu_str_builder_t me);
void msu_str_builder_push(msu_str_builder_t me, char c);
void msu_str_builder_pushstr(msu_str_builder_t me, const msu_str_t *s);
void msu_str_builder_pushs(msu_str_builder_t me, const char *s);
void msu_str_builder_pushn(msu_str_builder_t me, const char *s, size_t n);
void msu_str_builder_printf(msu_str_builder_t me, const char *fmt, ...);
const msu_str_t *msu_str_builder_to_string(msu_str_builder_t me); // return cloned inner const msu_str_t * and preserve contents
const msu_str_t *msu_str_builder_into_string_and_free(msu_str_builder_t me); // return the inner const msu_str_t * and free the builder
const msu_str_t *msu_str_builder_substring(msu_str_builder_t me, size_t start, size_t end);

typedef struct strlit {
    const char *src;
    size_t len;
} strlit;

#define STRLIT_VAL(s) ((strlit){s, sizeof(s)-1})
#define STRLIT(s) ((const msu_str_t *) &(strlit){s, sizeof(s)-1})
#define STRLIT_DEF(NAME, str) const msu_str_t *NAME = (const msu_str_t *) &(strlit){str, sizeof(str) - 1};

typedef struct strlistlit {
    const strlit *values;
    size_t len;
} strlistlit;

#endif // msu_str_H

#include "msulib/strlist.h"