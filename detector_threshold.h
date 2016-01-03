#ifndef DETECTOR_THRESHOLD_H
#define DETECTOR_THRESHOLD_H
#include "detector.h"

#include <memory>

#include <QMap>
#include <QVariant>
#include <QElapsedTimer>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace cv;

class DetectorThreshold : public Detector
{
public:
    DetectorThreshold(QMap<QString, QVariant> settings, int h, int w);

    Mat process(Mat *frame);
    std::vector<KeyPoint> detect(Mat *frame);

private:
    std::unique_ptr<SimpleBlobDetector> detector;
    double img_threshold;
};

#endif // DETECTOR_THRESHOLD_H
