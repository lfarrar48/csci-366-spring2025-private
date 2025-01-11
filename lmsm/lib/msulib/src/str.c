#include "msulib/str.h"
#include <stdlib.h>
#include <string.h>

//#define msu_str_fmt(s) ((int) (s).len), ((s).src)
//#define msu_str_LIT(s) ((string){s, strlen(s)})

#if defined(_WIN32)

#include <math.h>

// strndup() is not available on Windows
char *strndup(const char *s1, size_t n) {
    char *copy = malloc(sizeof(char) * (n + 1));
    memcpy(copy, s1, n);
    copy[n] = 0;
    return copy;
}

#endif

struct msu_str {
    const char *src;
    size_t len;
};

const char *msu_str_data(const msu_str_t *s) {
    return s ? s->src : "";
}

size_t msu_str_len(const msu_str_t *s) {
    return s ? s->len : 0;
}

void msu_str_free(const msu_str_t *s) {
    if (s) {
        free((void *) s->src);
        free((void *) s);
    }
}

const msu_str_t *mk_str(char *s, size_t n) {
    if (!n) return NULL;
    msu_str_t *out = malloc(sizeof(msu_str_t));
    out->src = s;
    out->len = n;
    return out;
}

const msu_str_t *msu_str_new(const char *s) {
    size_t slen = s ? strlen(s) : 0;
    char *ns = strndup(s, slen);
    return mk_str(ns, slen);
}

const msu_str_t *msu_str_new_substring(const char *s, size_t n) {
    char *ns = strndup(s, n);
    return mk_str(ns, n);
}

const msu_str_t *msu_str_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char *ns = malloc(sizeof(char) * (n + 1));
    va_start(args, fmt);
    vsnprintf(ns, n + 1, fmt, args);
    va_end(args);
    return mk_str(ns, n);
}

const msu_str_t *msu_str_clone(const msu_str_t *me) {
    if (!me) return NULL;
    char *src = strndup(me->src, me->len);
    return mk_str(src, me->len);
}

const msu_str_t *msu_str_lower(const msu_str_t *me) {
    const msu_str_t *out = msu_str_clone(me);
    char *src = (char *) out->src;
    const int diff = 'a' - 'A';
    for (size_t i = 0; i < out->len; i++) {
        if (src[i] >= 'A' && src[i] <= 'Z')
            src[i] += diff;
    }
    return out;
}

const msu_str_t *msu_str_upper(const msu_str_t *me) {
    const msu_str_t *out = msu_str_clone(me);
    char *src = (char *) out->src;
    const int diff = 'a' - 'A';
    for (size_t i = 0; i < out->len; i++) {
        if (src[i] >= 'a' && src[i] <= 'z')
            src[i] -= diff;
    }
    return out;
}

const msu_str_t *msu_str_replace_all(
        const msu_str_t *me,
        const msu_str_t *substring,
        const msu_str_t *replacement
) {
    if (!me || !substring) return me;

    msu_str_builder_t sb = msu_str_builder_new();
    size_t start = 0;
    size_t i = -1;
    while (msu_str_find_str(me, substring, &i)) {
        if (start < i) {
            msu_str_builder_pushn(sb, me->src + start, i - start);
        }
        msu_str_builder_pushstr(sb, replacement);

        start = i + substring->len;
    }
    if (start < me->len) {
        msu_str_builder_pushn(sb, me->src + start, me->len - start);
    }

    return msu_str_builder_into_string_and_free(sb);
}

const msu_str_t *msu_str_substring(const msu_str_t *me, size_t start, size_t end) {
    if (start >= end) return EMPTY_STRING;
    if (start >= msu_str_len(me)) start = msu_str_len(me);
    if (end >= msu_str_len(me)) end = msu_str_len(me);
    return msu_str_new_substring(msu_str_data(me) + start, end - start);
}

char msu_str_at(const msu_str_t *me, size_t index) {
    index = (msu_str_len(me) + index) % msu_str_len(me);
    return me ? me->src[index] : '\0';
}

bool msu_str_is_empty(const msu_str_t *me) {
    return !me || me->len == 0;
}

bool msu_str_is_blank(const msu_str_t *me) {
    for (int i = 0; i < msu_str_len(me); ++i) {
        if (!isspace(me->src[i])) return false;
    }
    return true;
}

size_t msu_str_hash(const msu_str_t *me, size_t seed) {
    size_t out;
    if (me) {
        out = murmurhash((const void *) me->src, me->len * sizeof(char), seed);
    } else {
        out = murmurhash(NULL, 0, seed);
    }
//    printf("hash(%s, %zu) = %u\n", msu_str_data(me), msu_str_len(me), out);
    return out;
}

