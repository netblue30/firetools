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
#include "firetools.h"
#include "db.h"

Db::Db(): cycle_(DbPid::MAXCYCLE - 1), g1h_cycle_(DbPid::MAXCYCLE - 1), g1h_cycle_delta_(DbPid::G1HCYCLE_DELTA - 1), 
	g12h_cycle_(DbPid::MAXCYCLE - 1), g12h_cycle_delta_(DbPid::G12HCYCLE_DELTA - 1), pidlist_(0) {}

void Db::newCycle() {
	if (++cycle_ >= DbPid::MAXCYCLE)
		cycle_ = 0;
	if (++g1h_cycle_delta_ >= DbPid::G1HCYCLE_DELTA) {
		g1h_cycle_delta_ = 0;
		if (++g1h_cycle_ >= DbPid::MAXCYCLE)
			g1h_cycle_ = 0;
		if (++g12h_cycle_delta_ >= DbPid::G12HCYCLE_DELTA) {
			g12h_cycle_delta_ = 0;
			if (++g12h_cycle_ >= DbPid::MAXCYCLE)
				g12h_cycle_ = 0;
		}
	}
}


DbPid *Db::findPid(pid_t pid) {
	if (!pidlist_) {
		return 0;
	}
	
	return pidlist_->find(pid);
}

DbPid *Db::newPid(pid_t pid) {
	assert(findPid(pid) == 0);
	
	DbPid *newpid = new DbPid(pid);
	if (!pidlist_)
		pidlist_ = newpid;
	else
		pidlist_->add(newpid);
		
	return newpid;
}

DbPid *Db::removePid(pid_t pid) {
	// find dbpid
	DbPid *dbpid = findPid(pid);
	if (!dbpid)
		return 0;
	
	// remove first element
	if (dbpid == pidlist_)
		pidlist_ = dbpid->getNext();
	else
		pidlist_->remove(dbpid);
	
	dbpid->resetNext();
	return dbpid;
}

void Db::dbgprint() {
	if (pidlist_)
		pidlist_->dbgprint();
}

void Db::dbgprintcycle() {
	printf("4min cycle %d, 1h delta %d, 1h cycle %d, 12h delta %d, 12h cycle %d\n",
		cycle_, g1h_cycle_delta_, g1h_cycle_, g12h_cycle_delta_, g12h_cycle_);
}

