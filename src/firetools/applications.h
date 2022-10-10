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
#ifndef APPLICATIONS_H
#define APPLICATIONS_H
#include <QList>
#include <QString>
#include <QIcon>

#define TOP 10
#define MARGIN 2
#define AFRAMES 6	// animation frames
#define ADELAY 20		// animation delay
#define ROWS 6

// applications.cpp
struct Application {
	QString name_;
	QString description_;
	QString exec_;
	QIcon app_icon_;
	
	Application(const char *name, const char *description, const char *exec, QIcon app_icon):
		name_(name), description_(description), exec_(exec), app_icon_(app_icon) {
		
		if (exec == NULL || strlen(exec) == 0)
			exec_ = QString("firejail ") + name;
	}	
};

extern QList<Application> applist;
int applications_init(const char *fname);
bool applist_check(QString name);
void applications_print();
void applist_print();

#endif
