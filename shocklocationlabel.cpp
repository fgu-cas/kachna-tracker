#include "shocklocationlabel.h"
#include <QPixmap>
#include <QPainter>
#include <math.h>
#define PI 3.14159265358979323846

#define SIZE 250

ShockLocationLabel::ShockLocationLabel(QWidget *parent) : QLabel(parent)
{
    update();
}

void ShockLocationLabel::update(){
    QPixmap pixmap(SIZE,SIZE);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, SIZE-1, SIZE-1);

    // Robot
    painter.setBrush(Qt::Dense6Pattern);
    painter.drawRect(SIZE/2-20, SIZE/2-20, 40, 40);

    // Shock
    QPoint point;
    if (centered){
        point.setX(SIZE/2);
        point.setY(SIZE/2);
    } else {
        point.setX(SIZE/2 + distance * sin(angle*PI/180));
        point.setY(SIZE/2 - distance * cos(angle*PI/180));
    }
    painter.setBrush(QBrush(Qt::yellow, Qt::FDiagPattern));
    painter.drawEllipse(point, radius, radius);

    setPixmap(pixmap);
}

void ShockLocationLabel::setDistance(int distance){
    this->distance = distance;
    update();
}

void ShockLocationLabel::setAngle(int angle){
    this->angle = angle;
    update();
}

void ShockLocationLabel::setRadius(int radius){
    this->radius = radius;
    update();
}

void ShockLocationLabel::setCentered(bool state){
    centered = state;
    update();
}
