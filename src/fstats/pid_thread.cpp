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
#include <QtGui>
#include <QElapsedTimer>

#include "pid_thread.h"
#include "../common/pid.h"
#include "db.h"

bool data_ready = false;


PidThread::PidThread(): ending_(false) {
	start();
}

// todo: implement cleanup
PidThread::~PidThread() {
	ending_ = true;
}

// store process data in database
static void store(int pid, int interval, int clocktick) {
	assert(pid < max_pids);
	DbPid *dbpid = Db::instance().findPid(pid);
	
	if (!dbpid) {
		dbpid = Db::instance().newPid(pid);
	}
	assert(dbpid);

	int cycle = Db::instance().getCycle();
	
	// store the data in database
	DbStorage *st = &dbpid->data_4min_[cycle];
	st->cpu_ = (float) ((pids[pid].utime + pids[pid].stime) * 100) / (interval * clocktick);
	st->rss_ = pids[pid].rss;
	st->shared_ =  pids[pid].shared;
	st->rx_ = ((float) pids[pid].rx) /( interval * 1000);
	st->tx_ = ((float) pids[pid].tx) /( interval * 1000);

	if (!dbpid->isConfigured()) {
		if (arg_debug)
			printf("configuring dbpid for sandbox %d\n", pid);		
		// user id
		dbpid->setUid(pids[pid].uid);
		
		// check network namespace
		char *name;
		if (asprintf(&name, "/run/firejail/network/%d-netmap", pid) == -1)
			errExit("asprintf");
		struct stat s;
		if (stat(name, &s) == 0) {
			dbpid->setNetworkDisabled(false);
		}		
		else {
			dbpid->setNetworkDisabled(true);
		}	
		free(name);
		
		// command line
		char *cmd =  pid_proc_cmdline(pid);;			
		dbpid->setCmd(cmd);
		free(cmd);
		dbpid->setConfigured();
	}
}

// remove closed processes from database
static void clear() {
	DbPid *dbpid = Db::instance().firstPid();
	
	while (dbpid) {
		DbPid *next = dbpid->getNext();
		pid_t pid = dbpid->getPid();
		if (pids[pid].level != 1) {
			// remove database entry
			DbPid *dbentry = Db::instance().removePid(pid);
			if (dbentry)
				delete dbentry;
		}
		dbpid = next;
	}
}

void PidThread::run() {
	// memory page size clicks per second
	int pgsz = getpagesize();
	int clocktick = sysconf(_SC_CLK_TCK);
	bool first = true;
	
	while (1) {
		if (ending_)
			break;

		// initialize process table - start with an empty proc table
		pid_read(0);
		
		// start cpu and network measurements
		unsigned utime = 0;
		unsigned stime = 0;
		unsigned long long rx;
		unsigned long long tx;
		for (int i = pids_first; i  <= pids_last; i++) {
			if (pids[i].level == 1) {
				// cpu
				pid_get_cpu_sandbox(i, &utime, &stime);
				pids[i].utime = utime;
				pids[i].stime = stime;

				pid_get_netstats_sandbox(i, &rx, &tx);
				pids[i].rx = rx;
				pids[i].tx = tx;
			}
		}
		
		
		if (!first) {
			// sleep 5 seconds
			msleep(500);
			data_ready = false;
			msleep(500);
		}
		else
			first = false;
		
		// start a new database cycle
		Db::instance().newCycle();
		
		// read the cpu time again, memory
		for (int i = pids_first; i  <= pids_last; i++) {
			if (pids[i].level == 1) {
				if (pids[i].zombie)
					continue;

				// cpu time
				pid_get_cpu_sandbox(i, &utime, &stime);
				if (pids[i].utime <= utime)
					pids[i].utime = utime - pids[i].utime;
				else
					pids[i].utime = 0;
					
				if (pids[i].stime <= stime)
					pids[i].stime = stime - pids[i].stime;
				else
					pids[i].stime = 0;
				
				// memory
				unsigned rss;
				unsigned shared;
				pid_get_mem_sandbox(i, &rss, &shared);
				pids[i].rss = rss * pgsz / 1024;
				pids[i].shared = shared * pgsz / 1024;
				
				// network
				// todo: speedup

				DbPid *dbpid = Db::instance().findPid(i);
				if (dbpid && dbpid->isConfigured() && dbpid->networkDisabled() == false) {
					pid_get_netstats_sandbox(i, &rx, &tx);
					if (rx >= pids[i].rx)
						pids[i].rx = rx - pids[i].rx;
					else
						pids[i].rx = 0;
					
					if (tx > pids[i].tx)
						pids[i].tx = tx - pids[i].tx;
					else
						pids[i].tx = 0;
				
				}
				else {
					pids[i].rx = 0;
					pids[i].tx = 0;
				}
				
				store(i, 1, clocktick);
				
			}
		}
		// remove closed process entries from database
		clear();

		// 4min to 1h transfer
		if (Db::instance().getG1HCycleDelta() == 0) {
//printf("transfer 75 sec data\n");
//Db::instance().dbgprintcycle();			
			
			// for each pid
			DbPid *dbpid = Db::instance().firstPid();
			while (dbpid) {
//printf("processing pid %d, 1h cycle\n", dbpid->getPid());
				int cycle = Db::instance().getCycle();
				int g1hcycle = Db::instance().getG1HCycle();
			
				DbStorage result;
				for (int i = 0; i < DbPid::G1HCYCLE_DELTA; i++) {
					result += dbpid->data_4min_[cycle];
					if (--cycle < 0)
						cycle = DbPid::MAXCYCLE - 1;
				}
				result /= DbPid::G1HCYCLE_DELTA;
				dbpid->data_1h_[g1hcycle] = result;


				if (Db::instance().getG12HCycleDelta() == 0) {
//printf("processing pid %d, 12h cycle\n", dbpid->getPid());
					int g12hcycle = Db::instance().getG12HCycle();
					g1hcycle = Db::instance().getG1HCycle();
				
					DbStorage result2;
					for (int i = 0; i < DbPid::G12HCYCLE_DELTA; i++) {
						result2 += dbpid->data_1h_[g1hcycle];
						if (--g1hcycle < 0)
							g1hcycle = DbPid::MAXCYCLE - 1;
					}
					result2 /= DbPid::G12HCYCLE_DELTA;
					dbpid->data_12h_[g12hcycle] = result2;
				}


				dbpid = dbpid->getNext();
			}
		}

		
//		Db::instance().dbgprint();
		emit cycleReady();
		data_ready = true;

	}
}
