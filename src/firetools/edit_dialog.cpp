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
#include "firetools.h"
#include <sys/utsname.h>
#include <QtGlobal>

#if QT_VERSION >= 0x050000
	#include <QtWidgets>
#else
	#include <QtGui>
#endif

#include <QUrl>
#include "edit_dialog.h"
#include "applications.h"
#include "../../firetools_config.h"



EditDialog::EditDialog(QString name, QString desc, QString cmd): QDialog() {
	// Editing
	QLabel *lname = new QLabel;
	lname->setText(tr("Name"));
	name_ = new QLineEdit;
	name_->setText(name);
//	if (applications_check_default(name.toLocal8Bit().data()))
//		name_->setEnabled(false);

	QLabel *ldesc = new QLabel;
	ldesc->setText(tr("Description"));
	desc_ = new QLineEdit;
	desc_->setText(desc);

 	QLabel *lcmd = new QLabel;
	lcmd->setText(tr("Command"));
	cmd_ = new QLineEdit;
	cmd_->setText(cmd);

	// Buttons
    	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	QPushButton *helpButton = new QPushButton("Help");
	connect(helpButton, SIGNAL(pressed()), this, SLOT(help()));

	QLabel *icon_txt1 = new QLabel("The launcher will use the name above to find an icon already");
	QLabel *icon_txt2 = new QLabel("installed on your system (name.png files in /usr directory). ");
	QLabel *icon_txt3 = new QLabel("You can supply your own icon by placing it in ~/.config/firetools.");


	// Layout - editing
	QGridLayout *layout = new QGridLayout;
	layout->addItem(new QSpacerItem(30, 15), 0, 0);
	layout->addItem(new QSpacerItem(300, 15), 0, 2);
	layout->addItem(new QSpacerItem(30, 15), 0, 3);
	layout->addWidget(lname, 1, 1);
	layout->addWidget(name_, 1, 2);
	layout->addWidget(ldesc, 2, 1);
	layout->addWidget(desc_, 2, 2);
	layout->addWidget(lcmd, 3, 1);
	layout->addWidget(cmd_, 3, 2);
	
	// Icon note
	layout->addItem(new QSpacerItem(10,20), 4, 0);
	layout->addWidget(icon_txt1, 5, 1, 1, 2);
	layout->addWidget(icon_txt2, 6, 1, 1, 2);
	layout->addWidget(icon_txt3, 7, 1, 1, 2);
	layout->addItem(new QSpacerItem(30, 15), 8, 0);
	
	// Layout - buttons
	layout->addItem(new QSpacerItem(30, 30), 9, 0);
	layout->addWidget(helpButton, 10, 1);
	layout->addWidget(buttonBox, 10, 2);
	layout->addItem(new QSpacerItem(30, 15), 11, 0);
	setLayout(layout);
//	resize(600, 500);
	setWindowTitle(tr("Edit Sandbox"));
}	

// Help dialog
void EditDialog::help() {
	QMessageBox msgBox;
	
	QString txt;
	txt += "<br/>";
	txt += "<b>Name:</b> unique name for this launcher;<br/>\n";
	txt += "<b>Description:</b> a short description of the program<br/>\n";
	txt += "<b>Command:</b> command for starting the program<br/><br/>\n";
	txt += "<b>Example</b><br/><br/>\n";
	txt += "Name: firefox<br/>\n";
	txt += "Description: Mozilla Firefox Browser<br/>\n";
	txt += "Command: firejail firefox<br>\n";
	txt += "<br/>";

	QMessageBox::about(this, tr("Editing Sandbox Entries"), txt);
}

