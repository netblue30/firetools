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
#ifndef APPDB_H
#define APPDB_H

#include "firejail_ui.h"
#include "../common/utils.h"
#include <QString>
class QListWidget;
class QLineEdit;

struct AppEntry {
	QString group_;
	QString app_;
	QString command_;
	AppEntry *next_;
	
	AppEntry(char *line) {
		assert(line);
		group_ = QString("");
		app_ = QString("");
		command_ = QString("");
		next_ = 0;

		char *ptr = strtok(line, ";");
		if (ptr) {
			group_ = QString(ptr);
			ptr = strtok(NULL, ";");
			if (ptr) {
				app_ = QString(ptr);
				ptr = strtok(NULL, ";");
				if (ptr)
					command_ = QString(ptr);
			}
		}
	}
	
	void print() {
		printf("%s;%s;%s\n", group_.toLatin1().data(), app_.toLatin1().data(), command_.toLatin1().data());
	}

};

AppEntry *appdb_load_file(void);
void appdb_print_list(AppEntry *ptr);
void appdb_load_group(AppEntry *ptr, QListWidget *group);
void appdb_load_app(AppEntry *ptr, QListWidget *app, QString group);
void appdb_set_command(AppEntry *ptr, QLineEdit *command, QString app);

#endif