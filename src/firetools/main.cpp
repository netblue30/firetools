/*
 * Copyright (C) 2015-2018 Firetools Authors
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

#include "firetools.h"
#include "mainwindow.h"
#include "../common/utils.h"
#include "../../firetools_config.h"

int arg_debug = 0;
int svg_not_found = 0;

// desktop file content for autostart
static const char *deskfile_minimize = 
"[Desktop Entry]\n"
"Name=Firetools\n"
"Comment=Firejail tools & stats\n"
"Exec=firetools --minimize\n"
"Icon=firetools\n"
"Terminal=false\n"
"Type=Application\n"
"Categories=Qt;System;Security;\n";


// Usage instructions for the command line output
static void usage() {
	printf("firetools - Firejail tools and stats utility\n\n");
	printf("Usage: firetools [options]\n\n");
	printf("Options:\n");
	printf("\t--autostart - configure firetools to run automatically in system tray\n");
	printf("\t\twhen X11 session is started\n\n");
	printf("\t--debug - debug mode\n\n");
	printf("\t--help - this help screen\n\n");
	printf("\t--minimize - start the program minimized in system tray\n\n");
	printf("\t--version - print software version and exit\n\n");
}

int main(int argc, char *argv[]) {
	int arg_minimize = 0;

	// Parse arguments
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
		else if (strcmp(argv[i], "--autostart") == 0) {
			// Find home directory
			char *home = get_home_directory();
			if (!home) {
				fprintf(stderr, "Error: cannot find user home directory");
				return 1;
			}
			
			// Create a .config/autostart directory if it doesn't exist
			char *autodir;
			if (asprintf(&autodir, "%s/.config/autostart", home) == -1)
				errExit("asprintf");
			int rv = mkdir(autodir, 0755);
			(void) rv;
			
			// Create desktop file
			char *autofile;
			if (asprintf(&autofile, "%s/.config/autostart/firetools.desktop", home) == -1)
				errExit("asprintf");
			FILE *fp = fopen(autofile, "w");
			if (!fp) {
				fprintf(stderr, "Error: cannot open %s\n", autofile);
				return 1;
			}
			fprintf(fp, "%s\n", deskfile_minimize);
			fclose(fp);
			printf("firetools autostart configured\n");
			return 0;
		}
		else if (strcmp(argv[i], "--minimize") == 0)
			arg_minimize = 1;
		else {
			fprintf(stderr, "Error: invalid option\n");
			usage();
			return 1;
		}		
	}

#if QT_VERSION >= 0x050000
	struct stat s;	
	// Test run time dependencies - print warning and continue program
	QString ppath = QLibraryInfo::location(QLibraryInfo::PluginsPath);
	ppath += "/imageformats/libqsvg.so";
	if (stat(ppath.toUtf8().constData(), &s) == -1) {
		fprintf(stderr, "Warning: QT5 SVG support not installed, please install libqt5svg5 package\n");
		svg_not_found = 1;
	}
#endif
	
	// Test run time dependencies - exit
	if (!which("firejail")) {
		fprintf(stderr, "Error: firejail package not found, please install it!\n");
		exit(1);
	}
	
	// Create firetools directory if it doesn't exist
	create_config_directory();
	
	// Initialize resources
	Q_INIT_RESOURCE(firetools);

	QApplication app(argc, argv);
	MainWindow fc;
	if (!arg_minimize)
		fc.show();

	// Configure system tray
	QSystemTrayIcon icon(QIcon(":resources/firetools-minimal.png"));
	icon.show();	
	icon.setToolTip("Firetools (click to open)");
	QMenu *trayIconMenu = new QMenu(&fc);
	trayIconMenu->addAction(fc.minimizeAction);
	trayIconMenu->addAction(fc.restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(fc.quitAction);
	icon.setContextMenu(trayIconMenu);
	icon.connect(&icon, SIGNAL(activated(QSystemTrayIcon: :ActivationReason)), &fc, SLOT(trayActivated(QSystemTrayIcon: :ActivationReason)));
	
	// Direct all errror to /dev/null to work around this qt bug:
	//      https://bugreports.qt.io/browse/QTBUG-43270
	FILE *rv = NULL;
	if (!arg_debug) {
		rv = freopen( "/dev/null", "w", stderr );
		(void) rv;
	}
	
	// Start application
	int tmp = app.exec();
	(void) tmp;
	
	if (rv)
		fclose(rv);
}