bool msu_str_eq(const msu_str_t *me, const msu_str_t *other) {
    if (!me) return !other || other->len == 0;
    if (!other) return me->len == 0;
    if (me->len != other->len) return false;
    return 0 == strncmp(me->src, other->src, me->len);
}

bool msu_str_eqs(const msu_str_t *me, const char *s) {
    size_t slen = s ? strlen(s) : 0;
    if (!me) return 0 == slen;
    if (me->len != slen) return false;
    return 0 == strncmp(me->src, s, me->len);
}

bool msu_str_eqn(const msu_str_t *me, const char *s, size_t n) {
    if (!me) return 0 == n;
    if (me->len != n) return false;
    return 0 == strncmp(me->src, s, n);
}

bool msu_str_sw(const msu_str_t *me, const msu_str_t *prefix) {
    if (msu_str_len(prefix) > msu_str_len(me)) return false;
    if (!msu_str_len(prefix)) return true;
    return 0 == strncmp(me->src, prefix->src, prefix->len);
}

bool msu_str_sws(const msu_str_t *me, const char *prefix) {
    size_t slen = strlen(prefix);
    if (slen > msu_str_len(me)) return false;
    if (!slen) return true;
    return 0 == strncmp(me->src, prefix, slen);
}

bool msu_str_swn(const msu_str_t *me, const char *prefix, size_t prefixlen) {
    if (prefixlen > msu_str_len(me)) return false;
    if (!prefixlen) return true;
    return 0 == strncmp(me->src, prefix, prefixlen);
}

bool msu_str_contains(const msu_str_t *me, const msu_str_t *needle) {
    if (msu_str_len(needle) > msu_str_len(me)) return false;
    if (msu_str_is_empty(needle)) return true;
    for (size_t i = 0; i < msu_str_len(me) - msu_str_len(needle) + 1; i++) {
        if (0 == strncmp(me->src + i, needle->src, needle->len)) {
            return true;
        }
    }
    return false;
}

bool msu_str_containss(const msu_str_t *me, const char *needle) {
    size_t needlelen = strlen(needle);
    if (needlelen > msu_str_len(me)) return false;
    if (!needlelen) return true;
    for (size_t i = 0; i < msu_str_len(me) - needlelen + 1; i++) {
        if (0 == strncmp(me->src + i, needle, needlelen)) {
            return true;
        }
    }
    return false;
}

bool msu_str_containsn(const msu_str_t *me, const char *needle, size_t needlelen) {
    if (needlelen > msu_str_len(me)) return false;
    if (!needlelen) return true;
    for (size_t i = 0; i < msu_str_len(me) - needlelen + 1; i++) {
        if (0 == strncmp(me->src + i, needle, needlelen)) {
            return true;
        }
    }
    return false;
}

bool msu_str_rfind_char(const msu_str_t *me, char c, size_t *roff) {
    if (!me) return false;
    size_t i = *roff;
    if (i == -1) i = me->len;
    for (; i != -1; --i) {
        if (me->src[i] == c) {
            *roff = i;
            return true;
        }
    }
    return false;
}

bool msu_str_find_char(const msu_str_t *me, char c, size_t *off) {
    if (!me) return false;
    for (size_t i = *off; i + 1 < me->len; i++) {
        if (me->src[i] == c) {
            *off = i;
            return true;
        }
    }
    return false;
}

bool msu_str_find_str(const msu_str_t *me, const msu_str_t *needle, size_t *off) {
    if (msu_str_is_empty(me) || msu_str_is_empty(needle)) return false;
    if (needle->len > me->len) return false;
    size_t i = *off == -1 ? 0 : *off + needle->len;
    for (; i < me->len + 1 - needle->len; i++) {
        if (0 == strncmp(me->src + i, needle->src, needle->len)) {
            *off = i;
            return true;
        }
    }
    return false;
}

list_of_msu_strs_t *msu_str_splitlines(const msu_str_t *me) {
    list_of_msu_strs_t *out = list_of_msu_strs_new();

    size_t start = 0;
    size_t i = 0;
    for (; i < msu_str_len(me); i++) {
        if (me->src[i] == '\n') {
            size_t end;
            if (i > 0 && me->src[i - 1] == '\r') {
                end = i - 1;
            } else {
                end = i;
            }
            const msu_str_t *s = msu_str_new_substring(me->src + start, end - start);
            list_of_msu_strs_append(out, s);
            start = i + 1;
        }
    }

    if (start <= msu_str_len(me)) {
        const msu_str_t *s = msu_str_new_substring(msu_str_data(me) + start, msu_str_len(me) - start);
        list_of_msu_strs_append(out, s);
    }

    return out;
}

