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

#ifndef __NS_FILE_H_
#define __NS_FILE_H_

#include "ns.h"

int get_file_name_length(const char *file_spec);
int get_file_name_from_file_spec(const char *file_spec, char *file_name);
int get_path_from_file_spec(char *file_spec, char *path);
char *get_file_ext(const char *filename);
int file_exists(const char *file_name);
char *create_new_file_name(char *path, char *base, char separator, unsigned int width, unsigned int sequence, char *ext, bool *collision_avoided);

#endif
