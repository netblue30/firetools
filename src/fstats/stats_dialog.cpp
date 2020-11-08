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
#include <QtGlobal>

#if QT_VERSION >= 0x050000
	#include <QtWidgets>
#else
	#include <QtGui>
#endif

#include <QUrl>
#include <QProcess>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <dirent.h>

#include "stats_dialog.h"
#include "db.h"
#include "graph.h"
#include "../common/utils.h"
#include "../common/pid.h"
#include "../../firetools_config.h"
#include "../../firetools_config_extras.h"
#include "pid_thread.h"
#include "fstats.h"
extern bool data_ready;

static QString getName(pid_t pid);
static QString getProfile(pid_t pid);
static bool userNamespace(pid_t pid);
static int getX11Display(pid_t pid);


// from fdns:procs.c - void procs_list(void) {
// returns malloc memory
static char *find_fdns_shm_file_name(void) {
	int procs_addr_default = 0;
	int procs_addr_loopback = 0;
	char *procs_addr_real = NULL;

	DIR *dir;
	if (!(dir = opendir("/run/fdns"))) {
		// sleep 2 seconds and try again
		sleep(2);
		if (!(dir = opendir("/proc")))
			return 0;
	}

	struct dirent *entry;
	int procs_addr_flag = 0;
	while ((entry = readdir(dir))) {
		if (*entry->d_name == '.')
			continue;

		char *fname;
		if (asprintf(&fname, "/proc/%s", entry->d_name) == -1)
			errExit("asprintf");
		if (access(fname, R_OK) == 0) {
			char *runfname;
			if (asprintf(&runfname, "/run/fdns/%s", entry->d_name) == -1)
				errExit("asprintf");
			if (arg_debug)
				printf("pid %s,", entry->d_name);
			FILE *fp = fopen(runfname, "r");
			if (fp) {
				static const int MAXBUF = 1024;
				char buf[MAXBUF];
				if (fgets(buf, MAXBUF, fp)) {
					char *ptr = strchr(buf, '\n');
					if (ptr)
						*ptr = '\0';

					if (!procs_addr_flag) {
						if (strcmp(buf, "127.1.1.1") == 0) {
							procs_addr_default = 1;
							procs_addr_flag = 1;
						}
						else if (strcmp(buf, "127.0.0.1") == 0) {
							procs_addr_loopback = 1;
							procs_addr_flag = 1;
						}
						else if (!procs_addr_real) {
							procs_addr_real = strdup(buf);
							if (!procs_addr_real)
								errExit("strdup");
						}
					}
				}
			}
			printf("\n");
			fclose(fp);
			free(runfname);
		}
		free(fname);
	}
	closedir(dir);

	char *rv = 0;
	if (procs_addr_default) {
		rv = strdup("/dev/shm/fdns-stats-127.1.1.1");
		if (!rv)
			errExit("strdup");
	}
	else if (procs_addr_loopback) {
		rv = strdup("/dev/shm/fdns-stats-127.0.0.1");
		if (!rv)
			errExit("strdup");
	}
	else if (procs_addr_real) {
		if (asprintf(&rv, "/dev/shm/fdns-stats-%s", procs_addr_real) == -1)
			errExit("asprintf");
	}

	if (procs_addr_real)
		free(procs_addr_real);

	return rv;
}


// dbus proxy path used by firejail and firemon
#define XDG_DBUS_PROXY_PATH "/usr/bin/xdg-dbus-proxy"
static int find_child(int id) {
	int i;
	int first_child = -1;
	// find the first child
	for (i = 0; i < max_pids && first_child == -1; i++) {
		if (pids[i].level == 2 && pids[i].parent == id) {
			// skip /usr/bin/xdg-dbus-proxy (started by firejail for dbus filtering)
			char *cmdline = pid_proc_cmdline(i);
			if (strncmp(cmdline, XDG_DBUS_PROXY_PATH, strlen(XDG_DBUS_PROXY_PATH)) == 0) {
				free(cmdline);
				continue;
			}
			free(cmdline);
			first_child = i;
			break;
		}
	}

	if (first_child == -1)
		return -1;

	// find the second-level child
	for (i = 0; i < max_pids; i++) {
		if (pids[i].level == 3 && pids[i].parent == first_child)
			return i;
	}

	// if a second child is not found, return the first child pid
	// this happens for processes sandboxed with --join
	return first_child;
}

StatsDialog::StatsDialog(): QDialog(), fdns_report_(0), fdns_seq_(0), fdns_fd_(0), fdns_first_run_(true),
		mode_(MODE_TOP), pid_(0), uid_(0), lts_(false),
	pid_initialized_(false), pid_seccomp_(false), pid_caps_(QString("")), pid_noroot_(false),
	pid_cpu_cores_(QString("")), pid_protocol_(QString("")), pid_name_(QString("")),
	profile_(QString("")), pid_x11_(0), fdns_dump_(""),
	have_join_(true), caps_cnt_(64), graph_type_(GRAPH_4MIN), net_none_(false), shm_file_name_(0) {

	// clean storage area
	cleanStorage();

	// detect LTS version
	char *str = run_program("firejail --version");
	if (str && strstr(str, "LTS"))
		lts_ = true;

	procView_ = new QTextBrowser;
	procView_->setOpenLinks(false);
	procView_->setOpenExternalLinks(false);
	procView_->setText("accumulating data...");

	connect(procView_,  SIGNAL(anchorClicked(const QUrl &)), this, SLOT(anchorClicked(const QUrl &)));

	QGridLayout *layout = new QGridLayout;
	layout->addWidget(procView_, 0, 0);
	setLayout(layout);

	// set screen size and title
	int x;
	int y;
	config_read_screen_size(&x, &y);
 	resize(x, y);
	setWindowTitle(tr("Firetools"));

	// detect if joining a sandbox is possible on this system
	struct utsname u;
	int rv = uname(&u);
	if (rv == 0) {
		int major;
		int minor;
		if (2 == sscanf(u.release, "%d.%d", &major, &minor)) {
			if (major < 3)
				have_join_ = false;
			else if (major == 3 && minor < 8)
				have_join_ = false;
		}
	}

	// detect the number of capabilities supported by the current kernel
	str = run_program("firejail --debug-caps");
	if (!str)
		return;
	int val;
	if (sscanf(str, "Your kernel supports %d", &val) == 1 && val <= 64) {
		if (arg_debug)
			printf("%d capabilities supported by the kernel\n", val);
		caps_cnt_ = val;
	}

	thread_ = new PidThread();
	connect(thread_, SIGNAL(cycleReady()), this, SLOT(cycleReady()));
	createTrayActions();
}

