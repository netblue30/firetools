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
#include <QLibraryInfo>

#include "../common/utils.h"
#include "../../firetools_config.h"
#include "stats_dialog.h"

int arg_debug = 0;
int svg_not_found = 0;


static void usage() {
printf("todo\n");
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
			printf("Firetools version " PACKAGE_VERSION "\n");
			return 0;
		}
		else {
			fprintf(stderr, "Error: invalid option\n");
			usage();
			return 1;
		}		
	}

	struct stat s;	
#if QT_VERSION >= 0x050000
	// test run time dependencies - print warning and continue program
	QString ppath = QLibraryInfo::location(QLibraryInfo::PluginsPath);
	ppath += "/imageformats/libqsvg.so";
	if (stat(ppath.toUtf8().constData(), &s) == -1) {
		fprintf(stderr, "Warning: QT5 SVG support not installed, please install libqt5svg5 package\n");
		svg_not_found = 1;
	}
#endif
	
	// test run time dependencies - exit
	if (!which("firejail")) {
		fprintf(stderr, "Error: firejail package not found, please install it!\n");
		exit(1);
	}
	
	// create firetools directory if it doesn't exist
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
	
	// initialize resources
	Q_INIT_RESOURCE(firetools);

	QApplication app(argc, argv);
	StatsDialog sd;
	
	// direct all errror to /dev/null to work around this qt bug:
	//      https://bugreports.qt.io/browse/QTBUG-43270
	FILE *rv = NULL;
	if (!arg_debug) {
		rv = freopen( "/dev/null", "w", stderr );
		(void) rv;
	}
	
	// start application
	int tmp = sd.exec();
	(void) tmp;
	
	if (rv)
		fclose(rv);
}

