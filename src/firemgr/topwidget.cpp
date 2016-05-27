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
#include "firemgr.h"
#include "topwidget.h"
#include "mainwindow.h"

#include <QtGlobal>
#if QT_VERSION >= 0x050000
	#include <QtWidgets>
#else
	#include <QtGui>
#endif

void TopWidget::paintEvent(QPaintEvent *event) {
	(void) event;
	QPainter painter(this);

	// draw
	painter.drawImage(0, 0, QImage(":resources/go-up.png"));
	painter.drawImage(34, 0, QImage(":resources/refresh.png"));
	painter.drawImage(68, 0, QImage(":resources/go-top.png"));
	painter.drawImage(102, 0, QImage(":resources/user-home.png"));
}

void TopWidget::mousePressEvent(QMouseEvent *event) {
	QPoint pos = event->pos();
	if (event->button() == Qt::LeftButton) {
		if (pos.x() <= 24)
			emit upClicked();
		else if (pos.x() >= 34 && pos.x() < 58)
			emit refreshClicked();
		else if (pos.x() >= 68 && pos.x() < 92)
			emit rootClicked();
		else if (pos.x() >= 102 && pos.x() < 12692)
			emit homeClicked();
	}

	event->accept();
}
