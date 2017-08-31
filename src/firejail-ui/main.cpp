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
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QLibraryInfo>
#include <QWizard>
#include <sys/utsname.h>

#include "firejail_ui.h"
#include "wizard.h"
#include "../common/utils.h"
#include "../../firetools_config.h"

int arg_debug = 0;
int arg_nofiretools = 0;
int kernel_major;
int kernel_minor;

static void usage() {
	printf("Firejail-ui - Firejail sandbox configuration wizard\n\n");
	printf("Usage: firejail-ui [options]\n\n");
	printf("Options:\n");
	printf("\t--debug - debug mode\n\n");
	printf("\t--help - this help screen\n\n");
	printf("\t--nofiretools - disable stats & tools checkbox\n\n");
	printf("\t--version - print software version and exit\n\n");
}


int main(int argc, char *argv[]) {
	// parse arguments
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--debug") == 0)
			arg_debug = 1;
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0) {
			usage();
			return 0;
		}
		else if (strcmp(argv[i], "--version") == 0) {
			printf("Firejail-ui version " PACKAGE_VERSION "\n");
			return 0;
		}
		else if (strcmp(argv[i], "--nofiretools") == 0) {
			arg_nofiretools = 1;
		}
		else {
			fprintf(stderr, "Error: invalid option\n");
			usage();
			return 1;
		}
	}

	// test run time dependencies - exit
	if (!which("firejail")) {
		fprintf(stderr, "Error: firejail package not found, please install it!\n");
		exit(1);
	}

	// create firetools directory if it doesn't exist
	create_config_directory();

	// read kernel version
	struct utsname u;
	int rv = uname(&u);
	if (rv != 0)
		errExit("uname");
	if (2 != sscanf(u.release, "%d.%d", &kernel_major, &kernel_minor)) {
		fprintf(stderr, "***********************************\n");
		fprintf(stderr, "Warning: cannot extract a sane Linux kernel version: %s.\n", u.version);
		fprintf(stderr, "         Assuming a default version of 3.2. Quite a number of sandboxing\n");
		fprintf(stderr, "         features are disabled.\n");
		fprintf(stderr, "***********************************\n");
	}
	if (kernel_major < 3) {
		fprintf(stderr, "Error: a Linux kernel 3.x or newer is required in order to run Firejail\n");
		exit(1);
	}
	if (arg_debug)
		printf("Linux kernel version %d.%d\n", kernel_major, kernel_minor);
	
	// initialize resources
	//Q_INIT_RESOURCE(firejail-ui);

	QApplication app(argc, argv);
	Wizard wizard;
	wizard.show();
	return app.exec();
}
