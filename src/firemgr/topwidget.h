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