StatsDialog::~StatsDialog() {
	if (fdns_fd_)
		::close(fdns_fd_);
	if (!isMaximized())
		config_write_screen_size(width(), height());
}

void StatsDialog::cleanStorage() {
	storage_dns_ = "";
	storage_caps_ = "";
	storage_seccomp_ = "";
	storage_intro_ = "";
	storage_network_ = "";
	storage_netfilter_ = "";
}

// Shutdown sequence
void StatsDialog::main_quit() {
	printf("exiting...\n");

	qApp->quit();
}

void StatsDialog::trayActivated(QSystemTrayIcon::ActivationReason reason) {
	if (reason == QSystemTrayIcon::Context)
		return;
	if (reason == QSystemTrayIcon::DoubleClick)
		return;
	if (reason == QSystemTrayIcon::MiddleClick)
		return;

	if (isVisible()) {
		hide();
//		stats_->hide();
	}
	else {
		show();
//		stats_->hide();
	}
}

void StatsDialog::createTrayActions() {
	minimizeAction = new QAction(tr("Mi&nimize"), this);
	connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
//	connect(minimizeAction, SIGNAL(triggered()), stats_, SLOT(hide()));

	restoreAction = new QAction(tr("&Restore"), this);
	connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
//	connect(restoreAction, SIGNAL(triggered()), stats_, SLOT(show()));

	quitAction = new QAction(tr("&Quit"), this);
	connect(quitAction, SIGNAL(triggered()), this, SLOT(main_quit()));
}

QString StatsDialog::header() {
	QString msg;
	if (mode_ == MODE_TOP) {
		msg += "<table><tr><td width=\"5\"></td><td>";
		msg += "<a href=\"about\">About</a>";
		msg += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"launcher\">Sandbox Launcher</a>";
		msg += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"newsandbox\">Configuration Wizard</a>";
		msg += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fdns\">Firejail DNS</a>";
		msg += "</td></tr></table>";
	}

	else if (mode_ == MODE_FDNS) {
		msg += "<table><tr><td width=\"5\"></td><td>";
		msg += "<a href=\"top\">Home</a>";
		msg += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"dump\">Report</a>";
		msg += "</td></tr></table>";
	}
	else if (mode_ == MODE_FDNS_DUMP) {
		msg += "<table><tr><td width=\"5\"></td><td>";
		msg += "<a href=\"top\">Home</a>";
		msg += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fdns\">Live</a>";
		msg += "</td></tr></table>";
	}

	else if (mode_ == MODE_PID) {
		msg += "<table><tr><td width=\"5\"></td><td>";
		msg += "<a href=\"top\">Home</a>";
		if (uid_ == getuid())
			msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"shut\">Shutdown</a>";
		if (have_join_ && uid_ == getuid())
			msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"join\">Join</a>";
		if (!lts_)
			msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fmgr\">File Manager</a>";
		msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"tree\">Process Tree</a>";
		msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"network\">Network</a>";
		msg += "</td></tr></table>";
	}

	else {
		msg += "<table><tr><td width=\"5\"></td><td>";
		msg += "<a href=\"top\">Home</a>";
		msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"back\">" + QString::number(pid_) + "</a>";
		if (uid_ == getuid())
			msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"shut\">Shutdown</a>";
		if (have_join_ && uid_ == getuid())
			msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"join\">Join</a>";
		if (!lts_)
			msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fmgr\">File Manager</a>";
		msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"tree\">Process Tree</a>";
		msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"network\">Network</a>";
		msg += "</td></tr></table>";
	}

	msg += "<hr>";
	return msg;
}

void StatsDialog::updateTop() {
	QString msg = header();
	msg += "<table><tr><td width=\"5\"></td><td><b>Sandbox List</b></td></tr></table><br/>\n";
	msg += "<table><tr><td width=\"5\"></td><td width=\"60\">PID</td/><td width=\"60\">CPU<br/>(%)</td><td>Memory<br/>(KiB)&nbsp;&nbsp;</td><td>RX<br/>(KB/s)&nbsp;&nbsp;</td><td>TX<br/>(KB/s)&nbsp;&nbsp;</td><td>Command</td>\n";

	int cycle = Db::instance().getCycle();
	assert(cycle < DbPid::MAXCYCLE);
	DbPid *ptr = Db::instance().firstPid();


	while (ptr) {
		pid_t pid = ptr->getPid();
		const char *cmd = ptr->getCmd();
		if (cmd) {
			char *str;
			DbStorage *st = &ptr->data_4min_[cycle];
			if (asprintf(&str, "<tr><td></td><td><a href=\"%d\">%d</a></td><td>%.02f</td><td>%d</td><td>%.02f</td><td>%.02f</td><td>%s</td></tr>",
				pid, pid, st->cpu_, (int) (st->rss_ + st->shared_),
				st->rx_, st->tx_, cmd) != -1) {

				msg += str;
				free(str);
			}
		}

		ptr = ptr->getNext();
	}

	msg += "</table>";
	procView_->setHtml(msg);
}

