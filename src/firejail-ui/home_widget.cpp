/*
 * Copyright (C) 2015-2016 Firetools Authors
 *
 * This file is part of firetools project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public  as published by
 * the Free Software Foundation; either version 2 of the , or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public  for more details.
 *
 * You should have received a copy of the GNU General Public  along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#if QT_VERSION >= 0x050000
	#include <QtWidgets>
#else
	#include <QtGui>
#endif

#include <QCheckBox>
#include "firejail_ui.h"
#include "home_widget.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


HomeWidget::HomeWidget(QWidget * parent): QListWidget(parent) {
	readFiles();
}

void HomeWidget::readFiles() {
	DIR *dir = opendir("/home/netblue");
	if (!dir)
// todo: error recovery
		errExit("opendir");
	
	struct dirent *entry;
	while ((entry = readdir(dir))) {
		// with a few exceptions, reject all dot files
		bool accept = false;
		if (strcmp(entry->d_name, ".config") == 0 ||
		    strcmp(entry->d_name, ".mozilla") == 0)
		    	accept = true;
		if (!accept && *entry->d_name == '.')
			continue;
		
		// allow only directorries 
		struct stat s;
		char *name;
		if (asprintf(&name, "/home/netblue/%s", entry->d_name) == -1)
			errExit("asprintf");
		if (stat(name, &s) == -1) {
			free(name);
			continue;
		}
		free(name);
		if (!S_ISDIR(s.st_mode))
			continue;

		QCheckBox *box = new QCheckBox(entry->d_name);
		QListWidgetItem *item = new QListWidgetItem();
		addItem(item);
		setItemWidget(item, box);
	}
	
	closedir(dir);

}