const msu_str_t *msu_str_get_line_for_index(const msu_str_t *me, size_t index) {
    if (index >= msu_str_len(me)) return EMPTY_STRING;

    size_t line_start = 0;
    for (size_t i = index; i > 0; i++) {
        if (me->src[i - 1] != '\n') {
            line_start = i;
            break;
        }
    }

    size_t line_end = msu_str_len(me);
    for (size_t i = index + 1; i < msu_str_len(me); i++) {
        if (me->src[i] == '\n' || (me->src[i] == '\r' && i + 1 < msu_str_len(me) && me->src[i + 1])) {
            line_end = i;
        }
    }

    return msu_str_substring(me, line_start, line_end);
}

size_t msu_str_get_lineno_for_index(const msu_str_t *me, size_t index) {
    size_t line = 1;
    for (int i = 0; i < index && i < msu_str_len(me); ++i) {
        if (msu_str_at(me, i) == '\n') line += 1;
    }
    return line;
}

size_t msu_str_get_lineoff_for_index(const msu_str_t *me, size_t index) {
    if (index >= msu_str_len(me)) return 0;
    size_t line = 1, column = 0;
    for (int i = 0; i < index; ++i) {
        if (msu_str_at(me, i) == '\n') {
            line += 1;
            column = 0;
        } else {
            column += 1;
        }
    }
    return column;
}

list_of_msu_strs_t *msu_str_splitwhite(const msu_str_t *s) {
    list_of_msu_strs_t *out = list_of_msu_strs_new();
    if (!s) return out;

    size_t i = 0;
    while (i < s->len && isspace(s->src[i])) {
        i += 1;
    }

    while (true) {
        size_t start = i;
        while (i < s->len && !isspace(s->src[i])) {
            i += 1;
        }

        size_t slen = i - start;
        if (!slen) break;

        const msu_str_t *tok = msu_str_new_substring(s->src + start, slen);
        list_of_msu_strs_append(out, tok);

        while (i < s->len && isspace(s->src[i])) {
            i += 1;
        }
    }

    return out;
}

list_of_msu_strs_t *msu_str_split(const msu_str_t *me, const msu_str_t *delim) {
    list_of_msu_strs_t *vars = list_of_msu_strs_new();
    if (!me || !delim || !delim->len) return vars;

    size_t start = 0;
    size_t i = -1;
    while (msu_str_find_str(me, delim, &i)) {
        const msu_str_t *s = msu_str_substring(me, start, i);
        list_of_msu_strs_append(vars, s);
        start = i + delim->len;
    }
    if (i == -1 || i < me->len) {
        const msu_str_t *s = msu_str_substring(me, i + 1, me->len);
        list_of_msu_strs_append(vars, s);
    }

    return vars;
}

bool msu_str_try_parse_int(const msu_str_t *me, int *out) {
    char *endptr;
    *out = (int) strtol(msu_str_data(me), &endptr, 10);
    return !*endptr;
}

const msu_str_t *__msu_str_new_unsafe(char *owned) {
    return mk_str(owned, strlen(owned));
}

void __msu_str_raw_unsafe(const msu_str_t **dest, const char *s, size_t n) {
    msu_str_t *x = (msu_str_t *) *dest; // who said const matters? SMH lmao
    x->src = s;
    x->len = n;
}

/// -----------------------------------

void msu_str_builder_ensure_capacity(msu_str_builder_t me, size_t cap) {
    static const size_t BUFSIZE = 128;
    size_t newcap = me->cap;
    while (newcap < cap) {
        newcap += BUFSIZE;
    }
    if (newcap != me->cap) {
        char *data = realloc(me->src, sizeof(char) * (1 + newcap));
        assert(data && "out of memory");
        me->src = data;
        me->cap = newcap;
    }
}

msu_str_builder_t msu_str_builder_new() {
    msu_str_builder_t out = malloc(sizeof(msu_sb_t));
    assert(out && "out of memory");
    out->src = NULL;
    out->cap = 0;
    out->len = 0;
    return out;
}

void msu_str_builder_reset(msu_str_builder_t me) {
    me->len = 0;
    me->src[0] = 0;
}

