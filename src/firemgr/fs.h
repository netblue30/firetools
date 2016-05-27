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
#ifndef FS_H
#define FS_H
#include <unistd.h>
#include <sys/types.h>
#include <QStringList>

class FS {
public:
	FS(pid_t pid);
	QString getType(QString path, QString file);
	void checkPath(QString path);
	QString checkFile(QString file);
	
private:
	void initialize(pid_t pid);

	pid_t pid_;
	QStringList paths_;
	QStringList ops_;
	
	QString path_;
};

#endif