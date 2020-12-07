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
#ifndef STATS_DIALOG_H
#define STATS_DIALOG_H
#include <sys/types.h>
#include <pwd.h>
#include <QWidget>
#include <QDialog>
#include <QAction>
#include <QSystemTrayIcon>
#include "fstats.h"

class QTextBrowser;
class QUrl;

class PidThread;


extern "C" {
typedef struct dns_report_t {
	volatile uint32_t seq;	//sqence number used to detect data changes

	// proxy config
	unsigned pid;
	int log_timeout;
	int disable_local_doh;
	int nofilter;
	int whitelist_active;
#define MAX_ENTRY_LEN 82 	// a full line on a terminal screen, \n and \0
	char fallback[MAX_ENTRY_LEN];

	// resolvers
#define RESOLVERS_CNT_MAX 10
	int resolvers;
	int encrypted[RESOLVERS_CNT_MAX];
	uint32_t peer_ip[RESOLVERS_CNT_MAX];

	// header
	char header1[MAX_ENTRY_LEN];
	char header2[MAX_ENTRY_LEN];

	// queries
	int logindex;
#define MAX_LOG_ENTRIES 512 	// 18 lines on the screen in order to handle tab terminals
	time_t tstamp[MAX_LOG_ENTRIES];
	char logentry[MAX_LOG_ENTRIES][MAX_ENTRY_LEN];
} DnsReport;
} // extern "C"

class StatsDialog: public QDialog {
Q_OBJECT

public:
	StatsDialog();
	~StatsDialog();

private slots:
	void main_quit();

public slots:
	void cycleReady();
	void anchorClicked(const QUrl & link);
	void trayActivated(QSystemTrayIcon::ActivationReason);

private:
	QString header();
	void kernelSecuritySettings();
	void updateTop();
	void updateFdns();
	inline QString printDump(int index);
	void updateFdnsDump();
	void updatePid();
	void updateTree();
	void updateSeccomp();
	void updateNetwork();
	void updateCaps();
	void updateFirewall();
	void cleanStorage();
	void createTrayActions();

private:
	DnsReport *fdns_report_;
	uint32_t fdns_seq_;
	int fdns_fd_;
	bool fdns_first_run_;

	QTextBrowser *procView_;

#define MODE_TOP 0
#define MODE_PID 1
#define MODE_TREE 2
#define MODE_SECCOMP 3
#define MODE_NETWORK 4
#define MODE_CAPS 5
#define MODE_FIREWALL 6
#define MODE_FDNS 7
#define MODE_FDNS_DUMP 8
#define MODE_MAX 9 // always the last one
	int mode_;
	int pid_;	// pid value for mode 1
	uid_t uid_;
	bool lts_;	// flag to detect LTS version of firejail

	// security settings
	bool pid_initialized_;
	bool pid_seccomp_;
	QString pid_caps_;
	bool pid_noroot_;
	QString pid_cpu_cores_;
	QString pid_protocol_;
	QString pid_name_;
	QString pid_mem_deny_exec_;
	QString pid_apparmor_;
	QString profile_;
	int pid_x11_;
	QString fdns_dump_;

	bool have_join_;
	int caps_cnt_;
	GraphType graph_type_;
	bool net_none_;

	PidThread *thread_;

	// storage for various sandbox settings
	QString storage_dns_;
	QString storage_caps_;
	QString storage_seccomp_;
	QString storage_intro_;
	QString storage_network_;
	QString storage_netfilter_;

	char *shm_file_name_;
public:
	QAction *minimizeAction;
	QAction *restoreAction;
	QAction *quitAction;
};


#endif