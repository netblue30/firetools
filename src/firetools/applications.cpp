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
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "firetools.h"
#include "applications.h"
#include "../common/utils.h"
#include "../../firetools_config_extras.h"
#include <QDirIterator>
#include <QPainter>
QList<Application> applist;

Application::Application(const char *name, const char *description, const char *icon, const char *exec):
	name_(name), description_(description), icon_(icon), exec_(exec) {

	app_icon_ = loadIcon(icon_);
	if (exec == NULL || strlen(exec) == 0)
		exec_ = QString("firejail ") + name;
};

Application::Application(QString name, QString description, QString icon, QString exec):
	name_(name), description_(description), icon_(icon), exec_(exec) {
	app_icon_ = loadIcon(icon_);
};

// Load an application from a desktop file
Application::Application(const char *name):
	name_(name), description_("unknown"),icon_("unknown"),  exec_("unknown") {

	// retrieve desktop file
	if (!have_config_file(name))
		return;
	char *fname = get_config_file_name(name);
	if (!fname)
		return;

	if (arg_debug)
		printf("loading %s\n", fname);

	// open file
	FILE *fp = fopen(fname, "r");
	if (!fp) {
		free(fname);
		return;
	}
	free(fname);

	// read file
#define MAXBUF 10000
	char buf[MAXBUF];
	while (fgets(buf, MAXBUF, fp)) {
		// remove '\n'
		char *ptr = strchr(buf, '\n');
		if (ptr)
			*ptr = '\0';

		// skip blancs
		char *start = buf;
		while (*start == ' ' || *start == '\t')
			start++;

		// parse
		if (strncmp(buf, "Comment=", 8) == 0)
				description_ = buf + 8;
		else if (strncmp(buf, "Exec=", 5) == 0)
				exec_ = buf + 5;
		else if (strncmp(buf, "Icon=", 5) == 0)
				icon_ = buf + 5;
	}
	fclose(fp);

	app_icon_ = loadIcon(icon_);
}

// Save the app's configuration
int Application::saveConfig() {
	char *fname = get_config_file_name(name_.toLocal8Bit().constData());
	if (!fname)
		return 1;

	// Open a file
	FILE *fp = fopen(fname, "w");
	if (!fp) {
		free(fname);
		return 1;
	}
	free(fname);

	fprintf(fp, "[Desktop Entry]\n");
	fprintf(fp, "Name=%s\n", name_.toLocal8Bit().constData());
	fprintf(fp, "Comment=%s\n", description_.toLocal8Bit().constData());
	fprintf(fp, "Icon=%s\n", icon_.toLocal8Bit().constData());
	fprintf(fp, "Exec=%s\n", exec_.toLocal8Bit().constData());
	fclose(fp);

	return 0;
}

/*
From: http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html

Icons and themes are looked for in a set of directories. By default, apps should look
in $HOME/.icons (for backwards compatibility), in $XDG_DATA_DIRS/icons and in /
usr/share/pixmaps (in that order). Applications may further add their own icon
directories to this list, and users may extend or change the list (in application/desktop
specific ways).In each of these directories themes are stored as subdirectories.
A theme can be spread across several base directories by having subdirectories of
the same name. This way users can extend and override system themes.

In order to have a place for third party applications to install their icons there
should always exist a theme called "hicolor" [1]. The data for the hicolor theme is
available for download at: http://www.freedesktop.org/software/icon-theme/. I
mplementations are required to look in the "hicolor" theme if an icon was not found
in the current theme.
*/

// compare strings
static inline bool compare_ignore_case(QString q1, QString q2) {
	q1 = q1.toLower();
	q2 = q2.toLower();
	return q1 == q2;
}

static QString walk(QString path, QString name) {
	QDirIterator it(path, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QFileInfo fi = it.fileInfo();
		if (fi.isFile() && compare_ignore_case(fi.baseName(), name)) {
			if (arg_debug)
				printf("\t- %s\n", fi.canonicalFilePath().toUtf8().data());
			return fi.canonicalFilePath();
		}
	}
	return QString("");
}

