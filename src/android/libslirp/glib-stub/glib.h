/*
    Copyright 2023 Jesse Talavera-Greenberg

    melonDS DS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS DS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS DS. If not, see http://www.gnu.org/licenses/.
*/

#ifndef MELONDS_DS_GLIB_STUB_H
#define MELONDS_DS_GLIB_STUB_H

/**!
 * @file glib.h
 * @brief Stub for the parts of glib.h that libslirp uses
 */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define G_BIG_ENDIAN __ORDER_BIG_ENDIAN__
#define G_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define G_BYTE_ORDER __BYTE_ORDER__
#ifdef _WIN32
#define G_OS_WIN32 1
#endif

#ifdef UNIX
#define G_OS_UNIX 1
#endif

#define G_STATIC_ASSERT(expr) static_assert(expr, #expr " failed")

#define ARRAY_SIZE(ARRAY) ((sizeof(ARRAY)/(&(ARRAY)==(ARRAY)?sizeof((ARRAY)[0]):0)))

#define G_N_ELEMENTS(arr) ARRAY_SIZE(arr)

#define g_assert_not_reached() do { assert(false && "unreachable"); __builtin_unreachable(); } while (0)

typedef bool gboolean;
typedef int gint;
typedef unsigned char guchar;
typedef char gchar;
typedef void *gpointer;
typedef uint32_t GRand;
typedef size_t gsize;
typedef ssize_t gssize;
typedef unsigned int guint;

typedef struct {
    const gchar *key;
    guint value;
} GDebugKey;


GRand *g_rand_new();
void g_rand_free(GRand *rand);
#define g_parse_debug_string(string, keys, nkeys) (0)
uint32_t g_rand_int_range(GRand *rand, uint32_t begin, uint32_t end);

typedef char **GStrv;

int g_strv_length(char **str_array);

#define G_UNLIKELY(x) __builtin_expect(x, 0)

void g_debug(const char *msg, ...);
void g_error(const char *msg, ...);
void g_warning(const char *msg, ...);
void g_critical(const char *msg, ...);

#define g_malloc malloc
#define g_free free
#define g_realloc realloc

#define g_strdup(STR) (STR ? strdup(STR) : NULL)
#define g_vsnprintf vsnprintf
#define g_strerror strerror
#define g_snprintf(string, n, format, ...) snprintf(string, n, format, ##__VA_ARGS__)
#define g_strlcpy(dst, src, size) strlcpy(dst, src, size)
gchar *g_strstr_len(const gchar *haystack, gssize haystack_len, const gchar *needle);
gboolean g_str_has_prefix(const gchar* str,const gchar* prefix);
gint g_ascii_strcasecmp(const gchar *s1, const gchar *s2);

#define g_new(typ, n) (n > 0 ? (typ*)malloc(sizeof(typ) * n) : NULL)
#define g_new0(typ, n) (n > 0 ? (typ*)calloc(sizeof(typ), n) : NULL)

#define g_assert assert
#define g_malloc0(size) calloc(1, (size))
#define g_getenv(name) getenv(name)

// this can result in double evaluation, but this is how they seem to define it as well
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define G_GNUC_PRINTF(format_idx, arg_idx)

#define g_return_if_fail(expr) \
    do {                       \
        if (!(expr))           \
            return;            \
    } while (false)

#define g_return_val_if_fail(expr, val) \
    do { \
        if (!(expr)) \
            return (val); \
    } while (false)

#define g_warn_if_reached() ((void)0)
#define g_warn_if_fail(expr) ((void)(expr))

#endif