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

#include "appdb.h"
#include <QListWidget>
#include <QLineEdit>
#define MAXBUF 4096

// return the list of applications
AppEntry* appdb_load_file(void) {
	FILE *fp = fopen("uimenus", "r");
	if (!fp)
		return 0;
	AppEntry *retval = 0;
	AppEntry *last = 0;
//	(void) last;
	
	char buf[MAXBUF];
	while (fgets(buf, MAXBUF, fp)) {
		char *ptr1 = buf;
		while (*ptr1 == ' ' || *ptr1 == '\t')
			ptr1++;
		char *ptr2 = strchr(ptr1, '\n');
		if (ptr2)
			*ptr2 = '\0';

		AppEntry *entry = new AppEntry(ptr1);
		if (entry->group_.isEmpty() || entry->app_.isEmpty() || entry->command_.isEmpty())
			delete entry;
		
		// add the app to the list
		if (!retval) {
			retval = entry;
			last = entry;
		}
		else {
			last->next_ = entry;
			last = entry;
		}
	}
	
	fclose(fp);
	return retval;
}

// print database
void appdb_print_list(AppEntry *ptr) {
	while (ptr) {
		ptr->print();
		ptr = ptr->next_;
	}
}

// add all groups to the widget
void appdb_load_group(AppEntry *ptr, QListWidget *group) {
	QString last;

	while (ptr) {
		if (last != ptr->group_) {
			new QListWidgetItem(ptr->group_, group);
		}
		last = ptr->group_;
		ptr = ptr->next_;
	}

}

// add all groups to the widget
void appdb_load_app(AppEntry *ptr, QListWidget *app, QString group) {
	app->clear();
	
	while (ptr) {
		if (group == ptr->group_) {
			new QListWidgetItem(ptr->app_, app);
		}
		ptr = ptr->next_;
	}
}

void appdb_set_command(AppEntry *ptr, QLineEdit *command, QString app) {
	while (ptr) {
		if (app == ptr->app_) {
			command->setText(ptr->command_);
			break;
		}
		ptr = ptr->next_;
	}
}
