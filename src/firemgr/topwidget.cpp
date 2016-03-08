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
