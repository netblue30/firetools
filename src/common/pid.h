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
#ifndef PID_H
#define PID_H
extern int max_pids;
#include "common.h"

typedef struct {
	short level;  // -1 not a firejail process, 0 not investigated yet, 1 firejail process, > 1 firejail child
	unsigned char zombie;
	pid_t parent;
	uid_t uid;
	char *user;
	char *cmd;
	unsigned utime;
	unsigned stime;
	unsigned rss;
	unsigned shared;
	unsigned long long rx;	// network rx, bytes
	unsigned long long tx;	// networking tx, bytes
} Process;
//extern Process pids[max_pids];
extern Process *pids;

// pid self-contained functions
void pid_getmem(unsigned pid, unsigned *rss, unsigned *shared);
void pid_get_cpu_time(unsigned pid, unsigned *utime, unsigned *stime);
unsigned long long pid_get_start_time(unsigned pid);
uid_t pid_get_uid(pid_t pid);
char *pid_get_user_name(uid_t uid);
int name2pid(const char *name, pid_t *pid);
char *pid_proc_comm(const pid_t pid);
char *pid_proc_cmdline(const pid_t pid);

// read all processes in pids array
void pid_read(pid_t mon_pid);

void pid_get_cpu_sandbox(unsigned pid, unsigned *utime, unsigned *stime);
void pid_get_mem_sandbox(unsigned pid, unsigned *rss, unsigned *shared);
void pid_get_netstats_sandbox(int pid, unsigned long long *rx, unsigned long long *tx);

#endif
