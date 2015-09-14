#include "detector.h"
#include <opencv2/imgproc/imgproc.hpp>

#include <stdexcept>

Detector::Detector(QMap<QString, QVariant> settings, int h, int w){
    cv::SimpleBlobDetector::Params params;
    params.minDistBetweenBlobs = 10.0f;
    params.filterByInertia = false;
    params.filterByConvexity = false;
    params.filterByColor = true;
    params.blobColor = 255;
    params.filterByCircularity = false;
    params.filterByArea = true;
    params.minArea = 20;
    params.maxArea = 700;

    maxRat = settings.value("tracking/maxRat").toDouble();
    minRat = settings.value("tracking/minRat").toDouble();
    maxRobot = settings.value("tracking/maxRobot").toDouble();
    minRobot = settings.value("tracking/minRobot").toDouble();

    img_threshold = settings.value("tracking/threshold").toInt();

    detector.reset(new SimpleBlobDetector(params));

    mask = Mat(h, w, CV_8UC1);
    mask.setTo(Scalar(0));
    circle(mask, Point2f(settings.value("arena/X").toFloat(), settings.value("arena/Y").toFloat()),
           settings.value("arena/radius").toInt(), Scalar(255), -1);
}


Detector::keyPoints Detector::detect(Mat *frame){
    Detector::keyPoints result;

    if (!frame->empty() && frame->channels() == 3){
        std::vector<KeyPoint> keypoints;
        Mat maskedMat;
        frame->copyTo(maskedMat, mask);
        Mat gray;
        cv::cvtColor(maskedMat, gray, CV_RGB2GRAY);
        Mat detectMat;
        threshold(gray, detectMat, img_threshold, 255, THRESH_BINARY);
        // Here I would've used the optional Mask parameter of the detect() function...
        // Except that SimpleBlobDetector doesn't support it! AGH
        detector->detect(detectMat, keypoints);

        for (unsigned i = 0; i<keypoints.size(); i++){
           KeyPoint keypoint = keypoints[i];
           if (keypoint.size > minRat && keypoint.size < maxRat){
               result.rat = keypoint;
           }
           if (keypoint.size > minRobot && keypoint.size < maxRobot){
               result.robot = keypoint;
           }
        }
    }

    return result;
}

std::vector<KeyPoint> Detector::detectAll(Mat *frame){
    std::vector<KeyPoint> keypoints_post;

    if (frame->channels() == 3){
        Mat maskedMat;
        frame->copyTo(maskedMat, mask);
        Mat gray;
        cv::cvtColor(maskedMat, gray, CV_RGB2GRAY);
        Mat detectMat;
        threshold(gray, detectMat, img_threshold, 255, THRESH_BINARY);
        std::vector<KeyPoint> keypoints_pre;
        detector->detect(detectMat, keypoints_pre);
        for (unsigned i = 0; i<keypoints_pre.size(); i++){
            KeyPoint keypoint = keypoints_pre[i];
            if (mask.at<uchar>(keypoint.pt.y, keypoint.pt.x) != 0){
                keypoints_post.push_back(keypoint);
            }
        }
    }

    return keypoints_post;
}
