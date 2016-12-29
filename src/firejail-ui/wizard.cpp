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
#include "wizard.h"
#include "home_widget.h"
#include "help_widget.h"
#include "appdb.h"

Wizard::Wizard(QWidget *parent): QWizard(parent) {
	setPage(Page_Config, new ConfigPage);
	setPage(Page_StartSandbox, new StartSandboxPage);

	setStartId(Page_Config);
	setOption(HaveHelpButton, true);
	setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo.png"));

	connect(this, SIGNAL(helpRequested()), this, SLOT(showHelp()));

	setWindowTitle(tr(" Wizard"));
}

void Wizard::showHelp() {
	HelpWidget hw;
	hw.exec();

}

#include <QProcess>
void Wizard::accept() {
	if (arg_debug)
		printf("Wizard::accept\n");

	// start a new process,
	QProcess *process = new QProcess();
	QStringList arguments;
	arguments << field("command").toString();
	process->startDetached(QString("firejail"), arguments);
	process->waitForStarted(1500);
	printf("Sandbox started\n");

	// force a program exit
	exit(0);		
}


ConfigPage::ConfigPage(QWidget *parent): QWizardPage(parent) {
	setTitle(tr("Configure Firejail Sandbox"));
	setSubTitle(tr("Firejail secures your Linux applications using the latest sandboxing technologies available "
		"in Linux kernel. Please configure the sandbox."));

	whitelisted_home_ = new QCheckBox("Restrict /home directory");
	private_dev_ = new QCheckBox("Restrict /dev directory");
	private_dev_->setChecked(true);
	private_tmp_ = new QCheckBox("Restrict /tmp directory");
	private_tmp_->setChecked(true);
	mnt_media_ = new QCheckBox("Restrict /mnt and /media");
	mnt_media_->setChecked(true);

	QGroupBox *fs_box = new QGroupBox(tr("File System"));
	QVBoxLayout *fs_box_layout = new QVBoxLayout;
	fs_box_layout->addWidget(whitelisted_home_);
	fs_box_layout->addWidget(private_dev_);
	fs_box_layout->addWidget(private_tmp_);
	fs_box_layout->addWidget(mnt_media_);
	fs_box->setLayout(fs_box_layout);
//	fs_box->setFlat(false);
//	fs_box->setCheckable(true);
	
	nosound_ = new QCheckBox("Disable sound");
	no3d_ = new QCheckBox("Disable 3D acceleration");
	nox11_ = new QCheckBox("Disable X11 support");
	nox11_->setEnabled(false);
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
	nonetwork_ = new QRadioButton("Disable networking");
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
	
	seccomp_ = new QCheckBox("seccomp, capabilities, nonewprivs");
	seccomp_->setChecked(true);
	apparmor_ = new QCheckBox("AppArmor");
//	apparmor_->setEnabled(false);
	overlayfs_ = new QCheckBox("OverlayFS");
//	overlayfs_->setEnabled(false);
	QGroupBox *kernel_box = new QGroupBox(tr("Kernel"));
	QVBoxLayout *kernel_box_layout = new QVBoxLayout;
	kernel_box_layout->addWidget(seccomp_);
	kernel_box_layout->addWidget(apparmor_);
	kernel_box_layout->addWidget(overlayfs_);
	kernel_box->setLayout(kernel_box_layout);

	QGridLayout *layout = new QGridLayout;
	layout->addWidget(fs_box, 0, 0);
	layout->addWidget(home_box, 0, 1, 2, 1);
	layout->addWidget(net_box, 1, 0);
	layout->addWidget(multimed_box, 2, 0);
	layout->addWidget(kernel_box, 2, 1);
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


StartSandboxPage::StartSandboxPage(QWidget *parent): QWizardPage(parent) {
	setTitle(tr("Start the Application"));
	setSubTitle(tr("Choose an application form the menus below, or type in the application name."));

	QGroupBox *app_box = new QGroupBox(tr("Applications"));
	QGridLayout *app_box_layout = new QGridLayout;
	group_ = new QListWidget;
	command_ = new QLineEdit;
	QLabel *label = new QLabel("Application:");
	app_ = new QListWidget;
	app_->setMinimumWidth(300);
	app_box_layout->addWidget(group_, 0, 0);
	app_box_layout->addWidget(app_, 0, 1);
	app_box_layout->addWidget(label, 1, 0);
	app_box_layout->addWidget(command_, 2, 0, 1, 2);
	app_box->setLayout(app_box_layout);
	
	
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(app_box, 0, 0);
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

void StartSandboxPage::groupClicked(QListWidgetItem *item) {
	QString group = item->text();
	if (arg_debug)
		printf("StartSandboxPage::groupClicked %s\n", group.toLatin1().data());


	appdb_load_app(appdb_, app_, group);
	app_->repaint();
}

void StartSandboxPage::appClicked(QListWidgetItem *item) {
	QString app = item->text();
	if (arg_debug)
		printf("StartSandboxPage::appClicked %s\n", app.toLatin1().data());


	appdb_set_command(appdb_, command_, app);
//	command_->repaint();
}



int StartSandboxPage::nextId() const {
	return -1;
}
