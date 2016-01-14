#ifndef DETECTORCOLOR_H
#define DETECTORCOLOR_H

#include "detector.h"

#include <memory>

#include <QMap>
#include <QVariant>
#include <QElapsedTimer>

#include <opencv2/core/core.hpp>


struct colorRange {
    int hue = 0;
    int hue_tolerance = 0;

    int saturation_low = 0;
    int value_low = 0;
};

using namespace cv;

class DetectorColor : public Detector
{
public:
    DetectorColor(const QMap<QString, QVariant> &settings, int h, int w);

    Mat process(Mat *frame);
    Mat filter(Mat* frame, colorRange range);
    std::vector<KeyPoint> detect(Mat *frame);
    keypointPair find(Mat *frame);

    enum CLASS_ID {
        RAT_FRONT,
        RAT_BACK,
        ROBOT_FRONT,
        ROBOT_BACK
    };
private:
    std::unique_ptr<SimpleBlobDetector> detector;

    colorRange ratFront;
    colorRange ratBack;
    colorRange robotFront;
    colorRange robotBack;
};

#endif // DETECTORCOLOR_H