QString StatsDialog::printDump(int index) {
	QString msg = "";
	QString str;
	struct tm *t = localtime(&fdns_report_->tstamp[index]);
	str.sprintf("%02d:%02d:%02d ", t->tm_hour, t->tm_min, t->tm_sec);
	if (strstr(fdns_report_->logentry[index], "dropped")) {
		msg += "<font color=\"red\">";
		msg += str + fdns_report_->logentry[index];
		msg += "</font>";
	}
	else
		msg += str + fdns_report_->logentry[index];

	msg += "<br/>";

	return msg;
}

void StatsDialog::updateFdnsDump() {
	if (!fdns_dump_.isEmpty())
		return;
	QString msg = header();

	if (access(shm_file_name_, R_OK)) {
		msg += QString("Error: cannot open ") + QString(shm_file_name_) + QString(", probably fdns is not running<br/>");
		fdns_fd_ = 0;
		procView_->setHtml(msg);
		return;
	}

	int fd = ::open(shm_file_name_, O_RDONLY);
	if (fd <= 0) {
		msg +=  "Error: cannot access Firejail DNS data";
		procView_->setHtml(msg);
		return;

	}

	DnsReport report;
	ssize_t len = ::read(fd, &report, sizeof(DnsReport));
	if (len != sizeof(DnsReport)) {
		msg += "Error: cannot access Firejail DNS data";
		procView_->setHtml(msg);
		return;
	}
	::close(fd);

	QDateTime current = QDateTime::currentDateTime();
	msg += "<b>Fireail DNS report for " + current.toString() + "</b><br/><br/>";

	msg += "<b>Stats:</b><br/>";
	msg += QString(fdns_report_->header1) + "<br/>";
	msg += QString(fdns_report_->header2) + "<br/><br/>";


	msg += "<b>Process:</b><br/>";
	QString qs;
	qs.sprintf("PID: %u<br/>", report.pid);
	msg += qs;
	qs.sprintf("Fallback server: %s<br/>", report.fallback);
	msg += qs;
	if (report.disable_local_doh)
		msg += "DoH disabled for applications behind the proxy<br/>";
	else
		msg += "DoH allowed for applications behind the proxy<br/>";
	qs.sprintf("To shutdown the proxy run <b>\"sudo kill -9 %u\"</b> in a terminal<br/><br/>", report.pid);
	msg += qs;

	msg += "<b>Queries:</b><br/>";
	qs.sprintf("(queries cleared after %d minutes)<br/>", report.log_timeout);
	msg += qs;
	for (int i = fdns_report_->logindex; i < MAX_LOG_ENTRIES; i++) {
		if (fdns_report_->tstamp && strlen(fdns_report_->logentry[i]))
			msg += printDump(i);
	}
	for (int i = 0; i < fdns_report_->logindex; i++) {
		if (fdns_report_->tstamp && strlen(fdns_report_->logentry[i]))
			msg += printDump(i);
	}

	procView_->setHtml(msg);
	fdns_dump_ = msg;
	if (fdns_fd_)
		::close(fdns_fd_);
	fdns_fd_ = 0;
	fdns_report_ = 0;
}


void StatsDialog::updateFdns() {
	QString msg = header();

	if (access(shm_file_name_, R_OK)) {
		msg += QString("Error: cannot open ") + QString(shm_file_name_) + QString(", probably fdns is not running<br/>");
		if (fdns_fd_)
			::close(fdns_fd_);
		fdns_fd_ = 0;
		fdns_report_ = 0;
		procView_->setHtml(msg);
		return;
	}

	// open fdns shared memory if necessary
	if (!fdns_fd_) {
		fdns_fd_ = shm_open(shm_file_name_ + 8, O_RDONLY, S_IRWXU);
		if (fdns_fd_ == -1) {
			msg += "Error: cannot access shared memory, probably fdns is not running<br/>";
			if (fdns_fd_)
				::close(fdns_fd_);
			fdns_fd_ = 0;
			fdns_report_ = 0;
			procView_->setHtml(msg);
			return;
		}
	}

	if (fdns_fd_ && fdns_report_ == 0) {
		fdns_report_ = (DnsReport *) mmap(0, sizeof(DnsReport), PROT_READ, MAP_SHARED, fdns_fd_, 0 );
		if (fdns_report_ == (void *) - 1) {
			msg += "Error: cannot map /sdv/shm/fdns_stats file in process memory<<br/>";
			fdns_report_ = 0;
			::close(fdns_fd_);
			fdns_fd_ = 0;
			procView_->setHtml(msg);
			return;
		}
	}

	if (fdns_fd_ && fdns_report_) {
		if (fdns_first_run_ || fdns_seq_ != fdns_report_->seq) {
			fdns_first_run_ = false;
			fdns_seq_ = fdns_report_->seq;

			// print header
			msg += "<b>";
			msg += fdns_report_->header1;
			msg += "</b><br/><b>";
			msg += fdns_report_->header2;
			msg += "</b><br/><br/>";

			// print log lines
			int row = 24;
			int i;
			int logrows = MAX_LOG_ENTRIES;
			if ((row - 4) > 0 && (row - 4) < MAX_LOG_ENTRIES)
				logrows = row - 4;

			int index = fdns_report_->logindex - logrows;
			for (i = 0; i < logrows; i++, index++) {
				int position = index;
				if (index < 0)
					position += MAX_LOG_ENTRIES;

				if (fdns_report_->tstamp && strlen(fdns_report_->logentry[position]))
					msg += printDump(position);
			}
			procView_->setHtml(msg);
		}
	}
	procView_->update();
}


