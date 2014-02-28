/*
 * Numerical Sequencer
 * Copyright (C) 2014 Jacob Degeling
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

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

char *get_zero_padded_number(size_t number, size_t width) {
    char *number_str; /*= "\0\0\0\0\0\0";*/
    size_t number_str_len = 0;

    number_str = malloc(7);
    assert(number_str != NULL);

    if ( number_str != NULL ) {
        memset(number_str, '\0', 7);

        sprintf(number_str, "%lu", number);
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
