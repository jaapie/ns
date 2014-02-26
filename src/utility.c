#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#include "utility.h"

void print_error(const char* fmt, ...) {
    va_list list;
    char buf[512];

    va_start(list, fmt);
    vsprintf(buf, fmt, list);
    va_end(list);

    fprintf(stderr, "error: %s", buf);
}

bool yes_no () {
    int c = getchar();

    if (feof(stdin)) {
        clearerr(stdin);
        return false;
    }

    bool yes = (c == 'y' || c == 'Y');

    while (c != '\n' && c != EOF)
        c = getchar ();

    return yes;
}

bool get_confirmation(const char* fmt, ...) {
    va_list list;
    char buf[512];

    va_start(list, fmt);
    vsprintf(buf, fmt, list);
    va_end(list);

    printf("%s", buf);

    return yes_no();
}

char *get_zero_padded_number(int number, int width) {
    char *number_str; /*= "\0\0\0\0\0\0";*/
    size_t number_str_len = 0;

    number_str = malloc(7);
    assert(number_str != NULL);

    if ( number_str != NULL ) {
        memset(number_str, '\0', 7);

        sprintf(number_str, "%d", number);
        number_str_len = strlen(number_str);

        if ( number_str_len < width ) {
            unsigned zero_count = width - (unsigned)number_str_len;
            char *padded_number = malloc(width + 1);
            assert(padded_number != NULL);

            if ( padded_number != NULL ) {
                memset(padded_number, '0', width + 1);
                strncpy(&padded_number[zero_count], number_str, number_str_len);
                padded_number[width] = '\0';
                free(number_str);

                return padded_number;
            } else {
                free(number_str);
                return NULL;
            }
        }

        return number_str;
    }
    else
        return NULL;
}
