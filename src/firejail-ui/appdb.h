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
#include <QString>
class QListWidget;
class QLineEdit;

struct AppEntry {
	QString group_;
	QString app_;
	QString command_;
	AppEntry *next_;
	
	AppEntry(char *line);
	void print() {
		printf("%s;%s;%s\n", group_.toUtf8().data(), app_.toUtf8().data(), command_.toUtf8().data());
	}

};

AppEntry *appdb_load_file(void);
void appdb_print_list(AppEntry *ptr);
void appdb_load_group(AppEntry *ptr, QListWidget *group);
void appdb_load_app(AppEntry *ptr, QListWidget *app, QString group);
void appdb_set_command(AppEntry *ptr, QLineEdit *command, QString app);

#endif