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
    Detector(const QMap<QString, QVariant> &settings, int h, int w);

    enum CLASS_ID {
        UNKNOWN,
        RAT,
        RAT_FRONT,
        RAT_BACK,
        ROBOT,
        ROBOT_FRONT,
        ROBOT_BACK
    };

    class Point {
    public:
        Point();

        Point2f pt;
        float angle;
        float size;
        float hue;
        CLASS_ID class_id;

        void operator=(cv::KeyPoint point);
    };


    struct pointPair { Detector::Point rat; Detector::Point robot; };


    virtual Mat process(Mat *frame) = 0;
    virtual Mat analyze(Mat *frame) = 0;
    virtual pointPair find(Mat *frame) = 0;
    virtual std::vector<Point> detect(Mat *frame) = 0;

protected:
    Mat mask;
    cv::Rect maskRect;
};




#endif // DETECTOR_H
