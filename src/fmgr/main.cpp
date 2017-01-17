/*
 * Copyright (C) 2015-2016 Firetools Authors
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
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>

#include "fmgr.h"
#include "mainwindow.h"
//#include "../common/utils.h"
#include "../../firetools_config.h"

int arg_debug = 0;

static void usage() {
	printf("firemgr - Firejail file manager\n\n");
	printf("Usage: firemgr [options] sandbox-pid\n\n");
	printf("Options:\n");
	printf("\t--debug - debug mode\n\n");
	printf("\t--help - this help screen\n\n");
	printf("\t--version - print software version and exit\n\n");
}

static bool is_pid(const char *str) {
	assert(str != NULL);
	const char *ptr = str;
	
	while (*ptr != '\0') {
		if (!isdigit(*ptr))
			return false;
		ptr++;
	}
	
	int pid = atoi(str);
	if (pid <= 0)
		return false;
	return true;	
}


int main(int argc, char *argv[]) {
	int i;
	
	// parse arguments
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--debug") == 0)
			arg_debug = 1;
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0) {
			usage();
			return 0;
		}
		else if (strcmp(argv[i], "--version") == 0) {
			printf("Firetools version " PACKAGE_VERSION "\n");
			return 0;
		}
		else if (*argv[i] == '-') {
			fprintf(stderr, "Error: invalid option\n");
			usage();
			return 1;
		}		
		else
			break;
	}
	
	// in this moment we should have a pid
	if (i == argc || i != (argc - 1) || !is_pid(argv[i])) {
		fprintf(stderr, "Error: process ID expected\n");
		usage();
		return 1;
	}
	pid_t pid = (pid_t) atoi(argv[i]);

	// initialize resources
	Q_INIT_RESOURCE(fmgr);

	QApplication app(argc, argv);
	MainWindow fm(pid);
	fm.show();
	return app.exec();
}

