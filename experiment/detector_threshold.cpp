#include "detector_threshold.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <qmath.h>
#include <stdexcept>


DetectorThreshold::DetectorThreshold(const QMap<QString, QVariant> &settings, int h, int w)
    : Detector(settings, h, w) {
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

    img_threshold = settings.value("tracking/threshold/threshold").toInt();

    detector.reset(new SimpleBlobDetector(params));

    maxRat = settings.value("tracking/threshold/maxRat").toDouble();
    minRat = settings.value("tracking/threshold/minRat").toDouble();
    maxRobot = settings.value("tracking/threshold/maxRobot").toDouble();
    minRobot = settings.value("tracking/threshold/minRobot").toDouble();
}

Mat DetectorThreshold::process(Mat *frame){
    Mat processedFrame;
    frame->copyTo(processedFrame, mask);
    cv::cvtColor(processedFrame, processedFrame, CV_BGR2GRAY);
    return processedFrame;
}

Mat DetectorThreshold::analyze(Mat *frame){
    Mat result;
    threshold(*frame, result, img_threshold, 255, THRESH_BINARY);
    return result;
}

std::vector<Detector::Point> DetectorThreshold::detect(Mat *frame){
    std::vector<Detector::Point> result;

    if (!frame->empty() && frame->channels() == 3){
        Mat workFrame = process(frame);
        workFrame = analyze(&workFrame);
        std::vector<KeyPoint> keyPoints;
        detector->detect(workFrame, keyPoints);
        for (KeyPoint keyPoint : keyPoints){
            Detector::Point point;
            point = keyPoint;
            result.push_back(point);
        }
    }

    return result;
}

Detector::pointPair DetectorThreshold::find(Mat *frame){
    pointPair result;

    std::vector<Detector::Point> keypoints = detect(frame);

    for (unsigned i = 0; i<keypoints.size(); i++){
       Detector::Point keypoint = keypoints[i];
       if (keypoint.size > minRat && keypoint.size < maxRat){    
           result.rat = keypoint;
       }

       if (keypoint.size > minRobot && keypoint.size < maxRobot){
           result.robot = keypoint;
       }
    }

    return result;
}
