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
#include <QTextEdit>
#include "../../firetools_config_extras.h"
#include "wizard.h"
#include "home_widget.h"
#include "help_widget.h"
#include "appdb.h"
#include <unistd.h>

//QString global_title("Firejail Configuration Wizard");
QString global_title("");
QTextEdit *global_profile;

QString global_subtitle(
	"<b>Firejail</b> is a SUID program that reduces the  risk  of  security "
	"breaches  by  restricting the running environment of untrusted "
	"applications using the latest Linux kernel sandboxing technologies. "
	"It allows a process and all its descendants to have their own private "
	"view of the globally shared kernel resources, such as the network stack, "
	"process table, and mount table."
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

	setWindowTitle(tr("Firetools Config"));

	setWizardStyle(QWizard::MacStyle);
	setPixmap(QWizard::BackgroundPixmap, QPixmap(":/resources/background.png"));
	//resize( QSize(600, 400).expandedTo(minimumSizeHint()) );

}

void Wizard::showHelp() {
	HelpWidget hw;
	hw.exec();
}
using namespace std;
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

	assert(global_profile);
 	QString profile = global_profile->toPlainText();
	dprintf(fd, "%s\n", qPrintable(profile));
	::close(fd);
	
	// split command into arguments
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
//	app_box->setStyleSheet("QGroupBox { color : black; }");

	QLabel *label1 = new QLabel(tr("Choose an application from the menus below"));
	label1->setFont(oldFont);
//	label1->setStyleSheet("QLabel { color : black; }");

	QGridLayout *app_box_layout = new QGridLayout;
	group_ = new QListWidget;
	group_->setFont(oldFont);
//	group_->setStyleSheet("QGridLayout { color : black; }");
	command_ = new QLineEdit;
	command_->setFont(oldFont);
//	group_->setStyleSheet("QLineEdit { color : black; }");

	browse_ = new QPushButton("browse filesystem");
	QIcon icon(":resources/gnome-fs-directory.png");
	browse_->setIcon(icon);
	connect(browse_, SIGNAL(clicked()), this, SLOT(browseClicked()));

	QLabel *label2 = new QLabel("or type in the program name:");
	label2->setFont(oldFont);
//	label2->setStyleSheet("QLabel { color : black; }");
	app_ = new QListWidget;
	app_->setFont(oldFont);
//	app_->setStyleSheet("QListWidget { color : black; }");
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
//	profile_box->setStyleSheet("QGroupBox { color : black; }");
	use_default_ = new QRadioButton("Build a default security profile");
	use_default_->setFont(oldFont);
//	use_default_->setStyleSheet("QRadioButton { color : black; }");
	use_default_->setChecked(true);
	use_custom_ = new QRadioButton("Build a custom security profile");
	use_custom_->setFont(oldFont);
//	use_custom_->setStyleSheet("QRadioButton { color : black; }");
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
	connect(group_, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
	            this, SLOT(groupChanged(QListWidgetItem*,QListWidgetItem*)));
	connect(app_, SIGNAL(itemClicked(QListWidgetItem*)),
	            this, SLOT(appClicked(QListWidgetItem*)));

	registerField("command*", command_);
	registerField("use_custom", use_custom_);

//	setFocusPolicy(Qt::StrongFocus);
}

void ApplicationPage::groupChanged(QListWidgetItem * current, QListWidgetItem * previous) {
	(void) previous;
	groupClicked(current);
}



