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

Application::Application(const char *name, const char *description, const char *exec, const char *icon):
	name_(name), description_(description), exec_(exec), icon_(icon) {
	
	app_icon_ = loadIcon(icon_);
};

Application::Application(QString name, QString description, QString exec, QString icon):
	name_(name), description_(description), exec_(exec), icon_(icon) {
	
	app_icon_ = loadIcon(icon_);
};

// load an application from a desktop file
Application::Application(const char *name):
	name_(name), description_("unknown"), exec_("unknown"), icon_("unknown") {

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

int Application::saveConfig() {
	char *fname = get_config_file_name(name_.toLocal8Bit().constData());
	if (!fname)
		return 1;
	
	// open file
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
QIcon Application::loadIcon(QString name) {
	if (arg_debug)
		printf("searching icon %s\n", name.toLocal8Bit().data());
	if (name.startsWith('/') || name.startsWith(":resources")) {
		if (arg_debug)
			printf("\t- full path\n");
		return QIcon(name);
	}

	// look for the file in firejail config directory under /home/user
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

		QDirIterator it("/usr/share/icons/hicolor/scalable", QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QFileInfo fi = it.fileInfo();
			if (fi.isFile() && fi.baseName() == name) {
				if (arg_debug)
					printf("\t- svg\n");
				return QIcon(fi.canonicalFilePath());
			}
		}
	}

	{
		QDirIterator it("/usr/share/icons/hicolor/64x64", QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QFileInfo fi = it.fileInfo();
			if (fi.isFile() && fi.baseName() == name) {
				if (arg_debug)
					printf("\t- /usr/share/icons/hicolor/64x64\n");
				return QIcon(fi.canonicalFilePath());
			}
		}
	}
	
	{
		QDirIterator it("/usr/share/icons/hicolor/128x128", QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QFileInfo fi = it.fileInfo();
			if (fi.isFile() && fi.baseName() == name) {
				if (arg_debug)
					printf("\t- /usr/share/icons/hicolor/128x128\n");
				return QIcon(fi.canonicalFilePath());
			}
		}
	}
	
	{
		QDirIterator it("/usr/share/icons/hicolor/256x256", QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QFileInfo fi = it.fileInfo();
			if (fi.isFile() && fi.baseName() == name) {
				if (arg_debug)
					printf("\t- /usr/share/icons/hicolor/256x256\n");
				return QIcon(fi.canonicalFilePath());
			}
		}
	}

	{	
		QDirIterator it("/usr/share/pixmaps", QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QFileInfo fi = it.fileInfo();
			if (fi.isFile() && fi.baseName() == name) {
				if (arg_debug)
					printf("\t- /usr/share/pixmaps\n");
				QIcon icon = QIcon(fi.canonicalFilePath());
//#if 0 // scale				
				QSize sz = icon.actualSize(QSize(64, 64));
				if (sz.height() < 64 && sz.width() < 64) {
					QPixmap pix = icon.pixmap(sz.height(), sz.width());
					QPixmap newpix = pix.scaled(50, 50);
					return QIcon(newpix);
				}
//#endif
				return icon;
				
			}
		}
	}	
	
	if (QIcon::hasThemeIcon(name)) {
		if (arg_debug)
			printf("\t- fromTheme\n");
		return QIcon::fromTheme(name);
	}

	// hardcoded icons
	if (name == "firefox" || name == "firefox-esr" || name == "iceweasel" || name == "icecat") {
		if (arg_debug)
			printf("\t- default icon\n");
		return QIcon(":resources/firefox.png");
	}
	else if (name == "icedove" || name == "thunderbird") {
		if (arg_debug)
			printf("\t- default icon\n");
		return QIcon(":resources/icedove.png");
	}		
	else if (name == "calibre-gui") {
		if (arg_debug)
			printf("\t- default icon\n");
		return QIcon(":resources/calibre.png");
	}		
	
	// create a new icon
	if (arg_debug)
		printf("\t- created\n");
	QPixmap pix(64, 64);
	pix.fill(Qt::red);
	QPainter painter( &pix );
	painter.setPen(Qt::white);
	painter.setFont(QFont("Sans"));
	painter.drawText(3, 20, name);
	painter.end();
	QIcon icon(pix);
	return icon;
}


struct DefaultApp {
	const char *name;
	const char *alias;
	const char *description;
	const char *command;
	const char *icon;
};

DefaultApp dapps[] = {
	// firetools
	{ "firetools", "", "Firetools", PACKAGE_LIBDIR "/fstats", ":resources/fstats" },
	{ "firejail-ui", "", "Firejail Configuration Wizard", "firejail-ui", "firejail-ui" },
	
	// browser
	{ "iceweasel", "", "Debian Iceweasel", "firejail iceweasel", ":resources/firefox.png" },
	{ "firefox", "iceweasel", "Mozilla Firefox", "firejail firefox", "firefox"},
	{ "icecat", "firefox", "GNU IceCat", "firejail icecat", "icecat"},
	{ "chromium", "", "Chromium Web Browser", "firejail chromium", "chromium"},
	{ "chromium-browser", "chromium", "Chromium Web Browser", "firejail chromium-browser", "chromium-browser"},
	{ "midori", "", "Midori Web Browser", "firejail midori", "midori" },
	{ "opera", "", "Opera Web Browser", "firejail opera", "opera" },
	{ "netsurf", "", "Netsurf Web Browser", "firejail netsurf", "netsurf" },

	// mail
	{ "icedove", "", "Debian Icedove", "firejail icedove", ":resources/icedove.png" },
	{ "thunderbird", "icedove","Thunderbird", "firejail thunderbird", "thunderbird" },

	// bittorrent
	{ "transmission-gtk", "", "Transmission BitTorrent Client", "firejail transmission-gtk", "transmission" },
	{ "transmission-qt", "transmission-gtk", "Transmission BitTorrent Client", "firejail transmission-qt", "transmission" },
	{ "deluge", "", "Deluge BitTorrent Client", "firejail deluge", "deluge" },
	{ "qbittorrent", "", "qBittorrent Client", "firejail qbittorrent", "qbittorrent" },

	// viewers
	{ "evince", "", "Evince PDF viewer", "firejail evince", "evince" },
	{ "qpdfview", "", "qPDFView", "firejail qpdfview", "qpdfview" },
	{ "xpdf", "", "Xpdf", "firejail xpdf", "xpdf" },
	{ "fbreader", "", "eBook reader", "firejail fbreader", "FBReader" },
	{ "cherrytree", "", "CherryTree note taking application", "firejail cherrytree", "cherrytree"},
	{ "gpicview", "", "GPicView", "firejail gpicview", "gpicview" },
	{ "gthumb", "", "gThumbView", "firejail gthumb", "gthumb" },
	{ "okular", "", "Okular", "firejail okular", "okular" },
	{ "eom", "", "Eye of MATE", "firejail eom", "eom" },
	{ "eog", "", "Eye of Gnome", "firejail eog", "eog" },
	{ "gwenview", "", "Gwenview", "firejail gwenview", "gwenview" },
	{ "calibre", "", "Calibre eBook reader", "firejail calibre", "calibre-gui" },

	// media player
	{ "vlc", "", "VideoLAN Client", "firejail vlc", "vlc" },
	{ "rhythmbox", "", "Rhythmbox", "firejail rhythmbox", "rhythmbox" },
	{ "totem", "", "Totem", "firejail totem", "totem" },
	{ "audacious", "", "Audacious", "firejail audacious", "audacious" },
	{ "gnome-mplayer", "", "GNOME MPlayer", "firejail gnome-mplayer", "gnome-mplayer" },
	{ "clementine", "", "Clementine", "firejail clementine", "application-x-clementine" },
	{ "deadbeef", "", "DeaDBeeF", "firejail deadbeef", "deadbeef" },
	{ "mpv", "", "MPV", "firejail mpv", "mpv" },
	{ "smplayer", "", "SMPlayer", "firejail smplayer", "smplayer" },
	{ "kino", "", "Kino", "firejail kino", "kino" },

	// editor
	{ "gimp", "", "Gimp", "firejail gimp", "gimp" },
	{ "inkscape", "", "Inkscape", "firejail inkscape", "inkscape" },
	{ "openshot", "", "OpenShot video editor", "firejail openshot", "openshot" },


	// chat
	{ "pidgin", "", "Pidgin", "firejail pidgin", "pidgin" },
	{ "xchat", "", "XChat", "firejail xchat", "xchat" },
	{ "hexchat", "", "HexChat", "firejail hexchat", "hexchat" },
	{ "quassel", "", "Quassel IRC", "firejail quassel", "quassel" },
	{ "empathy", "", "Empathy", "firejail empathy", "empathy" },
	
	// etc
	{ "filezilla", "", "FileZilla", "firejail filezilla", "filezilla" },
	{ "xterm", "", "xterm", "firejail xterm", ":resources/gnome-terminal" },
	{ "lxterminal", "", "LXDE terminal", "firejail lxterminal", "lxterminal" },
	{ "urxvt", "", "rxvt-unicode", "firejail urxvt", "urxvt" },
	{ 0, 0, 0, 0, 0 }
};

bool applications_check_default(const char *name) {
	DefaultApp *app = &dapps[0];
	while (app->name != NULL) {
		if (strcmp(app->name, name) == 0)
			return true;
		app++;
	}
	
	return false;
}

bool applist_check(QString name) {
	QList<Application>::iterator it;
	for (it = applist.begin(); it != applist.end(); ++it) {
		if (it->name_ == name)
			return true;
	}
	
	return false;
}

void applications_print() {
	DefaultApp *app = &dapps[0];
	while (app->name != NULL) {
		printf("\t%s\n", app->name);
		app++;
	}
}

void applist_print() {
	QList<Application>::iterator it;
	for (it = applist.begin(); it != applist.end(); ++it)
		printf("\t%s\n", it->name_.toLocal8Bit().constData());
}


void applications_init() {
	// load default apps
	DefaultApp *app = &dapps[0];
	while (app->name != 0) {
		// de we have the program?
		if (which(app->name) == false) {
			app++;
			continue;			
		}
		
		// is there an alias?
		if (*app->alias != '\0' && which(app->alias)) {
			app++;
			continue;
		}
		
		// is there a user config file?
		if (have_config_file(app->name))
			applist.append(Application(app->name));
		else
			applist.append(Application(app->name, app->description, app->command, app->icon));

		app++;
	}
	
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