static QIcon resize48x48(QIcon icon) {
	QSize sz = icon.actualSize(QSize(64, 64));
	if (arg_debug)
		printf("\t- input pixmap: w %d, h %d\n", sz.width(), sz.height());

	QPixmap pix = icon.pixmap(sz.height(), sz.width());
	QPixmap pixin;
	int delta = 0;

	if (sz.height() ==  sz.width() && sz.height() <= 40) {
		pixin = pix.scaled(40, 40);
		delta = 12;
	}
	else {
		pixin = pix.scaled(48, 48);
		delta = 8;
	}


	QPixmap pixout(64, 64);
	pixout.fill(QColor(0, 0, 0, 0));
	QPainter *paint = new QPainter(&pixout);
	paint->drawPixmap(delta, delta, pixin);
	if (arg_debug)
		printf("\t- output pixmap: w %d, h %d\n", pixout.width(), pixout.height());
	paint->end();
	return QIcon(pixout);
}

QIcon Application::loadIcon(QString name) {
	if (arg_debug)
		printf("searching icon %s\n", name.toLocal8Bit().data());

	if (name == ":resources/fstats" || name == ":resources/firejail-ui") {
		if (arg_debug)
			printf("\t- resource\n");
		return QIcon(name); // not resized, using the real 64x64 size
	}

	if (name.startsWith(":resources")) {
		if (arg_debug)
			printf("\t- resource\n");
		return resize48x48(QIcon(name));
	}

	if (name.startsWith('/')) {
		if (arg_debug)
			printf("\t- full path\n");
		return resize48x48(QIcon(name));
	}



	// Look for the file in Firejail config directory under /home/user
	QString conf = QDir::homePath() + "/.config/firetools/" + name + ".png";
	QFileInfo checkFile1(conf);
	if (checkFile1.exists() && checkFile1.isFile()) {
		if (arg_debug)
			printf("\t- local config dir, png file\n");
		return QIcon(conf);
	}
	conf = QDir::homePath() + "/.config/firetools/" + name + ".jpg";
	QFileInfo checkFile2(conf);
	if (checkFile2.exists() && checkFile2.isFile()) {
		if (arg_debug)
			printf("\t- local config dir, jpg file\n");
		return QIcon(conf);
	}

	if (!svg_not_found) {
		conf = QDir::homePath() + "/.config/firetools/" + name + ".svg";
		QFileInfo checkFile3(conf);
		if (checkFile3.exists() && checkFile3.isFile()) {
			if (arg_debug)
				printf("\t- local config dir, svg file\n");
			return QIcon(conf);
		}
	}

	if (QIcon::hasThemeIcon(name)) {
		if (arg_debug)
			printf("\t- fromTheme\n");
		return resize48x48(QIcon::fromTheme(name));
	}

	{
		QString qstr = walk("/usr/share/icons", name);
		if (!qstr.isEmpty()) {
			return resize48x48(QIcon(qstr));
		}
	}

	{
		QDirIterator it("/usr/share/pixmaps", QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QFileInfo fi = it.fileInfo();
			if (fi.isFile() && compare_ignore_case(fi.baseName(), name)) {
				if (arg_debug)
					printf("\t- /usr/share/pixmaps\n");
				QIcon icon = QIcon(fi.canonicalFilePath());
				return resize48x48(icon);
			}
		}
	}

	// Create a new icon
	if (arg_debug)
		printf("\t- created\n");

	// Create a new QPixmap instance for icons
	QPixmap pix(64, 64);

	// Set the background color for generated icons
	QColor iconBackgroundColor(68, 68, 68);

	// Fill the icon with a color
	pix.fill(iconBackgroundColor);


	// Create a QPainter instance
	QPainter painter( &pix );

	// Set color and font for the painter
	painter.setPen(Qt::white);
	painter.setFont(QFont("Sans"));

	// Draw application's name to the icon
	painter.drawText(3, 20, name);
	painter.end();

	// Use generated pixmap as an icon
	QIcon icon(pix);

	return icon;
}



bool applist_check(QString name) {
	QList<Application>::iterator it;
	for (it = applist.begin(); it != applist.end(); ++it) {
		if (it->name_ == name)
			return true;
	}

	return false;
}


void applist_print() {
	QList<Application>::iterator it;
	for (it = applist.begin(); it != applist.end(); ++it)
		printf("\t%s\n", it->name_.toLocal8Bit().constData());
}


