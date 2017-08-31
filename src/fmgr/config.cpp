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
#include "fmgr.h"
#include "../common/utils.h"

#define DEFAULT_X_SIZE 500
#define DEFAULT_Y_SIZE 500
#define MINSIZE 500
#define BUFSIZE 4096

void config_read_screen_size(int *x, int *y) {
	// set defaults
	*x = DEFAULT_X_SIZE;
	*y = DEFAULT_X_SIZE;

	// open config file
	char *cfgdir = get_config_directory();
	if (!cfgdir)
		return;
	char *fname;
	if (asprintf(&fname, "%s/fmgr.config", cfgdir) == -1)
		errExit("asprintf");
	FILE *fp = fopen(fname, "r");
	free(fname);
	if (!fp)
		return;
	
	// read file and parse it
	char buf[BUFSIZE];
	while (fgets(buf, BUFSIZE, fp)) {
		char *ptr = buf;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		if (strncmp(ptr, "x ", 2) == 0) {
			ptr += 2;
			if (sscanf(ptr, "%d", x) != 1) {
				fprintf(stderr, "Error: invalid X size in ~/.config/firetools/fmgr.config\n");
				return;
			}
		}
		else if (strncmp(ptr, "y ", 2) == 0) {
			ptr += 2;
			if (sscanf(ptr, "%d", y) != 1) {
				fprintf(stderr, "Error: invalid Y size in ~/.config/firetools/fmgr.config\n");
				return;
			}
		}
	}
	fclose(fp);
}

void config_write_screen_size(int x, int y) {
	x = (x < MINSIZE)? DEFAULT_X_SIZE: x;
	y = (y < MINSIZE)? DEFAULT_Y_SIZE: y;
	
	// open config file
	char *cfgdir = get_config_directory();
	if (!cfgdir)
		return;
	char *fname;
	if (asprintf(&fname, "%s/fmgr.config", cfgdir) == -1)
		errExit("asprintf");
	FILE *fp = fopen(fname, "w");
	free(fname);
	if (!fp)
		return;

	// write file
	fprintf(fp, "x %d\n", x);
	fprintf(fp, "y %d\n", y);
	fclose(fp);
}
