/*
 * Copyright (C) 2015-2017 Firetools Authors
 *
 * This file is part of firetools project
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
*/
#ifndef UTILS_H
#define UTILS_H

// run a user program using popen; returns static memory
char *run_program(const char *prog);

// returns true or false if the program was found using "which" shell command
bool which(const char *prog);

// check if a name.desktop file exists in config home directory
bool have_config_file(const char *name);

// get a coniguration file path based on the name; returns allocated memory
char *get_config_file_name(const char *name);

// get the full path of the home directory; returns allocated memory
char *get_home_directory();

// split a line into words
#define SARG_MAX 128
extern int sargc;
extern char *sargv[SARG_MAX];
void split_command(char *cmd);

#endif