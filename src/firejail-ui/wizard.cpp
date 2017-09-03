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

#if QT_VERSION >= 0x050000
	#include <QtWidgets>
#else
	#include <QtGui>
#endif

#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QProcess>
#include <QPushButton>
#include <QFileDialog>
#include "../../firetools_config_extras.h"
#include "wizard.h"
#include "home_widget.h"
#include "help_widget.h"
#include "appdb.h"
#include <unistd.h>

//QString global_title("Firejail Configuration Wizard");
QString global_title("");

QString global_subtitle(
	"<b>Firejail</b> reduces the  risk  of  security "
	"breaches  by  restricting the running environment of untrusted "
	"applications using the latest Linux kernel sandboxing technologies."
);
HomeWidget *global_home_widget;
QString global_ifname = "";
bool global_dns_enabled = false;
bool global_protocol_enabled = false;

Wizard::Wizard(QWidget *parent): QWizard(parent) {
	setPage(Page_Application, new ApplicationPage);
	setPage(Page_Config, new ConfigPage);
	setPage(Page_Config2, new ConfigPage2);
	setPage(Page_StartSandbox, new StartSandboxPage);
	setStartId(Page_Application);
	
	setOption(HaveHelpButton, true);

	connect(this, SIGNAL(helpRequested()), this, SLOT(showHelp()));

	setWindowTitle(tr("Firejail Configuration Wizard"));
	
	setWizardStyle(QWizard::MacStyle);
	setPixmap(QWizard::BackgroundPixmap, QPixmap(":/resources/background.png"));
	//resize( QSize(600, 400).expandedTo(minimumSizeHint()) );

}

void Wizard::showHelp() {
	HelpWidget hw;
	hw.exec();
}