void ApplicationPage::keyPressEvent(QKeyEvent *event) {
	switch (event->key()) {
		case Qt::Key_Return:
			printf("Return pressed\n");
			if (group_->hasFocus()) {
				printf("group focus\n'");
				groupClicked(group_->currentItem());
			}
			else if (app_->hasFocus()) {
				printf("app focus\n'");
				appClicked(app_->currentItem());
			}
			break;
		default:
			QWizardPage::keyPressEvent(event);
	}
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
//	setSubTitle(global_subtitle);

	QLabel *label1 = new QLabel(tr("<b>Step 3: Configure the sandbox</b>"));
//	label1->setStyleSheet("QLabel { color : black; }");

	whitelisted_home_ = new QCheckBox("Restrict /home directory");
//	whitelisted_home_->setStyleSheet("QCheckBox { color : black; }");
	registerField("restricted_home", whitelisted_home_);
	private_dev_ = new QCheckBox("Restrict /dev directory");
	private_dev_->setChecked(true);
//	private_dev_->setStyleSheet("QCheckBox { color : black; }");
	registerField("private_dev", private_dev_);

	private_tmp_ = new QCheckBox("Restrict /tmp directory");
	private_tmp_->setChecked(true);
//	private_tmp_->setStyleSheet("QCheckBox { color : black; }");
	registerField("private_tmp", private_tmp_);

	mnt_media_ = new QCheckBox("Restrict /mnt and /media");
	mnt_media_->setChecked(true);
//	mnt_media_->setStyleSheet("QCheckBox { color : black; }");
	registerField("mnt_media", mnt_media_);

	QGroupBox *fs_box = new QGroupBox(tr("File System"));
//	fs_box->setStyleSheet("QGroupBox { color : black; }");
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
//	sysnetwork_->setStyleSheet("QRadioButton { color : black; }");
	registerField("sysnetwork", sysnetwork_);

	nonetwork_ = new QRadioButton("Disable networking");
//	nonetwork_->setStyleSheet("QRadioButton { color : black; }");
	registerField("nonetwork", nonetwork_);

	if (global_ifname.isEmpty()) {
		netnamespace_ = new QRadioButton("Namespace");
		netnamespace_->setEnabled(false);
	}
	else
		netnamespace_ = new QRadioButton(QString("Namespace (") + global_ifname + ")");
//	netnamespace_->setStyleSheet("QRadioButton { color : black; }");
	registerField("netnamespace", netnamespace_);
	QGroupBox *net_box = new QGroupBox(tr("Networking"));
//	net_box->setStyleSheet("QGroupBox { color : black; }");
	QVBoxLayout *net_box_layout = new QVBoxLayout;
	net_box_layout->addWidget(sysnetwork_);
	net_box_layout->addWidget(netnamespace_);
	net_box_layout->addWidget(nonetwork_);
	net_box->setLayout(net_box_layout);

	home_ = new HomeWidget;
	QGroupBox *home_box = new QGroupBox(tr("Home Directory"));
//	home_box->setStyleSheet("QGroupBox { color : black; }");
	QVBoxLayout *home_box_layout = new QVBoxLayout;
	home_box_layout->addWidget(home_);
	home_box->setLayout(home_box_layout);
	home_->setEnabled(false);
	connect(whitelisted_home_, SIGNAL(toggled(bool)), this, SLOT(setHome(bool)));
	global_home_widget = home_;


	// DNS
	dns1_ = new QLineEdit;
	dns1_->setText("9.9.9.9");
	dns1_->setMaximumWidth(150);
	dns1_->setFixedWidth(170);
	registerField("dns1", dns1_);

	dns2_ = new QLineEdit;
	dns2_->setText("1.1.1.1");
	dns2_->setMaximumWidth(150);
	dns2_->setFixedWidth(170);
	registerField("dns2", dns2_);

	QGroupBox *dns_box = new QGroupBox(tr("DNS"));
	dns_box->setCheckable(true);
	dns_box->setChecked(false);
//	dns_box->setStyleSheet("QGroupBox { color : black; }");
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
	protocol_bluetooth_ = new QCheckBox("bluetooth");
	protocol_bluetooth_->setChecked(false);
	registerField("protocol_bluetooth", protocol_bluetooth_);

	QGroupBox *protocol_box = new QGroupBox(tr("Network Protocol"));
	protocol_box->setCheckable(true);
	protocol_box->setChecked(false);
//	protocol_box->setStyleSheet("QGroupBox { color : black; }");
	connect(protocol_box, SIGNAL(toggled(bool)), this, SLOT(setProtocol(bool)));
	QGridLayout *protocol_box_layout = new QGridLayout;
	protocol_box_layout->addWidget(protocol_unix_, 0, 0);
	protocol_box_layout->addWidget(protocol_inet_, 0, 1);
	protocol_box_layout->addWidget(protocol_inet6_, 1, 0);
	protocol_box_layout->addWidget(protocol_netlink_, 1, 1);
	protocol_box_layout->addWidget(protocol_packet_, 2, 0);
	protocol_box_layout->addWidget(protocol_bluetooth_, 2, 1);
	protocol_box->setLayout(protocol_box_layout);
	if (kernel_major == 3 && kernel_minor < 5) {
	   	if (arg_debug)
	   		printf("disabling protocol\n");
		protocol_box->setEnabled(false);
	}

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
//	setSubTitle(global_subtitle);

	QLabel *label1 = new QLabel(tr("<b>Step 3: Configure the sandbox... continued...</b>"));
//	label1->setStyleSheet("QLabel { color : black; }");
	nosound_ = new QCheckBox("Disable sound");
//	nosound_->setStyleSheet("QCheckBox { color : black; }");
	registerField("nosound", nosound_);

	nodvd_ = new QCheckBox("Disable CD-ROM/DVD devices");
//	nodvd_->setStyleSheet("QCheckBox { color : black; }");
	registerField("nodvd", nodvd_);

	novideo_ = new QCheckBox("Disable video camera devices");
//	novideo_->setStyleSheet("QCheckBox { color : black; }");
	registerField("novideo", novideo_);

	notv_ = new QCheckBox("Disable TV/DVB devices");
//	notv_->setStyleSheet("QCheckBox { color : black; }");
	registerField("notv", notv_);

	no3d_ = new QCheckBox("Disable 3D acceleration");
//	no3d_->setStyleSheet("QCheckBox { color : black; }");
	registerField("no3d", no3d_);

	nox11_ = new QCheckBox("Disable X11 support");
	registerField("nox11", nox11_);

	QGroupBox *multimed_box = new QGroupBox(tr("Multimedia"));
//	multimed_box->setStyleSheet("QGroupBox { color : black; }");
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
//	caps_->setStyleSheet("QCheckBox { color : black; }");
	registerField("caps", caps_);

	noroot_ = new QCheckBox("Restricted  user namespace (noroot)");
	if (kernel_major == 3 && kernel_minor < 8) {
	   	if (arg_debug)
	   		printf("disabling noroot\n");
		noroot_->setEnabled(false);
	}
	else
		noroot_->setChecked(true);
//	noroot_->setStyleSheet("QCheckBox { color : black; }");
	registerField("noroot", noroot_);

	apparmor_ = new QCheckBox("Enable AppArmor");
	apparmor_->setChecked(true);
	registerField("apparmor", apparmor_);

	QGroupBox *kernel_box = new QGroupBox(tr("Kernel"));
//	kernel_box->setStyleSheet("QGroupBox { color : black; }");
	QVBoxLayout *kernel_box_layout = new QVBoxLayout;
	kernel_box_layout->addWidget(seccomp_);
	kernel_box_layout->addWidget(caps_);
	kernel_box_layout->addWidget(noroot_);
	kernel_box_layout->addWidget(apparmor_);
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
//	setSubTitle(global_subtitle);

	// fonts
	QFont bold;
	bold.setBold(true);
	QFont oldFont;
	oldFont.setBold(false);

	global_profile = new QTextEdit();

	QLabel *label1 = new QLabel(tr("This is the configuration we created for your sandbox. "
		"You can modify it in the text box below.<br/><br/>"
		"For more information, visit us at <b>http://firejail.wordpress.com</b>."));
	QLabel *label2 = new QLabel(tr("Press <b>Done</b> to start the sandbox.<br/><br/>"));
	QWidget *empty1 = new QWidget;
	empty1->setMinimumHeight(12);
	QWidget *empty2 = new QWidget;
	empty2->setMinimumHeight(25);
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(empty1, 0, 0);
	layout->addWidget(label1, 1, 0);
	layout->addWidget(global_profile, 2, 0);
	layout->addWidget(label2, 3, 0);
	setLayout(layout);

}

