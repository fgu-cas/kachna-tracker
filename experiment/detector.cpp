#include "detector.h"

Detector::Detector(const QMap<QString, QVariant> &settings, int h, int w){
    int x, y, r;

    x = settings.value("arena/X").toInt();
    y = settings.value("arena/Y").toInt();
    r = settings.value("arena/radius").toInt();

    mask = Mat(h, w, CV_8UC1);
    mask.setTo(Scalar(0));
    circle(mask, Point2f(x, y), r, Scalar(255), -1);

    maskRect.x = (r<x ? x-r : 0);
    maskRect.y = (r<y ? y-r : 0);
    maskRect.width = r*2;
    maskRect.height = r*2;
}

Detector::Point::Point(){
    pt.x = -1;
    pt.y = -1;
    angle = -1;
    size = -1;
    hue = -1;
    class_id = -1;
}

void Detector::Point::operator=(KeyPoint point){
    pt = point.pt;
    angle = point.angle;
    size = point.size;
}
