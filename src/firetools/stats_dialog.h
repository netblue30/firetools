/*
 * Copyright (C) 2015 netblue30 (netblue30@yahoo.com)
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

#include "firetools.h"
class QTextBrowser;
class QUrl;

class StatsDialog: public QDialog {
Q_OBJECT

public:
	StatsDialog();

public slots:
	void cycleReady();
	void anchorClicked(const QUrl & link);

private:
	QString header();
	void kernelSecuritySettings();
	void updateTop();	
	void updatePid();	
	void updateTree();	
	void updateSeccomp();	
	void updateDns();	
	void updateCaps();	

private:
	QTextBrowser *procView_;

#define MODE_TOP 0
#define MODE_PID 1	
#define MODE_TREE 2
#define MODE_SECCOMP 3
#define MODE_DNS 4
#define MODE_CAPS 5
	int mode_;
	int pid_;	// pid value for mode 1
	uid_t uid_;

	// security settings
	bool pid_initialized_;
	bool pid_seccomp_;
	QString pid_caps_;
	bool pid_noroot_;
	QString pid_cpu_cores_;
	QString pid_protocol_;
	
	bool have_join_;
	int caps_cnt_;
	GraphType graph_type_;
};


#endif