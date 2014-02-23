#include "blobdetector.h"

#include <stdexcept>

#include <iostream>

BlobDetector::BlobDetector(QMap<QString, QVariant> settings){
    cv::SimpleBlobDetector::Params params;

    params.minThreshold = settings.value("threshold").toDouble();
    params.maxThreshold = 255;
    params.thresholdStep = 10;
    params.minDistBetweenBlobs = 10.0f;
    params.filterByInertia = false;
    params.filterByConvexity = false;
    params.filterByColor = true;
    params.blobColor = 255;
    params.filterByCircularity = false;
    params.filterByArea = true;
    params.minArea = settings.value("minArea").toDouble();
    params.maxArea = settings.value("maxArea").toDouble();

    maxRat = settings.value("maxRatSize").toDouble();
    minRat = settings.value("minRatSize").toDouble();
    maxRobot = settings.value("maxRobotSize").toDouble();
    minRobot = settings.value("minRobotSize").toDouble();

    detector = new SimpleBlobDetector(params);
}

BlobDetector::~BlobDetector(){
    delete detector;
}


BlobDetector::keyPoints BlobDetector::detect(Mat *frame){
    std::vector<KeyPoint> keypoints;
    detector->detect(*frame, keypoints);

    BlobDetector::keyPoints result;

    for (unsigned i = 0; i<keypoints.size(); i++){
           KeyPoint keypoint = keypoints[i];
           if (keypoint.size > minRat && keypoint.size < maxRat){
               result.rat = keypoint;
           }
           if (keypoint.size > minRobot && keypoint.size < maxRobot){
               result.robot = keypoint;
           }
       }

    return result;
}

std::vector<KeyPoint> BlobDetector::allKeypoints(Mat *frame){
    std::vector<KeyPoint> keypoints;
    detector->detect(*frame, keypoints);
    return keypoints;
}
