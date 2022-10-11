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
#include "../common/utils.h"

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
	st->cpu_ = (float) ((pids_data[pid].utime + pids_data[pid].stime) * 100) / (interval * clocktick);
	st->rss_ = pids_data[pid].rss;
	st->shared_ =  pids_data[pid].shared;
	st->rx_ = ((float) pids_data[pid].rx) /( interval * 1000);
	st->tx_ = ((float) pids_data[pid].tx) /( interval * 1000);

	if (!dbpid->isConfigured()) {
		if (arg_debug)
			printf("configuring dbpid for sandbox %d\n", pid);
		// user id
		dbpid->setUid(pids_data[pid].uid);

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
				pids_data[i].utime = utime;
				pids_data[i].stime = stime;

				// network
				pid_get_netstats_sandbox(i, &rx, &tx);
				pids_data[i].rx = rx;
				pids_data[i].tx = tx;
			}
		}


		if (!first) {
			// sleep 1 second
			msleep(500);
			data_ready = false;
			msleep(500);
		}
		else
			first = false;

		// start a new database cycle
		Db::instance().newCycle();

		timetrace_start();
		// read the cpu time again, memory
		for (int i = pids_first; i  <= pids_last; i++) {
			if (pids[i].level == 1) {
				// cpu time
				pid_get_cpu_sandbox(i, &utime, &stime);
				if (pids_data[i].utime <= utime)
					pids_data[i].utime = utime - pids_data[i].utime;
				else
					pids_data[i].utime = 0;

				if (pids_data[i].stime <= stime)
					pids_data[i].stime = stime - pids_data[i].stime;
				else
					pids_data[i].stime = 0;

				// memory
				unsigned rss;
				unsigned shared;
				pid_get_mem_sandbox(i, &rss, &shared);
				pids_data[i].rss = rss * pgsz / 1024;
				pids_data[i].shared = shared * pgsz / 1024;

				// network
				DbPid *dbpid = Db::instance().findPid(i);
				if (dbpid && dbpid->isConfigured() && dbpid->networkDisabled() == false) {
					pid_get_netstats_sandbox(i, &rx, &tx);
					if (rx >= pids_data[i].rx)
						pids_data[i].rx = rx - pids_data[i].rx;
					else
						pids_data[i].rx = 0;

					if (tx > pids_data[i].tx)
						pids_data[i].tx = tx - pids_data[i].tx;
					else
						pids_data[i].tx = 0;

				}
				else {
					pids_data[i].rx = 0;
					pids_data[i].tx = 0;
				}

				store(i, 1, clocktick);
			}
		}
		float delta = timetrace_end();
		if (arg_debug)
			printf("stats read %.02f ms, pid from %d to %d\n", delta, pids_first, pids_last);
		// remove closed process entries from database
		clear();

		// 4min to 1h transfer
		if (Db::instance().getG1HCycleDelta() == 0) {
			// for each pid
			DbPid *dbpid = Db::instance().firstPid();
			while (dbpid) {
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
