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
#include </usr/include/assert.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <libexif/exif-data.h>
#include <sys/stat.h>
#include <pcre.h>
#include <argp.h>

const char *argp_program_bug_address = "me@jacobdegeling.com";
const char *argp_program_version = "version 0.1";

void print_error(const char* fmt, ...)
{
	va_list list;
	char buf[512];
	
	va_start(list, fmt);
	vsprintf(buf, fmt, list);
	va_end(list);
	
	fprintf(stderr, "error: %s", buf);
}

int file_exists(const char *file_name)
{
	struct stat fstat;
	int ret = stat(file_name, &fstat);
	
	if (ret == 0)
	{
		if (fstat.st_mode & S_IFREG)
			return 1;
		else
			return 2;
	}
	else if (ret == -1)
 	{
		if (errno == ENOENT)
			return 0;
		else
			return -1;
	}
	
	return -1;
}

int get_path_from_file_spec(char *file_spec, char *path)
{
	int i;
	int occurrence_index = 0;
	size_t count = strlen(file_spec);
	
	assert(count > 0);
	if ( count == 0 )
		return 0;
	
	for ( i = 0; i < count; i++ )
	{
		if ( file_spec[i] == '/' )
			occurrence_index = i;
	}
	
	strncpy(path, file_spec, occurrence_index);
	path[occurrence_index] = '\0';
	
	return occurrence_index - 1;
}

char *get_file_ext(const char *filename)
{
	assert(filename != NULL);
	if ( filename == NULL )
		return NULL;
	
	char *ext = strrchr(filename, '.');
	
	if ( ext == NULL )
		return "";
	
	return &ext[1];
}

char *get_zero_padded_number(int number, int width)
{
	char *number_str; /*= "\0\0\0\0\0\0";*/
	size_t number_str_len = 0;
	
	number_str = malloc(7);
	assert(number_str != NULL);
	
	if ( number_str != NULL )
	{
		memset(number_str, '\0', 7);
		
		sprintf(number_str, "%d", number);
		number_str_len = strlen(number_str);
		
		if ( number_str_len < width )
		{
			unsigned zero_count = width - (unsigned)number_str_len;
			char *padded_number = malloc(width + 1);
			assert(padded_number != NULL);
			
			if ( padded_number != NULL )
			{
				memset(padded_number, '0', width + 1);
				strncpy(&padded_number[zero_count], number_str, number_str_len);
				padded_number[width] = '\0';
				free(number_str);
				
				return padded_number;
			}
			else
			{
				free(number_str);
				return NULL;
			}
		}
		
		return number_str;
	}
	else
		return NULL;
}

typedef struct file_item {
	char *file_name;
	char *file_name_new;
	time_t date_time;
	struct file_item *next;
} file_item_t;

file_item_t *file_list_create(int items)
{	
	return NULL;
}

void file_list_add(file_item_t **head, file_item_t *new)
{
	assert(head != NULL);
	assert(new != NULL);
	
	file_item_t *prev, *current;
	
	if (*head == NULL)
	{
		*head = new;
		return;
	}
	
	if (new->date_time < (*head)->date_time)
	{
		new->next = *head;
		*head = new;
		return;
	}
	
	current = *head;
	
	while (current != NULL && current->date_time <= new->date_time)
	{
		prev = current;
		current = current->next;
	}
	
	if (current != NULL)
	{
		prev->next = new;
		new->next = current;
	}
	else
	{
		prev->next = new;
	}
	
}

void file_list_destroy(file_item_t** head)
{
	file_item_t *temp, *current;
	
	assert(*head != NULL);
	
	if (*head == NULL)
		return;
	
	current = *head;
	
	while (current != NULL)
	{
		temp = current;
		current = current->next;

		if (temp->file_name_new != NULL)
			free(temp->file_name_new);
		
		free(current);
	}
	
}

void file_list_print(file_item_t *list)
{
	file_item_t *current = list;
	unsigned i = 0;

	assert(list != NULL);

	if (list == NULL)
		return;
	
	while (current != NULL)
	{
		if (current->date_time != 0)
			printf("%d) %s, %ld -> %s\n", i + 1, current->file_name, current->date_time, current->file_name_new );
		else
			fprintf(stderr, "File list item %d is empty.\n", i + 1);
		
		i++;
		current = current->next;
	}
}