void StatsDialog::updateFirewall() {
	DbPid *dbptr = Db::instance().findPid(pid_);
	if (!dbptr) {
		mode_ = MODE_TOP;
		return;
	}

	QString msg = storage_netfilter_;
	if (msg.isEmpty()) {
		if (arg_debug)
			printf("reading firewall configuration\n");
		msg = header() + storage_intro_;

		char *cmd;
		if (asprintf(&cmd, "firejail --netfilter.print=%d", pid_) != -1) {
			char *str = run_program(cmd);
			if (str)
				msg += "<pre>" + QString(str) + "</pre>";
		}
		storage_netfilter_ = msg;
		procView_->setHtml(msg);
	}
}


void StatsDialog::updateTree() {
	DbPid *dbptr = Db::instance().findPid(pid_);
	if (!dbptr) {
		mode_ = MODE_TOP;
		return;
	}

	if (arg_debug)
		printf("reading process tree configuration\n");
	QString msg = header() + storage_intro_;
	msg += "<table><tr><td width=\"5\"></td><td>";

	char *str = 0;
	char *cmd;
	if (asprintf(&cmd, "firemon --tree --wrap %d", pid_) != -1) {
		str = run_program(cmd);
		char *ptr = str;
		// htmlize!
		while (*ptr != 0) {
			if (*ptr == '\n') {
				*ptr = '\0';
				msg += QString(str) + "<br/>\n";
				ptr++;

				while (*ptr == ' ') {
					msg += "&nbsp;&nbsp;";
					ptr++;
				}
				str = ptr;
				continue;
			}
			ptr++;
		}
		free(cmd);
	}

	msg += "</td></tr></table>";
	procView_->setHtml(msg);
}


void StatsDialog::updateSeccomp() {
	DbPid *dbptr = Db::instance().findPid(pid_);
	if (!dbptr) {
		mode_ = MODE_TOP;
		return;
	}

	QString msg = storage_seccomp_;
	if (msg.isEmpty()) {
		if (arg_debug)
			printf("reading seccomp configuration\n");
		QString msg = header() + storage_intro_;
		msg += "<table><tr><td width=\"5\"></td><td>";

		char *str = 0;
		char *cmd;
		if (asprintf(&cmd, "firejail --seccomp.print=%d", pid_) != -1) {
			str = run_program(cmd);
			char *ptr = str;
			// htmlize!
			while (*ptr != 0) {
				if (*ptr == '\n') {
					*ptr = '\0';
					msg += QString(str) + "<br/>\n";
					ptr++;

					while (*ptr == ' ') {
						msg += "&nbsp;&nbsp;";
						ptr++;
					}
					str = ptr;
					continue;
				}
				ptr++;
			}
			free(cmd);
		}

		msg += "</td></tr></table>";
		procView_->setHtml(msg);
		storage_seccomp_ = msg;
	}
}


void StatsDialog::updateCaps() {
	DbPid *dbptr = Db::instance().findPid(pid_);
	if (!dbptr) {
		mode_ = MODE_TOP;
		return;
	}

	QString msg = storage_caps_;
	if (msg.isEmpty()) {
		if (arg_debug)
			printf("reading caps configuration\n");
		msg = header() + storage_intro_;
		msg += "<table><tr><td width=\"5\"></td><td>";

		char *str = 0;
		char *cmd;
		if (asprintf(&cmd, "firejail --caps.print=%d", pid_) != -1) {
			str = run_program(cmd);
			char *ptr = str;
			// htmlize!
			int cnt = 0;
			while (*ptr != 0) {
				if (*ptr == '\n') {
					// print only caps supported by the current kernel
					if (cnt >= caps_cnt_)
						break;
					cnt++;

					*ptr = '\0';
					msg += QString(str) + "<br/>\n";
					ptr++;
					str = ptr;
					continue;
				}
				ptr++;
			}
			free(cmd);
		}

		msg += "</pre></td></tr></table>";
		procView_->setHtml(msg);
		storage_caps_ = msg;
	}
}

static QString get_dns(int pid) {
	QString rv;

	char *str = 0;
	char *cmd;
	if (asprintf(&cmd, "firejail --dns.print=%d", pid) != -1) {
		str = run_program(cmd);
		char *ptr = str;

		// htmlize!
		while (*ptr != 0) {
			if (*ptr == '\n') {
				*ptr = '\0';
				bool skip = false;
				if (*str == '#')
					skip = true;
				if (!skip)
					rv += QString(str) + "<br/>\n";
				ptr++;

				while (*ptr == ' ') {
					if (!skip)
						rv += "&nbsp;&nbsp;";
					ptr++;
				}
				str = ptr;
				continue;
			}
			ptr++;
		}
	}
	free(cmd);
	return rv;
}

// build the network interface list for firejail versions 0.9.56 or older, including 0.9.56-LTS
static QString get_interfaces_old(int pid) {
	QString rv;

	char *fname;
	if (asprintf(&fname, "/run/firejail/network/%d-netmap", pid) == -1)
		errExit("asprintf");

	FILE *fp = fopen(fname, "r");
	if (fp) {
		char buf[4096];
		int i = -1;
		while (fgets(buf, 4096, fp)) {
			i++;
			char *ptr = strchr(buf, '\n');
			if (ptr)
				*ptr = '\0';

			// extract parent device
			ptr = strchr(buf, ':');
			if (!ptr)
				continue;
			char *parent_dev = buf;
			*ptr = '\0';
			ptr++;
			char *child_dev = ptr;

			QString str;
			str.sprintf("%s (parent device %s", child_dev, parent_dev);

			// detect bridge device
			char *sysfile;
			if (asprintf(&sysfile, "/sys/class/net/%s/bridge", parent_dev) == -1)
				errExit("asprintf");
			struct stat s;
			if (stat(sysfile, &s) == 0)
				str += ", bridge)";
			else
				str += ")";
			free(sysfile);

			rv += str + "<br/>";
		}
		fclose(fp);
	}
	free(fname);

	return rv;
}

