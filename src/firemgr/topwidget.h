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
#ifndef TOPWIDGET_H
#define TOPWIDGET_H
#include <QWidget>
#include <QSize>

class TopWidget: public QWidget {
	Q_OBJECT
public:
	TopWidget(QWidget *parent = 0): QWidget(parent) {}
	QSize minimumSizeHint() const {
		return QSize(126, 24);
	}
	
	QSize sizeHint() const {
		return QSize(126, 24);
	}

signals:
	void upClicked();
	void rootClicked();
	void refreshClicked();
	void homeClicked();

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);

private:
//	bool drag_;
};
#endif
