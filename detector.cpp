#include "detector.h"

Detector::Detector(const QMap<QString, QVariant> &settings, int h, int w){
    mask = Mat(h, w, CV_8UC1);
    mask.setTo(Scalar(0));
    circle(mask, Point2f(settings.value("arena/X").toInt(), settings.value("arena/Y").toInt()),
           settings.value("arena/radius").toInt(), Scalar(255), -1);
}
