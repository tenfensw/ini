#pragma once

#include <stdbool.h>

#ifndef _MSC_VER
#include <stdint.h>

typedef uint32_t ini_size_t;
#else
typedef unsigned __int32 ini_size_t;
#endif

#ifdef INI_NO_INT64
typedef int ini_int64_t;
typedef ini_size_t ini_uint64_t;
#elif defined(_MSC_VER)
typedef __int64 ini_int64_t;
typedef unsigned __int64 ini_uint64_t;
#else
typedef int64_t ini_int64_t;
typedef uint64_t ini_uint64_t;
#endif

#define INI_ERR_MSG_MAX_LEN 127

typedef struct {
    uint32_t lnn;

    char msg[INI_ERR_MSG_MAX_LEN + 1];
} ini_err;

#define INI_PAIR_KEY_MAX_LEN 31
#define INI_PAIR_VALUE_MAX_LEN 511

typedef struct {
    char key[INI_PAIR_KEY_MAX_LEN + 1];
    char value[INI_PAIR_VALUE_MAX_LEN + 1];

    union {
        int i_value;
        ini_size_t u_value;

        ini_int64_t i64_value;
        ini_uint64_t u64_value;

        double f_value;
    };

    ini_size_t lnn;

    void* prev;
} ini_pair;

ini_pair* ini_read_n(const char* ini, ini_size_t ini_len, ini_err* err_ptr);
#define ini_read(ini, err_ptr) ini_read_n(ini, 0, err_ptr)

ini_pair* ini_get(const char* key, ini_pair* prev);

void ini_free(ini_pair* prev);
