/*
 * Copyright (C) 2015 netblue30 (netblue30@yahoo.com)
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
#include "firemgr.h"
#include "fs.h"

#include <QtGlobal>
#if QT_VERSION >= 0x050000
	#include <QtWidgets>
#else
	#include <QtGui>
#endif

#include "mainwindow.h"
#include "topwidget.h"
#include <QtGui>

MainWindow::MainWindow(pid_t pid, QWidget *parent): QMainWindow(parent), pid_(pid) {
	// check firejail installed
	if (!which("firejail")) {
		QMessageBox::warning(this, tr("Firejail File Manager"),
			tr("<br/><b>Firejail</b> software not found. Please install it.<br/><br/><br/>"));
		exit(1);
	}
	
	// verify sandbox
	{
		char *cmd;
		if (asprintf(&cmd, "firejail --ls=%d / 2>&1", pid_) == -1)
			errExit("asprintf");
		char *out = run_program(cmd);
		if (out == NULL || strncmp(out, "Error", 5) == 0) {
			char *msg;
			if (asprintf(&msg, "<br/><b>Sandbox %d not found.<br/><br/><br/>", pid) == -1)
				errExit("asprintf");
			QMessageBox::warning(this, tr("Firejail File Manager"), tr(msg));
			exit(1);
		}
	}
	
	// initialize FS
	fs_ = new FS(pid);
	
	top_ = new TopWidget(this);
	connect(top_, SIGNAL(upClicked()), this, SLOT(handleUp()));
	connect(top_, SIGNAL(rootClicked()), this, SLOT(handleRoot()));
	connect(top_, SIGNAL(refreshClicked()), this, SLOT(handleRefresh()));
	connect(top_, SIGNAL(homeClicked()), this, SLOT(handleHome()));
	
	line_ = new QLineEdit(this);
	QString txt = build_line();
	line_->setText(txt);
	line_->setReadOnly(true);
	
	table_ = new QTableWidget(0, 6, this);
	QStringList header;
	header.append(" ");
	header.append(" ");
	header.append("Mount");
	header.append("Owner");
	header.append("Size");
	header.append("Name");
	table_->setHorizontalHeaderLabels(header);
	table_->verticalHeader()->setVisible(false);
	table_->setColumnWidth(0, 20);
	table_->setColumnWidth(1, 26);
	table_->setColumnWidth(2, 50);
	table_->setColumnWidth(3, 100);
	table_->setColumnWidth(4, 100);
	table_->setColumnWidth(5, 500);
	table_->horizontalHeader()->setStretchLastSection(true);	
	table_->setShowGrid(false);
	table_->setColumnHidden(0, true);
	connect(table_, SIGNAL(cellClicked(int, int)), this, SLOT (cellClicked(int, int)));
	print_files("/");

	QWidget *empty1 = new QWidget(this);
	empty1->setMinimumWidth(30);
	QWidget *empty2 = new QWidget(this);
	empty2->setMinimumWidth(10);
	
	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(top_, 0, 0);
	mainLayout->addWidget(empty1, 0, 1);
	mainLayout->addWidget(line_, 0, 2);
	mainLayout->addWidget(empty2, 0, 3);
	mainLayout->addWidget(table_, 1, 0, 1, 4);
	mainLayout->setColumnStretch(0, 1);
	mainLayout->setColumnStretch(1, 1);
	mainLayout->setColumnStretch(2, 200);
	mainLayout->setColumnStretch(3, 1);
	QWidget *mainWidget = new QWidget;
	mainWidget->setLayout(mainLayout);

	setCentralWidget(mainWidget);
	setMinimumWidth(500);
	char *title;
	if (asprintf(&title, "Firejail Sandbox %d", pid) == -1)
		errExit("asprintf");
	setWindowTitle(tr(title));
	free(title);
}


void MainWindow::print_files(const char *path) {
	char *cmd;
	if (asprintf(&cmd, "firejail --ls=%d %s 2>&1", pid_, path) == -1)
		errExit("asprintf");
	
	// clear table
	int rows = table_->rowCount();
	while (rows > 0) {
		table_->removeRow(0);
		rows--;
	}

	char *out = run_program(cmd);
	if (out == NULL || strncmp(out, "Error", 5) == 0) {
		char *msg;
		if (asprintf(&msg, "<br/><b>Directory %s not found.<br/><br/><br/>", path) == -1)
			errExit("asprintf");
		QMessageBox::warning(this, tr("Firejail File Manager"), tr(msg));
		free(msg);
		return;
	}

	// fs flags
	fs_->checkPath(path);
	
	char *ptr = strtok(out, "\n");
	rows = 0;
	while (ptr) {
		split_command(ptr);
		if (sargc == 5) {
			if (strcmp(sargv[4], "..") != 0 && strcmp(sargv[4], ".") != 0) {
				table_->setRowCount(rows + 1);
				
				// image
				if (*sargv[0] == 'd') {
					table_->setItem(rows, 0, new QTableWidgetItem("D"));
					QImage *img = new QImage(":resources/gnome-fs-directory.png");
					QTableWidgetItem *timage = new QTableWidgetItem;
					timage->setData(Qt::DecorationRole, QPixmap::fromImage(*img));
					table_->setItem(rows, 1, new QTableWidgetItem(*timage));
				}
				else if (*sargv[0] == 'l') {
					table_->setItem(rows, 0, new QTableWidgetItem("L"));
					QImage *img = new QImage(":resources/emblem-symbolic-link.png");
					QTableWidgetItem *timage = new QTableWidgetItem;
					timage->setData(Qt::DecorationRole, QPixmap::fromImage(*img));
					table_->setItem(rows, 1, new QTableWidgetItem(*timage));
				}
				else {
					table_->setItem(rows, 0, new QTableWidgetItem("F"));
					QImage *img = new QImage(":resources/empty.png");
					QTableWidgetItem *timage = new QTableWidgetItem;
					timage->setData(Qt::DecorationRole, QPixmap::fromImage(*img));
					table_->setItem(rows, 1, new QTableWidgetItem(*timage));
				}
				
				// fs flags
				QString s = fs_->checkFile(QString(sargv[4]));
				QTableWidgetItem *item =  new QTableWidgetItem(s);
				item->setTextAlignment(Qt::AlignCenter);					
				table_->setItem(rows, 2, item);
				
				item =  new QTableWidgetItem(sargv[1]);
				item->setTextAlignment(Qt::AlignCenter);					
				table_->setItem(rows, 3, item);

				item =  new QTableWidgetItem(sargv[3]);
				item->setTextAlignment(Qt::AlignCenter);
				table_->setItem(rows, 4, item);
				
				item =  new QTableWidgetItem(QString("  ") + QString(sargv[4]));
//				item->setTextAlignment(Qt::AlignHorizontal_Mask);
				table_->setItem(rows, 5, item);
				rows++;				
			}
		}
		
		ptr = strtok(NULL, "\n");
	}	
}

void MainWindow::handleUp() {
	if (path_.size() == 0)
		return handleRefresh();

	path_.takeLast();
	QString full_path = build_path();		
	print_files(full_path.toStdString().c_str());
	QString txt = build_line();
	line_->setText(txt);
}

void MainWindow::handleRefresh() {
	QString full_path = build_path();		
	print_files(full_path.toStdString().c_str());
	QString txt = build_line();
	line_->setText(txt);
}

void MainWindow::handleHome() {
	path_.clear();
	path_.append(QString("home"));
	path_.append(QString("netblue"));
	QString full_path = build_path();		
	print_files(full_path.toStdString().c_str());
	QString txt = build_line();
	line_->setText(txt);
}

void MainWindow::handleRoot() {
	path_.clear();
	print_files("/");
	QString txt = build_line();
	line_->setText(txt);
}

QString  MainWindow::build_path() {
	QString retval = QString("/");
	
	for (int i = 0; i < path_.size(); ++i) {
		retval += path_.at(i);
		retval += QString("/");
	}
	
	return retval;
}

QString  MainWindow::build_line() {
	QString retval;
	retval.sprintf("%d:///", pid_);
	
	for (int i = 0; i < path_.size(); ++i) {
		retval += path_.at(i);
		retval += QString("/");
	}
	
	return retval;
}

void MainWindow::cellClicked(int row, int column) {
	(void) column;

	QString type =  table_->item(row, 0)->text();
	if (type != "D")
		return;
	QString dir =  table_->item(row, 5)->text();
	dir = dir.mid(2);
	path_.append(dir);

	QString full_path = build_path();		
	print_files(full_path.toStdString().c_str());
	QString txt = build_line();
	line_->setText(txt);
}
