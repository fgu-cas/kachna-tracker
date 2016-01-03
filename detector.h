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

    virtual Mat process(Mat *frame) = 0;
    virtual std::vector<KeyPoint> detect(Mat *frame) = 0;

protected:
    Mat mask;
};




#endif // DETECTOR_H
