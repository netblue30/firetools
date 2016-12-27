/*
 * Copyright (C) 2015-2016 Firetools Authors
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
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include "help_widget.h"


HelpWidget::HelpWidget(QWidget * parent): QDialog(parent) {
	QString message = tr("<b>Sandbox Configuration:</b><br/>"
		"<br/>"
		"Firejail secures your Linux applications using the latest sandboxing technologies available "
		"in Linux kernel. Use this page to configure the sandbox, and "
		"press <i>Next</i> when finished.<br/>"
		"<br/>"
		"<br/>"
		"<b>Filesystem</b><br/><br/>"
		"<blockquote>"
		"<i>Restrict home directory:</i> Choose the directories visible inside the sandbox. "
		"By default, with the exception of some well-known password and encryption files, "
		"all home files and directories are available inside the sandbox.<br/><br/>"
		"<i>Restrict /dev directory:</i> Only a small number of devices are visible inside the sandbox. "
		"Sound and 3D acceleration should be available if this checkbox is set.<br/><br/>"
		"<i>Restrict /tmp directory:</i> Start with a clean /tmp directory, only X11 directory is available "
		"under /tmp.<br/><br/>"
		"<i>Restrict /mnt and /media:</i> Blacklist /mnt and /media directories.<br/><br/>"
		"</blockquote>"

		"<b>Networking</b><br/><br/>"
		"<blockquote>"
		"<i>System network:</i> Use the networking stack provided by the system.<br/><br/>"
		"<i>Network namespace:</i> Install a separate networking stack.<br/><br/>"
		"<i>Disable networking:</i> No network connectivity is available inside the sandbox.<br/><br/>"
		"</blockquote>"

		"<b>Multimedia</b><br/><br/>"
		"<blockquote>"
		"<i>Disable sound:</i> The sound subsystem is not available inside the sandbox.<br/><br/>"
		"<i>Disable 3D acceleration:</i> Hardware acceleration drivers are disabled.<br/><br/>"
		"<i>Disable X11:</i> X11 graphical user interface subsystem is disabled. "
		"Use this option when running console programs or servers.<br/><br/>"
		"</blockquote>"

		"<b>Kernel</b><br/><br/>"
		"<blockquote>"
		"<i>seccomp, capabilities, nonewprivs:</i> These are some very powerful security features "
		"implemented by the kernel. A Linux Kernel version 3.5 is required for this option to work.<br/><br/>"
		"<i>AppArmor:</i> If AppArmor is configured and running on  your system, this option implements a number "
		"of advanced security features inspired by GrSecurity kernels. Also, on "
		"Ubuntu 16.04 or later, this option disables dBus access.<br/><br/>"
		"<i>OverlayFS:</i> This option mounts an overlay filesystem on top of the sandbox. "
		"Any filesystem modifications are discarded when the sandbox is closed. "
		"A kernel 3.18 or newer is required by this option to work.<br/><br/>"
		"</blockquote>"
	);

	QTextBrowser *browser = new QTextBrowser;
	browser->setText(message);

	QDialogButtonBox *box = new QDialogButtonBox( Qt::Horizontal );
	QPushButton *button = new QPushButton( "Ok" );
	connect( button, SIGNAL(clicked()), this, SLOT(okClicked()) );
	box->addButton( button, QDialogButtonBox::AcceptRole );


	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(browser);
	layout->addWidget(box);
	setLayout(layout);
	setMinimumWidth(600);
	setMinimumHeight(400);
}

void HelpWidget::okClicked() {
	accept();
}