void Wizard::accept() {
	if (arg_debug)
		printf("Wizard::accept\n");
	QStringList arguments;

	if (field("use_custom").toBool()) {
		if (arg_debug)
			printf("building a custom profile\n");
	
		// build the profile in a termporary file
		char profname[] = "/tmp/firejail-ui-XXXXXX";
		int fd = mkstemp(profname);
		if (fd == -1)
			errExit("mkstemp");
		QString profarg = QString("--profile=") + QString(profname);
		arguments << profarg;
	
		// always print the profile on stdout
		printf("\n");
		printf("############## start of profile file\n");

		// include	
		dprintf(fd, "include /etc/firejail/disable-common.inc\n");
		dprintf(fd, "include /etc/firejail/disable-passwdmgr.inc\n");
		printf("include /etc/firejail/disable-common.inc\n");
		printf("include /etc/firejail/disable-passwdmgr.inc\n");
			
		// home directory
		if (field("restricted_home").toBool()) {
			QString whitelist = global_home_widget->getContent();
			if (whitelist.isEmpty())
				whitelist = QString("private\n");
			else
				whitelist += QString("include /etc/firejail/whitelist-common.inc\n");
			dprintf(fd, "%s", whitelist.toUtf8().data());
			printf("%s", whitelist.toUtf8().data());
		}
		
		// filesystem
		if (field("private_tmp").toBool()) {
			dprintf(fd, "private-tmp\n");
			printf("private-tmp\n");
		}
		if (field("private_dev").toBool()) {
			dprintf(fd, "private-dev\n");
			printf("private-dev\n");
		}
		if (field("mnt_media").toBool()) {
			dprintf(fd, "blacklist /mnt\n");
			dprintf(fd, "blacklist /media\n");
			printf("blacklist /mnt\nblacklist /media\n");
		}
	
		// network
		if (field("sysnetwork").toBool()) {
			;
		}
		else if (field("nonetwork").toBool()) {
			dprintf(fd, "net none\n");
			printf("net none\n");
		}	
		else if (field("netnamespace").toBool()) {
			dprintf(fd, "net %s\nnetfilter\n", global_ifname.toUtf8().data());
			printf("net %s\nnetfilter\n", global_ifname.toUtf8().data());
		}
		
		// dns
		if (global_dns_enabled) {
			QString dns1 = field("dns1").toString();
			if (!dns1.isEmpty()) {
				char *d1 = dns1.toUtf8().data();
				dprintf(fd, "dns %s\n", d1);
				printf("dns %s\n", d1);
			}
			QString dns2 = field("dns2").toString();
			if (!dns2.isEmpty()) {
				char *d2 = dns2.toUtf8().data();
				dprintf(fd, "dns %s\n", d2);
				printf("dns %s\n", d2);
			}
		}
		
		// network protocol
		if (global_protocol_enabled) {
			if (field("protocol_unix").toBool() ||
			    field("protocol_inet").toBool() ||
			    field("protocol_inet6").toBool() ||
			    field("protocol_netlink").toBool() ||
			    field("protocol_packet").toBool()) {
				QString protocol = QString("protocol ");
				if (field("protocol_unix").toBool())
					protocol += QString("unix,");
				if (field("protocol_inet").toBool())
					protocol += QString("inet,");				
				if (field("protocol_inet6").toBool())
					protocol += QString("inet6,");				
				if (field("protocol_netlink").toBool())
					protocol += QString("netlink,");				
				if (field("protocol_packet").toBool())
					protocol += QString("packet");				
				
				char *str = protocol.toUtf8().data();
				dprintf(fd, "%s\n", str);
				printf("%s\n", str);
			}
		}
		
		// multimedia
		if (field("nosound").toBool()) {
			dprintf(fd, "nosound\n");
			printf("nosound\n");
		}
		if (field("no3d").toBool()) {
			dprintf(fd, "no3d\n");
			printf("no3d\n");
		}
		if (field("nox11").toBool()) {
			dprintf(fd, "x11 none\n");
			printf("x11 none\n");
		}
		if (field("nodvd").toBool()) {
			dprintf(fd, "nodvd\n");
			printf("nodvd\n");
		}
		if (field("novideo").toBool()) {
			dprintf(fd, "novideo\n");
			printf("novideo\n");
		}
		if (field("notv").toBool()) {
			dprintf(fd, "notv\n");
			printf("notv\n");
		}
		
			
		// kernel
		if (field("seccomp").toBool()) {
			dprintf(fd, "seccomp\n");
			dprintf(fd, "nonewprivs\n");
			printf("seccomp\nnonewprivs\n");
		}
		if (field("caps").toBool()) {
			dprintf(fd, "caps.drop all\n");
			printf("caps.drop all\n");
		}
		if (field("noroot").toBool()) {
			dprintf(fd, "noroot\n");
			printf("noroot\n");
		}
		
		printf("############# end of profile file\n");
		printf("\n");
	}
	
	// debug
	if (field("debug").toBool())
		arguments << QString("--debug");
	if (field("trace").toBool())
		arguments << QString("--trace");

	// split command into argumentsd
	QString cmd = field("command").toString();
	QStringList cmds = cmd.split( " " );
	arguments += cmds;

	// start a new process,
	QProcess *process = new QProcess();
	process->startDetached(QString("firejail"), arguments);
	sleep(1);
	printf("Sandbox started, exiting firejail-ui...\n");

	if (field("mon").toBool()) {
		int rv = system(PACKAGE_LIBDIR "/fstats &");
		(void) rv;
	}

	// force a program exit
	exit(0);		
}

