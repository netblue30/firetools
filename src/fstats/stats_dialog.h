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


typedef struct dns_report_t {
	volatile uint32_t seq;	//sqence number used to detect data changes
#define MAX_ENTRY_LEN 82 	// a full line on a terminal screen, \n and \0
	char header1[MAX_ENTRY_LEN];
	char header2[MAX_ENTRY_LEN];
	int logindex;
#define MAX_LOG_ENTRIES 256 	// 18 lines on the screen in order to handle tab terminals
	char logentry[MAX_LOG_ENTRIES][MAX_ENTRY_LEN];
} DnsReport;

#if 0
extern "C" {
	typedef struct dns_report_t {
		volatile uint32_t seq;	//sqence number used to detect data changes
	#define MAX_HEADER 163 	// two full lines on a terminal screen, \n and \0
		char header[MAX_HEADER];
		int logindex;
	#define MAX_LOG_ENTRIES 18 	// 18 lines on the screen in order to handle tab terminals
	#define MAX_ENTRY_LEN 82 	// a full line on a terminal screen, \n and \0
		char logentry[MAX_LOG_ENTRIES][MAX_ENTRY_LEN];
	} DnsReport;
}
#endif

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
#define MODE_MAX 8 // always the last one
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
public:
	QAction *minimizeAction;
	QAction *restoreAction;
	QAction *quitAction;
};


#endif