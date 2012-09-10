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

//#include <argp.h>

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
			printf("%s\n", argv[optind++]);
	}
	
    return 0;
}