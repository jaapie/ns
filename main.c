//
//  main.c
//  ns - Numeical Sequencer
//
//  Created by Jacob Degeling on 9/09/12.
//  Copyright (c) 2012 Jacob Degeling. All rights reserved.
//
// Uses libexif
// Exif code usage helped by http://code.google.com/p/wiimc/source/browse/trunk/libs/libexif/contrib/examples/photographer.c?r=434

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include </usr/include/assert.h>
#include <string.h>
#include </usr/include/time.h>
#include <errno.h>
#include <libexif/exif-data.h>
#include <sys/stat.h>
#include <pcre.h>
#include <argp.h>

typedef struct file_item {
    char *file_name;
    char *file_name_new;
    bool collision_avoided;
    time_t date_time;
    struct file_item *next;
} file_item_t;

#define OPT_DRY_RUN 0x0001
#define OPT_INTERACTIVE 0x0002
#define OPT_VERBOSITY_QUIET 0x0008
#define OPT_VERBOSITY_NORMAL 0x0010
#define OPT_VERBOSITY_VERBOSE 0x0020
#define OPT_USE_COLOUR 0x0040
#define OPT_DONT_USE_COLOUR 0x0080

struct arguments {
    unsigned short int flags;
    unsigned int number_width;
    char separator;
    unsigned int sequence_start;
    char *base_name;
    file_item_t *file_list;
    int file_list_count;
};

const char *argp_program_bug_address = "me@jacobdegeling.com";
const char *argp_program_version = "Numerical Sequencer 0.1";