char *create_new_file_name( char *path, char *base, char separator, unsigned int width, unsigned int sequence, char *ext )
{
	char *new_name;
	unsigned int size = 0;
	
	assert(base != NULL);
	assert(ext != NULL);
	
	size = (unsigned int)(strlen(path) + strlen(base) + 1 + width + 1 + strlen(ext) + 1);
	
	new_name = malloc(size);
	
	assert(new_name != NULL);
	
	if ( new_name != NULL )
	{
		char *padded_number = get_zero_padded_number(sequence, width);
		sprintf(new_name, "%s/%s%c%s.%s", path, base, separator, padded_number, ext);
		free(padded_number);
		
		return new_name;
	}
	else
		return NULL;
}

int file_list_generate_new_filenames(file_item_t *list, char *base, char separator, unsigned int width, unsigned int sequence_start )
{
	unsigned int i = 0;
	char *ext;
	char path[1024];
	unsigned int length = 0;
	file_item_t *current = list;
	
	assert(list != NULL);
	assert(base != NULL);

	while(current != NULL)
	{
		assert(current->file_name != NULL);
		ext = get_file_ext(current->file_name);
		assert(strlen(ext) > 0);
		assert(strcmp(ext, "") != 0);
		
		length = get_path_from_file_spec(current->file_name, path);
		
		assert(length > 0);
		
		if (length > 0)
			current->file_name_new = create_new_file_name(path, base, separator, width, sequence_start + i, ext);
		else
			return -1;
		
		i++;
		current = current->next;
	}
	
	return 0;
}

int file_list_rename_files( file_item_t *list, int prompt_for_confirmation)
{
	file_item_t *current = list;

	assert(list != NULL);
	
	if (list == NULL)
		return -1;
	
	while(current != NULL)
	{
		if ( rename(current->file_name, current->file_name_new) == -1)
		{
			return errno;
		}
		
		current = current->next;
	}
	
	return 0;
}

time_t get_image_date_time(const char* image)
{
	ExifData *data;
	ExifEntry *entry;
	
	data = exif_data_new_from_file(image);
	
	if ( data != NULL )
	{
		entry = exif_content_get_entry(data->ifd[EXIF_IFD_EXIF], EXIF_TAG_DATE_TIME_DIGITIZED);
		
		if ( entry != NULL )
		{
			char date_time[1024];
			exif_entry_get_value(entry, date_time, 1024);
			
			//http://stackoverflow.com/questions/1002542/how-to-convert-datetime-to-unix-timestamp-in-c
			
			struct tm tm;
			time_t epoch;
			if ( strptime(date_time, "%Y:%m:%d %H:%M:%S", &tm) != NULL )
			{
				epoch = mktime(&tm);
				exif_data_unref(data);
				return epoch;
			}
		}
	}
	
	exif_data_unref(data);
	
	return (time_t)0;
}

#define OPT_DRY_RUN 0x0001
#define OPT_PROMPT_FOR_RENAME_CONFIRMATION 0x0002

#define OPT_VERBOSITY_QUIET 0x0008
#define OPT_VERBOSITY_NORMAL 0x0010
#define OPT_VERBOSITY_VERBOSE 0x0020
#define OPT_USE_COLOUR 0x0040
#define OPT_DONT_USE_COLOUR 0x0080

struct arguments {
	char **files;
	unsigned short int flags;
	unsigned int number_width;
	char separator;
	unsigned int sequence_start;
	char *base_name;
	file_item_t *file_list;
	int file_list_count;
};

