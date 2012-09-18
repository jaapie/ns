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
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <error.h>
#include <errno.h>
#include <libexif/exif-data.h>
#include <sys/stat.h>

void print_error(const char * prog_name, const char* fmt, ...)
{
	va_list list;
	char buf[512];
	
	va_start(list, fmt);
	vsprintf(buf, fmt, list);
	va_end(list);
	
	fprintf(stderr, "%s: error: %s", prog_name, buf);
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
} file_item_t;

file_item_t *file_list_create(int items)
{
	assert(items > 0);
	if ( items > 0 )
	{
		file_item_t *list = malloc(items * sizeof(file_item_t));
		assert(list!=NULL);
		return list;
	}
	
	return NULL;
}

void file_list_destroy(file_item_t* list, int items)
{
	int i;
	
	assert(items > 0);
	assert(list != NULL);
	
	if( list != NULL && items > 0 )
	{
		for ( i = 0; i < items; i++ )
		{
			//assert(list[i].file_name != NULL);
			if ( list[i].file_name != NULL )
				free(list[i].file_name);
			
			if ( list[i].file_name_new != NULL)
				free(list[i].file_name_new);
		}
		
		free(list);
	}
}

void file_list_print(file_item_t *list, int items)
{
	assert(list != NULL);
	assert( items > 0);
	
	if (list == NULL || items <= 0)
		return;
	
	int i;
	
	for (i = 0; i <items; i++)
	{
		if (list[i].date_time != 0)
			printf("%d) %s, %ld -> %s\n", i + 1, list[i].file_name, list[i].date_time, list[i].file_name_new );
		else
			fprintf(stderr, "File list item %d is empty.\n", i);
	}
}

int compare_file_items(const void *item1, const void *item2)
{
	file_item_t *fi1 = (file_item_t*)item1;
	file_item_t *fi2 = (file_item_t*)item2;
	
	if ( fi1->date_time > fi2->date_time)
		return 1;
	
	if ( fi1->date_time < fi2->date_time)
		return -1;
	
	return 0;
}

void file_list_sort(file_item_t *list, int items)
{
	assert(items > 0);
	assert(list != NULL);
	
	if( list != NULL && items > 0 )
	{
		qsort(list, items, sizeof(file_item_t), compare_file_items);
	}
}

char *create_new_file_name( char *base, char separator, unsigned int width, unsigned int sequence, char *ext )
{
	char *new_name;
	unsigned int size = 0;
	
	assert(base != NULL);
	assert(ext != NULL);
	
	size = (unsigned int)(strlen(base) + 1 + width + 1 + strlen(ext) + 1);
	
	new_name = malloc(size);
	
	assert(new_name != NULL);
	
	if ( new_name != NULL )
	{
		char *padded_number = get_zero_padded_number(sequence, width);
		sprintf(new_name, "%s%c%s.%s", base, separator, padded_number, ext);
		free(padded_number);
		
		return new_name;
	}
	else
		return NULL;
}

void file_list_generate_new_filenames(file_item_t *list, int items, char *base, char separator, unsigned int width, unsigned int sequence_start )
{
	int i;
	char *ext;
	
	assert(list != NULL);
	assert(base != NULL);
	assert(items > 0);
	
	for ( i = 0; i < items; i++)
	{
		assert(list[i].file_name != NULL);
		ext = get_file_ext(list[i].file_name);
		assert(strlen(ext) > 0);
		assert(strcmp(ext, "") != 0);
		list[i].file_name_new = create_new_file_name(base, separator, width, sequence_start + (unsigned int)i, ext);
	}
}

