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
#include "wizard.h"
#include "home_widget.h"
#include "help_widget.h"

Wizard::Wizard(QWidget *parent)
: QWizard(parent) {
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

#if 0
	static QString lastHelpMessage;

	QString message;

	switch (currentId()) {
		case Page_Config:
			message = tr("<b>Sandbox Configuration:</b><br/>"
			"<br/>"
			"Firejail secures your Linux applications using the latest sandboxing technologies available "
			"in Linux kernel. Use this page to configure the sandbox, and "
			"press Next when finished.<br/>"
			"<br/>"
			"<b>Restrict home directory:</b> Choose the directories visible inside the sandbox. "
			"If disabled, all home files and directories are available inside the sandbox.<br/>"
			"<b>Restrict /dev directory:</b> Only a small number of devices are visible insde the sandbox. "
			"Sound and 3D acceleration should be available if this checkbox is set.<br/>"
			"<b>Restrict /tmp directory:</b> Start with a clean /tmp directory, only X11 directory is available "
			"under /tmp.<br/>"
			"<b>Restrict /mnt and /media:</b> Blacklist /mnt and /media directories.<br/>"
			"<b>System network:</b> Use the networking stack provided by the system.<br/>"
			"<b>Network namespace:</b> Install a separate networking stack.<br/>"
			"<b>Disable networking:</b> No network connectivity is available inside the sandbox.<br/>"
			"<b>Disable sound:</b> The sound subsystem is not available inside the sandbox.<br/>"
			"<b>Disable 3D acceleration:</b> Hardware acceleration drivers are disabled.<br/>"
			"<b>Disable X11:</b> X11 graphical user interface subsystem is disabled. "
			"Use this option when running console programs or servers.<br/>"
			"<b>seccomp, capabilities, nonewprivs:</b> These are some very powerfull security features "
			"implemented by the kernel. Try to use them allways. A Linux Kernel version 3.5 is required for this option to work.<br/>"
			"<b>AppArmor:</b> If available on your platform, this option implements a number "
			"of advanced security features currently available on GrSecurity kernels. Also, on "
			"Ubuntu 16.04 or later, this option disabled dBus access.<br/>"
			"<b>OverlayFS:</b> This option mounts an overlay filesystem on top of the sandbox. "
			"Any filesystem modifications are discarded when the sandbox is closed. "
			"A kernel 3.18 or newer is required by this option to work.<br/>"
			);
			
			break;
		case Page_StartSandbox:
			message = tr("Start Sandbox");
			break;
		default:
			message = tr("This help is likely not to be of any help.");
	}

	if (lastHelpMessage == message)
		message = tr("Sorry, I already gave what help I could. "
			"Maybe you should try asking a human?");

	QMessageBox::information(this, tr(" Wizard Help"), message);

	lastHelpMessage = message;
#endif	
}


ConfigPage::ConfigPage(QWidget *parent): QWizardPage(parent) {
	setTitle(tr("Configure Firejail Sandbox"));
	setSubTitle(tr("Firejail secures your Linux applications using the latest sandboxing technologies available "
		"in Linux kernel."));

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




StartSandboxPage::StartSandboxPage(QWidget *parent)
: QWizardPage(parent) {
	setTitle(tr("Complete Your Registration"));
	setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark.png"));

	bottomLabel = new QLabel;
	bottomLabel->setWordWrap(true);

	agreeCheckBox = new QCheckBox(tr("I agree to the terms of the license"));

	registerField("conclusion.agree*", agreeCheckBox);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(bottomLabel);
	layout->addWidget(agreeCheckBox);
	setLayout(layout);
}


int StartSandboxPage::nextId() const
{
	return -1;
}


void StartSandboxPage::initializePage() {
#if 0
	QString licenseText;

	if (wizard()->hasVisitedPage(Wizard::Page_Home)) {
		licenseText = tr("<u>Evaluation  Agreement:</u> "
			"You can use this software for 30 days and make one "
			"backup, but you are not allowed to distribute it.");
	}
	else if (wizard()->hasVisitedPage(Wizard::Page_Fs)) {
		licenseText = tr("<u>First-Time  Agreement:</u> "
			"You can use this software subject to the license "
			"you will receive by email.");
	}
	else {
		licenseText = tr("<u>Upgrade  Agreement:</u> "
			"This software is licensed under the terms of your "
			"current license.");
	}
	bottomLabel->setText(licenseText);
#endif
}


void StartSandboxPage::setVisible(bool visible) {
	QWizardPage::setVisible(visible);

	if (visible) {
		wizard()->setButtonText(QWizard::CustomButton1, tr("&Print"));
		wizard()->setOption(QWizard::HaveCustomButton1, true);
		connect(wizard(), SIGNAL(customButtonClicked(int)),
			this, SLOT(printButtonClicked()));
	}
	else {
		wizard()->setOption(QWizard::HaveCustomButton1, false);
		disconnect(wizard(), SIGNAL(customButtonClicked(int)),
			this, SLOT(printButtonClicked()));
	}
}


void StartSandboxPage::printButtonClicked() {
#if 0
	QPrinter printer;
	QPrintDialog dialog(&printer, this);
	if (dialog.exec())
		QMessageBox::warning(this, tr("Print "),
			tr("As an environmentally friendly measure, the "
			"license text will not actually be printed."));
#endif
}
