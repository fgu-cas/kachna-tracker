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
    std::vector<KeyPoint> detect(Mat *frame);
    keypointPair find(Mat *frame);

private:
    std::unique_ptr<SimpleBlobDetector> detector;

    double maxRat;
    double minRat;
    double maxRobot;
    double minRobot;

    double img_threshold;

    int multiple_reaction;
    int skip_reaction;
    int skip_distance;
    int skip_timeout;

    double getDistance(KeyPoint a, KeyPoint b);
    keypointPair lastPoints;

    QElapsedTimer ratTimer;
    QElapsedTimer robotTimer;
};

#endif // DETECTOR_THRESHOLD_H
