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
#ifndef PID_THREAD_H
#define PID_THREAD_H
#include <QImage>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QWaitCondition>
#include <QStringList>
#include "fstats.h"

class PidThread : public QThread
{
Q_OBJECT

public:
	PidThread();
	~PidThread();

signals:
	void cycleReady();

protected:
	void run();
private:
	bool ending_;		
};

#endif