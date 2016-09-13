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
#include "common.h"
#include "pid.h"
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <dirent.h>
       
#define PIDS_BUFLEN 4096
Process *pids = 0;
int max_pids = 32769;
static int pid_proc_cmdline_x11(const pid_t pid);

// get the memory associated with this pid
void pid_getmem(unsigned pid, unsigned *rss, unsigned *shared) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/statm", pid) == -1) {
		perror("asprintf");
		exit(1);
	}

	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return;
	}
	free(file);
	
	unsigned a, b, c;
	if (3 != fscanf(fp, "%u %u %u", &a, &b, &c)) {
		fclose(fp);
		return;
	}
	*rss += b;
	*shared += c;
	fclose(fp);
}


void pid_get_cpu_time(unsigned pid, unsigned *utime, unsigned *stime) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/stat", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return;
	}
	free(file);
	
	char line[PIDS_BUFLEN];
	if (fgets(line, PIDS_BUFLEN - 1, fp)) {
		char *ptr = line;
		// jump 13 fields
		int i;
		for (i = 0; i < 13; i++) {
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto myexit;
			ptr++;
		}
		if (2 != sscanf(ptr, "%u %u", utime, stime))
			goto myexit;
	}

myexit:	
	fclose(fp);
}

unsigned long long pid_get_start_time(unsigned pid) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/stat", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return 0;
	}
	free(file);
	
	char line[PIDS_BUFLEN];
	unsigned long long retval = 0;
	if (fgets(line, PIDS_BUFLEN - 1, fp)) {
		char *ptr = line;
		// jump 21 fields
		int i;
		for (i = 0; i < 21; i++) {
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto myexit;
			ptr++;
		}
		if (1 != sscanf(ptr, "%llu", &retval))
			goto myexit;
	}
	
myexit:
	fclose(fp);
	return retval;
}

char *pid_get_user_name(uid_t uid) {
	struct passwd *pw = getpwuid(uid);
	if (pw)
		return strdup(pw->pw_name);
	return NULL;
}

uid_t pid_get_uid(pid_t pid) {
	uid_t rv = 0;
	
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/status", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return 0;
	}

	// look for firejail executable name
	char buf[PIDS_BUFLEN];
	while (fgets(buf, PIDS_BUFLEN - 1, fp)) {
		if (strncmp(buf, "Uid:", 4) == 0) {
			char *ptr = buf + 5;
			while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
				ptr++;
			}
			if (*ptr == '\0')
				goto doexit;
				
			rv = atoi(ptr);
			break; // break regardless!
		}
	}
doexit:	
	fclose(fp);
	free(file);
	return rv;
}


// recursivity!!!

