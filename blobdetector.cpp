#include "blobdetector.h"

#include <stdexcept>

#include <iostream>

BlobDetector::BlobDetector(QMap<QString, QVariant> settings, int h, int w){
    cv::SimpleBlobDetector::Params params;

    params.minThreshold = settings.value("tracking/threshold").toDouble();
    params.maxThreshold = 255;
    params.thresholdStep = 5;
    params.minDistBetweenBlobs = 10.0f;
    params.filterByInertia = false;
    params.filterByConvexity = false;
    params.filterByColor = true;
    params.blobColor = 255;
    params.filterByCircularity = false;
    params.filterByArea = true;
    params.minArea = settings.value("tracking/minArea").toDouble();
    params.maxArea = settings.value("tracking/maxArea").toDouble();

    maxRat = settings.value("tracking/maxRat").toDouble();
    minRat = settings.value("tracking/minRat").toDouble();
    maxRobot = settings.value("tracking/maxRobot").toDouble();
    minRobot = settings.value("tracking/minRobot").toDouble();

    detector = new SimpleBlobDetector(params);

    mask = Mat(h, w, CV_8UC1);
    mask.setTo(Scalar(0));
    circle(mask, Point2f(settings.value("mask/X").toFloat(), settings.value("mask/Y").toFloat()),
           settings.value("mask/radius").toInt(), Scalar(255), -1);

    hardMask = (settings.value("mask/type").toInt() == 1);
}

BlobDetector::~BlobDetector(){
    delete detector;
}


BlobDetector::keyPoints BlobDetector::detect(Mat *frame){
    std::vector<KeyPoint> keypoints;

    Mat detectMat;
    if (hardMask){
        frame->copyTo(detectMat, mask);
    } else {
        detectMat = *frame;
    }
    // Here I would've used the optional Mask parameter of the detect() function...
    // Except that SimpleBlobDetector doesn't support it! AGH
    detector->detect(detectMat, keypoints);

    BlobDetector::keyPoints result;

    for (unsigned i = 0; i<keypoints.size(); i++){
       KeyPoint keypoint = keypoints[i];
       if ( !hardMask && mask.at<uchar>(keypoint.pt.y, keypoint.pt.x) == 0){
               continue;
       }
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
    std::vector<KeyPoint> keypoints_pre;
    detector->detect(*frame, keypoints_pre);
    std::vector<KeyPoint> keypoints_post;
    for (unsigned i = 0; i<keypoints_pre.size(); i++){
        KeyPoint keypoint = keypoints_pre[i];
        if (mask.at<uchar>(keypoint.pt.y, keypoint.pt.x) != 0){
            keypoints_post.push_back(keypoint);
        }

    }
    return keypoints_post;
}