void print_error(const char* fmt, ...) {
    va_list list;
    char buf[512];

    va_start(list, fmt);
    vsprintf(buf, fmt, list);
    va_end(list);

    fprintf(stderr, "error: %s", buf);
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

void file_list_add(file_item_t **head, file_item_t *new) {
    assert(head != NULL);
    assert(new != NULL);

    file_item_t *prev, *current;

    if (*head == NULL) {
        *head = new;
        return;
    }

    if (new->date_time < (*head)->date_time) {
        new->next = *head;
        *head = new;
        return;
    }

    current = *head;

    while (current != NULL && current->date_time <= new->date_time) {
        prev = current;
        current = current->next;
    }

    if (current != NULL) {
        prev->next = new;
        new->next = current;
    } else {
        prev->next = new;
    }

}

void file_list_destroy(file_item_t** head) {
    file_item_t *temp, *current;

    assert(*head != NULL);

    if (*head == NULL)
        return;

    current = *head;

    while (current != NULL) {
        temp = current;
        current = current->next;

        if (temp->file_name_new != NULL)
            free(temp->file_name_new);

        free(temp);
    }
}

void file_list_print(file_item_t *list) {
    file_item_t *current = list;
    unsigned i = 0;

    assert(list != NULL);

    if (list == NULL)
        return;

    while (current != NULL) {
        if (current->date_time != 0)
            printf("%d) %s, %ld -> %s\n", i + 1, current->file_name, current->date_time, current->file_name_new );
        else
            fprintf(stderr, "File list item %d is empty.\n", i + 1);

        i++;
        current = current->next;
    }
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

int file_item_generate_new_filename(file_item_t *item, char *base, char separator, unsigned int width, unsigned int sequence_start, unsigned int sequence_number, bool interactive, int verbosity ) {
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

int process_files( struct arguments args) {
    unsigned int sequence_number = 0;
    file_item_t *current = args.file_list;
    bool interactive = args.flags & OPT_INTERACTIVE;
    int verbosity = args.flags & (OPT_VERBOSITY_QUIET | OPT_VERBOSITY_NORMAL | OPT_VERBOSITY_VERBOSE);
    bool dry_run = (args.flags & OPT_DRY_RUN);

    while (current != NULL) {
        file_item_generate_new_filename(current, args.base_name, args.separator, args.number_width, args.sequence_start, sequence_number, interactive, verbosity);

        //printf("%d) %s, %ld -> %s\n", sequence_number + 1, current->file_name, current->date_time, current->file_name_new );

        if (dry_run) {
            file_item_print(current, dry_run, verbosity);
        } else {
            file_item_print(current, dry_run, verbosity);

            if (interactive) {
                if (get_confirmation("Rename the file? [yn] ")) {
                    file_item_rename_file(current);
                }
            } else {
                file_item_rename_file(current);
            }
        }

        sequence_number++;
        current = current->next;
    }
    return false;
}

time_t get_image_date_time(const char* image) {
    ExifData *data;
    ExifEntry *entry;

    data = exif_data_new_from_file(image);

    if ( data != NULL ) {
        entry = exif_content_get_entry(data->ifd[EXIF_IFD_EXIF], EXIF_TAG_DATE_TIME_DIGITIZED);

        if ( entry != NULL ) {
            char date_time[1024];
            exif_entry_get_value(entry, date_time, 1024);

            //http://stackoverflow.com/questions/1002542/how-to-convert-datetime-to-unix-timestamp-in-c

            struct tm tm;
            time_t epoch;
            if ( strptime(date_time, "%Y:%m:%d %H:%M:%S", &tm) != NULL ) {
                epoch = mktime(&tm);
                exif_data_unref(data);
                return epoch;
            }
            else
                return 0;
        }
    }

    exif_data_unref(data);

    return (time_t)0;
}

error_t parse_options(int key, char *arg, struct argp_state *state) {
    struct arguments *a = state->input;
    switch (key) {
        case ARGP_KEY_INIT:
        {
            a->flags = OPT_VERBOSITY_NORMAL | OPT_DONT_USE_COLOUR;
            a->number_width = 4;
            a->sequence_start = 1;
            a->base_name = NULL;
            a->file_list_count = 0;
            a->file_list = NULL;
            break;
        }
        case ARGP_KEY_ARG: {
            if (file_exists(arg)) {
                time_t date_time = get_image_date_time(arg);

                if ( date_time  == 0 ) {
                    if (!(a->flags & OPT_VERBOSITY_QUIET))
                        print_error("\"%s\": not a JPEG file or no EXIF data.\n", arg);
                    break;
                } else {
                    file_item_t *new = (file_item_t*)malloc(sizeof(file_item_t));

                    new->file_name = arg;
                    new->date_time = date_time;
                    new->file_name_new = 0;
                    new->collision_avoided = false;
                    new->next = 0;

                    file_list_add(&a->file_list, new);
                    a->file_list_count++;
                }
            } else {
                argp_failure(state, 1, ENOENT, "%s", arg);
            }

            break;
        }
        case ARGP_KEY_NO_ARGS:
        {
#ifdef DEBUG
            printf("no args!\n");
#endif
            argp_usage(state);
            break;
        }
        case 'n':
            a->flags |= OPT_DRY_RUN;
            a->flags &= ~(OPT_VERBOSITY_QUIET);
            break;
        case 'v':
            a->flags |= OPT_VERBOSITY_VERBOSE;
            a->flags &= ~(OPT_VERBOSITY_NORMAL | OPT_VERBOSITY_QUIET);
            break;
        case 'q':
        {
            if (a->flags & OPT_DRY_RUN)
                break;

            a->flags |= OPT_VERBOSITY_QUIET;
            a->flags &= ~(OPT_VERBOSITY_NORMAL | OPT_VERBOSITY_VERBOSE);
            break;
        }
        case 'i':
            if (a->flags & OPT_DRY_RUN)
                break;

            if (a->flags & OPT_VERBOSITY_QUIET) // turn off quiet verbosity if interactive mode is specified
                a->flags &= ~OPT_VERBOSITY_QUIET;

            a->flags |= OPT_INTERACTIVE | OPT_VERBOSITY_NORMAL;
            break;
        case 'b':
        {
            //TODO: This name needs to be sanitised so that it is not illegal
            // windows illegal chars: ^:<>?*|\/
            // mac/*nix illegal chars: :
            // mac/*nix shouldn't start with a . (issue warning because a . is still legal)

            if (arg != NULL)
                a->base_name = arg;
            else
                argp_error(state, "Argument not specified for --base, -b option.");

            break;
        }
        case 's':
        {
            size_t size = strlen(arg);
            if ( size > 1 ) {
                argp_error(state, "The --separator or -s option's argument was too long (expected 1 character)");
                return 1;
            }
            else if (size == 0) {
                argp_error(state, "Argument not specified for --separator, -s option.");
            } else {
                assert(arg != NULL);
                a->separator = arg[0];
            }

            break;
        }
        case 'w':
        {
            if (arg != NULL) {
                char *endptr;
                a->number_width = (unsigned int)strtol(arg, &endptr, 10);
                if (endptr[0] != '\0') {
                    argp_error(state, "--width or -w requires a number. You passed: \"%s\".", endptr);
                }
            } else {
                argp_error(state, "Argument not specified for the --width, -w option.");
            }

            break;
        }
        case 'S':
        {
            if (arg != NULL) {
                char *endptr;
                a->sequence_start = (unsigned int)strtol(arg, &endptr, 10);
                if (endptr[0] != '\0') {
                    argp_error(state, "--sequence or -S requires a number. You passed: \"%s\".", endptr);
                }
            } else {
                argp_error(state, "Argument not specified for the --width, -w option.");
            }

            break;
        }
        default:
            //argp_usage(state);
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    struct arguments arguments;
    struct argp_option options[] =
    {
        {0, 0, 0, 0, "File Naming Options:", 20},
        {"base", 'b', "STRING", 0, "String portion of new filenames."},
        {"separator", 's', "CHAR", 0, "Separator for different parts of new filenames."},
        {"width", 'w', "NUM", 0, "Minimum field width for sequence number, defaults to 4."},
        {"start", 'S', "NUM", 0, "Starting integer for sequence, defaults to 0."},
        {0, 0, 0, 0, "Program Flow Options:", -10},
        {"quiet", 'q', 0, 0, "Program runs with no output."},
        {"verbose", 'v', 0, 0, "Program runs with verbose output."},
        {"interactive", 'i', 0, 0, "Prompt for confirmation before avoiding a collision. Ignores --quiet, -q."},
        {"dry-run", 'n', 0, 0, "Perform a dry run. Implies --verbose, -v, and ignores --interactive, -i."},
        {0}
    };

    char help_text[1024] = "Renames JPG and TIFF files using a base name, a separator, and a zero-padded sequence number. Files are sorted by the EXIF Date and Time Digitised attribute.\n\nns uses libexif to read EXIF data";

#ifdef MAC_OS_X
    strncat(help_text, ", and the argp-standalone package on Mac OS X", 1024);
#else
#endif

    struct argp argp = { options, parse_options, "FILE(S)", help_text};

    if (argp_parse(&argp, argc, argv, 0, 0, &arguments) == 0) {
#ifdef DEBUG
        int verbosity = arguments.flags & (OPT_VERBOSITY_QUIET | OPT_VERBOSITY_NORMAL | OPT_VERBOSITY_VERBOSE);

        if (verbosity & OPT_VERBOSITY_QUIET)
            printf("Quiet verbosity\n");

        if (verbosity & OPT_VERBOSITY_NORMAL)
            printf("Normal verbosity\n");

        if (verbosity & OPT_VERBOSITY_VERBOSE)
            printf("Verbose verbosity\n");
#endif

        if (arguments.flags & OPT_VERBOSITY_VERBOSE)
            printf("Found %d files\n", arguments.file_list_count);

        if (arguments.file_list_count > 0) {
            process_files(arguments);
        }

        file_list_destroy(&(arguments.file_list));
    } else {
        if (!(arguments.flags & OPT_VERBOSITY_QUIET))
            print_error("No JPG or TIFF files to work on!\n");
    }

    return 0;
}
