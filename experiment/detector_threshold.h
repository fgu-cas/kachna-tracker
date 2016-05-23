#ifndef DETECTOR_THRESHOLD_H
#define DETECTOR_THRESHOLD_H
#include "detector.h"

#include <memory>

#include <QMap>
#include <QVariant>
#include <QElapsedTimer>

#include <opencv2/core/core.hpp>

using namespace cv;

class DetectorThreshold : public Detector
{
public:
    DetectorThreshold(const QMap<QString, QVariant> &settings, int h, int w);

    Mat process(Mat *frame);
    Mat analyze(Mat *frame);
    std::vector<Detector::Point> detect(Mat *frame);
    pointPair find(Mat *frame);

private:
    std::unique_ptr<SimpleBlobDetector> detector;

    double maxRat;
    double minRat;
    double maxRobot;
    double minRobot;

    double img_threshold;
};

#endif // DETECTOR_THRESHOLD_H