error_t parse_options(int key, char *arg, struct argp_state *state)
{
	struct arguments *a = state->input;
	switch (key)
	{
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
		case ARGP_KEY_ARG:
		{
			if (file_exists(arg))
			{
				time_t date_time = get_image_date_time(arg);
				
				if ( date_time  == 0 )
				{
					if (!(a->flags & OPT_VERBOSITY_QUIET))
						print_error("\"%s\": not a JPEG file or no EXIF data.\n", arg);
					break;
				}
				else
				{
					file_item_t *new = (file_item_t*)malloc(sizeof(file_item_t));
					
					new->file_name = arg;
					new->date_time = date_time;
					new->file_name_new = 0;
					new->next = 0;
					
					file_list_add(&a->file_list, new);
					a->file_list_count++;
				}
			}
			else
			{
				argp_failure(state, 1, ENOENT, "\"%s\" was not found, or is not a file.\n", arg);
			}
			
			break;
		}
		case ARGP_KEY_NO_ARGS:
		{
			argp_usage(state);
			break;
		}
		case 'd':
			a->flags |= OPT_DRY_RUN;
			break;
		case 'v':
			a->flags |= OPT_VERBOSITY_VERBOSE;
			break;
		case 'q':
			a->flags |= OPT_VERBOSITY_QUIET;
			break;
		case 'c':
			a->flags |= OPT_PROMPT_FOR_RENAME_CONFIRMATION;
			break;
		case 'b':
		{
			//TODO: This name needs to be sanitised so that it is not illegal
			// windows illegal chars: ^:<>?*|\/
			// mac/*nix illegal chars: :
			// mac/*nix shouldn't start with a .

			if (arg != NULL)
				a->base_name = arg;
			else
				argp_error(state, "Argument not specified for --base, -b option.");
			
			break;
		}
		case 's':
		{
			size_t size = strlen(arg);
			if ( size > 1 )
			{
				argp_error(state, "The --size or -s option's argument was too long (expected 1 character)");
				return 1;
			}
			else if (size == 0)
			{
				argp_error(state, "Argument not specified for --separator, -s option.");
			}
			else
			{
				assert(arg != NULL);
				a->separator = arg[0];
			}
			
			break;
		}
		case 'w':
		{
			if (arg != NULL)
			{
				char *endptr;
				a->number_width = (unsigned int)strtol(arg, &endptr, 10);
				if (endptr[0] != '\0')
				{
					argp_error(state, "--width or -w requires a number. You passed: \"%s\".", endptr);
				}
			}
			else
			{
				argp_error(state, "Argument not specified for the --width, -w option.");
			}
			
			break;
		}
		case 'S':
		{
			if (arg != NULL)
			{
				char *endptr;
				a->sequence_start = (unsigned int)strtol(arg, &endptr, 10);
				if (endptr[0] != '\0')
				{
					argp_error(state, "--sequence or -S requires a number. You passed: \"%s\".", endptr);
				}
			}
			else
			{
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

int main(int argc, char *argv[])
{
	struct arguments arguments;
	struct argp_option options[] = {
		{"base", 'b', "STRING", 0, "String portion of new filenames."},
		{"separator", 's', "CHAR", 0, "Separator for different parts of new filenames."},
		{"width", 'w', "NUM", 0, "Minimum field width for sequence number."},
		{"start", 'S', "NUM", 0, "Starting integer for sequence."},
		//{"quiet", 'q', 0, 0, "Program runs with no output. Implies --noconfirm, -i."},
		{"verbose", 'v', 0, 0, "Program runs with verbose output."},
		{"confirm", 'c', 0, 0, "Prompt for confirmation before renaming a file."},
		//{"noconfirm", 'i', 0, 0, "Don't prompt for comfirmation before renaming a file."},
		{"dry", 'd', 0, 0, "Perform a dry run. Implies --verbose, -v."},
		{0}
	};
	struct argp argp = { options, parse_options, "FILE(S)", "Renames JPG and TIFF files using a base name, a separator, and a zero-padded sequence number. Files are sorted by the EXIF Original Date and Time attribute."};
	
	if (argp_parse(&argp, argc, argv, 0, 0, &arguments) == 0)
	{
		if (arguments.flags & OPT_VERBOSITY_VERBOSE)
			printf("found %d files\n", arguments.file_list_count);
		
		if (arguments.file_list_count > 0)
		{
			file_list_generate_new_filenames(arguments.file_list, arguments.base_name, arguments.separator, arguments.number_width, arguments.sequence_start);
			
			if (arguments.flags & OPT_VERBOSITY_VERBOSE)
				file_list_print(arguments.file_list);
			
			if (!(arguments.flags & OPT_DRY_RUN))
				file_list_rename_files(arguments.file_list, (arguments.flags & OPT_PROMPT_FOR_RENAME_CONFIRMATION));
		}
				
		file_list_destroy(&arguments.file_list);
	}
	else
	{
		if (!(arguments.flags & OPT_VERBOSITY_QUIET))
			print_error("No JPG or TIFF files to work on!\n");
	}
	
    return 0;
}