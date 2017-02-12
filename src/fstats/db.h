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
#ifndef DB_H
#define DB_H

#include "fstats.h"
#include "dbpid.h"


class Db {
public:
	static Db& instance() {
		static Db myinstance;
		return myinstance;
	}
	
	void newCycle();
	int getCycle() {
		return cycle_;
	}
	int getG1HCycle() {
		return g1h_cycle_;
	}
	int getG1HCycleDelta() {
		return g1h_cycle_delta_;
	}
	int getG12HCycle() {
		return g12h_cycle_;
	}
	int getG12HCycleDelta() {
		return g12h_cycle_delta_;
	}
	DbPid *firstPid() {
		return pidlist_;
	}
	DbPid *newPid(pid_t pid);
	DbPid *findPid(pid_t pid);
	DbPid *removePid(pid_t pid);

	void dbgprint();
	void dbgprintcycle();
		
private:
	Db();
	Db(Db const&);
	void operator=(Db const&);

private:
	int cycle_;
	int g1h_cycle_;
	int g1h_cycle_delta_;
	int g12h_cycle_;
	int g12h_cycle_delta_;
	DbPid *pidlist_;
};


#endif
