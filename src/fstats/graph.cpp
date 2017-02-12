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
#include <QtGui>
#include <QUrl>
#include "graph.h"
#include "dbpid.h"
#include "db.h"

static QByteArray byteArray[4];
static const char *id_label[4] = {
	"CPU (%)",
	"Memory (KiB)",
	"RX (KB/s)",
	"TX (KB/s)"
};

QString graph(int id, DbPid *dbpid, int cycle, GraphType gt) {
	assert(id < 4);
	assert(dbpid);

	// adjust cycle for 1H
	if (gt == GRAPH_1H)
		cycle =  Db::instance().getG1HCycle();
	else if (gt == GRAPH_12H)
		cycle =  Db::instance().getG12HCycle();

	assert(cycle < DbPid::MAXCYCLE);
	int maxcycle = DbPid::MAXCYCLE;
	int i;	
	int j;
	
	// set pixmap
#define TOPMARGIN 20
#define RIGHTMARGIN 60	
	QPixmap *pixmap = new QPixmap((maxcycle - 1) * 4 + RIGHTMARGIN, TOPMARGIN + 100 + 30);
	QPainter *paint = new QPainter(pixmap);
	paint->fillRect(0, 0, (maxcycle - 1) * 4 + 100, TOPMARGIN + 100 + 30, Qt::white);
	paint->setPen(Qt::black);
	paint->drawRect(0, TOPMARGIN, (maxcycle - 1) * 4, 100);
	paint->setPen(QColor(80, 80, 80, 128));
	paint->drawLine(0, TOPMARGIN + 25, (maxcycle - 1) * 4, TOPMARGIN + 25);
	paint->drawLine(0, TOPMARGIN + 50, (maxcycle - 1) * 4, TOPMARGIN + 50);
	paint->drawLine(0, TOPMARGIN + 75, (maxcycle - 1) * 4, TOPMARGIN + 75);
	paint->drawLine((maxcycle - 1) * 1, TOPMARGIN, (maxcycle - 1) * 1, TOPMARGIN + 100);
	paint->drawLine((maxcycle - 1) * 2, TOPMARGIN, (maxcycle - 1) * 2, TOPMARGIN + 100);
	paint->drawLine((maxcycle - 1) * 3, TOPMARGIN, (maxcycle - 1) * 3, TOPMARGIN + 100);
	
	// extract maximum value
	float maxval = 0;
	for (i = 0; i < maxcycle; i++) {
		float val;
		if (gt == GRAPH_4MIN)
			val  = dbpid->data_4min_[i].get(id);
		else if (gt == GRAPH_1H)
			val  = dbpid->data_1h_[i].get(id);
		else if (gt == GRAPH_12H)
			val  = dbpid->data_12h_[i].get(id);
		else
			assert(0);
			
		if (val > maxval)
			maxval = val;
	}

	// adjust maxval
	maxval = qCeil(maxval);
	if (maxval < 2)
		maxval = 2;
	else if (maxval < 5)
		maxval = 5;
	else if (maxval < 10)
		maxval = 10;
	else if (maxval < 20)
		maxval = 20;
	else if (maxval < 50)
		maxval = 50;
	else if (maxval < 100)
		maxval = 100;
	else if (maxval < 200)
		maxval = 200;
	else if (maxval < 500)
		maxval = 500;
	else if (maxval < 1000)
		maxval = 1000;
	else if (maxval < 2000)
		maxval = 2000;
	else if (maxval < 5000)
		maxval = 5000;
	else if (maxval < 10000)
		maxval = 10000;
	else if (maxval < 20000)
		maxval = 20000;
	else if (maxval < 50000)
		maxval = 50000;
	else if (maxval < 100000)
		maxval = 100000;
	else if (maxval < 200000)
		maxval = 200000;
	else if (maxval < 500000)
		maxval = 500000;
	else if (maxval < 1000000)
		maxval = 1000000;
	else if (maxval < 2000000)
		maxval = 2000000;

	paint->setPen(Qt::red);
	for (i = 0, j = cycle + 1; i < maxcycle - 1; i++) {
		float y1;
		if (gt == GRAPH_4MIN)
			y1 = dbpid->data_4min_[j].get(id);
		else if (gt == GRAPH_1H)
			y1 = dbpid->data_1h_[j].get(id);
		else if (gt == GRAPH_12H)
			y1 = dbpid->data_12h_[j].get(id);
		else
			assert(0);
			
		y1 = (y1 / maxval) * 100;
		y1 = 100 - y1 + TOPMARGIN;
		j++;
		if (j >= maxcycle)
			j = 0;

		float y2;
		if (gt == GRAPH_4MIN)
			y2 = dbpid->data_4min_[j].get(id);
		else if (gt == GRAPH_1H)
			y2 = dbpid->data_1h_[j].get(id);
		else if (gt == GRAPH_12H)
			y2 = dbpid->data_12h_[j].get(id);
		else
			assert(0);
			
		y2 = (y2 / maxval) * 100;
		y2 = 100 - y2 + TOPMARGIN;
		paint->drawLine(i * 4, (int) y1, (i + 1) * 4, (int) y2);
	}

	// axis
	paint->setPen(Qt::black);
	QString ymax = QString::number((int) maxval);
	paint->drawText((maxcycle - 1) * 4 + 3, TOPMARGIN + 3, QString::number((int) maxval));
	if (qCeil(maxval / 2) == maxval / 2)
		paint->drawText((maxcycle - 1) * 4 + 3, TOPMARGIN + 50 + 3, QString::number((int) maxval / 2));
	else
		paint->drawText((maxcycle - 1) * 4 + 3, TOPMARGIN + 50 + 3, QString::number(maxval / 2, 'f', 1));
	paint->drawText((maxcycle - 1) * 4 + 3, TOPMARGIN + 100 + 3, QString("0"));
	if (gt == GRAPH_12H)
		paint->drawText(0 + 2, TOPMARGIN + 100 + 15, QString("(hours)"));
	if (gt == GRAPH_1H)
		paint->drawText(0 + 2, TOPMARGIN + 100 + 15, QString("(minutes)"));
	else
		paint->drawText(0 + 2, TOPMARGIN + 100 + 15, QString("(seconds)"));
	if (gt == GRAPH_4MIN) {
		paint->drawText((maxcycle - 1) * 2 - 5, TOPMARGIN + 100 + 15, QString("-30"));
		paint->drawText((maxcycle - 1) * 3 - 5, TOPMARGIN + 100 + 15, QString("-15"));
	}
	else if (gt == GRAPH_1H) {
		paint->drawText((maxcycle - 1) * 2 - 5, TOPMARGIN + 100 + 15, QString("-30"));
		paint->drawText((maxcycle - 1) * 3 - 5, TOPMARGIN + 100 + 15, QString("-15"));
	}
	else if (gt == GRAPH_12H) {
		paint->drawText((maxcycle - 1) * 2 - 5, TOPMARGIN + 100 + 15, QString("-6"));
		paint->drawText((maxcycle - 1) * 3 - 5, TOPMARGIN + 100 + 15, QString("-3"));
	}
	else
		assert(0);
	
	
	// title
	paint->setPen(Qt::black);
	paint->drawText(0 + 2, TOPMARGIN - 2, QString(id_label[id]));
	
	// generate image
	QBuffer buffer(&byteArray[id]);
	pixmap->save(&buffer, "PNG");
//	QString url = QString("<img src=\":resources/fjail.png\"  />");
	QString url = QString("<img src=\"data:image/png;base64,") + byteArray[id].toBase64() + "\"  />";
	delete paint;
	delete pixmap;
	return url;
}
// qt bug reported at https://bugreports.qt.io/browse/QTBUG-43270:
// A html source containing an embedded image read into QTextBrowser creates an error message 
// "QFSFileEngine::open: No file name specified", even though the image is parsed and rendered fine.

//http://stackoverflow.com/questions/6598554/is-there-any-way-to-insert-qpixmap-object-in-html

#if 0
    QString html;
     
    QImage img("c:\\temp\\sample.png"); // todo: generate image in memory
    myTextEdit->document()->addResource(QTextDocument::ImageResource, QUrl("sample.png" ), img);
     
    html.append("<p><img src=\":sample.png\"></p>");
     
    myTextEdit->setHtml(html);
#endif
