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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWidget>
#include <QAction>
#include <QSystemTrayIcon>

class MainWindow : public QWidget {
Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	QSize sizeHint() const;

protected:
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	bool event(QEvent *event);

public slots:
	void trayActivated(QSystemTrayIcon::ActivationReason);
	
private slots:
	void edit();
	void remove();
	void run();
	void runTools();
	void help();
	void main_quit();
	void newSandbox();
	void runAbout();

signals:
	void cycleReadySignal();

	

private:
    	void createTrayActions();
   	void createLocalActions();
	
private:
	QPoint dragPosition_;
	QAction *qedit_;
	QAction *qrun_;
	QAction *qhelp_;
	QAction *qdelete_;
	int active_index_;
	int animation_id_;
	int edit_index_;
	
public:	
	// tray
	QAction *minimizeAction;
	QAction *restoreAction;
	QAction *quitAction;
	
};
#endif
