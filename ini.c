#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <ctype.h>

#ifndef INI_NO_INT64
#include <inttypes.h>
#endif

#include "ini.h"

void ini_free(ini_pair* last) {
    while (last) {
        ini_pair* prev = last->prev;
        free(last);

        last = prev;
    }
}

#define IS_QUOTED(input, input_len) \
    (input && input_len >= 2 && \
     input[0] == input[input_len - 1] && \
     input[0] == '"')

void ini_unquote_value(ini_pair* cur, const ini_size_t value_len) {
    if (!IS_QUOTED(cur->value, value_len))
        return;

    char buf[INI_PAIR_VALUE_MAX_LEN + 1];

    ini_size_t len = 0;
    bool is_escape = false;

    for (ini_size_t index = 1; index < value_len - 1; index++) {
        const char cc = cur->value[index];

        if (cc == '\\' && !is_escape)
            is_escape = true;
        else {
            is_escape = false;

            buf[len++] = cc;
            buf[len] = '\0';
        }
    }

    memset(cur->value, 0, INI_PAIR_VALUE_MAX_LEN + 1);
    strcpy(cur->value, buf);
}

#undef IS_QUOTED

#define IS_BOOL(input, opt1, opt2) \
    (input && (tolower(input[0]) == opt1 || tolower(input[0]) == opt2))

#define IS_TRUE(input) \
    IS_BOOL(input, 't', 'y')

#define IS_FALSE(input) \
    IS_BOOL(input, 'f', 'n')

void ini_conv_value(ini_pair* cur, const ini_size_t value_len,
                    const bool is_num, const bool is_dec) {
    if (value_len < 1)
        return;

    ini_unquote_value(cur, value_len);

    if (is_num) {
        if (is_dec)
            sscanf(cur->value, "%lf", &cur->f_value);
        else
#ifndef INI_NO_INT64
            sscanf(cur->value, "%" SCNi64, &cur->i64_value);
#else
            sscanf(cur->value, "%d", &cur->i_value);
#endif
    } else if (IS_TRUE(cur->value))
        cur->u_value = 1;
    else if (IS_FALSE(cur->value))
        cur->u_value = 0;
}

#undef IS_FALSE
#undef IS_TRUE

#undef IS_BOOL

#define ERR_SET(ptr, lnn_value, ...) { \
    if (ptr) { \
        memset(ptr, 0, sizeof(ini_err)); \
        \
        ptr->lnn = lnn_value; \
        snprintf(ptr->msg, INI_ERR_MSG_MAX_LEN, __VA_ARGS__); \
    } \
}

#define ERR_RET(ptr, ...) { \
    ERR_SET(ptr, (last ? last->lnn : 0), __VA_ARGS__) \
    \
    ini_free(last); \
    return NULL; \
}

#define LAST_NEW(last) { \
    ini_pair* prev = last; \
    \
    last = malloc(sizeof(ini_pair)); \
    memset(last, 0, sizeof(ini_pair)); \
    \
    last->lnn = lnn; \
    last->prev = prev; \
}

ini_pair* ini_read_n(const char* ini, ini_size_t ini_len, ini_err* err_ptr) {
    if (!ini) {
        ERR_SET(err_ptr, 0, "NULL input INI C string")
        return NULL;
    } else if (ini_len < 1)
        ini_len = (ini_size_t)strlen(ini);

    ini_pair* last = NULL;

    bool is_begin = true;
    bool is_key = true;

    bool is_comment = false;

    bool is_num = true;
    bool is_dec = false;

    ini_size_t lnn = 0;
    ini_size_t len = 0;

    for (ini_size_t index = 0; index < ini_len; index++) {
        char cc = ini[index];

        if (isspace(cc) && len < 1)
            continue;
        else if (cc == '#' && is_begin)
            is_comment = true;
        else if ((cc == '=' || (len + 1) >= INI_PAIR_KEY_MAX_LEN) && is_key) {
            if (is_begin)
                ERR_RET(err_ptr, "cannot have value with empty key")

            is_key = false;
            len = 0;
        } else if (cc == '\n') {
            if (!is_key)
                ini_conv_value(last, len, is_num, is_dec);

            is_begin = true;
            is_key = true;

            is_comment = false;

            is_num = true;
            is_dec = false;

            len = 0;
            lnn++;

            continue;
        } else if (!is_comment) {
            if (is_begin)
                LAST_NEW(last)

            if (is_key)
                last->key[len++] = cc;
            else {
                if (!isdigit(cc) && cc != '-' && cc != '.')
                    is_num = false;
                else if (cc == '.' && is_num)
                    is_dec = true;

                last->value[len++] = cc;
            }
        }

        is_begin = false;
    }

    return last;
}

#undef LAST_NEW

#undef ERR_RET
#undef ERR_SET

ini_pair* ini_get(const char* key, ini_pair* last) {
    while (last) {
        if (key && strcmp(key, last->key) == 0)
            return last;

        last = last->prev;
    }

    return NULL;
}
