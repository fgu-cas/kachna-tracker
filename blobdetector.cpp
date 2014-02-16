#include "blobdetector.h"

#include <stdexcept>

#include <iostream>

BlobDetector::BlobDetector(int threshold, double maxRatSize, double minRatSize, double maxRobotSize, double minRobotSize){
    cv::SimpleBlobDetector::Params params;

    params.minThreshold = threshold;
    params.maxThreshold = 255;
    params.thresholdStep = 10;
    params.minDistBetweenBlobs = 10.0f;
    params.filterByInertia = false;
    params.filterByConvexity = false;
    params.filterByColor = true;
    params.blobColor = 255;
    params.filterByCircularity = false;
    params.filterByArea = true;
    double minSize = std::min(minRobotSize, minRatSize);
    double maxSize = std::max(maxRobotSize, maxRatSize);
    params.minArea = minSize;
    params.maxArea = maxSize;

    detector = new SimpleBlobDetector(params);
}

BlobDetector::~BlobDetector(){
    delete detector;
}

BlobDetector::keyPoints BlobDetector::detect(Mat *frame){
    std::vector<KeyPoint> keypoints;
    detector->detect(*frame, keypoints);

    BlobDetector::keyPoints result;

    if (keypoints.size() > 1){
        if (keypoints.at(0).size > keypoints.at(1).size){
            result.rat = keypoints[0];
            result.robot = keypoints[1];
        } else {
            result.robot = keypoints[0];
            result.rat = keypoints[1];
        }
    }

    return result;
}