ApplicationPage::ApplicationPage(QWidget *parent): QWizardPage(parent) {
	setTitle(global_title);
	setSubTitle(global_subtitle);

	// fonts
	QFont bold;
	bold.setBold(true);
	QFont oldFont;
	oldFont.setBold(false);

	QGroupBox *app_box = new QGroupBox(tr("Step 1: Choose an application"));
	app_box->setFont(bold);
	app_box->setStyleSheet("QGroupBox { color : black; }");
	
	QLabel *label1 = new QLabel(tr("Choose an application form the menus below"));
	label1->setFont(oldFont);
	label1->setStyleSheet("QLabel { color : black; }");

	QGridLayout *app_box_layout = new QGridLayout;
	group_ = new QListWidget;
	group_->setFont(oldFont);
	group_->setStyleSheet("QGridLayout { color : black; }");
	command_ = new QLineEdit;
	command_->setFont(oldFont);
	group_->setStyleSheet("QLineEdit { color : black; }");
	
	browse_ = new QPushButton("browse filesystem");
	QIcon icon(":resources/gnome-fs-directory.png");
	browse_->setIcon(icon);
	connect(browse_, SIGNAL(clicked()), this, SLOT(browseClicked()));
	
	QLabel *label2 = new QLabel("or type in the program name:");
	label2->setFont(oldFont);
	label2->setStyleSheet("QLabel { color : black; }");
	app_ = new QListWidget;
	app_->setFont(oldFont);
	app_->setStyleSheet("QListWidget { color : black; }");
	app_->setMinimumWidth(300);
	app_box_layout->addWidget(label1, 0, 0, 1, 2);
	app_box_layout->addWidget(group_, 1, 0);
	app_box_layout->addWidget(app_, 1, 1);
	app_box_layout->addWidget(browse_, 2, 0);
	app_box_layout->addWidget(label2, 2, 1);
	app_box_layout->addWidget(command_, 3, 0, 1, 2);
	app_box->setLayout(app_box_layout);
	
	QGroupBox *profile_box = new QGroupBox(tr("Step 2: Choose a security profile"));
	profile_box->setFont(bold);
	profile_box->setStyleSheet("QGroupBox { color : black; }");
	use_default_ = new QRadioButton("Use a default security profile");	
	use_default_->setFont(oldFont);
	use_default_->setStyleSheet("QRadioButton { color : black; }");
	use_default_->setChecked(true);
	use_custom_ = new QRadioButton("Build a custom security profile");
	use_custom_->setFont(oldFont);
	use_custom_->setStyleSheet("QRadioButton { color : black; }");
	QVBoxLayout *profile_box_layout = new QVBoxLayout;
	profile_box_layout->addWidget(use_default_);
	profile_box_layout->addWidget(use_custom_);
	profile_box->setLayout(profile_box_layout);
	
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(app_box, 0, 0);
	layout->addWidget(profile_box, 1, 0);
	setLayout(layout);
	
	// load database
	appdb_ = appdb_load_file();
	if (arg_debug)
		appdb_print_list(appdb_);
	appdb_load_group(appdb_, group_);

	// connect widgets
	connect(group_, SIGNAL(itemClicked(QListWidgetItem*)), 
	            this, SLOT(groupClicked(QListWidgetItem*)));
	connect(app_, SIGNAL(itemClicked(QListWidgetItem*)), 
	            this, SLOT(appClicked(QListWidgetItem*)));
	            
	registerField("command*", command_);
	registerField("use_custom", use_custom_); 
}

void ApplicationPage::browseClicked() {
	QString fname = QFileDialog::getOpenFileName(this, tr("Choose Application"));
	if (fname.isNull())
		return;
		
	// check the file is an executable
	const char *cmd = fname.toUtf8().data();
	if (arg_debug)
		printf("Command: %s\n", cmd);
	if (access(cmd, X_OK))
		QMessageBox::warning(this, "Error", "The file is not an executable program" ); 
	else
		command_->setText(fname);
}

void ApplicationPage::groupClicked(QListWidgetItem *item) {
	QString group = item->text();
	if (arg_debug)
		printf("ApplicationPage::groupClicked %s\n", group.toLatin1().data());


	appdb_load_app(appdb_, app_, group);
	app_->repaint();
}

void ApplicationPage::appClicked(QListWidgetItem *item) {
	QString app = item->text();
	if (arg_debug)
		printf("ApplicationPage::appClicked %s\n", app.toLatin1().data());


	appdb_set_command(appdb_, command_, app);
}

int ApplicationPage::nextId() const {
	if (use_custom_->isChecked())
		return Wizard::Page_Config;
	else
		return Wizard::Page_StartSandbox;
}



