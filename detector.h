#ifndef DETECTOR_H
#define DETECTOR_H

#include <QMap>
#include <QVariant>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace cv;

class Detector
{
public:
    Detector(QMap<QString, QVariant> settings, int h, int w);
    ~Detector();

    struct keyPoints { KeyPoint rat; KeyPoint robot; };

    keyPoints detect(Mat *frame);
    std::vector<KeyPoint> detectAll(Mat *frame);

private:
    SimpleBlobDetector *detector;

    double maxRat;
    double minRat;
    double maxRobot;
    double minRobot;

    double img_threshold;

    Mat mask;
};




#endif // DETECTOR_H