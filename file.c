#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#include "file.h"
#include "utility.h"
#include "options.h"

int get_file_name_length(const char *file_spec) {
    int i = 0;
    size_t occurrence_index = 0;
    size_t count = strlen(file_spec);

    assert(file_spec != NULL);
    assert(count > 0);

    if (file_spec == NULL || count == 0) {
        errno = EINVAL;
        return -1;
    }

    for (i = 0; i < count; i++) {
        if (file_spec[i] == '/')
            occurrence_index = i;
    }

    return (int)(count - occurrence_index);
}

int get_file_name_from_file_spec(const char *file_spec, char *file_name) {
    int i = 0, occurrence_index = -1;
    int count = (int)strlen(file_spec);

    assert(file_spec != NULL);
    assert(file_name != NULL);
    assert(count > 0);

    if (file_spec == NULL || file_name == NULL || count == 0) {
        errno = EINVAL;
        return -1;
    }

    for (i = 0; i < count; i++) {
        if (file_spec[i] == '/')
            occurrence_index = i;
    }

    if (occurrence_index == -1) {
        occurrence_index = 0;
        strncpy(file_name, file_spec, count);
    } else {
        occurrence_index++;
        strncpy(file_name, &file_spec[occurrence_index], count - occurrence_index);
        file_name[count - occurrence_index] = '\0';
    }

    return count - occurrence_index;
}

int get_path_from_file_spec(char *file_spec, char *path) {
    int i;
    int occurrence_index = -1;
    size_t count = strlen(file_spec);

    assert(count > 0);
    if (count == 0)
        return 0;

    for (i = 0; i < count; i++) {
        if (file_spec[i] == '/')
            occurrence_index = i;
    }

    strncpy(path, file_spec, occurrence_index + 1);
    path[occurrence_index + 1] = '\0';

    return occurrence_index + 1;
}

char *get_file_ext(const char *filename) {
    assert(filename != NULL);
    if ( filename == NULL )
        return NULL;

    char *ext = strrchr(filename, '.');

    if ( ext == NULL )
        return "";

    return &ext[1];
}

int file_exists(const char *file_name) {
    struct stat fstat;
    int ret = stat(file_name, &fstat);

    if (ret == 0) {
        if (fstat.st_mode & S_IFREG)
            return 1;
        else
            return 2;
    } else if (ret == -1) {
        if (errno == ENOENT)
            return 0;
        else
            return -1;
    }

    return -1;
}

char *create_new_file_name(char *path, char *base, char separator, unsigned int width, unsigned int sequence, char *ext, bool *collision_avoided) {
    char *new_name;
    unsigned int size = 0;

    assert(base != NULL);
    assert(ext != NULL);
    assert(collision_avoided != NULL);

    size = (unsigned int)(strlen(path) + strlen(base) + 1 + width + 1 + strlen(ext) + 1);

    new_name = malloc(size);

    assert(new_name != NULL);

    if ( new_name != NULL ) {
        int new_index = 2;
        char *padded_number = get_zero_padded_number(sequence, width);
        sprintf(new_name, "%s%s%c%s.%s", path, base, separator, padded_number, ext);

        int exists = file_exists(new_name);

        if (exists > 0) {
            // file exists, must avoid collision!
            // reallocate the memory for the new name.
            // add on 3 bytes: 1 for separator, and 2 for numbers 2..99

            *collision_avoided = true;
            new_name = realloc(new_name, size + 3);

            while (exists > 0) {
                sprintf(new_name, "%s%s%c%s%c%d.%s", path, base, separator, padded_number, separator, new_index, ext);
                exists = file_exists(new_name);
                new_index++;
            }
        }

        free(padded_number);
        return new_name;
    } else {
        errno = ENOMEM;
        return NULL;
    }
}
