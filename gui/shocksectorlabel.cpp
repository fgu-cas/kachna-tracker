#include "shocksectorlabel.h"

#include <QPixmap>
#include <QPainter>

#define SIZE 250

ShockSectorLabel::ShockSectorLabel(QWidget *parent) : QLabel(parent)
{
	update();
}

void ShockSectorLabel::update() {
	QRect boundary;
	boundary.setTopLeft(QPoint(10, 10));
	boundary.setBottomRight(QPoint(SIZE - 10, SIZE - 10));

	QPixmap pixmap(SIZE, SIZE);
	pixmap.fill(Qt::white);

	QPainter painter(&pixmap);

	// Arena
	painter.setPen(Qt::black);
	painter.drawEllipse(boundary);


	// Shock sector
	int a = angle - (range / 2);
	int alen = range;

	painter.setBrush(QBrush(Qt::yellow, Qt::FDiagPattern));
	painter.drawPie(boundary, a, alen);


	setPixmap(pixmap);
}

void ShockSectorLabel::setAngle(double angle) {
	this->angle = round((90 - angle) * 16);
	update();
}

void ShockSectorLabel::setRange(double range) {
	this->range = round(range * 16);
	update();
}