void msu_str_builder_push(msu_str_builder_t me, char c) {
    msu_str_builder_ensure_capacity(me, me->cap + 1);
    me->src[me->len] = c;
    me->len += 1;
    me->src[me->len] = 0;
}

void msu_str_builder_pushstr(msu_str_builder_t me, const msu_str_t *s) {
    if (!s) return;
    msu_str_builder_ensure_capacity(me, me->cap + s->len);
    memcpy(me->src + me->len, s->src, sizeof(char) * s->len);
    me->len += s->len;
    me->src[me->len] = 0;
}

void msu_str_builder_pushs(msu_str_builder_t me, const char *s) {
    size_t n = s ? strlen(s) : 0;
    if (!n) return;
    msu_str_builder_ensure_capacity(me, me->cap + n);
    memcpy(me->src + me->len, s, sizeof(char) * n);
    me->len += n;
    me->src[me->len] = 0;
}

void msu_str_builder_pushn(msu_str_builder_t me, const char *s, size_t n) {
    if (!n) return;
    msu_str_builder_ensure_capacity(me, me->cap + n);
    memcpy(me->src + me->len, s, sizeof(char) * n);
    me->len += n;
    me->src[me->len] = 0;
}

void msu_str_builder_printf(msu_str_builder_t me, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    msu_str_builder_ensure_capacity(me, me->cap + n);
    va_start(args, fmt);
    vsnprintf(me->src + me->len, n + 1, fmt, args);
    va_end(args);
    me->len += n;
}

const msu_str_t *msu_str_builder_to_string(msu_str_builder_t me) {
    char *ns = strndup(me->src, me->len);
    const msu_str_t *out = mk_str(ns, me->len);
    return out;
}

const msu_str_t *msu_str_builder_into_string_and_free(msu_str_builder_t me) {
    const msu_str_t *out = mk_str(me->src, me->len);
    memset(me, 0, sizeof(msu_sb_t));
    msu_str_builder_free(me);
    return out;
}

const msu_str_t *msu_str_builder_substring(msu_str_builder_t me, size_t start, size_t end) {
    if (start >= me->len) return NULL;
    if (end > me->len) return NULL;
    if (end <= start) return NULL;
    char *s = strndup(me->src + start, end - start);
    return mk_str(s, end - start);
}

void msu_str_builder_free(msu_str_builder_t me) {
    if (me) {
        free(me->src);
        free(me);
    }
}

bool msu_str_ends_with(const msu_str_t *me, const char *suffix) {
    const char *str = me->src;
    if (!str || !suffix)
        return 0;
    size_t suffix_len = strlen(suffix);
    if (suffix_len > me->len)
        return false;
    return strncmp(str + me->len - suffix_len, suffix, suffix_len) == 0;
}

const msu_str_t *msu_str_slice_left(const msu_str_t *me, int length) {
    if (length >= msu_str_len(me)) return EMPTY_STRING;
    size_t size = me->len - length;
    char *ns = strndup(me->src + length, size);
    return mk_str(ns, size);
}

const msu_str_t *msu_str_slice_right(const msu_str_t *me, int length) {
    if (length >= msu_str_len(me)) return EMPTY_STRING;
    size_t size = me->len - length;
    char *ns = strndup(me->src, size);
    return mk_str(ns, size);
}

const msu_str_t *msu_str_trim_ws(const msu_str_t *me) {
    size_t start;
    for (start = 0; start < msu_str_len(me); start++) {
        if (!isspace(msu_str_at(me, start))) {
            break;
        }
    }
    if (start == msu_str_len(me)) return EMPTY_STRING;

    size_t end;
    for (end = me->len; end > 0; end--) {
        if (!isspace(msu_str_at(me, end - 1))) {
            break;
        }
    }

    return msu_str_substring(me, start, end);
}

const msu_str_t *msu_str_trim_ws_left(const msu_str_t *me) {
    size_t start;
    for (start = 0; start < msu_str_len(me); start++) {
        if (!isspace(msu_str_at(me, start))) {
            break;
        }
    }

    return msu_str_substring(me, start, msu_str_len(me));
}

const msu_str_t *msu_str_trim_ws_right(const msu_str_t *me) {
    size_t end;
    for (end = msu_str_len(me); end > 0; end--) {
        if (!isspace(msu_str_at(me, end - 1))) {
            break;
        }
    }
    return msu_str_substring(me, 0, end);
}
const msu_str_t *t1 = (const msu_str_t *) &(strlit){"hey!!", 2};