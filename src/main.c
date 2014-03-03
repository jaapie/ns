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

// Uses libexif
// Exif code usage helped by http://code.google.com/p/wiimc/source/browse/trunk/libs/libexif/contrib/examples/photographer.c?r=434

#include "ns.h"

#include "options.h"
#include "file_item.h"
#include "file.h"
#include "utility.h"

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

int process_files( struct arguments args) {
    unsigned int sequence_number = 0;
    file_item_t *current = args.file_list;
    bool interactive = args.flags & OPT_INTERACTIVE;
    int verbosity = args.flags & (OPT_VERBOSITY_QUIET | OPT_VERBOSITY_NORMAL | OPT_VERBOSITY_VERBOSE);
    bool dry_run = (args.flags & OPT_DRY_RUN);

    while (current != NULL) {
        file_item_generate_new_filename(current, args.base_name, args.separator, args.number_width, args.sequence_start, sequence_number); //, interactive, verbosity);

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
        {"base", 'b', "STRING", 0, "String portion of new filenames.", 0},
        {"separator", 's', "CHAR", 0, "Separator for different parts of new filenames.", 0},
        {"width", 'w', "NUM", 0, "Minimum field width for sequence number, defaults to 4.", 0},
        {"start", 'S', "NUM", 0, "Starting integer for sequence, defaults to 0.", 0},
        {0, 0, 0, 0, "Program Flow Options:", -10},
        {"quiet", 'q', 0, 0, "Program runs with no output.", 0},
        {"verbose", 'v', 0, 0, "Program runs with verbose output.", 0},
        {"interactive", 'i', 0, 0, "Prompt for confirmation before avoiding a collision. Ignores --quiet, -q.", 0},
        {"dry-run", 'n', 0, 0, "Perform a dry run. Implies --verbose, -v, and ignores --interactive, -i.", 0},
        {0, 0, 0, 0, 0, 0}
    };

    char help_text[1024] = "Renames JPG and TIFF files using a base name, a separator, and a zero-padded sequence number. Files are sorted by the EXIF Date and Time Digitised attribute.\n\nns uses libexif to read EXIF data";

#ifdef MAC_OS_X
    strncat(help_text, ", and the argp-standalone package on Mac OS X", 1024);
#else
#endif

    struct argp argp = { options, parse_options, "FILE(S)", help_text, 0, 0, 0};

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
