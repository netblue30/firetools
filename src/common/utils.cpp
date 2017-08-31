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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include "common.h"
#include "utils.h"

#define MAXBUF (1024 * 1024) // 1MB output buffer
static char outbuf[MAXBUF + 1];

// run a user program using popen; returns static memory
char *run_program(const char *prog) {
	// open pipe
	FILE *fp;
	fp = popen(prog, "r");
	if (fp == NULL)
		return NULL;

	// read pipe
	int len;
	char *ptr = outbuf;
	size_t size = MAXBUF;
	while ((len = fread(ptr, 1, size, fp)) > 0) {
		size -= len;
		ptr += len;
		if (size < 80)	// probably not enough room to read a new full line
			break;
	}
	*ptr = '\0';

	pclose(fp);
	return outbuf;
}

// returns true or false if the program was found using "which" shell command
bool which(const char *prog) {
	// build command
	char *cmd;
	if (asprintf(&cmd, "which %s", prog) == -1) {
		perror("asprintf");
		exit(1);
	}
	
	// run command
	char *res = run_program(cmd);
	if (strstr(res, prog))
		return true;
	return false;
}

// check if a name.desktop file exists in config home directory
bool have_config_file(const char *name) {
	assert(name);
	
	// build the full path
	char *path = get_config_file_name(name);
	if (!path)
		return false;
	
	// check file
	struct stat s;
	bool rv = true;
	if (stat(path, &s) == -1)
		rv = false;
	else if (!S_ISREG(s.st_mode))
		rv = false;

	free(path);
	return rv;		
}

// get a coniguration file path based on the name; returns allocated memory
char *get_config_file_name(const char *name) {
	assert(name);

	// build the full path
	char *path;
	char *homedir = get_home_directory();
	if (asprintf(&path, "%s/.config/firetools/%s.desktop", homedir, name) == -1)
		errExit("asprintf");
	free(homedir);
	return path;
}	

// get the full path of the home directory; returns allocated memory
char *get_home_directory() {
	// access account information
	struct passwd *pw = getpwuid(getuid());
	if (!pw)
		errExit("getpwuid");
		
	// extract home directory
	if (pw->pw_dir != NULL) {
		char *homedir = strdup(pw->pw_dir);
		if (!homedir)
			errExit("strdup");
		return homedir;
	}
	
	return 0;
}

// get config directory name; returns allocated memory
char *get_config_directory() {
	char *homedir = get_home_directory();
	if (!homedir)
		return 0;
	
	char *rv;
	if (asprintf(&rv, "%s/.config/firetools", homedir) == -1)
		errExit("asprintf");
	free(homedir);
	return rv;
}

// create firetools directory if it doesn't exist
void create_config_directory() {
	struct stat s;
	char *path;
	char *homedir = get_home_directory();
	if (asprintf(&path, "%s/.config/firetools", homedir) == -1)
		errExit("asprintf");
	free(homedir);
	if (stat(path, &s) == -1) {
		/* coverity[toctou] */
		int rv = mkdir(path, 0755);
		if (rv == -1) {
			fprintf(stderr, "Error: cannot create %s directory\n", path);
			exit(1);
		}
	}
	free(path);
}

int sargc;
char *sargv[SARG_MAX];

void split_command(char *cmd) {
	int i;
	char *ptr = cmd;

	sargc = 0;
	memset(sargv, 0, sizeof(sargv));

	if (!ptr || *ptr == '\0')
		return;

	for (i = 0; i < SARG_MAX; i++) {
		char *start;

		// skip space
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		if (*ptr == '\n' || *ptr == '\r' || *ptr == '\0')
			break;
		start = ptr;

		// advance
		while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r')
			ptr++;
		if (*ptr == '\0') {
			sargv[sargc++] = start;
			break;
		}
		*ptr++ = '\0';
		sargv[sargc++] = start;
	}
}
