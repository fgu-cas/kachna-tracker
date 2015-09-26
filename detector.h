#ifndef DETECTOR_H
#define DETECTOR_H

#include <memory>

#include <QMap>
#include <QVariant>
#include <QElapsedTimer>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace cv;

class Detector
{
public:
    Detector(QMap<QString, QVariant> settings, int h, int w);

    struct keyPoints { KeyPoint rat; KeyPoint robot; };

    Mat process(Mat *frame);
    std::vector<KeyPoint> detect(Mat *frame);

private:
    std::unique_ptr<SimpleBlobDetector> detector;
    double img_threshold;

    Mat mask;
};




#endif // DETECTOR_H
