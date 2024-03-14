#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "ini.h"

char* read_bytes_from_path(const char* path, ini_size_t* len_ptr) {
    if (!path)
        return NULL;

    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        perror("open() failed");
        return NULL;
    }

    ini_size_t len = (ini_size_t)lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    if (len_ptr)
        (*len_ptr) = len;

    char* result = malloc(len);
    memset(result, 0, len);

    if (read(fd, result, len) < len) {
        perror("read() failed");

        free(result);
        close(fd);

        return NULL;
    }

    close(fd);
    return result;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/file.ini\n", argv[0]);
        return 1;
    }

    ini_size_t raw_len = 0;
    char* raw = read_bytes_from_path(argv[1], &raw_len);

    if (!raw)
        return 1;

    ini_err err;
    ini_pair* last = ini_read_n(raw, raw_len, &err);

    if (last) {
        ini_pair* cur = last;

        while (cur) {
            fprintf(stdout, "cur <%p>, ->key: %s, ->value: %s"
                            ", ->u_value = %u\n",
                            cur, cur->key, cur->value,
                            cur->u_value);
            cur = cur->prev;
        }

        ini_free(last);
    } else {
        fprintf(stderr, "ini_read_n() failed - at line %u - %s\n",
                        err.lnn, err.msg);
        return 1;
    }

    return 0;
}
