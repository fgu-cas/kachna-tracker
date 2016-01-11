#ifndef DETECTORCOLOR_H
#define DETECTORCOLOR_H

#include "detector.h"

#include <memory>

#include <QMap>
#include <QVariant>
#include <QElapsedTimer>

#include <opencv2/core/core.hpp>


struct colorRange {
    int hue;
    int hue_tolerance;

    int saturation_low;
    int value_low;
};

using namespace cv;

class DetectorColor : public Detector
{
public:
    DetectorColor(QMap<QString, QVariant> settings, int h, int w);

    Mat process(Mat *frame);
    Mat filter(Mat* frame, colorRange range);
    std::vector<KeyPoint> detect(Mat *frame);
private:
    std::unique_ptr<SimpleBlobDetector> detector;
};

#endif // DETECTORCOLOR_H