ConfigPage::ConfigPage(QWidget *parent): QWizardPage(parent) {
	setTitle(global_title);
	setSubTitle(global_subtitle);

	QLabel *label1 = new QLabel(tr("<b>Step 3: Configure the sandbox</b>"));
	label1->setStyleSheet("QLabel { color : black; }");

	whitelisted_home_ = new QCheckBox("Restrict /home directory");
	whitelisted_home_->setStyleSheet("QCheckBox { color : black; }");
	registerField("restricted_home", whitelisted_home_);
	private_dev_ = new QCheckBox("Restrict /dev directory");
	private_dev_->setChecked(true);
	private_dev_->setStyleSheet("QCheckBox { color : black; }");
	registerField("private_dev", private_dev_);
	
	private_tmp_ = new QCheckBox("Restrict /tmp directory");
	private_tmp_->setChecked(true);
	private_tmp_->setStyleSheet("QCheckBox { color : black; }");
	registerField("private_tmp", private_tmp_);
	
	mnt_media_ = new QCheckBox("Restrict /mnt and /media");
	mnt_media_->setChecked(true);
	mnt_media_->setStyleSheet("QCheckBox { color : black; }");
	registerField("mnt_media", mnt_media_);

	QGroupBox *fs_box = new QGroupBox(tr("File System"));
	fs_box->setStyleSheet("QGroupBox { color : black; }");
	QVBoxLayout *fs_box_layout = new QVBoxLayout;
	fs_box_layout->addWidget(whitelisted_home_);
	fs_box_layout->addWidget(private_dev_);
	fs_box_layout->addWidget(private_tmp_);
	fs_box_layout->addWidget(mnt_media_);
	fs_box->setLayout(fs_box_layout);
//	fs_box->setFlat(false);
//	fs_box->setCheckable(true);
	
	
	// networking
	global_ifname = detect_network();
	sysnetwork_ = new QRadioButton("System network");
	sysnetwork_->setChecked(true);
	sysnetwork_->setStyleSheet("QRadioButton { color : black; }");
	registerField("sysnetwork", sysnetwork_);
	
	nonetwork_ = new QRadioButton("Disable networking");
	nonetwork_->setStyleSheet("QRadioButton { color : black; }");
	registerField("nonetwork", nonetwork_);
	
	if (global_ifname.isEmpty()) {
		netnamespace_ = new QRadioButton("Namespace");
		netnamespace_->setEnabled(false);
	}
	else
		netnamespace_ = new QRadioButton(QString("Namespace (") + global_ifname + ")");
	netnamespace_->setStyleSheet("QRadioButton { color : black; }");
	registerField("netnamespace", netnamespace_);
	QGroupBox *net_box = new QGroupBox(tr("Networking"));
	net_box->setStyleSheet("QGroupBox { color : black; }");
	QVBoxLayout *net_box_layout = new QVBoxLayout;
	net_box_layout->addWidget(sysnetwork_);
	net_box_layout->addWidget(netnamespace_);
	net_box_layout->addWidget(nonetwork_);
	net_box->setLayout(net_box_layout);

	home_ = new HomeWidget;
	QGroupBox *home_box = new QGroupBox(tr("Home Directory"));
	home_box->setStyleSheet("QGroupBox { color : black; }");
	QVBoxLayout *home_box_layout = new QVBoxLayout;
	home_box_layout->addWidget(home_);
	home_box->setLayout(home_box_layout);
	home_->setEnabled(false);
	connect(whitelisted_home_, SIGNAL(toggled(bool)), this, SLOT(setHome(bool)));
	global_home_widget = home_;


	// DNS	
	dns1_ = new QLineEdit;
	dns1_->setText("8.8.8.8");
	dns1_->setMaximumWidth(150);
	dns1_->setFixedWidth(170);
	registerField("dns1", dns1_);
	
	dns2_ = new QLineEdit;
	dns2_->setText("8.8.4.4");
	dns2_->setMaximumWidth(150);
	dns2_->setFixedWidth(170);
	registerField("dns2", dns2_);

	QGroupBox *dns_box = new QGroupBox(tr("DNS"));
	dns_box->setCheckable(true);
	dns_box->setChecked(false);
	dns_box->setStyleSheet("QGroupBox { color : black; }");
	connect(dns_box, SIGNAL(toggled(bool)), this, SLOT(setDns(bool)));
	QVBoxLayout *dns_box_layout = new QVBoxLayout;
	dns_box_layout->addWidget(dns1_);
	dns_box_layout->addWidget(dns2_);
	dns_box->setLayout(dns_box_layout);

	// protocol
	protocol_unix_ = new QCheckBox("unix");
	protocol_unix_->setChecked(true);
	registerField("protocol_unix", protocol_unix_);
	protocol_inet_ = new QCheckBox("inet");
	protocol_inet_->setChecked(true);
	registerField("protocol_inet", protocol_inet_);
	protocol_inet6_ = new QCheckBox("inet6");
	protocol_inet6_->setChecked(true);
	registerField("protocol_inet6", protocol_inet6_);
	protocol_netlink_ = new QCheckBox("netlink");
	protocol_netlink_->setChecked(false);
	registerField("protocol_netlink", protocol_netlink_);
	protocol_packet_ = new QCheckBox("packet");
	protocol_packet_->setChecked(false);
	registerField("protocol_packet", protocol_packet_);
	
	QGroupBox *protocol_box = new QGroupBox(tr("Network Protocol"));
	protocol_box->setCheckable(true);
	protocol_box->setChecked(false);
	protocol_box->setStyleSheet("QGroupBox { color : black; }");
	connect(protocol_box, SIGNAL(toggled(bool)), this, SLOT(setProtocol(bool)));
	QGridLayout *protocol_box_layout = new QGridLayout;
	protocol_box_layout->addWidget(protocol_unix_, 0, 0);
	protocol_box_layout->addWidget(protocol_inet_, 0, 1);
	protocol_box_layout->addWidget(protocol_inet6_, 1, 0);
	protocol_box_layout->addWidget(protocol_netlink_, 1, 1);
	protocol_box_layout->addWidget(protocol_packet_, 2, 0);
	protocol_box->setLayout(protocol_box_layout);

	QWidget *w = new QWidget;
	w->setMinimumHeight(8);
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(label1, 0, 0);
	layout->addWidget(w, 1, 0);
	layout->addWidget(fs_box, 2, 0);
	layout->addWidget(home_box, 2, 1, 2, 1);
	layout->addWidget(net_box, 3, 0);
	layout->addWidget(dns_box, 4, 0);
	layout->addWidget(protocol_box, 4, 1);
	setLayout(layout);
}