void applications_init() {
	// load default apps
	if (arg_debug)
		printf("Loading default applications\n");


	FILE *fp = fopen( PACKAGE_LIBDIR "/uiapps", "r");
	if (!fp) {
		fprintf(stderr, "Error: cannot find the application list; Firetools was not properly installed\n");
		exit(1);
	}

	char buf[1024];
	int line = 0;
	while (fgets(buf, sizeof(buf), fp)) {
		line++;

		// comment
		if (*buf == '#')
			continue;
		char *ptr = strchr(buf, '\n');
		if (ptr)
			*ptr = '\0';
		char *name = buf;
		while (*name == ' ' || *name == '\t')
			name++;
		if (*name == '\0')
			continue;
		ptr = strchr(name, ';');
		if (!ptr) {
			fprintf(stderr, "Error: invalid line %d in %s\n", line, PACKAGE_LIBDIR "/uiapps");
			exit(1);
		}
		*ptr++ = '\0';
		char *description = ptr;
		ptr = strchr(description, ';');
		if (!ptr) {
			fprintf(stderr, "Error: invalid line %d in %s\n", line, PACKAGE_LIBDIR "/uiapps");
			exit(1);
		}
		*ptr++ = '\0';
		char *icon = ptr;
		char *command = NULL;
		ptr = strchr(icon, ';');
		if (ptr) {
			*ptr++ = '\0';
			command = ptr;
		}
		if (arg_debug) {
			printf("checking #%s#%s#%s#%s\n",
				name, description, icon, (command)? command: "");
		}

		// do we have the program?
		if (which(name) == false)
			continue;

		if (command && strncmp(command, "PACKAGE_LIBDIR", 14) == 0) {
			char *newcmd;
			if (asprintf(&newcmd, PACKAGE_LIBDIR "%s", command + 14) == -1)
				errExit("asprintf");
			command = newcmd;
		}

		// is there a user config file?
		if (have_config_file(name))
			applist.append(Application(name));
		else
			applist.append(Application(name, description, icon, command));
	}
	fclose(fp);

	// load user apps from home directory
	char *home = get_home_directory();
	if (!home)
		return;
	char *homecfg;
	if (asprintf(&homecfg, "%s/.config/firetools", home) == -1)
		errExit("asprintf");
	free(home);
	DIR *dir = opendir(homecfg);
	if (!dir) {
		free(homecfg);
		return;
	}

	// walk home config directory
	struct dirent *entry;
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;

		// look only at .desktop files
		int len = strlen(entry->d_name);
		if (len <= 8)
			continue;
		char *fname = strdup(entry->d_name);
		if (!fname)
			errExit("strdup");
		char *ending = fname + len - 8;
		if (strcmp(ending, ".desktop") != 0) {
			free(fname);
			continue;
		}

		// check if the app is in default list
		fflush(0);
		*ending = '\0';
#if 0
		DefaultApp *app = &dapps[0];
		bool found = false;
		while (app->name != 0) {
			if (strcmp(fname, app->name) == 0) {
				found = true;
			}

			app++;
		}
		if (found) {
			free(fname);
			continue;
		}
#endif
		// load file
		applist.append(Application(fname));
		free(fname);
	}

	free(homecfg);
	closedir(dir);
}


int applications_get_index(QPoint pos) {
	int nelem = applist.count();
	int cols = nelem / ROWS + 1;

	if (pos.y() < (MARGIN * 2 + TOP))
		return -1;

	if (pos.x() > (MARGIN * 2) && pos.x() < (MARGIN * 2 + cols * 64)) {
		int index_y = (pos.y() - 2 * MARGIN - TOP) / 64;
		int index_x = (pos.x() - 2 * MARGIN) / 64;
		int index = index_y + index_x * ROWS;

		if (index < nelem)
			return index;
	}
	return -1;
}

int applications_get_position(QPoint pos) {
	int nelem = applist.count();
	int cols = nelem / ROWS + 1;

	if (pos.y() < (MARGIN * 2 + TOP))
		return -1;

	if (pos.x() > (MARGIN * 2) && pos.x() < (MARGIN * 2 + cols * 64)) {
		int index_y = (pos.y() - 2 * MARGIN - TOP) / 64;
		int index_x = (pos.x() - 2 * MARGIN) / 64;
		int index = index_y + index_x * ROWS;

//		if (index < nelem)
			return index;
	}
	return -1;
}
