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
#ifndef EDIT_DIALOG_H
#define EDIT_DIALOG_H
#include <QWidget>
#include <QDialog>
#include <QLineEdit>

class EditDialog: public QDialog {
Q_OBJECT

public:
	EditDialog(QString name, QString desc, QString cmd);
	
	
	QString getName() {
		return name_->text();
	}
	
	QString getDescription() {
		return desc_->text();
	}
	
	QString getCommand() {
		return cmd_->text();
	}
	
private slots:
	void help();

private:
	QLineEdit *name_;
	QLineEdit *desc_;
	QLineEdit *cmd_;
};


#endif