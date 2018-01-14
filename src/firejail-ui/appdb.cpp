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

#if QT_VERSION >= 0x050000
	#include <QtWidgets>
#else
	#include <QtGui>
#endif

#include "appdb.h"
#include "../../firetools_config_extras.h"
#include <QListWidget>
#include <QLineEdit>
#define MAXBUF 4096

static bool check_executable(const char *exec) {
	struct stat s;
	if (stat(exec, &s) == 0)
		return true;
	
	// check well-known paths
	const char *path[] = {
		"/usr/bin/",
		"/bin/",
		"/usr/games/",
		"/usr/local/bin/",
		"/sbin/",
		"/usr/sbin/",
		NULL
	};
	
	int i = 0;
	while (path[i] != NULL) {
		bool found = false;
		
		char *name;
		if (asprintf(&name, "%s%s", path[i], exec) == -1)
			errExit("asprintf");
		if (stat(name, &s) == 0)
			found = true;
		free(name);
		if (found)
			return true;
		i++;
	}
	return false;
}


AppEntry::AppEntry(char *line) {
	assert(line);
	if (arg_debug)
		printf("processing \"%s\"\n", line);
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
			if (ptr) {
				command_ = QString(ptr);
				if (command_.endsWith("*")) {
					command_ = "";
					return;
				}
				
				// try to find the executable
				char *str = strdup(command_.toUtf8().data());
				if (!str)
					errExit("strdup");
				
				// skip excutables ending in *
				
				char *ptr = strchr(str, ' ');
				if (ptr)
					*ptr = '\0';

				if (check_executable(str) == false) {				
					if (arg_debug)
						printf("executable %s not found\n", str);
					command_ = QString("");
				}
				free(str);
			}
		}
	}
}



// return the list of applications
AppEntry* appdb_load_file(void) {
	const char *fname = PACKAGE_LIBDIR "/uimenus";
	FILE *fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "Error: cannot find uimenus file in %s\n", fname);
		return 0;
	}
	AppEntry *retval = 0;
	AppEntry *last = 0;
	
	char buf[MAXBUF];
	while (fgets(buf, MAXBUF, fp)) {
		char *ptr1 = buf;
		while (*ptr1 == ' ' || *ptr1 == '\t')
			ptr1++;
		char *ptr2 = strchr(ptr1, '\n');
		if (ptr2)
			*ptr2 = '\0';

		AppEntry *entry = new AppEntry(ptr1);
		if (entry->group_.isEmpty() || entry->app_.isEmpty() || entry->command_.isEmpty()) {
			if (arg_debug)
				printf("line not accepted\n");
			delete entry;
			continue;
		}
		
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
	if (arg_debug)
		printf("menus loaded\n");	
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