int file_list_rename_files( file_item_t *list, int items, int prompt_for_confirmation)
{
	int i;
	assert(list != NULL);
	assert(items > 0);
	
	if (list == NULL || items <= 0)
		return -1;
	
	for ( i = 0; i < items; i++)
	{
		if ( rename(list[i].file_name, list[i].file_name_new) == -1)
		{
			return errno;
		}
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

void get_program_name(char * const* argv, char *program_name)
{
	assert( argv != NULL);
	assert( *argv != NULL);
	assert(program_name != NULL);
	
	if ( argv == NULL || *argv == NULL || program_name == NULL )
		return;
	else
	{
		char *prog = rindex(argv[0], '/');
		strncpy(program_name, &prog[1], strlen(&prog[1]));
	}
}

void usage( const char *program_name )
{
	fprintf(stderr, "%s: usage: [OPTIONS] files\n", program_name);
}

void help ()
{
	printf("help\n");
}

void version ()
{
	printf("Numerical Sequencer version 0.1\n");
}

int main(int argc, char * const* argv)
{
	char name[64] ="ns\0";
	int opt = 0;
	int i;
	int opts_found = 0;
	file_item_t *file_list = NULL;
	int file_list_count= 0;
	char *base_name = NULL;
	unsigned int number_width = 4;
	char separator = '_';
	unsigned int sequence_start = 1;
	static int dry_run;
	unsigned int actual_file_count = 0;

	static struct option options[] =
	{
		{"help", no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 'v'},
		{"base", required_argument, NULL, 'b'},
		{"separator", required_argument, NULL, 's'},
		{"width", required_argument, NULL, 'w'},
		{"start", required_argument, NULL, 'S'},
		{"dry", no_argument, &dry_run, 1 },
		{0, 0, 0, 0}
	};
	
	get_program_name(argv, name);
	
	if ( argc <= 1)
	{
		usage(name);
		return 1;
	}
	
	while ((opt = getopt_long(argc, argv, "hvb:s:w:S:d", options, NULL)) != -1 )
	{
		switch (opt)
		{
			case 'h':
				help ();
				return 0;
			case 'v':
				version ();
				return 0;
			case 'd':
				dry_run = 1;
				break;
			case 'b':
			{
				size_t size = strlen(optarg);
				assert(size > 0);
				if ( size > 0 )
				{
					base_name = malloc(size + 1);
					assert(base_name != NULL);
					if ( base_name != NULL)
					{
						//TODO: This name needs to be sanitised so that it is not illegal
						// windows illegal chars: ^:<>?*|\/
						// mac/*nix illegal chars: :
						// mac/*nix shouldn't start with a .
						strncpy(base_name, optarg, size + 1);
					}
					else
					{
						print_error(name, "Unable to allocate memory for base file name\n");
						return 1;
					}
				}
				else
				{
					print_error(name, "Unable to determine length of base file name\n");
					return 1;
				}
				
				break;
			}
			case 's':
			{
				size_t size = strlen(optarg);
				if ( size > 1 )
				{
					print_error(name, "The --size or -s option's argument was too long (expected 1 character)\n");
					return 1;
				}
				else
				{
					separator = optarg[0];
				}
				break;
			}
			case 'w':
			{
				char *endptr;
				number_width = (unsigned int)strtol(optarg, &endptr, 10);
				if (endptr[0] != '\0')
				{
					print_error(name, "--width or -w requires a number. You passed: \"%s\".\n", endptr);
					return 1;
				}
				
				break;
			}
			case 'S':
			{
				char *endptr;
				sequence_start = (unsigned int)strtol(optarg, &endptr, 10);
				if (endptr[0] != '\0')
				{
					print_error(name, "--sequence or -S requires a number. You passed: \"%s\".\n", endptr);
					return 1;
				}
				
				break;
			}
			case '?':
				print_error(name, "Unknown argument \"%c\"\n", (char)optopt);
				usage(name);
				return 1;
			case ':':
				print_error(name, "Missing option argument \"%c\"\n", (char)optopt);
				usage(name);
				return 1;
			default:
				usage(name);
				return 1;
		}
		
		opts_found++;
	}
	
	if ( optind < argc )
	{
		file_list_count = argc - optind;
		file_list = file_list_create(file_list_count);
		
		for (i = 0; i < file_list_count; i++)
		{
			if (file_exists(argv[optind]) == 1)
			{
				time_t date_time = get_image_date_time(argv[optind]);
				
				if ( date_time  == 0 )
				{
					print_error(name, "\"%s\": not a JPEG file or no EXIF data.\n", argv[optind]);
				}
				else
				{
					size_t size = strlen(argv[optind]) + 1;
					
					file_list[i].file_name = (char*)malloc(size);
					assert(file_list[i].file_name != NULL);
					strncpy(file_list[i].file_name, argv[optind], size);
					
					//printf("%s -> %s\n", argv[optind], file_list[i].file_name);
					
					file_list[i].date_time = date_time;
					
					actual_file_count++;
				}
			}
			else
			{
				print_error(name, "%s is not a file\n", argv[optind]);
			}
			
			optind++;
		}
		if (actual_file_count > 0)
		{
			file_list_sort(file_list, file_list_count);
			file_list_generate_new_filenames(file_list, file_list_count, base_name, separator, number_width, sequence_start);
			file_list_print(file_list, file_list_count);
		}
		
		file_list_destroy(file_list, file_list_count);
	}
	else
	{
		print_error(name, "No JPG or TIFF files to work on!\n");
	}
	
	if ( base_name != NULL )
		free(base_name);
	
    return 0;
}