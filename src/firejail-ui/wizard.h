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
#ifndef LICENSEWIZARD_H
#define LICENSEWIZARD_H

#include "firejail_ui.h"
#include <QWizard>

class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QPushButton;
class HomeWidget;
class QListWidget;
class QListWidgetItem;
struct AppEntry;

class Wizard : public QWizard {
	Q_OBJECT

public:
	enum { Page_Application, Page_Config, Page_Config2, Page_StartSandbox };

	Wizard(QWidget *parent = 0);
	void accept();

private slots:
	void showHelp();
};

class ApplicationPage : public QWizardPage {
	Q_OBJECT

public:
	ApplicationPage(QWidget *parent = 0);

	int nextId() const;
	
private slots:
	void groupClicked(QListWidgetItem*);
	void appClicked(QListWidgetItem*);
	void browseClicked();

private:
	AppEntry *appdb_;
	QListWidget *app_;
	QListWidget *group_;
	QLineEdit *command_;
	QRadioButton *use_default_;
	QRadioButton *use_custom_;
	QPushButton *browse_;
};

class ConfigPage : public QWizardPage {
	Q_OBJECT

public:
	ConfigPage(QWidget *parent = 0);
	bool validatePage();

	int nextId() const;


public slots:
	void setHome(bool);
	void setDns(bool);
	void setProtocol(bool);

private:
	// filesystem
	QCheckBox *whitelisted_home_;
	QCheckBox *private_dev_;
	QCheckBox *private_tmp_;
	QCheckBox *mnt_media_;
	HomeWidget *home_;

	// networking
	QRadioButton *sysnetwork_;
	QRadioButton *nonetwork_;
	QRadioButton *netnamespace_;
	
	// DNS
	QLineEdit *dns1_;
	QLineEdit *dns2_;
	QCheckBox *dns1check_;
	QCheckBox *dns2check_;
	
	// protocol
	QCheckBox *protocol_unix_;
	QCheckBox *protocol_inet_;
	QCheckBox *protocol_inet6_;
	QCheckBox *protocol_netlink_;
	QCheckBox *protocol_packet_;
};

class ConfigPage2 : public QWizardPage {
	Q_OBJECT

public:
	ConfigPage2(QWidget *parent = 0);

	int nextId() const;
	void initializePage();

public slots:

private:
	// multimedia
	QCheckBox *nosound_;
	QCheckBox *no3d_;
	QCheckBox *nodvd_;
	QCheckBox *novideo_;
	QCheckBox *notv_;
	QCheckBox *nox11_;
	
	// kernel
	QCheckBox *seccomp_;
	QCheckBox *caps_;
	QCheckBox *noroot_;
};

class StartSandboxPage : public QWizardPage {
	Q_OBJECT

public:
	StartSandboxPage(QWidget *parent = 0);

	int nextId() const;
	
private slots:

private:
	QCheckBox *debug_;
	QCheckBox *trace_;
	QCheckBox *mon_;
};
#endif
