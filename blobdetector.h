#ifndef BLOBDETECTOR_H
#define BLOBDETECTOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace cv;

class BlobDetector
{
public:
    BlobDetector(int threshold,
                 double maxArea,
                 double minArea,
                 double maxRatSize,
                 double minRatSize,
                 double maxRobotSize,
                 double minRobotSize);
    ~BlobDetector();

    struct keyPoints { KeyPoint rat; KeyPoint robot; };

    keyPoints detect(Mat *frame);
    std::vector<KeyPoint> allKeypoints(Mat *frame);

private:
    SimpleBlobDetector *detector;

    double maxRat;
    double minRat;
    double maxRobot;
    double minRobot;
};




#endif // BLOBDETECTOR_H
