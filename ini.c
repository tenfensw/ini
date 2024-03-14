#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <ctype.h>

#include "ini.h"

void ini_free(ini_pair* last) {
    while (last) {
        ini_pair* prev = last->prev;
        free(last);

        last = prev;
    }
}

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

    ini_size_t lnn = 0;
    ini_size_t len = 0;

    for (ini_size_t index = 0; index < ini_len; index++) {
        char cc = ini[index];
        fprintf(stderr, "index = %u, cc = '%c'\n", index, cc);

        if (isspace(cc) && is_begin)
            continue;
        else if (cc == '#' && is_begin)
            is_comment = true;
        else if ((cc == '=' || (len + 1) >= INI_PAIR_KEY_MAX_LEN) && is_key) {
            if (is_begin)
                ERR_RET(err_ptr, "cannot have value with empty key")

            is_key = false;
            len = 0;
        } else if (cc == '\n') {
            is_begin = true;
            is_key = true;

            is_comment = false;

            len = 0;
            lnn++;

            continue;
        } else if (!is_comment) {
            if (is_begin)
                LAST_NEW(last)

            if (is_key)
                last->key[len++] = cc;
            else
                last->value[len++] = cc;

            fprintf(stderr, "is_key = %u, len = %u, cc = '%c'\n",
                            is_key, len, cc);
        }

        is_begin = false;
    }

    return last;
}

#undef LAST_NEW

#undef ERR_RET
#undef ERR_SET

ini_pair* ini_read(const char* ini, ini_err* err_ptr) {
    return ini_read_n(ini, 0, err_ptr);
}

ini_pair* ini_get(const char* key, ini_pair* last) {
    while (last) {
        if (key && strcmp(key, last->key) == 0)
            return last;

        last = last->prev;
    }

    return NULL;
}
