#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "options.h"
#include "file.h"
#include "file_item.h"

int file_item_generate_new_filename(file_item_t *item, char *base, char separator, unsigned int width, unsigned int sequence_start, unsigned int sequence_number) { /*, bool interactive, int verbosity ) { */
    char *ext;
    char path[1024];
    unsigned int length = 0;

    assert(item != NULL);
    assert(base != NULL);

    ext = get_file_ext(item->file_name);
    assert(strlen(ext) > 0);
    assert(strcmp(ext, "") != 0);

    length = get_path_from_file_spec(item->file_name, path);

    //assert(length > 0); //TODO: if length is 0 it means that there is no path...
                        // that's OK. You just have to prepend './' to the filename.
                        // Therefore this assertion is not needed.

    /* if (length == 0) { */

    /* } */

    /* TODO:
     * could refactor the below referenced function so that most of the work
     * is done in this function, therefore not needing to pass the verbosity
     * and interactive parameters in which this create_new_file_name function
     * should not know about anyhow.
     */ 

    item->file_name_new = create_new_file_name(path, base, separator, width, sequence_start + sequence_number, ext, &(item->collision_avoided));

    return 0;
}

void file_item_print(const file_item_t * restrict item, bool dry_run, int verbosity) {
    char name[255];
    char name_new[255];

    get_file_name_from_file_spec(item->file_name, name);
    get_file_name_from_file_spec(item->file_name_new, name_new);

    switch (verbosity) {
        case OPT_VERBOSITY_NORMAL:
            printf("%s -> %s%c\n", name, name_new, item->collision_avoided ? '*' :' ');
            break;
        case OPT_VERBOSITY_QUIET:
            break;
        case OPT_VERBOSITY_VERBOSE:
        {
            char digitised_time[20]; // 01-01-2012 01:01:24 = 19 chars + 1 NUL

            strftime(digitised_time, 20, "%d-%m-%Y %H:%M:%S", localtime(&(item->date_time)));

            printf("%s (taken on %s) %s renamed to %s%s\n", name, digitised_time, dry_run ? "would be" : "will be", name_new, item->collision_avoided ? " (avoiding collision)" : "");
            break;
        }
        default:
            break;
    }
}

int file_item_rename_file( file_item_t *item) {
    assert(item != NULL);

    if (item == NULL)
        return -1;

    if ( rename(item->file_name, item->file_name_new) == -1) {
        return errno;
    }

    return 0;
}