// build the network interface list for firejail versions 0.9.57 and up
// returns an empty string if --net.print is not available in the currently installed firejail version
static QString get_interfaces_new(int pid) {
	QString rv;
	char *str = 0;
	char *cmd;
	if (asprintf(&cmd, "firejail --net.print=%d 2>&1", pid) != -1) {
		str = run_program(cmd);
		free(cmd);

		// htmlize!
		char *ptr = strtok(str, "\n");
		if (!ptr || strncmp(ptr, "Error", 5) == 0)
			goto errexit;
		while ((ptr = strtok(NULL, "\n")) != NULL) {
			if (strncmp(ptr, "Error", 5) == 0)
				goto errexit;
			if (strncmp(ptr, "Interface ", 10) == 0)
				continue;
			if (strncmp(ptr, "lo ", 3) == 0)
				continue;

			// parse the interface line, example
			//eth0-12202       c6:7f:d1:a9:3d:bc  192.168.1.82     255.255.255.0    UP
			// ifname
			char *ifname = ptr;
			while (*ptr != ' ' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto errexit;
			*ptr = '\0';
			ptr++;

			// skip mac address
			while (*ptr == ' ')
				ptr++;
			while (*ptr != ' ' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto errexit;
			while (*ptr == ' ')
				ptr++;

			// ip address
			char *ip = ptr;
			while (*ptr != ' ' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto errexit;
			*ptr = '\0';
			ptr++;
			while (*ptr == ' ')
				ptr++;

			// extract mask...
			char *mask	= ptr;
			while (*ptr != ' ' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto errexit;
			*ptr = '\0';
			// ... and build a CIDR addrss
			uint32_t mask_uint32;
			if (atoip(mask, &mask_uint32))
				goto errexit;
			int bits = mask2bits(mask_uint32);
			rv += QString(ifname) + "&nbsp;&nbsp;&nbsp;" + QString(ip) + "/" +
				QString::number(bits) + "<br/>";
		}
	}

	return rv;

errexit:
	return QString(); // empty string
}

void StatsDialog::updateNetwork() {
	int cycle = Db::instance().getCycle();
	assert(cycle < DbPid::MAXCYCLE);
	DbPid *dbptr = Db::instance().findPid(pid_);
	if (!dbptr) {
		mode_ = MODE_TOP;
		return;
	}

	// DNS
	QString msg = header() + storage_intro_;
	if (storage_dns_.isEmpty()) {
		if (arg_debug)
			printf("reading dns configuration\n");

		storage_dns_ += "<table><tr><td width=\"5\"></td><td><b>DNS</b><br/>";
		storage_dns_ += get_dns(pid_);
		storage_dns_ += "</td>";
	}
	msg += storage_dns_;

	// network interfaces
	if (storage_network_.isEmpty()) {
//printf("network namespace disabled %d, net_none_ %d\n", dbptr->networkDisabled(), net_none_);
		if (net_none_)
			storage_network_ = "<td><b>Network Interfaces</b><br/>lo<br/>";
		else if (dbptr->networkDisabled())
			storage_network_ = "<td>Using the system network namespace";
		else {

			storage_network_ = "<td><b>Network Interfaces</b><br/>lo<br/>";
			QString tmp = get_interfaces_new(pid_);
			if (tmp.isEmpty())
				tmp = get_interfaces_old(pid_);
			storage_network_ += tmp;
		}
		storage_network_ += "</td></tr>";

	}
	msg += storage_network_;



	// graph type
	msg += "<tr><td></td>";
	if (dbptr->networkDisabled() == false && net_none_ == false) {
		if (graph_type_ == GRAPH_4MIN) {
			msg += "<td><b>Stats: </b>1min <a href=\"1h\">1h</a> <a href=\"12h\">12h</a></td>";
		}
		else if (graph_type_ == GRAPH_1H) {
			msg += "<td><b>Stats: </b><a href=\"1min\">1min</a> 1h <a href=\"12h\">12h</a></td>";
		}
		else if (graph_type_ == GRAPH_12H) {
			msg += "<td><b>Stats: </b><a href=\"1min\">1min</a> <a href=\"1h\">1h</a> 12h</td>";
		}
		else
			assert(0);
	}

	// netfilter
	if (dbptr->networkDisabled() == false && net_none_ == false)
		msg += "<td><b>Firewall</b>: <a href=\"firewall\">enabled</a></td></tr>\n";
	else
		msg += "<td><b>Firewall</b>: system firewall</td></tr>\n";


	if (dbptr->networkDisabled() == false && net_none_ == false)
		msg += "<tr><td></td><td>"+ graph(2, dbptr, cycle, graph_type_) + "</td><td>" + graph(3, dbptr, cycle, graph_type_) + "</td></tr>";

	msg += QString("</table><br/>");

	// bandwidth limits
	if (dbptr->networkDisabled() == false && net_none_ == false) {
		char *fname;
		if (asprintf(&fname, "/run/firejail/bandwidth/%d-bandwidth", pid_) == -1)
			errExit("asprintf");
		FILE *fp = fopen(fname, "r");
		if (fp) {
			msg += "<br/><table><tr><td width=\"5\"></td><td>";
			msg += "<b>Bandwidth limits:</b><br/><br/>\n";
			char buf[1024];
			while (fgets(buf, 1024, fp)) {
				msg += buf;
				msg += "<br/>";
			}
			fclose(fp);
			msg += "</td></tr></table>";
		}
		free(fname);
	}

	procView_->setHtml(msg);

}

void StatsDialog::kernelSecuritySettings() {
	if (arg_debug)
		printf("Checking security settings for pid %d\n", pid_);

	// reset all
	pid_seccomp_ = false;
	pid_caps_ = QString("");
	pid_cpu_cores_ = QString("");
	pid_protocol_ = QString("");
	pid_mem_deny_exec_ = QString("disabled");
	pid_apparmor_ = QString("");

	// caps
	char *cmd;
	if (asprintf(&cmd, "firemon --caps %d", pid_) == -1)
		return;
	char *str = run_program(cmd);
	if (str) {
		char *ptr = strstr(str, "CapBnd:");
		if (ptr)
			pid_caps_ = QString(ptr + 7);
		else
			pid_caps_ = QString("");
	}
	free(cmd);

	// seccomp
	if (asprintf(&cmd, "firemon --seccomp %d", pid_) == -1)
		return;
	str = run_program(cmd);
	if (str) {
		char *ptr = strstr(str, "Seccomp");
		if (ptr) {
			if (strstr(ptr, "2"))
				pid_seccomp_ = true;
		}
	}
	free(cmd);

	// cpu cores
	if (asprintf(&cmd, "firemon --cpu %d", pid_) == -1)
		return;
	str = run_program(cmd);
	if (str) {
		char *ptr = strstr(str, "Cpus_allowed_list:");
		if (ptr) {
			ptr += 18;
			pid_cpu_cores_ = QString(ptr);
		}
	}
	free(cmd);

	// protocols
	if (asprintf(&cmd, "firejail --protocol.print=%d", pid_) == -1)
		return;

	str = run_program(cmd);
	if (str) {
		if (strncmp(str, "Cannot", 6) == 0)
			pid_protocol_ = QString("disabled");
		else
			pid_protocol_ = QString(str);
	}
	free(cmd);

	// mem deny exec
	if (asprintf(&cmd, "firejail --ls=%d /run/firejail/mnt", pid_) == -1)
		return;
	str = run_program(cmd);
	if (str) {
		if (strstr(str, "seccomp.mdwx"))
			pid_mem_deny_exec_ = "enabled";
	}
	free(cmd);

	// apparmor
	if (asprintf(&cmd, "firejail --apparmor.print=%d", pid_) == -1)
		return;
	str = run_program(cmd);
	if (str) {
		const char *tofind = "AppArmor: ";
		char *ptr = strstr(str, tofind);
		if (ptr)
			pid_apparmor_ = QString(ptr + strlen(tofind));
	}
	free(cmd);
}

void StatsDialog::updatePid() {
	QString msg = "";

	int cycle = Db::instance().getCycle();
	assert(cycle < DbPid::MAXCYCLE);
	DbPid *ptr = Db::instance().findPid(pid_);
	if (!ptr) {
		mode_ = MODE_TOP;
		return;
	}

	const char *cmd = ptr->getCmd();
	if (!cmd) {
		mode_ = MODE_TOP;
		return;
	}

	// initialize static values
	if (pid_initialized_ == false) {
		kernelSecuritySettings();
		pid_noroot_ = userNamespace(pid_);
		pid_name_ = getName(pid_);
		profile_ = getProfile(pid_);
		pid_x11_ = getX11Display(pid_);
		pid_initialized_ = true;

		// detect --net=none
		int child = find_child(pid_);
		char *fname;
		if (asprintf(&fname, "/proc/%d/net/dev", child) == -1)
			errExit("asprintf");
		FILE *fp = fopen(fname, "r");
		if (fp) {
			char buf[4096];
			int cnt = 0;
			while (fgets(buf, 4096, fp))
				cnt++;
			fclose(fp);
			if (cnt <= 3)
				net_none_ = true;
			else
				net_none_ = false;
		}
		free(fname);
	}

	// get user name
	DbStorage *st = &ptr->data_4min_[cycle];
	struct passwd *pw = getpwuid(ptr->getUid());
	if (!pw)
		errExit("getpwuid");
	uid_ = pw->pw_uid;

	// add header
	msg += header();

	// add intro
	storage_intro_ = "<table>";
	if (!pid_name_.isEmpty())
		storage_intro_ += "<tr><td width=\"5\"></td><td><b>Sandbox name:</b> " + pid_name_ + "</td></tr>";
	storage_intro_ += "<tr><td width=\"5\"></td><td><b>Command:</b> " + QString(cmd) + "</td></tr>";
	if (!profile_.isEmpty())
		storage_intro_ += "<tr><td width=\"5\"></td><td><b>Profile:</b> " + profile_ + "</td></tr>";
	storage_intro_ += "</table><br/>";
	msg += storage_intro_;

	msg += "<table>";
	msg += QString("<tr><td width=\"5\"></td><td><b>PID:</b> ") +  QString::number(pid_) + "</td>";
	if (ptr->networkDisabled() || net_none_)
		msg += "<td><b>RX:</b> unknown</td></tr>";
	else
		msg += QString("<td><b>RX:</b> ") + QString::number(st->rx_) + " KB/s</td></tr>";

	msg += QString("<tr><td></td><td><b>User:</b> ") + pw->pw_name  + "</td>";
	if (ptr->networkDisabled() || net_none_)
		msg += "<td><b>TX:</b> unknown</td></tr>";
	else
		msg += QString("<td><b>TX:</b> ") + QString::number(st->tx_) + " KB/s</td></tr>";

	msg += QString("<tr><td></td><td><b>CPU:</b> ") + QString::number(st->cpu_) + "%</td>";
	msg += QString("<td><b>Seccomp:</b> ");
	if (pid_seccomp_)
		msg += "<a href=\"seccomp\">enabled</a>";
	else
		msg += "disabled";
	msg += "</td></tr>";

	msg += QString("<tr><td></td><td><b>Memory:</b> ") + QString::number((int) (st->rss_ + st->shared_)) + " KiB&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
	msg += QString("<td><b>Capabilities:</b> <a href=\"caps\">") + pid_caps_ + "</a></td></tr>";

	msg += QString("<tr><td></td><td><b>RSS</b> " + QString::number((int) st->rss_) + ", <b>shared</b> " + QString::number((int) st->shared_)) + "</td>";

	// user namespace
	msg += "<td><b>User Namespace:</b> ";

	if (pid_noroot_)
		msg += "enabled";
	else
		msg += "disabled";
	msg += "</td></tr>";

	msg += QString("<tr><td></td><td><b>CPU Cores:</b> ") + pid_cpu_cores_ + "</td>";
	if (pid_seccomp_)
		msg += QString("<td><b>Protocols:</b> ") + pid_protocol_ + "</td>";
	else
		msg += QString("<td><b>Protocols:</b> disabled</td>");
	msg += "</td></tr>";

	msg += "<tr><td></td>";

	// X11 display
	if (pid_x11_) {
		msg += "<td><b>X11 Dispaly:</b> " + QString::number(pid_x11_) + "</td>";
	}
	else
		msg +="<td></td>";

	// memory deny exec
	msg += "<td><b>Memory deny exec:</b> " + pid_mem_deny_exec_ + "</td></tr>";


	// apparmor
	if (!pid_apparmor_.isEmpty())
		msg += "<tr><td></td><td></td><td><b>AppArmor: </b>" + pid_apparmor_ + "</td></tr>";


	// graph type
	msg += "<tr></tr>";
	msg += "<tr><td></td>";
	if (graph_type_ == GRAPH_4MIN) {
		msg += "<td><b>Stats: </b>1min <a href=\"1h\">1h</a> <a href=\"12h\">12h</a></td>";
	}
	else if (graph_type_ == GRAPH_1H) {
		msg += "<td><b>Stats: </b><a href=\"1min\">1min</a> 1h <a href=\"12h\">12h</a></td>";
	}
	else if (graph_type_ == GRAPH_12H) {
		msg += "<td><b>Stats: </b><a href=\"1min\">1min</a> <a href=\"1h\">1h</a> 12h</td>";
	}
	else
		assert(0);

	// graphs
	msg += "<tr></tr>";
	msg += "<tr><td></td><td>"+ graph(0, ptr, cycle, graph_type_) + "</td><td>" + graph(1, ptr, cycle, graph_type_) + "</td></tr>";

	msg += QString("</table><br/>");

	procView_->setHtml(msg);
}

void StatsDialog::cycleReady() {
	if (mode_ == MODE_TOP)
		updateTop();
	else if (mode_ == MODE_FDNS)
		updateFdns();
	else if (mode_ == MODE_FDNS_DUMP)
		updateFdnsDump();
	else if (mode_ == MODE_PID)
		updatePid();
	else if (mode_ == MODE_TREE)
		updateTree();
	else if (mode_ == MODE_SECCOMP)
		updateSeccomp();
	else if (mode_ == MODE_NETWORK)
		updateNetwork();
	else if (mode_ == MODE_CAPS)
		updateCaps();
	else if (mode_ == MODE_FIREWALL)
		updateFirewall();
}

void StatsDialog::anchorClicked(const QUrl & link) {
	cleanStorage(); // full storage cleanup on any click
	QString linkstr = link.toString();

	if (linkstr == "top") {
		mode_ = MODE_TOP;
	}
	else if (linkstr == "back") {
		if (mode_ == MODE_PID)
			mode_ = MODE_TOP;
		else if (mode_ == MODE_TREE)
			mode_ = MODE_PID;
		else if (mode_ == MODE_SECCOMP)
			mode_ = MODE_PID;
		else if (mode_ == MODE_NETWORK)
			mode_ = MODE_PID;
		else if (mode_ == MODE_CAPS)
			mode_ = MODE_PID;
		else if (mode_ == MODE_FIREWALL)
			mode_ = MODE_PID;
		else if (mode_ == MODE_TOP)
			;
		else
			assert(0);
	}
	else if (linkstr == "tree") {
		mode_ = MODE_TREE;
	}
	else if (linkstr == "seccomp") {
		mode_ = MODE_SECCOMP;
	}
	else if (linkstr == "caps") {
		mode_ = MODE_CAPS;
	}
	else if (linkstr == "1h") {
		graph_type_ = GRAPH_1H;
	}
	else if (linkstr == "12h") {
		graph_type_ = GRAPH_12H;
	}
	else if (linkstr == "1min") {
		graph_type_ = GRAPH_4MIN;
	}
	else if (linkstr == "network") {
		mode_ = MODE_NETWORK;
	}
	else if (linkstr == "firewall") {
		mode_ = MODE_FIREWALL;
	}
	else if (linkstr == "shut") {
		QMessageBox msgBox;
		msgBox.setText(QString("Are you sure you want to shutdown PID ") + QString::number(pid_) + "?\n");
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int rv = msgBox.exec();
		if (rv == QMessageBox::Ok) {
			// shutdown sandbox
			QApplication::setOverrideCursor(Qt::WaitCursor);
			char *cmd;
			if (asprintf(&cmd, "firejail --shutdown=%d", pid_) != -1) {
				int rv = system(cmd);
				(void) rv;
				free(cmd);
			}
			QApplication::restoreOverrideCursor();
			mode_ = MODE_TOP;
		}
	}
	else if (linkstr == "join") {
		// join the process in a new xterm
		char *cmd;
		if (asprintf(&cmd, "xterm -T \"Firejail Sandbox %d\" -e firejail --join=%d&", pid_, pid_) != -1) {
			int rv = system(cmd);
			(void) rv;
			free(cmd);
		}
	}
	else if (linkstr == "fmgr") {
		char *cmd;
		if (asprintf(&cmd, PACKAGE_LIBDIR "/fmgr %d&", pid_) != -1) {
			int rv = system(cmd);
			(void) rv;
			free(cmd);
		}
	}
	else if (linkstr == "about") {
		QString msg = "<table cellpadding=\"10\"><tr><td><img src=\":/resources/fstats.png\"></td>";
		msg += "<td>" + tr(
			"Firetools is a GUI application for Firejail. "
			"It offers a system tray launcher for sandboxed apps, "
			"sandbox editing, management, and statistics. "
			"The software package also includes a sandbox configuration wizard, firejail-ui.<br/><br/>"
			"Firejail  is  a  SUID sandbox program that reduces the risk of security "
			"breaches by restricting the running environment of  untrusted  applications "
			"using Linux namespaces, Linux capabilities and seccomp-bpf.<br/><br/>") +
			tr("Firetools version:") + " " + PACKAGE_VERSION + "<br/>" +
			tr("QT version: ") + " " + QT_VERSION_STR + "<br/>" +
			tr("License:") + " GPL v2<br/>" +
			tr("Homepage:") + " " + QString(PACKAGE_URL) + "</td></tr></table><br/><br/>";

		QMessageBox::about(this, tr("About"), msg);

	}
	else if (linkstr == "fdns") {
		if (mode_ != MODE_FDNS_DUMP) {
			if (shm_file_name_)
				free(shm_file_name_);
			shm_file_name_ = find_fdns_shm_file_name();
			if (fdns_report_)
				fdns_report_ = 0;
			if (fdns_fd_) {
				::close(fdns_fd_);
				sleep(1); // give the kernel some time to close the shared mem file in order to open another one
			}
		}
		mode_ = MODE_FDNS;
	}
	else if (linkstr == "dump") {
		fdns_dump_ = QString("");
		mode_ = MODE_FDNS_DUMP;
	}
	else if (linkstr == "newsandbox") {
		// start firejail-ui as a separate process

		QProcess *process = new QProcess();
		QStringList arguments;
		arguments << "--nofiretools";
		process->startDetached(QString("firejail-ui"), arguments);
	}
	else if (linkstr == "launcher") {
		// start firejail-ui as a separate process
		int rv = system("firetools &");
		(void) rv;
	}
	else { // linstr == "home"
		pid_ = linkstr.toInt();
		pid_initialized_ = false;
		pid_caps_ = QString("");
		pid_name_ = QString("");
		pid_x11_ = 0;
		mode_ = MODE_PID;
	}

	// reset fdns
	fdns_first_run_ = true;

	if (data_ready)
		cycleReady();
}


static bool userNamespace(pid_t pid) {
	if (arg_debug)
		printf("Checking user namespace for pid %d\n", pid);

	// test user namespaces available in the kernel
	struct stat s1;
	struct stat s2;
	struct stat s3;
	if (stat("/proc/self/ns/user", &s1) == 0 &&
	    stat("/proc/self/uid_map", &s2) == 0 &&
	    stat("/proc/self/gid_map", &s3) == 0);
	else
		return false;

	pid = find_child(pid);
	if (pid == -1)
		return false;

	// read uid map
	char *uidmap;
	if (asprintf(&uidmap, "/proc/%u/uid_map", pid) == -1)
		errExit("asprintf");
	FILE *fp = fopen(uidmap, "r");
	if (!fp) {
		free(uidmap);
		return false;
	}

	// check uid map
	int u1;
	int u2;
	bool found = false;
	if (fscanf(fp, "%d %d", &u1, &u2) == 2) {
		if (u1 != 0 || u2 != 0)
			found = true;
	}
	fclose(fp);
	free(uidmap);
	return found;
}

static QString getName(pid_t pid) {
	QString retval("");

	char *fname;
	if (asprintf(&fname, "/run/firejail/name/%d", (int) pid) == -1)
		errExit("asprintf");
	FILE *fp = fopen(fname, "r");
	if (fp) {
		char name[250];
		if (fgets(name, 250, fp))
			retval = QString(name);
		fclose(fp);
	}
	free(fname);

	return retval;
}

static QString getProfile(pid_t pid) {
	QString retval("");

	char *fname;
	if (asprintf(&fname, "/run/firejail/profile/%d", (int) pid) == -1)
		errExit("asprintf");
	FILE *fp = fopen(fname, "r");
	if (fp) {
		char name[250];
		if (fgets(name, 250, fp))
			retval = QString(name);
		fclose(fp);
	}
	free(fname);

	return retval;
}

static int getX11Display(pid_t pid) {
	int retval = 0;

	char *fname;
	if (asprintf(&fname, "/run/firejail/x11/%d", (int) pid) == -1)
		errExit("asprintf");
	FILE *fp = fopen(fname, "r");
	if (fp) {
		int val;
		if (fscanf(fp, "%d", &val) == 1)
			retval = val;
		fclose(fp);
	}
	free(fname);

	return retval;
}
