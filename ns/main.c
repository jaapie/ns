//
//  main.c
//  ns - Numeical Sequencer
//
//  Created by Jacob Degeling on 9/09/12.
//  Copyright (c) 2012 Jacob Degeling. All rights reserved.
//

#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include <libexif/exif-data.h>

//#include <argp.h>

time_t get_image_datetime(const char* image)
{
	ExifData *data;
	ExifEntry *entry;
	
	data = exif_data_new_from_file(image);
	
	if ( data != NULL )
	{
		entry = exif_content_get_entry(data->ifd[EXIF_IFD_0], EXIF_TAG_DATE_TIME);
		
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
	printf("%s: usage: [OPTIONS] files\n", program_name);
}

void help ()
{
	printf("help\n");
}

void version ()
{
	printf("version\n");
}

int main(int argc, char * const* argv)
{

	char name[64] ="\0";
	int opt = 0;
	int opts_found = 0;

	static struct option options[] =
	{
		{"help", no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 'v'},
		{"name", required_argument, NULL, 'n'},
		{0, 0, 0, 0}
	};
	
	get_program_name(argv, name);
	
	if ( argc <= 1)
	{
		usage(name);
		return 1;
	}
	
	while ((opt = getopt_long(argc, argv, "hvn:", options, NULL)) != -1 )
	{
		switch (opt)
		{
			case 'h':
				help ();
				return 0;
			case 'v':
				version ();
				return 0;
			case 'n':
				printf("name is %s\n", optarg);
				break;
			case '?':
				//printf("Unknown argument \"%c\"\n", (char)optopt);
				usage(name);
				return 1;
			case ':':
				//printf("Missing option argument \"%c\"\n", (char)optopt);
				usage(name);
				return 1;
			default:
				usage(name);
				break;
		}
		
		opts_found++;
	}
	
	printf("Found %d option arguments.\n", opts_found);
	
	if ( optind < argc )
	{
		printf("Non-option arguments:\n");
		while(optind < argc)
		{
			time_t date_time = get_image_datetime(argv[optind]);
			
			if ( date_time  == 0 )
			{
				fprintf(stderr, "Not an JPEG file or no EXIF data.\n");
			}
			else
			{
				printf("%s, %ld\n", argv[optind], date_time );
			}
			
			optind++;
		}
	}
	
    return 0;
}