void StartSandboxPage::initializePage() {
	QString txt = "# Custom profile for " + field("command").toString() + "\n";

	// include
	txt += "\n# file system\n";
	txt += "include /etc/firejail/disable-common.inc\n";

	// home directory
	if (field("restricted_home").toBool()) {
		QString whitelist = global_home_widget->getContent();
		if (whitelist.isEmpty())
			whitelist = QString("private\n");
		else
			whitelist += QString("include /etc/firejail/whitelist-common.inc\n");

		txt += whitelist;
	}

	// filesystem
	if (field("private_tmp").toBool())
		txt += "private-tmp\n";
	if (field("private_dev").toBool())
		txt += "private-dev\n";
	if (field("mnt_media").toBool()) {
		txt += "blacklist /mnt\n";
		txt += "blacklist /media\n";
	}

	// network
	txt += "\n# network\n";
	if (field("sysnetwork").toBool())
		;
	else if (field("nonetwork").toBool())
		txt += "net none\n";
	else if (field("netnamespace").toBool())
		txt += "net " + global_ifname + "\n";

	// dns
	if (global_dns_enabled) {
		QString dns1 = field("dns1").toString();
		if (!dns1.isEmpty())
			txt += "dns " + dns1 + "\n";

		QString dns2 = field("dns2").toString();
		if (!dns2.isEmpty())
			txt += "dns " + dns2 + "\n";
	}

	// network protocol
	if (global_protocol_enabled) {
		if (field("protocol_unix").toBool() ||
		    field("protocol_inet").toBool() ||
		    field("protocol_inet6").toBool() ||
		    field("protocol_netlink").toBool() ||
		    field("protocol_bluetooth").toBool() ||
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
			if (field("protocol_bluetooth").toBool())
				protocol += QString("bluetooth");
			txt += protocol + "\n";
		}
	}

	// multimedia
	txt += "\n# multimedia\n";
	if (field("nosound").toBool())
		txt += "nosound\n";
	if (field("no3d").toBool())
		txt += "no3d\n";
	if (field("nox11").toBool())
		txt += "x11 none\n";
	if (field("nodvd").toBool())
		txt += "nodvd\n";
	if (field("novideo").toBool())
		txt += "novideo\n";
	if (field("notv").toBool())
		txt += "notv\n";


	// kernel
	txt += "\n# kernel\n";
	if (field("seccomp").toBool()) {
		txt += "seccomp\n";
		txt += "nonewprivs\n";
	}
	if (field("caps").toBool())
		txt += "caps.drop all\n";
	if (field("noroot").toBool())
		txt += "noroot\n";
	if (field("apparmor").toBool())
		txt += "apparmor\n";

	global_profile->setText(txt);
}

int StartSandboxPage::nextId() const {
	return -1;
}