bool ConfigPage::validatePage() {
	if (global_dns_enabled) {
		uint32_t addr;

		QString ip = dns1_->text();
		if (!ip.isEmpty()) {
			const char *str = ip.toUtf8().data();
			if (atoip(str, &addr)) {
				QMessageBox::warning(this, "Error", QString("Invalid IP address ") + ip); 
				return false;
			}
		}
		
		ip = dns2_->text();
		if (!ip.isEmpty()) {
			const char *str = ip.toUtf8().data();
			if (atoip(str, &addr)) {
				QMessageBox::warning(this, "Error", QString("Invalid IP address ") + ip); 
				return false;
			}
		}

		return true;
	}
	else
		return true;
}

void ConfigPage::setDns(bool on) {
	global_dns_enabled = on;
}

void ConfigPage::setProtocol(bool on) {
	global_protocol_enabled = on;
}

void ConfigPage::setHome(bool active) {
	home_->setEnabled(active);
}

int ConfigPage::nextId() const {
	return Wizard::Page_Config2;
}


ConfigPage2::ConfigPage2(QWidget *parent): QWizardPage(parent) {
	setTitle(global_title);
	setSubTitle(global_subtitle);

	QLabel *label1 = new QLabel(tr("<b>Step 3: Configure the sandbox... continued...</b>"));
	label1->setStyleSheet("QLabel { color : black; }");
	nosound_ = new QCheckBox("Disable sound");
	nosound_->setStyleSheet("QCheckBox { color : black; }");
	registerField("nosound", nosound_);
	
	nodvd_ = new QCheckBox("Disable CD-ROM/DVD devices");
	nodvd_->setStyleSheet("QCheckBox { color : black; }");
	registerField("nodvd", nodvd_);
	
	novideo_ = new QCheckBox("Disable video camera devices");
	novideo_->setStyleSheet("QCheckBox { color : black; }");
	registerField("novideo", novideo_);
	
	notv_ = new QCheckBox("Disable TV/DVB devices");
	notv_->setStyleSheet("QCheckBox { color : black; }");
	registerField("notv", notv_);

	no3d_ = new QCheckBox("Disable 3D acceleration");
	no3d_->setStyleSheet("QCheckBox { color : black; }");
	registerField("no3d", no3d_);
	
	nox11_ = new QCheckBox("Disable X11 support");
	registerField("nox11", nox11_);
	
	QGroupBox *multimed_box = new QGroupBox(tr("Multimedia"));
	multimed_box->setStyleSheet("QGroupBox { color : black; }");
	QVBoxLayout *multimed_box_layout = new QVBoxLayout;
	multimed_box_layout->addWidget(nosound_);
	multimed_box_layout->addWidget(novideo_);
	multimed_box_layout->addWidget(nodvd_);
	multimed_box_layout->addWidget(notv_);
	multimed_box_layout->addWidget(no3d_);
	multimed_box_layout->addWidget(nox11_);
	multimed_box->setLayout(multimed_box_layout);
//	multimed_box->setFlat(false);
//	multimed_box->setCheckable(true);

	seccomp_ = new QCheckBox("Enable seccomp-bpf");
	if (kernel_major == 3 && kernel_minor < 5) {
	   	if (arg_debug)
	   		printf("disabling seccomp-bpf\n");
		seccomp_->setEnabled(false);
	}
	else
		seccomp_->setChecked(true);
	registerField("seccomp", seccomp_);
	
	caps_ = new QCheckBox("Disable all Linux capabilities");
	caps_->setChecked(true);
	caps_->setStyleSheet("QCheckBox { color : black; }");
	registerField("caps", caps_);
	
	noroot_ = new QCheckBox("Restricted  user namespace (noroot)");
	if (kernel_major == 3 && kernel_minor < 8) {
	   	if (arg_debug)
	   		printf("disabling noroot\n");
		noroot_->setEnabled(false);
	}
	else
		noroot_->setChecked(true);
	noroot_->setStyleSheet("QCheckBox { color : black; }");
	registerField("noroot", noroot_);

	QGroupBox *kernel_box = new QGroupBox(tr("Kernel"));
	kernel_box->setStyleSheet("QGroupBox { color : black; }");
	QVBoxLayout *kernel_box_layout = new QVBoxLayout;
	kernel_box_layout->addWidget(seccomp_);
	kernel_box_layout->addWidget(caps_);
	kernel_box_layout->addWidget(noroot_);
	kernel_box->setLayout(kernel_box_layout);

	QWidget *w = new QWidget;
	w->setMinimumHeight(8);
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(label1, 0, 0);
	layout->addWidget(w, 1, 0);
	layout->addWidget(multimed_box, 2, 0);
	layout->addWidget(kernel_box, 3, 0);
	setLayout(layout);
}

