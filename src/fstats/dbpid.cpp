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
#include "dbpid.h"

DbPid::DbPid(pid_t pid): next_(0), pid_(pid), cmd_(0), netnamespace_(false), netnone_(false), uid_(0), configured_(false) {
}

DbPid::~DbPid() {
	if (cmd_)
		delete cmd_;

	if (next_)
		delete next_;
}

void DbPid::setCmd(const char *cmd) {
	if (cmd == 0) {
		if (cmd_)
			delete cmd_;
		cmd_ = 0;
	}
	else {
		if (cmd_) {
			if (strcmp(cmd_, cmd)) {
				delete cmd_;
				cmd_ = 0;
			}
		}

		if (!cmd_) {
			cmd_ = new char[strlen(cmd) + 1];
			strcpy(cmd_, cmd);
		}
	}
}

void DbPid::add(DbPid *dbpid) {
	assert(dbpid);
	if (!next_) {
		next_ = dbpid;
		return;
	}

	next_->add(dbpid);
}

void DbPid::remove(DbPid *dbpid) {
	assert(dbpid);
	if (next_ == dbpid) {
		next_ = dbpid->next_;
		return;
	}

	if (next_)
		next_->remove(dbpid);
}

DbPid *DbPid::find(pid_t pid) {
	if (pid_ == pid) {
		return this;
	}

	if (next_) {
		return next_->find(pid);
	}

	return 0;
}

void DbPid::dbgprint() {
	printf("***\n");
	printf("*** PID %d, %s\n", pid_, cmd_);
	printf("***\n");

	for (int i = 0; i < MAXCYCLE; i++)
		data_4min_[i].dbgprint(i);

	if (next_)
		next_->dbgprint();
}


