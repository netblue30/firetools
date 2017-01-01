/*
 * Copyright (C) 2015-2016 Firetools Authors
 *
 * This file is part of firetools project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public  as published by
 * the Free Software Foundation; either version 2 of the , or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public  for more details.
 *
 * You should have received a copy of the GNU General Public  along
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
#include "wizard.h"
#include "home_widget.h"
#include "help_widget.h"
#include "appdb.h"

QString global_title("Firejail Configuration Wizard");

QString global_subtitle(
	"Firejail reduces the  risk  of  security "
	"breaches  by  restricting the running environment of untrusted "
	"applications using the latest Linux kernel sandboxing technologies."
);


Wizard::Wizard(QWidget *parent): QWizard(parent) {
	setPage(Page_Application, new ApplicationPage);
	setPage(Page_Config, new ConfigPage);
	setPage(Page_StartSandbox, new StartSandboxPage);
	setStartId(Page_Application);
	
	setOption(HaveHelpButton, true);

	connect(this, SIGNAL(helpRequested()), this, SLOT(showHelp()));

	setWindowTitle(tr("Firejal Wizard"));
	
	setWizardStyle(QWizard::MacStyle);
	setPixmap(QWizard::BackgroundPixmap, QPixmap(":/resources/firejail-ui.png"));
}

void Wizard::showHelp() {
	HelpWidget hw;
	hw.exec();
}

void Wizard::accept() {
	if (arg_debug)
		printf("Wizard::accept\n");
	QStringList arguments;

	// build the profile in a termporary file
	char profname[] = "/tmp/firejail-ui-XXXXXX";
	int fd = mkstemp(profname);
	if (fd == -1)
		errExit("mkstemp");
	QString profarg = QString("--profile=") + QString(profname);
	arguments << profarg;

	// include	
	dprintf(fd, "include /etc/firejail/disable-common.inc\n");
	dprintf(fd, "include /etc/firejail/disable-passwdmgr.inc\n");
	
	// filesystem
	dprintf(fd, "\n# filesystem\n");
	if (field("private_tmp").toBool())
		dprintf(fd, "private-tmp\n");
	if (field("private_dev").toBool())
		dprintf(fd, "private-dev\n");
	if (field("mnt_media").toBool()) {
		dprintf(fd, "blacklist /mnt\n");
		dprintf(fd, "blacklist /media\n");
	}
	if (field("overlayfs").toBool())
		arguments << QString("--overlay-tmpfs");

	// kernel
	dprintf(fd, "\n# kernel\n");
	if (field("seccomp").toBool()) {
		dprintf(fd, "seccomp\n");
		dprintf(fd, "caps.drop all\n");
		dprintf(fd, "nonewprivs\n");
	}
	if (field("noroot").toBool())
		dprintf(fd, "noroot\n");
	if (field("apparmor").toBool())
		dprintf(fd, "apparmor");
	
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

	// force a program exit
	exit(0);		
}


ConfigPage::ConfigPage(QWidget *parent): QWizardPage(parent) {
	setTitle(global_title);
	setSubTitle(global_subtitle);

	QLabel *label1 = new QLabel(tr("<b>Step 3: Configure the sandbox"));


	whitelisted_home_ = new QCheckBox("Restrict /home directory");
	private_dev_ = new QCheckBox("Restrict /dev directory");
	private_dev_->setChecked(true);
	registerField("private_dev", private_dev_);
	
	private_tmp_ = new QCheckBox("Restrict /tmp directory");
	private_tmp_->setChecked(true);
	registerField("private_tmp", private_tmp_);
	
	mnt_media_ = new QCheckBox("Restrict /mnt and /media");
	mnt_media_->setChecked(true);
	registerField("mnt_media", mnt_media_);

	overlayfs_ = new QCheckBox("OverlayFS");
//	overlayfs_->setEnabled(false);
	registerField("overlayfs", overlayfs_);

	QGroupBox *fs_box = new QGroupBox(tr("File System"));
	QVBoxLayout *fs_box_layout = new QVBoxLayout;
	fs_box_layout->addWidget(whitelisted_home_);
	fs_box_layout->addWidget(private_dev_);
	fs_box_layout->addWidget(private_tmp_);
	fs_box_layout->addWidget(mnt_media_);
	fs_box_layout->addWidget(overlayfs_);
	fs_box->setLayout(fs_box_layout);
//	fs_box->setFlat(false);
//	fs_box->setCheckable(true);
	
	nosound_ = new QCheckBox("Disable sound");
	registerField("nosound", nosound_);
	
	no3d_ = new QCheckBox("Disable 3D acceleration");
	registerField("no3d", no3d_);
	
	nox11_ = new QCheckBox("Disable X11 support");
	nox11_->setEnabled(false);
	registerField("nox11", nox11_);
	
	QGroupBox *multimed_box = new QGroupBox(tr("Multimedia"));
	QVBoxLayout *multimed_box_layout = new QVBoxLayout;
	multimed_box_layout->addWidget(nosound_);
	multimed_box_layout->addWidget(no3d_);
	multimed_box_layout->addWidget(nox11_);
	multimed_box->setLayout(multimed_box_layout);
//	multimed_box->setFlat(false);
//	multimed_box->setCheckable(true);


	
	
	sysnetwork_ = new QRadioButton("System network");
	sysnetwork_->setChecked(true);
	registerField("sysnetwork", sysnetwork_);
	
	nonetwork_ = new QRadioButton("Disable networking");
	registerField("nonetwork", nonetwork_);
	
	namespace_network_ = new QRadioButton("Namespace");	
	QGroupBox *net_box = new QGroupBox(tr("Networking"));
	QVBoxLayout *net_box_layout = new QVBoxLayout;
	net_box_layout->addWidget(sysnetwork_);
	net_box_layout->addWidget(namespace_network_);
	net_box_layout->addWidget(nonetwork_);
	net_box->setLayout(net_box_layout);
	connect(sysnetwork_, SIGNAL(toggled(bool)), this, SLOT(setX11(bool)));

	home_ = new HomeWidget;
	QGroupBox *home_box = new QGroupBox(tr("Home Directory"));
	QVBoxLayout *home_box_layout = new QVBoxLayout;
	home_box_layout->addWidget(home_);
	home_box->setLayout(home_box_layout);
	home_->setEnabled(false);
	connect(whitelisted_home_, SIGNAL(toggled(bool)), this, SLOT(setHome(bool)));
	
	seccomp_ = new QCheckBox("Seccomp etc.");
	seccomp_->setChecked(true);
	registerField("seccomp", seccomp_);
	
	noroot_ = new QCheckBox("Restricted  user namespace (noroot)");
//	apparmor_->setEnabled(false);
	registerField("noroot", noroot_);

	apparmor_ = new QCheckBox("AppArmor");
//	apparmor_->setEnabled(false);
	registerField("apparmor", apparmor_);

	QGroupBox *kernel_box = new QGroupBox(tr("Kernel"));
	QVBoxLayout *kernel_box_layout = new QVBoxLayout;
	kernel_box_layout->addWidget(seccomp_);
	kernel_box_layout->addWidget(noroot_);
	kernel_box_layout->addWidget(apparmor_);
	kernel_box->setLayout(kernel_box_layout);

	QWidget *w = new QWidget;
	w->setMinimumHeight(8);
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(label1, 0, 0);
	layout->addWidget(w, 1, 0);
	layout->addWidget(fs_box, 2, 0);
	layout->addWidget(home_box, 2, 1, 2, 1);
	layout->addWidget(net_box, 3, 0);
	layout->addWidget(multimed_box, 4, 0);
	layout->addWidget(kernel_box, 4, 1);
	setLayout(layout);
}

void ConfigPage::setX11(bool inactive) {
	nox11_->setEnabled(!inactive);
}

void ConfigPage::setHome(bool active) {
	home_->setEnabled(active);
}

int ConfigPage::nextId() const {
	return Wizard::Page_StartSandbox;
}


ApplicationPage::ApplicationPage(QWidget *parent): QWizardPage(parent) {
	setTitle(global_title);
	setSubTitle(global_subtitle);

	// fonts
	QFont bold;
	bold.setBold(true);
	QFont oldFont;
	oldFont.setBold(false);

	QGroupBox *app_box = new QGroupBox(tr("Step 1: Chose an application"));
	app_box->setFont(bold);
	
	QLabel *label1 = new QLabel(tr("Choose an application form the menus below, or type in the program name."));
	label1->setFont(oldFont);
	QGridLayout *app_box_layout = new QGridLayout;
	group_ = new QListWidget;
	group_->setFont(oldFont);
	command_ = new QLineEdit;
	command_->setFont(oldFont);
	QLabel *label2 = new QLabel("Program:");
	label2->setFont(oldFont);
	app_ = new QListWidget;
	app_->setFont(oldFont);
	app_->setMinimumWidth(300);
	app_box_layout->addWidget(label1, 0, 0, 1, 2);
	app_box_layout->addWidget(group_, 1, 0);
	app_box_layout->addWidget(app_, 1, 1);
	app_box_layout->addWidget(label2, 2, 0);
	app_box_layout->addWidget(command_, 3, 0, 1, 2);
	app_box->setLayout(app_box_layout);
	

	QGroupBox *profile_box = new QGroupBox(tr("Step 2: Chose a security profile"));
	profile_box->setFont(bold);
	use_default_ = new QRadioButton("Use a default security profile");	
	use_default_->setFont(oldFont);
	use_default_->setChecked(true);
	use_custom_ = new QRadioButton("Build a custom security profile");
	use_custom_->setFont(oldFont);
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


StartSandboxPage::StartSandboxPage(QWidget *parent): QWizardPage(parent) {
	setTitle(global_title);
	setSubTitle(global_subtitle);
	
	// fonts
	QFont bold;
	bold.setBold(true);
	QFont oldFont;
	oldFont.setBold(false);
	

	QGroupBox *debug_box = new QGroupBox(tr("Step 4: Sandbox Debugging"));
	debug_box->setFont(bold);
	debug_ = new QCheckBox("Enable sandbox debugging");	
	debug_->setFont(oldFont);
	trace_ = new QCheckBox("Trace filesystem and network access");
	trace_->setFont(oldFont);
	QVBoxLayout *debug_box_layout = new QVBoxLayout;
	debug_box_layout->addWidget(debug_);
	debug_box_layout->addWidget(trace_);
	debug_box->setLayout(debug_box_layout);

	QLabel *label1 = new QLabel(tr("Press <b>Finish</b> to start the sandbox.<br/><br/>"
		"For more information, visit us at <i>http://firejail.wordpress.com</i>. "
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
}

int StartSandboxPage::nextId() const {
	return -1;
}
