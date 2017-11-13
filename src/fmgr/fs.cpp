/*
 * Copyright (C) 2015-2017 Firetools Authors
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
#include "fs.h"
#include "fmgr.h"
#include <string.h>

FS::FS(pid_t pid): pid_(pid) {
	initialize(pid);
}

void FS::initialize(pid_t pid) {
	char *cmd;
	if (asprintf(&cmd, "firejail --fs.print=%d", (int) pid) == -1)
		errExit("asprintf");
		
	char *str = run_program(cmd);
	if (str == NULL)
		return;
	char *ptr = strtok(str, "\n");
	while (ptr) {
		if (arg_debug)
			printf("fs.print: %s\n", ptr);

		if (strncmp(ptr, "tmpfs ", 6) == 0) {
			paths_.append(QString(ptr + 6));
			ops_.append(QString("T"));
		}
		else if (strncmp(ptr, "blacklist ", 10) == 0 ) {
			paths_.append(QString(ptr + 10));
			ops_.append(QString("B"));
		}
		else if (strncmp(ptr, "blacklist-nolog ", 16) == 0 ) {
			paths_.append(QString(ptr + 16));
			ops_.append(QString("B"));
		}
		else if (strncmp(ptr, "read-only ", 10) == 0 ) {
			paths_.append(QString(ptr + 10));
			ops_.append(QString("R"));
		}
		else if (strncmp(ptr, "clone ", 6) == 0 ) {
			paths_.append(QString(ptr + 6));
			ops_.append(QString("C"));
		}
		else if (strncmp(ptr, "create ", 7) == 0 ) {
			paths_.append(QString(ptr + 7));
			ops_.append(QString("G")); // generated
		}

		ptr = strtok(NULL, "\n");
	}
	paths_.replaceInStrings(" ", "\\ ");
}
 
void FS::checkPath(QString path) {
	if (arg_debug)
		printf("checkPath %s\n", path.toUtf8().constData());
	path_ = path;
}

QString FS::checkFile(QString file) {
	file = file.replace(" ", "\\ ");
	QString full_path = path_ + file;
	if (arg_debug) 
		printf("checkFile full path %s\n", full_path.toUtf8().constData());

	QString str = "";
	for (int i = 0; i < paths_.size(); ++i) {
		if (full_path == paths_.at(i)) {
			str += ops_.at(i);
		}
	}

	if (arg_debug) 
		printf("checkFile database %s, result %s\n", full_path.toUtf8().constData(), str.toUtf8().constData());
	return str;
}
