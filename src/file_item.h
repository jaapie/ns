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

#ifndef __NS_FILE_ITEM_H_
#define __NS_FILE_ITEM_H_

typedef struct file_item {
    char *file_name;
    char *file_name_new;
    bool collision_avoided;
    time_t date_time;
    struct file_item *next;
} file_item_t;

int file_item_generate_new_filename(file_item_t *item, char *base, char separator, unsigned int width, unsigned int sequence_start, unsigned int sequence_number); /*, bool interactive, int verbosity ); */
int file_item_rename_file( file_item_t *item);
void file_item_print(const file_item_t * restrict item, bool dry_run, int verbosity);

#endif