int ConfigPage2::nextId() const {
	return Wizard::Page_StartSandbox;
}


void ConfigPage2::initializePage() {
	if (field("sysnetwork").toBool())
		nox11_->setEnabled(false);
	else
		nox11_->setEnabled(true);
}


StartSandboxPage::StartSandboxPage(QWidget *parent): QWizardPage(parent) {
	setTitle(global_title);
	setSubTitle(global_subtitle);
	
	// fonts
	QFont bold;
	bold.setBold(true);
	QFont oldFont;
	oldFont.setBold(false);

	QGroupBox *debug_box = new QGroupBox(tr("Step 4: Debugging"));
	debug_box->setFont(bold);
	debug_box->setStyleSheet("QGroupBox { color : black; }");
	debug_ = new QCheckBox("Enable sandbox debugging");	
	debug_->setFont(oldFont);
	debug_->setStyleSheet("QCheckBox { color : black; }");
	trace_ = new QCheckBox("Trace filesystem and network access");
	trace_->setFont(oldFont);
	trace_->setStyleSheet("QCheckBox { color : black; }");
	mon_ = new QCheckBox("Sandbox monitoring and statistics");
	mon_->setFont(oldFont);
	mon_->setStyleSheet("QCheckBox { color : black; }");

	if (!isatty(0)) {
		debug_->setEnabled(false);
		trace_->setEnabled(false);
	}
	if (arg_nofiretools)
		mon_->setEnabled(false);

	QVBoxLayout *debug_box_layout = new QVBoxLayout;
	debug_box_layout->addWidget(debug_);
	debug_box_layout->addWidget(trace_);
	debug_box_layout->addWidget(mon_);
	debug_box->setLayout(debug_box_layout);

	QLabel *label1 = new QLabel(tr("Press <b>Done</b> to start the sandbox.<br/><br/>"
		"For more information, visit us at <b>http://firejail.wordpress.com</b>. "
		"Thank you for using Firejail!"));
	QWidget *empty1 = new QWidget;
	empty1->setMinimumHeight(12);
	QWidget *empty2 = new QWidget;
	empty2->setMinimumHeight(25);
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(empty1, 0, 0);
	layout->addWidget(debug_box, 1, 0);
	layout->addWidget(empty2, 2, 0);
	layout->addWidget(label1, 3, 0);
	setLayout(layout);

	registerField("debug", debug_);
	registerField("trace", trace_);
	registerField("mon", mon_);
}

int StartSandboxPage::nextId() const {
	return -1;
}