// mon_pid: pid of sandbox to be monitored, 0 if all sandboxes are included
void pid_read(pid_t mon_pid) {
	if (pids == NULL) {
		FILE *fp = fopen("/proc/sys/kernel/pid_max", "r");
		if (fp) {
			int val;
			if (fscanf(fp, "%d", &val) == 1) {
				if (val >= max_pids)
					max_pids = val + 1;
			}
			fclose(fp);
		}
		pids = (Process *) malloc(sizeof(Process) * max_pids);
		if (pids == NULL) 
			errExit("malloc");
	}

	memset(pids, 0, sizeof(Process) * max_pids);
	pid_t mypid = getpid();

	DIR *dir;
	if (!(dir = opendir("/proc"))) {
		// sleep 2 seconds and try again
		sleep(2);
		if (!(dir = opendir("/proc"))) {
			fprintf(stderr, "Error: cannot open /proc directory\n");
			exit(1);
		}
	}
	
	pid_t child = -1;
	struct dirent *entry;
	char *end;
	while (child < 0 && (entry = readdir(dir))) {
		pid_t pid = strtol(entry->d_name, &end, 10);
		pid %= max_pids;
		if (end == entry->d_name || *end)
			continue;
		if (pid == mypid)
			continue;
		
		// open stat file
		char *file;
		if (asprintf(&file, "/proc/%u/status", pid) == -1) {
			perror("asprintf");
			exit(1);
		}
		FILE *fp = fopen(file, "r");
		if (!fp) {
			free(file);
			continue;
		}

		// look for firejail executable name
		char buf[PIDS_BUFLEN];
		while (fgets(buf, PIDS_BUFLEN - 1, fp)) {
			if (strncmp(buf, "Name:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}

				if ((strncmp(ptr, "firejail", 8) == 0) && (mon_pid == 0 || mon_pid == pid)) {
					if (pid_proc_cmdline_x11(pid))
						pids[pid].level = -1;
					else
						pids[pid].level = 1;
				}
				else
					pids[pid].level = -1;
			}
			if (strncmp(buf, "State:", 6) == 0) {
				if (strstr(buf, "(zombie)"))
					pids[pid].zombie = 1;
			}
			else if (strncmp(buf, "PPid:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}
				unsigned parent = atoi(ptr);
				parent %= max_pids;
				if (pids[parent].level > 0) {
					pids[pid].level = pids[parent].level + 1;
					pids[pid].parent = parent;
				}
			}
			else if (strncmp(buf, "Uid:", 4) == 0) {
				if (pids[pid].level > 0) {
					char *ptr = buf + 5;
					while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
						ptr++;
					}
					if (*ptr == '\0') {
						fprintf(stderr, "Error: cannot read /proc file\n");
						exit(1);
					}
					pids[pid].uid = atoi(ptr);
				}
				break;
			}
		}
		fclose(fp);
		free(file);
	}
	closedir(dir);
}

// return 1 if error
int name2pid(const char *name, pid_t *pid) {
	pid_t parent = getpid();
	
	DIR *dir;
	if (!(dir = opendir("/proc"))) {
		// sleep 2 seconds and try again
		sleep(2);
		if (!(dir = opendir("/proc"))) {
			fprintf(stderr, "Error: cannot open /proc directory\n");
			exit(1);
		}
	}
	
	struct dirent *entry;
	char *end;
	while ((entry = readdir(dir))) {
		pid_t newpid = strtol(entry->d_name, &end, 10);
		if (end == entry->d_name || *end)
			continue;
		if (newpid == parent)
			continue;

		// check if this is a firejail executable
		char *comm = pid_proc_comm(newpid);
		if (comm) {
			// remove \n
			char *ptr = strchr(comm, '\n');
			if (ptr)
				*ptr = '\0';
			if (strcmp(comm, "firejail")) {
				free(comm);
				continue;
			}
			free(comm);
		}
		
		char *cmd = pid_proc_cmdline(newpid);
		if (cmd) {
			// mark the end of the name
			char *ptr = strstr(cmd, "--name=");
			char *start = ptr;
			if (!ptr) {
				free(cmd);
				continue;
			}
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			*ptr = '\0';
			int rv = strcmp(start + 7, name);
			if (rv == 0) {
				free(cmd);
				*pid = newpid;
				closedir(dir);
				return 0;
			}
			free(cmd);
		}
	}
	closedir(dir);
	return 1;
}

#define BUFLEN 4096
char *pid_proc_comm(const pid_t pid) {
	// open /proc/pid/cmdline file
	char *fname;
	int fd;
	if (asprintf(&fname, "/proc/%d//comm", pid) == -1)
		return NULL;
	if ((fd = open(fname, O_RDONLY)) < 0) {
		free(fname);
		return NULL;
	}
	free(fname);

	// read file
	char buffer[BUFLEN];
	ssize_t len;
	if ((len = read(fd, buffer, sizeof(buffer) - 1)) <= 0) {
		close(fd);
		return NULL;
	}
	buffer[len] = '\0';
	close(fd);

	// remove \n
	char *ptr = strchr(buffer, '\n');
	if (ptr)
		*ptr = '\0';

	// return a malloc copy of the command line
	char *rv = strdup(buffer);
	if (strlen(rv) == 0) {
		free(rv);
		return NULL;
	}
	return rv;
}

char *pid_proc_cmdline(const pid_t pid) {
	// open /proc/pid/cmdline file
	char *fname;
	int fd;
	if (asprintf(&fname, "/proc/%d/cmdline", pid) == -1)
		return NULL;
	if ((fd = open(fname, O_RDONLY)) < 0) {
		free(fname);
		return NULL;
	}
	free(fname);

	// read file
	unsigned char buffer[BUFLEN];
	ssize_t len;
	if ((len = read(fd, buffer, sizeof(buffer) - 1)) <= 0) {
		close(fd);
		return NULL;
	}
	buffer[len] = '\0';
	close(fd);

	// clean data
	int i;
	for (i = 0; i < len; i++) {
		if (buffer[i] == '\0')
			buffer[i] = ' ';
	}

	// return a malloc copy of the command line
	char *rv = strdup((char *) buffer);
	if (strlen(rv) == 0) {
		free(rv);
		return NULL;
	}
	return rv;
}


// recursivity!!!
void pid_get_cpu_sandbox(unsigned pid, unsigned *utime, unsigned *stime) {
	if (pids[pid].level == 1) {
		*utime = 0;
		*stime = 0;
	}
	
	unsigned utmp = 0;
	unsigned stmp = 0;
	pid_get_cpu_time(pid, &utmp, &stmp);
	*utime += utmp;
	*stime += stmp;
	
	int i;
	for (i = pid + 1; i < max_pids; i++) {
		if (pids[i].parent == (int) pid)
			pid_get_cpu_sandbox(i, utime, stime);
	}
}

void pid_get_mem_sandbox(unsigned pid, unsigned *rss, unsigned *shared) {
	if (pids[pid].level == 1) {
		*rss = 0;
		*shared = 0;
	}
	
	pid_getmem(pid, rss, shared);
	
	int i;
	for (i = pid + 1; i < max_pids; i++) {
		if (pids[i].parent == (int) pid)
			pid_get_mem_sandbox(i, rss, shared);
	}
}

#define MAXBUF PIDS_BUFLEN
void pid_get_netstats_sandbox(int parent, unsigned long long *rx, unsigned long long *tx) {
	*rx = 0;
	*tx = 0;
	
	// find the first child
	int child = -1;
	for (child = parent + 1; child < max_pids; child++) {
		if (pids[child].parent == parent)
			break;
	}

	if (child == -1)
		return;

	// open /proc/child/net/dev file and read rx and tx
	char *fname;
	if (asprintf(&fname, "/proc/%d/net/dev", child) == -1)
		errExit("asprintf");

	FILE *fp = fopen(fname, "r");
	if (!fp) {
		free(fname);
		return;
	}
	
	char buf[MAXBUF];
	while (fgets(buf, MAXBUF, fp)) {
		if (strncmp(buf, "Inter", 5) == 0)
			continue;
		if (strncmp(buf, " face", 5) == 0)
			continue;
		
		char *ptr = buf;
		while (*ptr != '\0' && *ptr != ':') {
			ptr++;
		}
		
		if (*ptr == '\0') {
			fclose(fp);
			free(fname);
			return;
		}
		ptr++;
		
		long long unsigned rxval;
		long long unsigned txval;
		unsigned a, b, c, d, e, f, g;
		int rv = sscanf(ptr, "%llu %u %u %u %u %u %u %u %llu",
			&rxval, &a, &b, &c, &d, &e, &f, &g, &txval);
		if (rv == 9) {
			*rx += rxval;
			*tx += txval;
		}
	}

	free(fname);
	fclose(fp);
	return;
}

// return 1 if firejail --x11 on command line
static int pid_proc_cmdline_x11(const pid_t pid) {
	// if comm is not firejail return 0
	char *comm = pid_proc_comm(pid);
	if (strcmp(comm, "firejail") != 0) {
		free(comm);
		return 0;
	}
	free(comm);

	// open /proc/pid/cmdline file
	char *fname;
	int fd;
	if (asprintf(&fname, "/proc/%d/cmdline", pid) == -1)
		return 0;
	if ((fd = open(fname, O_RDONLY)) < 0) {
		free(fname);
		return 0;
	}
	free(fname);

	// read file
	unsigned char buffer[BUFLEN];
	ssize_t len;
	if ((len = read(fd, buffer, sizeof(buffer) - 1)) <= 0) {
		close(fd);
		return 0;
	}
	buffer[len] = '\0';
	close(fd);

	// skip the first argument
	int i;
	for (i = 0; buffer[i] != '\0'; i++);

	// parse remaining command line options
	while (1) {
		// extract argument
		i++;
		if (i >= len)
			break;
		char *arg = (char *)buffer + i;

		// detect the last command line option
		if (strcmp(arg, "--") == 0)
			break;
		if (strncmp(arg, "--", 2) != 0)
			break;
			
		// check x11
		if (strncmp(arg, "--x11", 5) == 0)
			return 1;
		i += strlen(arg);
	}
	return 0;
}



