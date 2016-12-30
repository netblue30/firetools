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
#include "../../firetools_config_extras.h"

#define MAXBUF 4096

HelpWidget::HelpWidget(QWidget * parent): QDialog(parent) {
	QString message;
	const char *fname = PACKAGE_LIBDIR "/uihelp";
//todo error recovery
	FILE *fp = fopen(fname, "r");
	if (!fp)
		errExit("fopen");
	char buf[MAXBUF];
	while (fgets(buf, MAXBUF, fp))
		message += QString(buf);
	fclose(fp);

	QTextBrowser *browser = new QTextBrowser;
	browser->setHtml(message);

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
